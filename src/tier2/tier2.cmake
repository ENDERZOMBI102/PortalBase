# tier2.cmake

set( TIER2_DIR ${CMAKE_CURRENT_LIST_DIR} )
set( TIER2_SOURCE_FILES
#	"${TIER2_DIR}/dbg.cpp"

	# Header files
#	"${TIER2_DIR}/memalloc.hpp"

	# Public
	"${SRCDIR}/public/tier2/beamsegdraw.h"
	"${SRCDIR}/public/tier2/camerautils.h"
	"${SRCDIR}/public/tier2/fileutils.h"
	"${SRCDIR}/public/tier2/keybindings.h"
	"${SRCDIR}/public/tier2/meshutils.h"
	"${SRCDIR}/public/tier2/p4helpers.h"
	"${SRCDIR}/public/tier2/renderutils.h"
	"${SRCDIR}/public/tier2/riff.h"
	"${SRCDIR}/public/tier2/soundutils.h"
	"${SRCDIR}/public/tier2/tier2.h"
	"${SRCDIR}/public/tier2/tier2dm.h"
	"${SRCDIR}/public/tier2/utlstreambuffer.h"
	"${SRCDIR}/public/tier2/vconfig.h"
)

#add_library( tier22 SHARED ${TIER2_SOURCE_FILES} )
#target_compile_definitions( tier22
#	PRIVATE
#	TIER0_DLL_EXPORT
#)
#target_link_libraries( tier22
#	PRIVATE
#		SDL3::SDL3-shared
#)
#link_to_bin( TARGET tier22 )
#declare_library( TARGET tier22 )


add_library( tier2 IMPORTED STATIC )
set_target_properties( tier2
	PROPERTIES
		IMPORTED_LOCATION "${LIBPUBLIC}/tier2${CMAKE_IMPORT_LIBRARY_SUFFIX}"
)
