add_executable(sampler
	main.cpp
	sampler.cpp
)

target_link_libraries(sampler PRIVATE
	telemetry::telemetry
	telemetry::appFs
	common
	unirec::unirec++
	unirec::unirec
	trap::trap
	argparse
)

install(TARGETS sampler DESTINATION ${INSTALL_DIR_BIN})
