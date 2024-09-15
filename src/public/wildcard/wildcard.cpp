//
// Created by ENDERZOMBI102 on 09/09/2024.
//
#include "wildcard/wildcard.hpp"
#include "tier0/platform.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "memdbgon.h"


#if IsWindows()
	#define IsSeparator( c ) ((c) == '\\' || (c) == '/')
#else
	#define IsSeparator( c ) ((c) == '/')
#endif

namespace Wildcard {
	auto Match( const char* pString, const char* pPattern, const bool pPartial ) -> bool {
		// empty string can only match with empty pattern
		if ( *pString == '\0' ) {
			return pPartial || *pPattern == '\0';
		}

		// actual match
		while ( *pString ) {
			if ( *pPattern == '?' ) {
				// anything one time, if we get here, we HAVE a character
				pPattern += 1;
				pString += 1;
			} else if ( *pPattern == '*' ) {
				// anything N times but / (and \ if on windows)
				pPattern += 1;
				// if we immediately get a `.`, the match fails
				if ( *pString == '.' ) {
					return false;
				}
				// FIXME: This isn't how it should behave, but it'll do for now
				while ( *pString && !IsSeparator( *pString ) && *pString != *pPattern ) {
					pString += 1;
				}
			} else {
				// literal matching
				if ( *pString != *pPattern ) {
					return false;
				}
				pPattern += 1;
				pString += 1;
			}
		}
		// if we got here, the string has finished, but maybe the pattern wasn't...
		return pPartial || *pPattern == '\0' && *pString == '\0';
	}
}
