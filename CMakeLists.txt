cmake_minimum_required(VERSION 2.8)

set(PROJECT "beach-judge")

set(BEACHJUDGE_VERSION_MAJOR 0)
set(BEACHJUDGE_VERSION_MINOR 02)

project(${PROJECT})

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(INC
)

set(SRC
)

set(INCLUDE
	${INC_DIR}
	${EXT_INC_DIR}
)

set(LIB "")

include_directories(${INCLUDE})

add_executable(${PROJECT} ${SRC} ${INC})
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
target_link_libraries(${PROJECT} ${LIB})
