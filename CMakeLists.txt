cmake_minimum_required(VERSION 2.8)

set(BEACHJUDGE_VERSION_MAJOR 0)
set(BEACHJUDGE_VERSION_MINOR 3)
set(BEACHJUDGE_VERSION_REVISION 0)

set(TARGET "beachJudge")
project(${TARGET})

######### Paths #########

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BUILD_DIR ${CMAKE_BINARY_DIR})
set(BUILD_EXT_DIR ${BUILD_DIR}/external)
set(SOURCE_EXT_DIR ${ROOT_DIR}/external)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(JS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/js)
set(STYLES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/styles)
set(TEMPLATES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/templates)
set(RES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/res)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Macros.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/DependencyManifest.cmake")

######### Dependencies #########

set(INCLUDE ${INC_DIR})
set(LIB "")

# RethinkDB
include("${CMAKE_MODULE_PATH}/FetchRethinkDB.cmake")
fetch_rethinkdb()

# NodeJS
include("${CMAKE_MODULE_PATH}/FetchNodeJS.cmake")
fetch_nodejs()

# Python2
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
include("${CMAKE_MODULE_PATH}/FetchPython2.cmake")
fetch_python2()
endif()

set(BABEL ${BUILD_DIR}/external/nodejs/babel)
set(PID_FILE ${BUILD_DIR}/server.pid)

# Less

set(_F
	${SRC_DIR}/less/admin.less
	${SRC_DIR}/less/judge.less
	${SRC_DIR}/less/public.less
	${SRC_DIR}/less/user.less
)
source_group("Source\\Less" FILES ${_F})
list(APPEND TARGET_SERVER_FILES ${_F})

set(_F
	${SRC_DIR}/less/common/form.less
	${SRC_DIR}/less/common/layout.less
	${SRC_DIR}/less/common/theme.less
)
source_group("Source\\Less\\Common" FILES ${_F})
list(APPEND TARGET_SERVER_FILES ${_F})

# Pug

set(_F
	${SRC_DIR}/pug/layouts/admin.pug
	${SRC_DIR}/pug/layouts/default.pug
	${SRC_DIR}/pug/layouts/judge.pug
)
source_group("Source\\Pug\\Layouts" FILES ${_F})
list(APPEND TARGET_SERVER_FILES ${_F})

set(_F
	${SRC_DIR}/pug/static/admin.pug
	${SRC_DIR}/pug/static/judge.pug
	${SRC_DIR}/pug/static/login.pug
	${SRC_DIR}/pug/static/user.pug
)
source_group("Source\\Pug\\Static" FILES ${_F})
list(APPEND TARGET_SERVER_FILES ${_F})

# JS

set(_F
	${SRC_DIR}/js/admin/problem.js
	${SRC_DIR}/js/admin/userManagement.js
)
source_group("Source\\JS\\Admin" FILES ${_F})
list(APPEND TARGET_SERVER_FILES ${_F})

set(_F
	${SRC_DIR}/js/common/common.js
)
source_group("Source\\JS\\Common" FILES ${_F})
list(APPEND TARGET_SERVER_FILES ${_F})

set(_F
	${SRC_DIR}/js/judge/empty.js
)
source_group("Source\\JS\\Judge" FILES ${_F})
list(APPEND TARGET_SERVER_FILES ${_F})

set(_F
	${SRC_DIR}/js/public/login.js
)
source_group("Source\\JS\\Public" FILES ${_F})
list(APPEND TARGET_SERVER_FILES ${_F})

# Node

set(_F
	${SRC_DIR}/node/config.js
	${SRC_DIR}/node/generate-css.js
	${SRC_DIR}/node/generate-database.js
	${SRC_DIR}/node/generate-html.js
	${SRC_DIR}/node/generate-js.js
	${SRC_DIR}/node/index.js
	${SRC_DIR}/node/server.js
	${SRC_DIR}/node/start.js
	${SRC_DIR}/node/stop.js
)
source_group("Source\\Node" FILES ${_F})
set(SOURCES_JS ${_F})
list(APPEND TARGET_SERVER_FILES ${_F})

set(_F "")
foreach(FILE ${SOURCES_JS})
	get_filename_component(NAME ${FILE} NAME_WE)
	get_filename_component(EXT ${FILE} EXT)
	string(SUBSTRING ${EXT} 1 -1 EXT)
	set(GENFILE ${BUILD_DIR}/generated/node/${NAME}.${EXT})
	list(APPEND _F ${GENFILE})
endforeach()
source_group("Generated\\Node" FILES ${_F})
list(APPEND TARGET_SERVER_FILES ${_F})
set_source_files_properties(${_F} PROPERTIES GENERATED TRUE)

# CMake

set(_F
	${ROOT_DIR}/CMakeLists.txt
	${ROOT_DIR}/cmake/Macros.cmake
	${ROOT_DIR}/cmake/DependencyManifest.cmake
)
source_group("Source\\CMake" FILES ${_F})
list(APPEND TARGET_SERVER_FILES ${_F})

set(_F
	${ROOT_DIR}/cmake/Modules/FetchNodeJS.cmake
	${ROOT_DIR}/cmake/Modules/FetchRethinkDB.cmake
)
source_group("Source\\CMake\\Modules" FILES ${_F})
list(APPEND TARGET_SERVER_FILES ${_F})

# Targets

add_custom_target(ServerStop COMMAND ${NPM} run stop)
add_custom_target(ServerGeneration SOURCES ${TARGET_SERVER_FILES} COMMAND ${NPM} run build DEPENDS ServerStop)
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
add_custom_target(ServerStart COMMAND ${NPM} start DEPENDS ServerGeneration BYPRODUCTS ${PID_FILE} USES_TERMINAL)
endif()
add_custom_target(Dummy DEPENDS ServerStart)
