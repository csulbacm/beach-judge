macro(fetch_sqlite)
if(NOT SQLITE_FOUND)


set(T_SQL_PATH "${PROJECT_BINARY_DIR}/external/sqlite")

#TODO: Windows fetch

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	if(NOT EXISTS ${T_SQL_PATH}/README)
		set(T_SQL_DL "${PROJECT_BINARY_DIR}/external/sqlite.tar.gz")
		if(NOT EXISTS ${T_SQL_DL})
			message("Downloading SQLite...")
			file(DOWNLOAD "https://www.sqlite.org/2015/sqlite-autoconf-3090100.tar.gz" ${T_SQL_DL} SHOW_PROGRESS)
		endif()
		message("Unzipping SQLite...")
		execute_process(COMMAND cmake -E tar jxf ${T_SQL_DL} WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/external)
		execute_process(COMMAND sh -c "cmake -E rename ${PROJECT_BINARY_DIR}/external/sqlite-autoconf-* ${T_SQL_PATH}")
	endif()

	if(NOT EXISTS ${T_SQL_PATH}/.libs/libsqlite3.a)
		message("Configuring SQLIte...")
		execute_process(COMMAND sh ./configure WORKING_DIRECTORY ${T_SQL_PATH})
		message("Building SQLite...")
		execute_process(COMMAND make WORKING_DIRECTORY ${T_SQL_PATH})
	endif()

endif()

set(CMAKE_INCLUDE_PATH ${T_SQL_PATH})
set(CMAKE_LIBRARY_PATH ${T_SQL_PATH}/.libs)
find_package(SQLite REQUIRED)
unset(CMAKE_INCLUDE_PATH)
unset(CMAKE_LIBRARY_PATH)


endif()
endmacro()
