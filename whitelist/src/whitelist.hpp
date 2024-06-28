/**
 * @file
 * @author Pavel Siska <siska@cesnet.cz>
 * @brief Declaration of the Whitelist class.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "configParser.hpp"
#include "logger.hpp"
#include "whitelistRule.hpp"

#include <memory>
#include <unirec++/unirec.hpp>
#include <vector>

namespace Whitelist {

/**
 * @brief Represents a whitelist for Nemea++ records.
 */
class Whitelist {
public:
	/**
	 * @brief Constructor for Whitelist.
	 * @param configParser Pointer to the ConfigParser providing whitelist rules.
	 */
	explicit Whitelist(const ConfigParser* configParser);

	/**
	 * @brief Checks if the given UnirecRecordView is whitelisted.
	 * @param unirecRecordView The Unirec record to check against the whitelist.
	 * @return True if whitelisted, false otherwise.
	 */
	bool isWhitelisted(const Nemea::UnirecRecordView& unirecRecordView) const;

private:
	std::vector<WhitelistRule> m_whitelistRules;
};

} // namespace Whitelist
