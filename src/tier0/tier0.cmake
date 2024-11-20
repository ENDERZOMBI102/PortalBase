# tier0.cmake

set( TIER0_DIR ${CMAKE_CURRENT_LIST_DIR} )
set( TIER0_SOURCE_FILES
	"${TIER0_DIR}/dbg.cpp"
	"${TIER0_DIR}/commandline.cpp"
	"${TIER0_DIR}/platform.cpp"
	"${TIER0_DIR}/threadtools.cpp"
	"${TIER0_DIR}/memalloc.cpp"
	"${TIER0_DIR}/vprof.cpp"

	# Header files
	"${TIER0_DIR}/memalloc.hpp"

	# Private
#	"${TIER0_DIR}/ccvarsystem.hpp"

	# Public
	"${SRCDIR}/public/tier0/annotations.h"
	"${SRCDIR}/public/tier0/basetypes.h"
	"${SRCDIR}/public/tier0/commonmacros.h"
	"${SRCDIR}/public/tier0/cpumonitoring.h"
	"${SRCDIR}/public/tier0/dbg.h"
	"${SRCDIR}/public/tier0/dbgflag.h"
	"${SRCDIR}/public/tier0/dynfunction.h"
	"${SRCDIR}/public/tier0/etwprof.h"
	"${SRCDIR}/public/tier0/EventMasks.h"
	"${SRCDIR}/public/tier0/EventModes.h"
	"${SRCDIR}/public/tier0/fasttimer.h"
	"${SRCDIR}/public/tier0/icommandline.h"
	"${SRCDIR}/public/tier0/IOCTLCodes.h"
	"${SRCDIR}/public/tier0/K8PerformanceCounters.h"
	"${SRCDIR}/public/tier0/l2cache.h"
	"${SRCDIR}/public/tier0/mem.h"
	"${SRCDIR}/public/tier0/memalloc.h"
	"${SRCDIR}/public/tier0/memdbgoff.h"
	"${SRCDIR}/public/tier0/memdbgon.h"
	"${SRCDIR}/public/tier0/memoverride.cpp"
	"${SRCDIR}/public/tier0/minidump.h"
	"${SRCDIR}/public/tier0/platform.h"
#	$<$<BOOL:${WIN32}>:"${SRCDIR}/public/tier0/pointeroverride.asm">
	"${SRCDIR}/public/tier0/progressbar.h"
	"${SRCDIR}/public/tier0/protected_things.h"
	"${SRCDIR}/public/tier0/stacktools.h"
	"${SRCDIR}/public/tier0/systeminformation.h"
	"${SRCDIR}/public/tier0/testthread.h"
	"${SRCDIR}/public/tier0/threadtools.h"
	"${SRCDIR}/public/tier0/tmapi_dummy.h"
	"${SRCDIR}/public/tier0/tslist.h"
	"${SRCDIR}/public/tier0/validator.h"
	"${SRCDIR}/public/tier0/valobject.h"
	"${SRCDIR}/public/tier0/valve_off.h"
	"${SRCDIR}/public/tier0/valve_on.h"
	"${SRCDIR}/public/tier0/vprof.h"
	"${SRCDIR}/public/tier0/vprof_telemetry.h"
	"${SRCDIR}/public/tier0/wchartypes.h"
)

add_library( tier02 SHARED ${TIER0_SOURCE_FILES} )
target_compile_definitions( tier02
	PRIVATE
		TIER0_DLL_EXPORT
)
target_link_libraries( tier02
	PRIVATE
		SDL3::SDL3-shared
)
link_to_bin( TARGET tier02 )
declare_library( TARGET tier02 )

add_library( tier0 IMPORTED SHARED )

if ( UNIX )
	set( TIER0_NAME "libtier0.so" )
	configure_file( "${LIBPUBLIC}/libtier0.so" "${GAMEDIR}/bin/libtier0.so" COPYONLY )
else ()
	set( TIER0_NAME "tier0.lib" )
endif ()

set_target_properties( tier0
	PROPERTIES
		IMPORTED_IMPLIB "${LIBPUBLIC}/${TIER0_NAME}"
		IMPORTED_NO_SONAME true
)
