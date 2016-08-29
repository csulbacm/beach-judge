macro(fetch_python2)

if(NOT PYTHON2_FOUND)
	set(T___NAME ${T_PYTHON2_NAME})
	set(T___PATH ${T_PYTHON2_PATH})
	set(T___DL_FILE ${BUILD_EXT_DIR}/${T_PYTHON2_DL_FILE})
	set(T___DL_FOLDER ${BUILD_EXT_DIR}/${T_PYTHON2_DL_FOLDER})
	set(T___DL_MD5 ${T_PYTHON2_DL_MD5})
	set(T___DL_URI ${T_PYTHON2_DL_URI})
	set(T___CK_FILE ${T___PATH}/${T_PYTHON2_CK_FILE})

	if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
		if(NOT EXISTS ${T___CK_FILE})
			if(NOT EXISTS ${T___DL_FILE})
				message("Downloading ${T___NAME} <${T___DL_URI}>...")
				file(DOWNLOAD ${T___DL_URI} ${T___DL_FILE} STATUS T___STATUS SHOW_PROGRESS EXPECTED_MD5 ${T___DL_MD5})
				list(GET T___STATUS 0 T___STATUS)
				if(T___STATUS)
					file(REMOVE ${T___DL_FILE})
					return()
				endif()
			endif()
			file(MAKE_DIRECTORY ${T___PATH})
			message("Unzipping ${T___NAME}...")

			string(REPLACE "/" "\\" T___DL_FILE "${T___DL_FILE}")
			string(REPLACE "/" "\\" T___PATH "${T___PATH}")
			execute_process(COMMAND msiexec /a ${T___DL_FILE} /qb /quiet TARGETDIR=${T___PATH})
		endif()

		# Get pip
		if(NOT EXISTS ${T___PATH}/Scripts/pip.exe)
			file(DOWNLOAD "https://bootstrap.pypa.io/get-pip.py" "${T___PATH}/get-pip.py" SHOW_PROGRESS EXPECTED_MD5 1ebd3cb7a5a3073058d0c9552ab074bd)
			execute_process(COMMAND ${T___PATH}/python.exe get-pip.py WORKING_DIRECTORY ${T___PATH})
		endif()

		set(PYTHON2_FOUND 1 CACHE INTERNAL "")
	endif()

	if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
		#TODO: Implement this
	endif()
endif()

endmacro()
