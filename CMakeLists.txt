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
	${INC_DIR}/Judge/User.h
)

set(SRC
	${SRC_DIR}/main.cpp
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
list(APPEND INCLUDE ${LIBWEBSOCKETS_INCLUDE_DIR})

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
set(BUILD_RES mkdir -p ${RES_DIR})
message("Compiling resources...")

# Less
list(APPEND BUILD_RES COMMAND lessc ${STYLES_DIR}/site.less -x > ${RES_DIR}/site.min.css)

# Jade
file(GLOB files "${TEMPLATES_DIR}/*.jade")
foreach(file ${files})
	string(REPLACE ${TEMPLATES_DIR} ${RES_DIR} res ${file})
	string(REPLACE ".jade" ".html" res ${res})
	list(APPEND BUILD_RES COMMAND jade -p ${file} < ${file} > ${res})
endforeach()

# Javascript
file(GLOB files "${JS_DIR}/*.js")
list(APPEND BUILD_RES COMMAND cat ${files} | uglifyjs > ${RES_DIR}/site.min.js)

add_custom_target(res ${BUILD_RES})

add_executable(${PROJECT} ${SRC} ${INC})
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
target_link_libraries(${PROJECT} ${LIB})
add_dependencies(${PROJECT} res)
