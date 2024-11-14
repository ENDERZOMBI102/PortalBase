//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: `windows.h` with only the bare minimum.
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef WINLITE_H
	#define WINLITE_H
	#pragma once

	#if IsWindows()
		//
		// Prevent tons of unused windows definitions
		//
		#if !defined( WIN32_LEAN_AND_MEAN )
			#define WIN32_LEAN_AND_MEAN
		#endif
		#define NOWINRES
		#define NOSERVICE
		#define NOMINMAX
		#define NOMCX
		#define NOIME
		#pragma warning(push, 1)
			#pragma warning(disable: 4005)
			#include <windows.h>
		#pragma warning(pop)
		#undef PostMessage

		#pragma warning( disable: 4800 )	// forcing value to bool 'true' or 'false' (performance warning)
	#endif
#endif
