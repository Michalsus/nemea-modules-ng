add_executable(sampler
	main.cpp
)

target_link_libraries(sampler PRIVATE
	common
	rapidcsv
	unirec::unirec++
	unirec::unirec
	trap::trap
	argparse
)

install(TARGETS sampler DESTINATION ${INSTALL_DIR_BIN})
