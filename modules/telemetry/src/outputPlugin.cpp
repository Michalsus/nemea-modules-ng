/**
 * @file
 * @author Pavel Siska <siska@cesnet.cz>
 * @brief Implementation of the OutputPlugin.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "outputPlugin.hpp"

#include <algorithm>
#include <stdexcept>

namespace TelemetryStats {

std::map<std::string, std::string> OutputPlugin::parseParams(const std::string& params)
{
	std::map<std::string, std::string> map;
	std::string paramsString = params;
	size_t start = 0;
	size_t end = 0;

	auto noSpaceEnd = std::remove(paramsString.begin(), paramsString.end(), ' ');
	paramsString.erase(noSpaceEnd, paramsString.end());

	if (params.empty()) {
		return map;
	}

	while (end != std::string::npos) {
		end = paramsString.find(',', start);
		const std::string tmp = paramsString.substr(start, end - start);

		const size_t mid = tmp.find('=');
		if (mid == std::string::npos || mid + 1 >= tmp.size() || tmp.substr(0, mid).empty()) {
			throw std::invalid_argument("OutputPlugin::parseParams() has failed");
		}

		auto ret = map.emplace(tmp.substr(0, mid), tmp.substr(mid + 1));
		if (!ret.second) {
			throw std::invalid_argument("OutputPlugin::parseParams() has failed");
		}
		start = end + 1;
	}

	return map;
}

} // namespace TelemetryStats
