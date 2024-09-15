//
// Created by ENDERZOMBI102 on 09/09/2024.
//
#pragma once


namespace Wildcard {
	/**
	 * Performs wildcard matching, also known as globbing.
	 * @param pString The string to check.
	 * @param pPattern The pattern to match against.
	 * @param pPartial If `true`, return `true` as long as the leading part of the wildcard is respected.
	 * @return
	 */
	auto Match( const char* pString, const char* pPattern, bool pPartial = false ) -> bool;
}
