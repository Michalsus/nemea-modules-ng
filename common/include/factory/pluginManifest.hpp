/**
 * @file
 * @author Pavel Siska <siska@cesnet.cz>
 * @brief Definition of the PluginManifest struct and related utilities.
 *
 * This file contains the definition of the PluginManifest struct, which represents
 * metadata and functionalities associated with a plugin. It also defines a type alias
 * for a plugin usage information function and a custom comparator for PluginManifest
 * instances.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <functional>
#include <string>

namespace nm {

/**
 * @brief Type alias for a plugin usage information function.
 *
 * This function provides usage information for a plugin.
 */
using PluginUsage = std::function<void()>;

/**
 * @brief Struct representing the metadata and functionalities associated with a plugin.
 */
struct PluginManifest {
	std::string name; ///< Name of the plugin.
	std::string description; ///< Description of the plugin.
	std::string version; ///< Version of the plugin.
	PluginUsage pluginUsage; ///< Function providing usage information for the plugin.
};

inline bool operator<(const PluginManifest& lhs, const PluginManifest& rhs)
{
	return lhs.name < rhs.name;
}

inline bool operator==(const PluginManifest& lhs, const PluginManifest& rhs)
{
	return lhs.name == rhs.name;
}

} // namespace nm
