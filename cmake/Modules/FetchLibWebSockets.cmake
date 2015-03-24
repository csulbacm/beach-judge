macro(bj_fetch_libwebsockets)
	if(NOT LIBWEBSOCKETS_FOUND)
		find_package(LibWebSockets)
		if(NOT LIBWEBSOCKETS_FOUND)
			set(T_LWS_PATH "${PROJECT_BINARY_DIR}/external/libwebsockets")
			message("Unable to find LibWebSockets with default paths.")

			#TODO: Windows fetch

			if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

				if(NOT EXISTS ${T_LWS_PATH}/README.md)
					set(T_LWS_DOWNLOAD_FILE "${PROJECT_BINARY_DIR}/external/libwebsockets.tar.gz")
					if(NOT EXISTS ${T_LWS_DOWNLOAD_FILE})
						message("Downloading LibWebSockets...")
						file(DOWNLOAD "https://api.github.com/repos/warmcat/libwebsockets/tarball/master" ${T_LWS_DOWNLOAD_FILE})
					endif()

					message("Unzipping LibWebSockets...")
					execute_process(COMMAND cmake -E tar jxf ${T_LWS_DOWNLOAD_FILE} WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/external)
					execute_process(COMMAND sh -c "cmake -E rename ${PROJECT_BINARY_DIR}/external/warmcat-libwebsockets* ${T_LWS_PATH}")
				endif()

				if(NOT EXISTS ${T_LWS_PATH}/lib/libwebsockets.a)
					if(NOT EXISTS ${T_LWS_PATH}/CMakeCache.txt)
						message("Generating LibWebSockets CMake files...")
						execute_process(COMMAND cmake -G ${CMAKE_GENERATOR} WORKING_DIRECTORY ${T_LWS_PATH})
					endif()

					message("Building LibWebSockets...")
					execute_process(COMMAND cmake --build . --target websockets WORKING_DIRECTORY ${T_LWS_PATH})
				endif()

			endif()

			message("Adding LibWebSockets to prefix path.")
			set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${T_LWS_PATH}/lib)
			find_package(LibWebSockets REQUIRED)
		endif()
	endif()
endmacro()
