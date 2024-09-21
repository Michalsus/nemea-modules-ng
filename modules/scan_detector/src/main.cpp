/**
 * @file
 * @author Michal Matejka <xmatejm00@stud.fit.vutbr.cz>
 * @brief Scan Detector
 *
 * Nemea module for finding scanning IP addresses, which addresses they are scanning, which ports and making statistics based on this data.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

//#include "logger.hpp"

#include <iostream>
#include <stdexcept>
#include <argparse/argparse.hpp>
#include <unirec++/unirec.hpp>
#include <unordered_map>
#include <map>
#include <chrono>
#include <mutex>
#include <vector>
#include <cstdint>
#include <functional>
#include <thread>
#include "CircBuff.cpp"	

using namespace Nemea;


/**
 * @brief Hash function for ip_addr_t
 *
 * Custom hash function for use in unordered_map
 *
 * @param ip_addr_t IP address to be hashed by
 */
struct IPAddressHash {
    std::size_t operator()(const ip_addr_t& ip) const {
        // Combine the two 64-bit parts to produce a single hash value
        const uint64_t* ptr = ip.ui64;
        std::size_t h1 = std::hash<uint64_t>{}(ptr[0]);
        std::size_t h2 = std::hash<uint64_t>{}(ptr[1]);
        return h1 ^ (h2 << 1); // Combine the two hash values
    }
};

/**
 * @brief Equality operator for ip_addr_t
 *
 * Custom equality operator for use in unordered_map
 *
 * @param ip_addr_t First IP address to be compared
 * @param ip_addr_t Second IP address to be compared
 */
struct IPAddressEqual {
    bool operator()(const ip_addr_t& lhs, const ip_addr_t& rhs) const {
        return lhs.ui64[0] == rhs.ui64[0] && lhs.ui64[1] == rhs.ui64[1];
    }
};

struct TrafficData 
{
    uint64_t src;
    uint64_t dst;
    uint64_t syn;
	bool deathFlag;

    TrafficData() : src(0), dst(0), syn(0), deathFlag(false) {}
};

struct IpData 
{
	std::map<int, int> portMap;
	uint64_t count;

	IpData() : count(0) {}
};

struct SusIpData
{
	std::vector<UnirecRecord> inRecords;
	std::vector<UnirecRecord> outRecords;
	std::unordered_map<ip_addr_t, IpData, IPAddressHash, IPAddressEqual> dstIpMap;
	uint64_t syn;

	SusIpData() : syn(0) {}
};

//function declarations, definitions are after main
void handleFormatChange(UnirecInputInterface& iInterface);
void processNextRecord(UnirecInputInterface& iInterface, CircularBuffer& circBuff);
void processUnirecRecords(UnirecInputInterface& iInterface, CircularBuffer& circBuff);
void categorizeUnirecRecord(UnirecRecord unirecRecord);
void monitorOfIpMap();
void monitorOfSusIpMap();

//definition of global variables
int bufferSize = 1'000'000;
size_t minSize = 30;
double susNorRatio = 0.9;
double srcDstRatio = 0.5;
double synSrcRatio = 0.5;
std::mutex writeAccess;
//Hash map for storing statistics about each ip adresses
std::unordered_map<ip_addr_t, TrafficData, IPAddressHash, IPAddressEqual> ipMap;
std::unordered_map<ip_addr_t, SusIpData, IPAddressHash, IPAddressEqual> susIpMap;



int main(int argc, char** argv)
{
	argparse::ArgumentParser program("Scan Detector");

	Unirec unirec({1, 0, "scan_detector", "Scan Detector module"});

	//nemea::loggerInit();
	//auto logger = nemea::loggerGet("main");

	try {
		unirec.init(argc, argv);
	} catch (HelpException& ex) {
		std::cerr << program;
		return EXIT_SUCCESS;
	} catch (std::exception& ex) {
		//logger->error(ex.what());
		return EXIT_FAILURE;
	}

	try {//TODO: change this
		program.parse_args(argc, argv);
	} catch (const std::exception& ex) {
		//logger->error(ex.what());
		std::cerr << program;

		return EXIT_FAILURE;
	}

	try {
		UnirecInputInterface iInterface = unirec.buildInputInterface();
		iInterface.setRequieredFormat("ipaddr SRC_IP, ipaddr DST_IP, uint8 TCP_FLAGS, uint16 DST_PORT");

		CircularBuffer circBuff(bufferSize, iInterface.getTemplate(), 0);

		//initiate the threads
		std::thread t1(monitorOfIpMap);
		std::thread t2(monitorOfSusIpMap);

		processUnirecRecords(iInterface, circBuff);

		t1.join();
		t2.join();

	} catch (std::exception& ex) {
		//logger->error(ex.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * @brief Handle a format change exception by adjusting the template.
 *
 * This function is called when a `FormatChangeException` is caught in the main loop.
 * It adjusts the template in the bidirectional interface to handle the format change.
 *
 * @param iInterface Bidirectional interface for Unirec communication.
 */
void handleFormatChange(UnirecInputInterface& iInterface)
{
	iInterface.changeTemplate();
}

/**
 * @brief Process the next Unirec record and categorize them.
 *
 * This function receives the UnirecRecord and puts it into the Buffer. Then makes statistics about 
 * both the DST_IP and SRC_IP. If the SRC_IP is already in suspicious category, then it makes more detailed statistics.
 *
 * @param iInterface Bidirectional interface for Unirec communication
 */
void processNextRecord(UnirecInputInterface& iInterface, CircularBuffer& circBuff)
{

	writeAccess.lock();
	std::optional<UnirecRecordView> uniRecord = iInterface.receive();
	if (!uniRecord) {
		return;
	}
	//if (uniRecord.has_value()){
	//	std::cout << "Record has no value" << std::endl;
	//	writeAccess.unlock();
	//	exit(EXIT_FAILURE);
	//	//return;
	//}
	UnirecRecord unirecRecord(iInterface.getTemplate(), 0);
    unirecRecord.copyFieldsFrom(*uniRecord);
	//update statistics for incoming record
	categorizeUnirecRecord(unirecRecord);

	//question is if the use of std::optional here isnt redundant 
	std::optional<UnirecRecord> tmpRecord = circBuff.buffInsert(unirecRecord);
	if(!tmpRecord){//if the buffer is still not full
		return;
	}
	//convert std::opitonal to UnirecRecord
	unirecRecord = tmpRecord.value();

	categorizeUnirecRecord(unirecRecord);
	writeAccess.unlock();
	return;
}

/**
 * @brief Process Unirec records.
 *
 * The `processUnirecRecords` function continuously receives Unirec records through the provided
 * bidirectional interface (`iInterface`) and does categorization. The loop runs indefinitely until
 * an end-of-file condition is encountered.
 *
 * @param iInterface Bidirectional interface for Unirec communication.
 */
void processUnirecRecords(UnirecInputInterface& iInterface, CircularBuffer& circBuff)
{
	while (true) {
		try {
			//printf("%d\n", circBuff.size());
			processNextRecord(iInterface, circBuff);
		} catch (FormatChangeException& ex) {
			printf("format change\n");
			handleFormatChange(iInterface);
			writeAccess.unlock();
		} catch (EoFException& ex) {
			break;
		} catch (std::exception& ex) {
			std::cerr << "Exception caught: " << ex.what() << std::endl;
			throw;
		}
	}
}

/**
 * @brief Monitoring of IP addresses in IpMap
 * 
 * This function is used by a thread. The thread after activation will periodically go through
 * the IpMap and tries to find suspicious IP's. The IP's are then added into a separate map.
 * Second function is erasing members that do not have records associated with them in the buffer.
 * The thread will give them one cycle to change their status, then the redundant entry will be erased.
 */
void monitorOfIpMap(){
	while (true) {
		for (auto it = ipMap.begin(); it != ipMap.end(); ) {
			auto& [key, entry] = *it;
			//if ip wasnt used for a extended period, the node will be erased
			if(entry.dst == 0 && entry.src == 0 && entry.syn == 0){
				if (entry.deathFlag == false){
					entry.deathFlag = true;
				}
				else {
					writeAccess.lock();
					it = ipMap.erase(it);
					writeAccess.unlock();
				}
				continue;
			}
			entry.deathFlag = false;
			if (((double)entry.dst/entry.src) < srcDstRatio){
				if (((double)entry.syn/ entry.src) > synSrcRatio){
					//ip is sus
					writeAccess.lock();
					susIpMap[key];
					it = ipMap.erase(it);
					writeAccess.unlock();
				}
				else{
					++it;
				}
			}
			else{
				++it;
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

/**
 * @brief Monitoring of Suspicious IP addresses in susIpMap
 * 
 * This function is used by a thread. The thread after activation will periodically go through 
 * the entries in susIpMap and tries to find scanning IP addresses among them. 
 * Based on different criteria the IP is either classified as a scanner and a report with the unirec
 * records is sent or classified as a normal address and erased from the susIpMap. 
 */
void monitorOfSusIpMap(){
	while(true) {
		std::this_thread::sleep_for(std::chrono::seconds(5));
		for (auto it = susIpMap.begin(); it != susIpMap.end(); ){
			const auto& [key, entry] = *it;

			//if the ratio of outgoing and incoming classifies it as a no scanner
			if((double)entry.outRecords.size()/entry.inRecords.size() < srcDstRatio){
				continue;
			}
			//if the ratio between outgoing and number of syn flags do not exceed treshold
			if((double)entry.outRecords.size()/entry.syn < synSrcRatio){
				continue;
			}

			if (entry.dstIpMap.size() < minSize){
				continue;
			}
			int cnt = 0;
			for (auto it2 = entry.dstIpMap.begin(); it2 != entry.dstIpMap.end(); ){
				const auto& [key1, entry1] = *it2;
				if (entry1.count == entry1.portMap.size()){
					++cnt;
				}
			}

			if ((double)cnt/entry.dstIpMap.size() > susNorRatio){
				//report to Warden ?
			}
			else {
				//erase it from susIpMap ?
			}
		}
	}
}

/**
 * @brief Categorizes based on IPs
 * 
 * This function takes a Unirec Record and updates statistics held in tables based on the SRC_IP and DST_IP.
 * 
 * @param unirecRecord received Unirec Record 
 */
void categorizeUnirecRecord(UnirecRecord unirecRecord){
	static const ur_field_id_t SRC_IP = ur_get_id_by_name("SRC_IP");
	static const ur_field_id_t DST_IP = ur_get_id_by_name("DST_IP");
	static const ur_field_id_t TCP_FLAGS = ur_get_id_by_name("TCP_FLAGS");
	static const ur_field_id_t DST_PORT = ur_get_id_by_name("DST_PORT");

	IpAddress srcStruct = unirecRecord.getFieldAsType<IpAddress>(SRC_IP);
	printf("mrdka\n");
	ip_addr_t src = srcStruct.ip;
	IpAddress dstStruct = unirecRecord.getFieldAsType<IpAddress>(DST_IP);
	ip_addr_t dst = dstStruct.ip;
	uint8_t tcp = unirecRecord.getFieldAsType<uint8_t>(TCP_FLAGS);
	if(susIpMap.find(src) != susIpMap.end()){
		if(tcp == 2){
			susIpMap[src].syn++;
		}
		//associative array of ips and number of times they were mentioned
		susIpMap[src].dstIpMap[dst].count++;
		//array of ports and number of times they were used
		uint16_t port = unirecRecord.getFieldAsType<uint16_t>(DST_PORT);
		susIpMap[src].dstIpMap[dst].portMap[port]++;
		//insert unirecRecord from suspicious ip
		susIpMap[src].outRecords.push_back(unirecRecord);
	}
	else if (susIpMap.find(dst) != susIpMap.end()){
		susIpMap[dst].inRecords.push_back(unirecRecord);
	}
	else {
		ipMap[src].src++;
		ipMap[dst].dst++;
		if(tcp == 2){
			ipMap[src].syn++;
		}
	}
}

