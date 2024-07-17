/**
 * @file
 * @author Pavel Siska <siska@cesnet.cz>
 * @brief Declares the NcursesOutputPlugin class, a plugin for printing telemetry data using
 * Ncurses.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "logger/logger.hpp"
#include "ncurses-wrapper.hpp"
#include "outputPlugin.hpp"

#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <telemetry.hpp>
#include <thread>

namespace TelemetryStats {

/**
 * @brief The NcursesOutputPlugin class is responsible for printing telemetry data
 *        using the Ncurses library at a specified interval to stdout.
 */
class NcursesOutputPlugin : public OutputPlugin {
public:
	/**
	 * @brief Constructs an NcursesOutputPlugin object with given parameters.
	 *
	 * @param params The parameters string containing configuration options.
	 * @param rootDirectory The root directory where telemetry data is accessed.
	 */
	NcursesOutputPlugin(
		const std::string& params,
		const std::shared_ptr<telemetry::Directory>& rootDirectory);

	/**
	 * @brief Destroys the NcursesOutputPlugin object, stopping the printing thread.
	 */
	~NcursesOutputPlugin() override;

private:
	void validateParams(const std::map<std::string, std::string>& paramsMap) const;

	/**
	 * @brief Worker function executed by the thread, printing telemetry data at specified
	 * intervals.
	 *
	 * @param interval The interval (in seconds) at which telemetry data are printed.
	 * @param statsFile The telemetry file containing data to be printed.
	 */
	void loopThread(float interval, const std::shared_ptr<telemetry::File>& statsFile);

	/**
	 * @brief Prints the contents of a telemetry file using the Ncurses library.
	 *
	 * @param file The telemetry file whose content is to be printed.
	 */
	void printFile(const std::shared_ptr<telemetry::File>& file);

	Ncurces m_ncurses;
	std::thread m_thread;
	std::mutex m_mutex;
	std::condition_variable m_condition;
	std::atomic<bool> m_stopFlag {false};
	std::shared_ptr<spdlog::logger> m_logger = Nm::loggerGet("AppFsOutputPlugin");
};

} // namespace TelemetryStats
