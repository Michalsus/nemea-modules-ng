add_executable(telemetry_stats
	main.cpp
	outputPlugin.cpp
	appfs-plugin.cpp
	ncurses-stdout-plugin.cpp
	ncurses-wrapper.cpp
)

target_link_libraries(telemetry_stats PRIVATE
	ncurses
	telemetry::telemetry
	telemetry::appFs
	common
	unirec::unirec++
	unirec::unirec
	trap::trap
	argparse
)

install(TARGETS telemetry_stats DESTINATION ${INSTALL_DIR_BIN})
