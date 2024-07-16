/**
 * @file
 * @author Pavel Siska <siska@cesnet.cz>
 * @brief Telemetry functions for Unirec interfaces.
 *
 * This file contains functions to retrieve telemetry data from Unirec interfaces.
 *
 * Example output format:
 *
 * @code
 * receivedBytes = XXX
 * receivedRecords = XXX
 * missedRecords = XXX
 * missed = XX %
 * @endcode
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <telemetry.hpp>
#include <unirec++/unirec.hpp>

namespace Nm {

/**
 * @brief Retrieves telemetry data for an input Unirec interface.
 *
 * This function retrieves and returns telemetry data for a given input Unirec interface.
 *
 * @param interface The input Unirec interface.
 * @return telemetry::Content The telemetry data of the interface.
 */
telemetry::Content getInterfaceTelemetry(const Nemea::UnirecInputInterface& interface);

/**
 * @brief Retrieves telemetry data for a bidirectional Unirec interface.
 *
 * This function retrieves and returns telemetry data for a given bidirectional Unirec interface.
 *
 * @param interface The bidirectional Unirec interface.
 * @return telemetry::Content The telemetry data of the interface.
 */
telemetry::Content getInterfaceTelemetry(const Nemea::UnirecBidirectionalInterface& interface);

} // namespace Nm
