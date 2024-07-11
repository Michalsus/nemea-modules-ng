/**
 * @file
 * @author Michal Matejka <xmatejm00@stud.fit.vutbr.cz>
 * @brief Scan Detector
 *
 * Nemea module for 
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
#include "CircBuff.cpp"

using namespace Nemea;


struct TrafficData 
{
    uint64_t src;
    uint64_t dst;
    uint64_t syn;
	bool deathFlag;

    TrafficData() : src(0), dst(0), syn(0), deathFlag(false) {}
};

struct SusIpData
{
	std::vector<UnirecRecordView> records;
	std::map<int, int> portMap;
	std::map<ip_addr_t, int> dstIpMap;
	uint64_t syn;

	SusIpData() : syn(0) {}
};


int bufferSize = 1'000'000;
double srcDstRatio = 0,5;
double synSrcRatio = 0,5;
CircularBuffer circBuff(bufferSize);
std::mutex writeAccess;
//Hash map for storing statistics about each ip adresses
std::unordered_map<ip_addr_t, TrafficData, IPAddressHash, IPAddressEqual> ipMap;
std::unordered_map<ip_addr_t, SusIpData, IPAddressHash, IPAddressEqual> susIpMap;

/**
 * @brief Handle a format change exception by adjusting the template.
 *
 * This function is called when a `FormatChangeException` is caught in the main loop.
 * It adjusts the template in the bidirectional interface to handle the format change.
 *
 * @param biInterface Bidirectional interface for Unirec communication.
 */
void handleFormatChange(UnirecBidirectionalInterface& biInterface)
{
	biInterface.changeTemplate();
}

/**
 * @brief Process the next Unirec record and sample them.
 *
 * This function receives the next Unirec record through the bidirectional interface
 * and performs sampling.
 *
 * @param biInterface Bidirectional interface for Unirec communication.
 */
void processNextRecord(UnirecBidirectionalInterface& biInterface, std::unordered_map& ipMap, std::unordered_map& susIpMap)
{
	writeAccess.lock();
	std::optional<UnirecRecordView> unirecRecord = biInterface.receive();
	if (!unirecRecord) {
		return;
	}
	//update statistics for incoming record
	ip_addr_t src = unirecRecord.getFieldAsType<int>("SRC_IP");//?check if the ip is in the sus category
	ipMap[src].src++;
	ip_addr_t dst = unirecRecord.getFieldAsType<int>("DST_IP");
	ipMap[dst].dst++;
	auto tcp = unirecRecord.getFieldAsType<int>("TCP_FLAGS");
	if(tcp == 2){
		ipMap[src].syn++;
	}

	unirecRecord = circBuff.buffInsert(unirecRecord);
	if (unirecRecord != nullptr){
		src = unirecRecord.getFieldAsType<int>("SRC_IP");
		if (susIpMap.find(src) != susIpMap.end()){
			//number of only syn traffic from suspicious ip
			auto tcp = unirecRecord.getFieldAsType<int>("TCP_FLAGS");
			if(tcp == 2){
				susIpMap[src].syn++;
			}
			//associative array of ips and number of times they were mentioned
			dst = unirecRecord.getFieldAsType<int>("DST_IP");
			susIpMap[src].dstIpMap.insert(dst);
			susIpMap[src].dstIpMap[dst]++;
			//array of ports and number of times they were used
			auto port = unirecRecord.getFieldAsType<int>("DST_PORT");
			susIpMap[src].portMap.insert(port);
			susIpMap[src].portMap[port]++;
			//insert unirecRecord from suspicious ip
			susIpMap[src].records.push_back(unirecRecord);
		}
		else {
			ipMap[src].src--;
			dst = unirecRecord.getFieldAsType<int>("DST_IP");
			ipMap[dst].dst--;
			auto tcp = unirecRecord.getFieldAsType<int>("TCP_FLAGS");
			if(tcp == 2){
				ipMap[src].syn--;
			}
			biInterface.send(*unirecRecord);
		}
	}
	writeAccess.unlock();
	return;
}

/**
 * @brief Process Unirec records.
 *
 * The `processUnirecRecords` function continuously receives Unirec records through the provided
 * bidirectional interface (`biInterface`) and performs sampling. The loop runs indefinitely until
 * an end-of-file condition is encountered.
 *
 * @param biInterface Bidirectional interface for Unirec communication.
 */
void processUnirecRecords(UnirecBidirectionalInterface& biInterface, std::unordered_map& ipMap, std::unordered_map& susIpMap)
{
	while (true) {
		try {
			processNextRecord(biInterface, ipMap, susIpMap);
		} catch (FormatChangeException& ex) {
			handleFormatChange(biInterface);
		} catch (EoFException& ex) {
			break;
		} catch (std::exception& ex) {
			throw;
		}
	}
}

/**
 * @brief Monitoring of IP addresses in IpMap
 * 
 * This function is written for a thread. The thread after activation will periodically go through
 * the IpMap and tries to find suspicious IP's. The IP's are then added into a separate map.
 * The thread goes through once and then it sleeps for a certain period of time.
 * Second function is erasing members that do not have records associated with them in the buffer.
 * The thread will give them one cycle to change their status, then the redundant entry will be erased.
 */
void monitorOfIpMap(){
	while (true) {
		for (auto it = ipMap.begin(); it != ipMap.end(); ) {
			const auto& [key, entry] = *it;
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
				if (((double)entry.src/ entry.syn) > synSrcRatio){
					//ip is sus
					writeAccess.lock();
					susIpMap.insert(key);
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
 * 
 */
void monitorOfSusIpMap(){
	
}

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

int main(int argc, char** argv)
{
	argparse::ArgumentParser program("Scan Detector");

	Unirec unirec({1, 1, "scan_detector", "Scan Detector module"});

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
		program.add_argument("-r", "--rate")
			.required()
			.help(
				"Specify the sampling rate 1:r. Every -rth sample will be forwarded to the output.")
			.scan<'i', int>();
		program.parse_args(argc, argv);
	} catch (const std::exception& ex) {
		//logger->error(ex.what());
		std::cerr << program;
		return EXIT_FAILURE;
	}

	try {
		UnirecBidirectionalInterface biInterface = unirec.buildBidirectionalInterface();
		biInterface.setRequieredFormat("");

		processUnirecRecords(biInterface, ipMap, susIpMap);

	} catch (std::exception& ex) {
		//logger->error(ex.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}