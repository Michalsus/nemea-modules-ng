/**
 * @file
 * @author Pavel Siska <siska@cesnet.cz>
 * @brief Declaration of the IpAddressPrefix class for IP address whitelisting.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <unirec++/ipAddress.hpp>

namespace Whitelist {

/**
 * @brief Represents an IP address with a specified prefix for whitelisting.
 */
class IpAddressPrefix {
public:
	/**
	 * @brief Maximum prefix length for IPv4 addresses.
	 */
	static const size_t IPV4_MAX_PREFIX = 32;

	/**
	 * @brief Maximum prefix length for IPv6 addresses.
	 */
	static const size_t IPV6_MAX_PREFIX = 128;

	/**
	 * @brief Constructor for the IpAddressPrefix class.
	 * @param ipAddress The IP address.
	 * @param prefix The prefix length.
	 */
	IpAddressPrefix(Nemea::IpAddress ipAddress, size_t prefix);

	/**
	 * @brief Checks if a given IP address belongs to the same prefix.
	 * @param ipAddress The IP address to check.
	 * @return True if the IP address belongs to the same prefix, false otherwise.
	 */
	bool isBelong(const Nemea::IpAddress& ipAddress) const noexcept;

private:
	Nemea::IpAddress m_address;
	Nemea::IpAddress m_mask;
};

} // namespace Whitelist
