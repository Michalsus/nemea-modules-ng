/**
 * @file
 * @author Pavel Siska <siska@cesnet.cz>
 * @brief
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "unirec/unirec-telemetry.hpp"

namespace Nm {

static double getMissedPercentage(const Nemea::InputInteraceStats& stats)
{
	if (stats.receivedRecords == 0 && stats.missedRecords == 0) {
		return 0;
	}

	const double fraction = static_cast<double>(stats.missedRecords)
		/ static_cast<double>(stats.receivedRecords + stats.missedRecords);

	const int fractionToPercentage = 100;
	return fraction * fractionToPercentage;
}

static telemetry::Content createInterfaceTelemetry(const Nemea::InputInteraceStats& stats)
{
	telemetry::Dict dict;
	dict["receivedBytes"] = stats.receivedBytes;
	dict["receivedRecords"] = stats.receivedRecords;
	dict["missedRecords"] = stats.missedRecords;
	dict["missed"] = telemetry::ScalarWithUnit(getMissedPercentage(stats), "%");
	return dict;
}

telemetry::Content getInterfaceTelemetry(const Nemea::UnirecBidirectionalInterface& interface)
{
	const auto stats = interface.getInputInterfaceStats();
	return createInterfaceTelemetry(stats);
}

telemetry::Content getInterfaceTelemetry(const Nemea::UnirecInputInterface& interface)
{
	const auto stats = interface.getInputInterfaceStats();
	return createInterfaceTelemetry(stats);
}

} // namespace Nm
