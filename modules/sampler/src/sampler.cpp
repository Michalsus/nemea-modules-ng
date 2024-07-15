/**
 * @file
 * @author Pavel Siska <siska@cesnet.cz>
 * @brief Implementation of the Sampler class.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sampler.hpp"

namespace Sampler {

Sampler::Sampler(std::size_t samplingRate)
	: m_samplingRate(samplingRate)
{
}

bool Sampler::shouldBeSampled() noexcept
{
	m_totalRecords++;

	if ((m_totalRecords % m_samplingRate) == 0) {
		m_sampledRecords++;
		return true;
	}

	return false;
}

SamplerStats Sampler::getStats() const noexcept
{
	SamplerStats stats;
	stats.totalRecords = m_totalRecords;
	stats.sampledRecords = m_sampledRecords;
	return stats;
}

} // namespace Sampler
