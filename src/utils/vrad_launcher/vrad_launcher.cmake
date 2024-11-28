# vrad_launcher.cmake

set( VRAD_LAUNCHER_DIR ${CMAKE_CURRENT_LIST_DIR} )
set( VRAD_LAUNCHER_SOURCE_FILES
	"${VRAD_LAUNCHER_DIR}/vrad_launcher.cpp"

	# Header Files
	"${SRCDIR}/public/tier1/interface.h"
	"${SRCDIR}/public/ivraddll.h"
)

set( vrad_launcher_exclude_source
	"${SRCDIR}/public/tier0/memoverride.cpp"
)

add_executable( vrad_launcher ${VRAD_LAUNCHER_SOURCE_FILES} )

set_target_properties( vrad_launcher
	PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${GAMEDIR}/bin"
)

if (WIN32)
	target_link_options( vrad_launcher
		PRIVATE
			/LARGEADDRESSAWARE
	)
endif ()

target_link_libraries( vrad_launcher
	PRIVATE
		tier0
		tier1
		vstdlib
		sourcepp::steampp
)
add_dependencies( vrad_launcher vrad_dll )
