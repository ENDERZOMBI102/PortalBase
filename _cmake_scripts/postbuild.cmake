# postbuild.cmake

include_guard(GLOBAL)

function(target_strip_symbols target)
	set( bindir )
	set( thirdparty )
	get_target_property( bindir ${target} "BINARY_DIR" )
	string( FIND "${bindir}" "thirdparty" thirdparty )
	if ( ${IS_LINUX} AND "${thirdparty}" STREQUAL "-1" )
		add_custom_command(
			TARGET ${target} POST_BUILD
			COMMAND ${CMAKE_OBJCOPY} "$<TARGET_FILE:${target}>" "$<TARGET_FILE:${target}>.dbg"
			COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink="$<TARGET_FILE:${target}>.dbg" "$<TARGET_FILE:${target}>"
			COMMAND ${CMAKE_STRIP} -x "$<TARGET_FILE:${target}>" $<$<CONFIG:Release>:-S>
		)
	endif()
endfunction()


function( postbuild_copy Target Dest )
	add_custom_command(
		TARGET ${Target}
		POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_PROPERTY:${Target},OUTPUT_NAME>" "${Dest}"
	)
endfunction()