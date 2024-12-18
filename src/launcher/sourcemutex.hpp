//
// Created by ENDERZOMBI102 on 11/12/2024.
// 
#pragma once


/**
 * Manages the source engine mutex.
 * On windows: hl2_singleton_mutex via CreateMutex
 * On linux:   /tmp/source_engine_${gamecrc32}.lock via exclusive open
 */
namespace SourceMutex {
	/**
	 * Acquires the mutex.
	 */
	void Lock();

	/**
	 * Releases the mutex.
	 */
	void Unlock();
}
