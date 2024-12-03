# bootstrap.cmake

set( BOOTSTRAP_DIR ${CMAKE_CURRENT_LIST_DIR} )
set( BOOTSTRAP_SOURCE_FILES
	"${BOOTSTRAP_DIR}/main.cpp"
)

add_executable( bootstrap ${BOOTSTRAP_SOURCE_FILES} )

set_target_properties( bootstrap
	PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${GAMEDIR}/bin"
		RUNTIME_OUTPUT_NAME "aurosrc"
)
target_link_libraries( bootstrap
	PRIVATE
		SDL3::SDL3-static  # TODO: When we switch off SDL2 completely revert this to use the shared version
)
