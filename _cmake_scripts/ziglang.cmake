# ziglang.cmake

find_program( ZIG_PATH "zig" REQUIRED )
message( NOTICE "Zig found at ${ZIG_PATH}" )

# add_zig_library( TARGET <name> [SHARED|STATIC] SOURCES source1 [source2, ...] )
function( add_zig_library )
	cmake_parse_arguments( ZIG_LIB "SHARED;STATIC" "TARGET" "SOURCES" ${ARGN} )
	# check parameter validity
	if ( ZIG_LIB_SHARED AND ZIG_LIB_STATIC )
		message( SEND_ERROR "Cannot specify both `STATIC` and `SHARED`!" )
	elseif ( ZIG_LIB_SHARED )
		set( type "-dynamic" )
		set( suffix "${CMAKE_SHARED_LIBRARY_SUFFIX}" )
	elseif ( ZIG_LIB_STATIC )
		set( type "-static" )
		set( suffix "${CMAKE_STATIC_LIBRARY_SUFFIX}" )
	else ()
		message( SEND_ERROR "You must specify one of `SHARED` or `STATIC`." )
	endif ()
	if ( NOT DEFINED ZIG_LIB_TARGET )
		message( SEND_ERROR "You must name the target." )
	endif ()
	if ( NOT DEFINED ZIG_LIB_SOURCES )
		message( SEND_ERROR "No sources specified for target ${ZIG_LIB_TARGET}." )
	endif ()
	# add target
	add_custom_target( ${ZIG_LIB_TARGET}
		COMMAND ${ZIG_PATH} build-lib ${type} -Bsymbolic -target ${VCPKG_TARGET_TRIPLET} ${ZIG_LIB_SOURCES}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		SOURCES ${ZIG_LIB_SOURCES}
		VERBATIM
	)
	set_target_properties( ${ZIG_LIB_TARGET}
		PROPERTIES
			OUTPUT_NAME "${ZIG_LIB_TARGET}${suffix}"
			LIBRARY_OUTPUT_NAME "${ZIG_LIB_TARGET}${suffix}"
	)
endfunction()

# add_zig_executable( TARGET <name> SOURCES source1 [source2, ...] )
function( add_zig_executable )
	cmake_parse_arguments( ZIG_EXEC "" "TARGET" "SOURCES" ${ARGN} )
	# check parameter validity
	if ( NOT DEFINED ZIG_EXEC_TARGET )
		message( SEND_ERROR "You must name the target." )
	endif ()
	if ( NOT DEFINED ZIG_EXEC_SOURCES )
		message( SEND_ERROR "No sources specified for target ${ZIG_LIB_TARGET}." )
	endif ()
	# add target
	add_custom_target( ${ZIG_EXEC_TARGET}
		COMMAND ${ZIG_PATH} build-exe -Bsymbolic -target ${VCPKG_TARGET_TRIPLET} ${ZIG_EXEC_SOURCES}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		SOURCES ${ZIG_EXEC_SOURCES}
		VERBATIM
	)
	set_target_properties( ${ZIG_EXEC_TARGET}
		PROPERTIES
			OUTPUT_NAME "${ZIG_EXEC_TARGET}${CMAKE_EXECUTABLE_SUFFIX}"
	)
endfunction()
