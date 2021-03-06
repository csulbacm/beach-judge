find_program(unzip "unzip")

macro(do_unzip File Target)
	if(unzip)
		execute_process(COMMAND unzip ${File} -d ${Target})
	else()
		execute_process(COMMAND ${ROOT_DIR}/tools/unzip.bat ${File} ${Target})
	endif()
endmacro()

macro(npm_install Target)
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	if(EXISTS ${T_NODEJS_PATH}/node_modules/${Target})
#		execute_process(COMMAND CMD /c ${NPM} update ${Target} WORKING_DIRECTORY ${T_NODEJS_PATH})
	else()
		execute_process(COMMAND CMD /c ${NPM} install ${Target} WORKING_DIRECTORY ${T_NODEJS_PATH})
	endif()
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	if(EXISTS ${T_NODEJS_PATH}/node_modules/${Target})
#		execute_process(COMMAND ${NPM} update ${Target} WORKING_DIRECTORY ${T_NODEJS_PATH})
	else()
		execute_process(COMMAND ${NPM} install ${Target} WORKING_DIRECTORY ${T_NODEJS_PATH})
	endif()
endif()
endmacro()
