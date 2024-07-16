/**
 * @file
 * @author Pavel Siska <siska@cesnet.cz>
 * @brief Header file for the Ncurces class which provides a simple wrapper around the ncurses
 * library.
 *
 * This file contains the definition of the Ncurces class, which provides a basic interface for
 * initializing ncurses, printing strings to the screen, and cleaning up the ncurses environment.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <mutex>
#include <ncurses.h>
#include <string_view>

namespace TelemetryStats {

/**
 * @brief A simple wrapper class for ncurses functionality.
 *
 * The Ncurces class provides a basic interface for initializing the ncurses library, printing
 * strings to the screen, and cleaning up the ncurses environment.
 */
class Ncurces {
public:
	/**
	 * @brief Constructor for Ncurces.
	 *
	 * Initializes the ncurses environment.
	 */
	Ncurces();

	/**
	 * @brief Prints a string to the screen using ncurses.
	 *
	 * Clears the screen, prints the provided string, and refreshes the screen.
	 *
	 * @param string The string to be printed.
	 */
	void print(const std::string_view& string);

	/**
	 * @brief Destructor for Ncurces.
	 *
	 * Cleans up the ncurses environment.
	 */
	~Ncurces();

private:
	std::mutex m_mutex;
};

} // namespace TelemetryStats
