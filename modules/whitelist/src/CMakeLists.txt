add_executable(whitelist
	main.cpp
	configParser.cpp
	csvConfigParser.cpp
	ipAddressPrefix.cpp
)

target_link_libraries(whitelist PRIVATE
	common
	rapidcsv
	unirec::unirec++
)

install(TARGETS whitelist DESTINATION ${INSTALL_DIR_BIN})
