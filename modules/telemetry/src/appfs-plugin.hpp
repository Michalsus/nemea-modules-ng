/**
 * @file
 * @author Pavel Siska <siska@cesnet.cz>
 * @brief Declaration of the AppFsOutputPlugin class.
 *
 * This file contains the declaration of the AppFsOutputPlugin class,
 * which provides telemetry data over the AppFsFuse interface.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "logger/logger.hpp"
#include "outputPlugin.hpp"

#include <appFs.hpp>
#include <map>
#include <memory>
#include <string>
#include <telemetry.hpp>

namespace TelemetryStats {

/**
 * @brief A class for providing telemetry data over AppFsFuse interface.
 */
class AppFsOutputPlugin : public OutputPlugin {
public:
	/**
	 * @brief Constructs an AppFsOutputPlugin object.
	 *
	 * @param params The parameters for configuring the plugin.
	 * @param rootDirectory A shared pointer to the root directory used by AppFsFuse.
	 */
	AppFsOutputPlugin(
		const std::string& params,
		const std::shared_ptr<telemetry::Directory>& rootDirectory);

	/**
	 * @brief Destructs the AppFsOutputPlugin object.
	 */
	~AppFsOutputPlugin() override;

private:
	void validateParams(const std::map<std::string, std::string>& paramsMap) const;

	std::unique_ptr<telemetry::appFs::AppFsFuse> m_appFsFuse;
	std::shared_ptr<spdlog::logger> m_logger = Nm::loggerGet("AppFsOutputPlugin");
};

} // namespace TelemetryStats
