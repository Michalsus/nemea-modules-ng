/**
 * @file
 * @author Pavel Siska <siska@cesnet.cz>
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

namespace {

template <typename UnirecType, typename VariantType = UnirecType>
bool compareField(
	const Nemea::UnirecRecordView& unirecRecordView,
	ur_field_id_t unirecFieldId,
	const std::optional<Whitelist::RuleFieldValue>& fieldValue)
{
	if (!fieldValue.has_value()) {
		return true;
	}

	UnirecType recordValue = unirecRecordView.getFieldAsType<UnirecType>(unirecFieldId);
	VariantType variantValue = std::get<VariantType>(fieldValue.value());

	if constexpr (std::is_same_v<UnirecType, std::string_view>) {
		return std::regex_search(recordValue.data(), variantValue);
	} else if constexpr (std::is_same_v<UnirecType, Nemea::IpAddress>) {
		return variantValue.isBelong(recordValue);
	} else {
		return recordValue == variantValue;
	}
}

bool isRuleFieldMatched(
	const Whitelist::RuleField& ruleField,
	const Nemea::UnirecRecordView& unirecRecordView)
{
	const auto& [unirecFieldId, fieldPattern] = ruleField;

	switch (ur_get_type(unirecFieldId)) {
	case UR_TYPE_CHAR:
		return compareField<char>(unirecRecordView, unirecFieldId, fieldPattern);
	case UR_TYPE_STRING:
		return compareField<std::string_view, std::regex>(
			unirecRecordView,
			unirecFieldId,
			fieldPattern);
		break;
	case UR_TYPE_UINT8:
		return compareField<uint8_t>(unirecRecordView, unirecFieldId, fieldPattern);
		break;
	case UR_TYPE_INT8:
		return compareField<int8_t>(unirecRecordView, unirecFieldId, fieldPattern);
		break;
	case UR_TYPE_UINT16:
		return compareField<uint16_t>(unirecRecordView, unirecFieldId, fieldPattern);
		break;
	case UR_TYPE_INT16:
		return compareField<int16_t>(unirecRecordView, unirecFieldId, fieldPattern);
		break;
	case UR_TYPE_UINT32:
		return compareField<uint32_t>(unirecRecordView, unirecFieldId, fieldPattern);
		break;
	case UR_TYPE_INT32:
		return compareField<int32_t>(unirecRecordView, unirecFieldId, fieldPattern);
		break;
	case UR_TYPE_UINT64:
		return compareField<uint64_t>(unirecRecordView, unirecFieldId, fieldPattern);
		break;
	case UR_TYPE_INT64:
		return compareField<int64_t>(unirecRecordView, unirecFieldId, fieldPattern);
		break;
	case UR_TYPE_IP:
		return compareField<Nemea::IpAddress, Whitelist::IpAddressPrefix>(
			unirecRecordView,
			unirecFieldId,
			fieldPattern);
		break;
	default:
		throw std::runtime_error("Unsopported format");
	}

	return false;
}

bool isWhitelistRuleMatched(
	const Whitelist::WhitelistRule& whitelistRule,
	const Nemea::UnirecRecordView& unirecRecordView)
{
	auto lambdaPredicate
		= [&](const auto& ruleField) { return !isRuleFieldMatched(ruleField, unirecRecordView); };

	return std::any_of(whitelistRule.begin(), whitelistRule.end(), lambdaPredicate);
}

} // namespace

namespace Whitelist {

Whitelist::Whitelist(const ConfigParser* configParser)
{
	const std::string unirecTemplateDescription = configParser->getUnirecTemplateDescription();

	WhitelistRuleBuilder whitelistRuleBuilder(unirecTemplateDescription);

	for (const auto& ruleDescription : configParser->getWhitelistRulesDescription()) {
		auto rule = whitelistRuleBuilder.build(ruleDescription);
		m_whitelistRules.emplace_back(rule);
	}
}

bool Whitelist::isWhitelisted(const Nemea::UnirecRecordView& unirecRecordView) const
{
	auto lambdaPredicate = [&](const auto& whitelistRule) {
		return !isWhitelistRuleMatched(whitelistRule, unirecRecordView);
	};

	return std::any_of(m_whitelistRules.begin(), m_whitelistRules.end(), lambdaPredicate);
}

} // namespace Whitelist
