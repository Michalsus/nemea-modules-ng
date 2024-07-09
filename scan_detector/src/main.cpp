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
#include <cstdint>
#include <functional>
#include "CircBuff.cpp"

using namespace Nemea;

struct TrafficData {
    uint64_t incoming;
    uint64_t outgoing;
    uint64_t synflag;

    TrafficData() : incoming(0), outgoing(0), synflag(0) {}
};

int bufferSize = 1000000;
CircularBuffer circBuff(bufferSize);

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

UnirecRecordView* processNextRecord(UnirecBidirectionalInterface& biInterface)
{
	std::optional<UnirecRecordView> unirecRecord = biInterface.receive();
	if (!unirecRecord) {
		return;
	}
    //TODO: categorization of the record into one of the tables set (whitelist, sus, ?blacklist?)
	return *unirecRecord;//this propably sends the record to output, but i need to save it into the buffer and make stats with it
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
void processUnirecRecords(UnirecBidirectionalInterface& biInterface, std::unordered_map& ipMap)
{
	while (true) {
		try {
			UnirecRecordView* in_rec = processNextRecord(biInterface);
			auto result = circBuff.buffInsert(in_rec);
			//in the result there will be either null or record that was the oldest
			//after the record is in the result, the table of suspicous addresses needs to be checked if there is the ip address that is sus otherwise the record will be "destroyed" after statistics bout it have changed
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

	//Hash map for storing statistics about each ip adresses
	std::unordered_map<ip_addr_t, TrafficData, IPAddressHash, IPAddressEqual> ipMap;

	try {
		UnirecBidirectionalInterface biInterface = unirec.buildBidirectionalInterface();
		biInterface.setRequieredFormat("");

		processUnirecRecords(biInterface, ipMap);

	} catch (std::exception& ex) {
		//logger->error(ex.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}