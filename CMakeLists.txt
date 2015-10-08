cmake_minimum_required(VERSION 2.8)

set(BEACHJUDGE_VERSION_MAJOR 0)
set(BEACHJUDGE_VERSION_MINOR 02)

set(PROJECT "beachJudge")
project(${PROJECT})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(JS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/js)
set(STYLES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/styles)
set(TEMPLATES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/templates)
set(RES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/res)

set(INC
	${INC_DIR}/Judge/Config.h
	${INC_DIR}/Judge/Base.h
	${INC_DIR}/Judge/Judge.h
	${INC_DIR}/Judge/Persistence.h
	${INC_DIR}/Judge/Session.h
	${INC_DIR}/Judge/Types.h
	${INC_DIR}/Judge/User.h
)

set(SRC
	${SRC_DIR}/main.cpp
	${SRC_DIR}/Persistence.cpp
	${SRC_DIR}/User.cpp
)

set(INCLUDE
	${INC_DIR}
)

set(LIB "")


# libwebsockets
include("${CMAKE_MODULE_PATH}/FetchLibWebSockets.cmake")
fetch_libwebsockets()
list(APPEND LIB ${LIBWEBSOCKETS_LIBRARIES} ssl crypto z)
list(APPEND INCLUDE ${LIBWEBSOCKETS_INCLUDE_DIR} ${LIBWEBSOCKETS_INCLUDE_DIR}/..)

# SSL Cert/Key Generation
#  Generation from libwebsockets
find_package(OpenSSL)
find_package(OpenSSLbins)
if(OPENSSL_EXECUTABLE)
	set(JUDGE_SSL_KEY "${PROJECT_BINARY_DIR}/beachJudge.key.pem")
	set(JUDGE_SSL_CERT "${PROJECT_BINARY_DIR}/beachJudge.pem")

	if(NOT EXISTS ${JUDGE_SSL_KEY} OR NOT EXISTS ${JUDGE_SSL_CERT})
		message("Generating SSL certificate and key...")
		execute_process(
			COMMAND printf "GB\\nErewhon\\nAll around\\nbeachJudge\\n\\nlocalhost\\nnone@invalid.org\\n"
			COMMAND "${OPENSSL_EXECUTABLE}"
				req -new -newkey rsa:1024 -days 10000 -nodes -x509 -keyout "${JUDGE_SSL_KEY}" -out "${JUDGE_SSL_CERT}"
			RESULT_VARIABLE OPENSSL_RETURN_CODE)
	
		if (OPENSSL_RETURN_CODE)
			message(WARNING "!!! Failed to generate SSL certificate for Test Server!!!:\nOpenSSL return code = ${OPENSSL_RETURN_CODE}")
		else()
			message("SUCCESSFULLY generated SSL certificate")
		endif()
	endif()
endif()

configure_file(${INC_DIR}/Judge/Config.h.in ${INC_DIR}/Judge/Config.h)
include_directories(${INCLUDE})

# Resourece Compilation
add_custom_target(res_dir COMMAND ${CMAKE_COMMAND} -E make_directory ${RES_DIR})
set(RES res_dir)

# Less
file(GLOB files "${STYLES_DIR}/*.less")
file(GLOB common "${STYLES_DIR}/common/*.less")
foreach(file ${files})
	string(REPLACE ${STYLES_DIR} ${RES_DIR} res ${file})
	string(REPLACE ".less" ".min.css" res ${res})
	list(APPEND RES ${res})
	add_custom_command(OUTPUT ${res} 
		DEPENDS ${file} ${common}
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/less.sh ${file} ${res})
endforeach()

# Jade
file(GLOB files "${TEMPLATES_DIR}/views/*.jade")
foreach(file ${files})
	string(REPLACE ${TEMPLATES_DIR}/views ${RES_DIR} res ${file})
	string(REPLACE ".jade" ".html" res ${res})
	#TODO: Remove -P when finalized
	list(APPEND RES ${res})
	add_custom_command(OUTPUT ${res} 
		DEPENDS ${file}
		COMMAND jade -P -p ${TEMPLATES_DIR}/file < ${file} > ${res})
endforeach()

# Javascript
#TODO: Remove -b flag when finalized
set(UGFLAG "-b")
file(GLOB files "${JS_DIR}/judge/*.js")
set(res ${RES_DIR}/judge.min.js)
list(APPEND RES ${res})
add_custom_command(OUTPUT ${res} 
	DEPENDS ${files}
	COMMAND cat ${files} | uglifyjs ${UGFLAG} > ${res})
file(GLOB files "${JS_DIR}/public/*.js")
set(res ${RES_DIR}/public.min.js)
list(APPEND RES ${res})
add_custom_command(OUTPUT ${res} 
	DEPENDS ${files}
	COMMAND cat ${files} | uglifyjs ${UGFLAG} > ${res})
file(GLOB files "${JS_DIR}/common/*.js")
set(res ${RES_DIR}/common.min.js)
list(APPEND RES ${res})
add_custom_command(OUTPUT ${res} 
	DEPENDS ${files}
	COMMAND cat ${files} | uglifyjs ${UGFLAG} > ${res})

add_custom_target(resources ALL DEPENDS ${RES})

add_executable(${PROJECT} ${SRC} ${INC})
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
target_link_libraries(${PROJECT} ${LIB})
add_dependencies(${PROJECT} resources)

#TODO: Remove this for release
if(UNIX)
	add_custom_target(r COMMAND bash -c ${PROJECT_SOURCE_DIR}/tools/tsock.sh DEPENDS ${PROJECT})
endif()
