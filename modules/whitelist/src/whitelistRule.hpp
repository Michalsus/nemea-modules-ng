/**
 * @file
 * @author Pavel Siska <siska@cesnet.cz>
 * @author Daniel Pelanek <xpeland00@vutbr.cz>
 * @brief Defines data structures for a whitelist rule.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "ipAddressPrefix.hpp"

#include <cstdint>
#include <optional>
#include <regex>
#include <unirec++/unirec.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace Whitelist {

/**
 * @brief Stores statistics about a whitelist rule.
 */
struct RuleStats {
	uint64_t matchedCount; /**< Number of times the rule has been matched. */
};

/**
 * @brief Represents possible values for a rule field in the whitelist.
 */
using RuleFieldValue = std::variant<
	char,
	uint8_t,
	uint16_t,
	uint32_t,
	uint64_t,
	int8_t,
	int16_t,
	int32_t,
	int64_t,
	std::regex,
	IpAddressPrefix>;

/**
 * @brief Represents a field in a whitelist rule.
 */
using RuleField = std::pair<ur_field_id_t, std::optional<RuleFieldValue>>;

/**
 * @brief Represents a single whitelist rule.
 */
class WhitelistRule {
public:
	/**
	 * @brief Constructor for a Whitelist Rule.
	 * @param ruleFields reference to a vector of rule fields.
	 */
	explicit WhitelistRule(const std::vector<RuleField>& ruleFields);

	/**
	 * @brief Checks if the given UnirecRecordView matches this rule
	 * @param unirecRecordView The Unirec record which is tried to match
	 * @return True if matched, false otherwise
	 */
	bool isMatched(const Nemea::UnirecRecordView& unirecRecordView);

	/**
	 * @brief Gets the statistics for this rule.
	 * @return A constant reference to the RuleStats structure.
	 */
	const RuleStats& getStats() const noexcept;

private:
	const std::vector<RuleField> m_ruleFields;
	RuleStats m_stats;
};

} // namespace Whitelist
