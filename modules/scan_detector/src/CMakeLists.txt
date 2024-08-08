add_executable(scan
	main.cpp
    CircBuff.cpp
)

target_link_libraries(scan PRIVATE
	common
	rapidcsv
	unirec::unirec++
	unirec::unirec
	trap::trap
	argparse
)

install(TARGETS scan DESTINATION ${INSTALL_DIR_BIN})