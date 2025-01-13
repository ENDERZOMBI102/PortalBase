# filesystem_stdio.cmake

set( FILESYSTEM_STDIO_DIR ${CMAKE_CURRENT_LIST_DIR} )
set( FILESYSTEM_STDIO_SOURCE_FILES
	"${FILESYSTEM_STDIO_DIR}/basefilesystem.cpp"
	"${FILESYSTEM_STDIO_DIR}/filesystem.cpp"
	"${FILESYSTEM_STDIO_DIR}/queuedloader.cpp"
	"${FILESYSTEM_STDIO_DIR}/driver/fsdriver.cpp"
	"${FILESYSTEM_STDIO_DIR}/driver/packfsdriver.cpp"
	"${FILESYSTEM_STDIO_DIR}/driver/plainfsdriver.cpp"
	"${FILESYSTEM_STDIO_DIR}/driver/rootfsdriver.cpp"

	# Header files
	"${FILESYSTEM_STDIO_DIR}/basefilesystem.hpp"
	"${FILESYSTEM_STDIO_DIR}/filesystem.hpp"
	"${FILESYSTEM_STDIO_DIR}/queuedloader.hpp"
	"${FILESYSTEM_STDIO_DIR}/driver/fsdriver.hpp"
	"${FILESYSTEM_STDIO_DIR}/driver/packfsdriver.hpp"
	"${FILESYSTEM_STDIO_DIR}/driver/plainfsdriver.hpp"
	"${FILESYSTEM_STDIO_DIR}/driver/rootfsdriver.hpp"

	# Public
	"${SRCDIR}/public/filesystem.h"
	"${SRCDIR}/public/filesystem/IQueuedLoader.h"
	"${SRCDIR}/public/wildcard/wildcard.cpp"
	"${SRCDIR}/public/wildcard/wildcard.hpp"
)

add_library( filesystem_stdio MODULE ${FILESYSTEM_STDIO_SOURCE_FILES} )
target_link_libraries( filesystem_stdio
	PUBLIC
		tier1
		appframework
	PRIVATE
		SDL3::SDL3-shared
		vpkpp
)
set_target_properties( filesystem_stdio
	PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY "${GAMEDIR}/bin"
		PREFIX ""
)
