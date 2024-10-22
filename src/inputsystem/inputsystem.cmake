# inputsystem.cmake

set( INPUTSYSTEM_DIR ${CMAKE_CURRENT_LIST_DIR} )
set( INPUTSYSTEM_SOURCE_FILES
	"${INPUTSYSTEM_DIR}/inputsystem.cpp"
	"${INPUTSYSTEM_DIR}/inputsystem.hpp"
	"${INPUTSYSTEM_DIR}/inputmaps.cpp"
	"${INPUTSYSTEM_DIR}/inputmaps.hpp"
)

add_library( inputsystem MODULE ${INPUTSYSTEM_SOURCE_FILES} )

set_target_properties( inputsystem
	PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY "${GAMEDIR}/bin"
		PREFIX ""
)

target_include_directories( inputsystem
	PRIVATE
		"${SRCDIR}/common"
		"${SRCDIR}/public"
)
target_link_libraries( inputsystem
	PRIVATE
		appframework
		tier0
		tier1
		vstdlib
		tier2
		SDL3::SDL3-shared
)
