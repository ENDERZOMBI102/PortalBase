# tier3.cmake

set( TIER3_DIR ${CMAKE_CURRENT_LIST_DIR} )
set( TIER3_SOURCE_FILES
#	"${TIER3_DIR}/dbg.cpp"

	# Header files
#	"${TIER3_DIR}/memalloc.hpp"

	# Public
	"${SRCDIR}/public/tier3/choreoutils.h"
	"${SRCDIR}/public/tier3/mdlutils.h"
	"${SRCDIR}/public/tier3/scenetokenprocessor.h"
	"${SRCDIR}/public/tier3/tier3.h"
	"${SRCDIR}/public/tier3/tier3dm.h"
)

#add_library( tier32 SHARED ${TIER3_SOURCE_FILES} )
#target_compile_definitions( tier32
#	PRIVATE
#		TIER0_DLL_EXPORT
#)
#target_link_libraries( tier32
#	PRIVATE
#		SDL3::SDL3-shared
#)
#link_to_bin( TARGET tier32 )
#declare_library( TARGET tier32 )

add_library( tier3 IMPORTED STATIC )
set_target_properties( tier3
	PROPERTIES
		IMPORTED_LOCATION "${LIBPUBLIC}/tier3${CMAKE_IMPORT_LIBRARY_SUFFIX}"
)
