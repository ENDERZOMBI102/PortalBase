//
// Created by ENDERZOMBI102 on 11/12/2024.
// 
#include "sourcemutex.hpp"
#include "tier0/platform.h"
#if IsWindows()
	#include <synchapi.h>
#elif IsPosix()
	#include <cstdio>
	#include <iterator>
	#include "tier0/icommandline.h"
	#include "strtools.h"
#endif
// This must be the final include in a .cpp file!!!
#include "tier0/memdbgon.h"


namespace SourceMutex {
	#if IsWindows()
		namespace { HANDLE s_SourceMutex{}; }

		void Lock() {
			// do not try to get another mutex
			if ( s_SourceMutex ) {
				return;
			}
			s_SourceMutex = CreateMutex( nullptr, true, "hl2_singleton_mutex" );
		}

		void Unlock() {
			ReleaseMutex( s_SourceMutex );
			s_SourceMutex = nullptr;
		}
	#elif IsPosix()
		namespace { FILE* s_SourceMutex{}; }

		void Lock() {
			// do not try to get another mutex
			if ( s_SourceMutex ) {
				return;
			}

			const char* const game{ CommandLine()->ParmValue( "-game", "hl2" ) };
			const uint32 size{ strlen( game ) };
			// TODO: Implement crc32 to match source's behavior
			uint32 crc32{1};
			for ( int i = 0; i < size; i += 1 ) {
				crc32 *= game[i] | i * game[i];
			}

			char buffer[250] {};
			V_snprintf( buffer, std::size( buffer ), "/tmp/source_engine_%u.lock", crc32 );
			s_SourceMutex = fopen( buffer, "" );
		}

		void Unlock() {
			fclose( s_SourceMutex );
			s_SourceMutex = nullptr;
		}
	#endif
}