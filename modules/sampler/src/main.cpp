/**
 * @file
 * @author Karel Hynek <hynekkar@cesnet.cz>
 * @brief Sampling Module: Sample flowdata
 *
 * This file contains the main function and supporting functions for the Unirec Sampling Module.
 * This module process Unirec records thourgh a bidirectional interface and samples them accoring
 * to user specified sampling rate. It utilizes the Unirec++ library for
 * record handling, argparse for command-line argument parsing.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "logger.hpp"
#include "unirec-telemetry.hpp"

#include <appFs.hpp>
#include <argparse/argparse.hpp>
#include <atomic>
#include <csignal>
#include <iostream>
#include <stdexcept>
#include <telemetry.hpp>
#include <unirec++/unirec.hpp>

using namespace Nemea;

std::atomic<bool> g_stopFlag(false);

void signalHandler(int signum)
{
	nm::loggerGet("signalHandler")->info("Interrupt signal {} received", signum);
	g_stopFlag.store(true);
}

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
 * @param samplingRate Sampling rate value defined by user.
 */

void processNextRecord(UnirecBidirectionalInterface& biInterface, int samplingRate)
{
	static int samplingCounter = 0;
	std::optional<UnirecRecordView> unirecRecord = biInterface.receive();
	if (!unirecRecord) {
		return;
	}

	if (samplingCounter == 0) {
		biInterface.send(*unirecRecord);
	}

	samplingCounter = (++samplingCounter >= samplingRate) ? (0) : samplingCounter;
}

/**
 * @brief Process Unirec records.
 *
 * The `processUnirecRecords` function continuously receives Unirec records through the provided
 * bidirectional interface (`biInterface`) and performs sampling. The loop runs indefinitely until
 * an end-of-file condition is encountered.
 *
 * @param biInterface Bidirectional interface for Unirec communication.
 * @param samplingRate Sampling rate value defined by user.
 */
void processUnirecRecords(UnirecBidirectionalInterface& biInterface, int samplingRate)
{
	while (!g_stopFlag.load()) {
		try {
			processNextRecord(biInterface, samplingRate);
		} catch (FormatChangeException& ex) {
			handleFormatChange(biInterface);
		} catch (EoFException& ex) {
			break;
		} catch (std::exception& ex) {
			throw;
		}
	}
}

int main(int argc, char** argv)
{
	argparse::ArgumentParser program("Unirec Sampler");

	Unirec unirec({1, 1, "sampler", "Unirec sampling module"});

	nm::loggerInit();
	auto logger = nm::loggerGet("main");

	signal(SIGINT, signalHandler);

	try {
		program.add_argument("-r", "--rate")
			.required()
			.help(
				"Specify the sampling rate 1:r. Every -rth sample will be forwarded to the output.")
			.scan<'i', int>();
		program.add_argument("-m", "--appfs-mountpoint")
			.required()
			.help("path where the appFs directory will be mounted")
			.default_value(std::string(""));
	} catch (const std::exception& ex) {
		logger->error(ex.what());
		return EXIT_FAILURE;
	}

	try {
		unirec.init(argc, argv);
	} catch (HelpException& ex) {
		std::cerr << program;
		return EXIT_SUCCESS;
	} catch (std::exception& ex) {
		logger->error(ex.what());
		return EXIT_FAILURE;
	}

	try {
		program.parse_args(argc, argv);
	} catch (const std::exception& ex) {
		logger->error(ex.what());
		return EXIT_FAILURE;
	}

	std::shared_ptr<telemetry::Directory> telemetryRootDirectory;
	telemetryRootDirectory = telemetry::Directory::create();

	std::unique_ptr<telemetry::appFs::AppFsFuse> appFs;

	try {
		auto mountPoint = program.get<std::string>("--appfs-mountpoint");
		if (!mountPoint.empty()) {
			const bool tryToUnmountOnStart = true;
			const bool createMountPoint = true;
			appFs = std::make_unique<telemetry::appFs::AppFsFuse>(
				telemetryRootDirectory,
				mountPoint,
				tryToUnmountOnStart,
				createMountPoint);
			appFs->start();
		}
	} catch (std::exception& ex) {
		logger->error(ex.what());
		return EXIT_FAILURE;
	}

	try {
		const int samplingRate = program.get<int>("--rate");
		if (samplingRate <= 0) {
			std::cerr << "Sampling rate must be higher than zero.\n";
			return EXIT_FAILURE;
		}

		UnirecBidirectionalInterface biInterface = unirec.buildBidirectionalInterface();

		auto telemetryInputDirectory = telemetryRootDirectory->addDir("input");
		const telemetry::FileOps inputFileOps
			= {[&biInterface]() { return nm::getInterfaceTelemetry(biInterface); }, nullptr};
		const auto inputFile = telemetryInputDirectory->addFile("stats", inputFileOps);

		processUnirecRecords(biInterface, samplingRate);

	} catch (std::exception& ex) {
		logger->error(ex.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
