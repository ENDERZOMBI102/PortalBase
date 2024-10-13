# init.cmake

# Function to add a git module as project
function( submodule name folder )
	# check for existence
	if ( NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/${folder}/CMakeLists.txt" )
		message( FATAL_ERROR "Third-party submodule library `${name}` was not found! Did you clone with `--recursive`?\nNote: you may run `git submodule init && git submodule update` to fix" )
	endif()

	add_subdirectory( "${CMAKE_CURRENT_LIST_DIR}/${folder}" EXCLUDE_FROM_ALL SYSTEM )
endfunction()


# For git submodules, declare them here
# ----- SDL -----
set( SDL_STATIC ON )
set( SDL_SHARED ON )
set( SDL_ATOMIC OFF )
set( SDL_AUDIO  OFF )
set( SDL_RENDER OFF )
# `SDL_HIDAPI` is needed for bug on `src/joystick/linux/SDL_sysjoystick.c:323`,
# where an usage of `IsVirtualJoystick` (hidden behind SDL_HIDAPI) isn't disabled when the flag is not given.
set( SDL_HIDAPI ON )
set( SDL_POWER  OFF )
set( SDL_FILE   OFF )
set( SDL_LOADSO ON )  # needed by other modules
set( SDL_FILESYSTEM OFF )
set( SDL_SENSOR OFF )
set( SDL_LOCALE OFF )
set( SDL_MISC   OFF )
submodule( SDL "SDL" )
# ----- SourcePP -----
set( SOURCEPP_BUILD_C_WRAPPERS OFF CACHE INTERNAL "" )
set( XZ_ASM_I386 OFF CACHE INTERNAL "" )
submodule( SourcePP "sourcepp" )
