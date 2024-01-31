# bootstrap.cmake

set( BOOTSTRAP_DIR ${CMAKE_CURRENT_LIST_DIR} )
set(
	BOOTSTRAP_SOURCE_FILES
		"${BOOTSTRAP_DIR}/bootstrap.zig"
)

#add_dlang_project( NAMED bootstrap IN ${BOOTSTRAP_DIR} )

add_zig_executable( TARGET bootstrap SHARED SOURCES ${BOOTSTRAP_SOURCE_FILES} )
postbuild_copy( bootstrap "${GAMEDIR}/bin/aurosrc${CMAKE_EXECUTABLE_SUFFIX}" )
