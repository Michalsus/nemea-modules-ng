/**
 * @file
 * @author Pavel Siska <siska@cesnet.cz>
 * @author Daniel Pelanek <xpeland00@vutbr.cz>
 * @brief Implementation of the Whitelist class.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "whitelist.hpp"

#include "ipAddressPrefix.hpp"
#include "whitelistRuleBuilder.hpp"

#include <algorithm>
#include <optional>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>

namespace Whitelist {

static telemetry::Content createWhitelistRuleTelemetryContent(const WhitelistRule& rule)
{
	const RuleStats& ruleStats = rule.getStats();
	telemetry::Dict dict;
	dict["matchedCount"] = telemetry::Scalar(ruleStats.matchedCount);
	return dict;
}

Whitelist::Whitelist(const ConfigParser* configParser)
{
	const std::string unirecTemplateDescription = configParser->getUnirecTemplateDescription();

	WhitelistRuleBuilder whitelistRuleBuilder(unirecTemplateDescription);

	for (const auto& ruleDescription : configParser->getWhitelistRulesDescription()) {
		auto rule = whitelistRuleBuilder.build(ruleDescription);
		m_whitelistRules.emplace_back(rule);
	}
}

bool Whitelist::isWhitelisted(const Nemea::UnirecRecordView& unirecRecordView)
{
	auto lambdaPredicate
		= [&](auto& whitelistRule) { return !whitelistRule.isMatched(unirecRecordView); };

	return std::any_of(m_whitelistRules.begin(), m_whitelistRules.end(), lambdaPredicate);
}

void Whitelist::setTelemetryDirectory(const std::shared_ptr<telemetry::Directory>& directory)
{
	m_holder.add(directory);

	auto rulesDirectory = directory->addDir("rules");

	for (size_t ruleIndex = 0; ruleIndex < m_whitelistRules.size(); ruleIndex++) {
		const auto& whitelistRule = m_whitelistRules.at(ruleIndex);
		const telemetry::FileOps fileOps
			= {[&whitelistRule]() { return createWhitelistRuleTelemetryContent(whitelistRule); },
			   nullptr};
		auto ruleFile = rulesDirectory->addFile(std::to_string(ruleIndex), fileOps);
		m_holder.add(ruleFile);
	}

	const telemetry::AggOperation aggFileOps = {
		telemetry::AggMethodType::SUM,
		"matchedCount",
		"totalMatchedCount",
	};

	auto aggFile = directory->addAggFile("aggStats", "rules/.*", {aggFileOps});
	m_holder.add(aggFile);
}

} // namespace Whitelist
