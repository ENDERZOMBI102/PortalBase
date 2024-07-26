#pragma once

//#define IMGUI_DISABLE                                     // Disable everything: all headers and source files will be empty.

// Use tier0's asserts
#include "tier0/dbg.h"

#undef IM_ASSERT
#define IM_ASSERT Assert

// We provide our own allocators.
#define IMGUI_DISABLE_DEFAULT_ALLOCATORS


// We do not define Dear ImGui's api to cross dll boundaries.
// Instead, everything that wants to use Dear ImGui can link devui_static.lib.
#define IMGUI_API

// Disable obsolete APIs. No need to make more work for ourselves.
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_DISABLE_OBSOLETE_KEYIO

// Let's not link win32 for everything we include Dear ImGui with...
#define IMGUI_DISABLE_WIN32_FUNCTIONS

// We need to disable Dear ImGui's file functions, so we can pass them to Source's filesystem. Otherwise, we won't be able to access VPKs
#define IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS
using ImFileHandle = void*;
IMGUI_API ImFileHandle ImFileOpen( const char* filename, const char* mode );
IMGUI_API bool ImFileClose( ImFileHandle file );
IMGUI_API uint64 ImFileGetSize( ImFileHandle file );
IMGUI_API uint64 ImFileRead( void* data, uint64 size, uint64 count, ImFileHandle file );
IMGUI_API uint64 ImFileWrite( const void* data, uint64 size, uint64 count, ImFileHandle file );


// Source's colors are stored as BGRA. Setting this allows us to avoid per vertex swizzles in mesh builder.
// On Linux and Mac, mesh builder already does these swizzles regardless
#if !defined( OPENGL_COLOR_SWAP )
//#define IMGUI_USE_BGRA_PACKED_COLOR
#endif


//---- Use stb_sprintf.h for a faster implementation of vsnprintf instead of the one from libc (unless IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS is defined)
// Compatibility checks of arguments and formats done by clang and GCC will be disabled in order to support the extra formats provided by stb_sprintf.h.
//#define IMGUI_USE_STB_SPRINTF

#include "mathlib/vector2d.h"
#include "mathlib/vector4d.h"
#define IM_VEC2_CLASS_EXTRA                             \
	ImVec2( const Vector2D& f ) : x( f.x ), y( f.y ) {} \
	operator Vector2D() const { return Vector2D( x, y ); }

#define IM_VEC4_CLASS_EXTRA                                                 \
	ImVec4( const Vector4D& f ) : x( f.x ), y( f.y ), z( f.z ), w( f.w ) {} \
	operator Vector4D() const { return Vector4D( x, y, z, w ); }

#include "strtools.h"
#ifdef strncpy
	#undef strncpy
	#define strncpy Q_strncpy
#endif

class IMaterial;
#define ImTextureID IMaterial*