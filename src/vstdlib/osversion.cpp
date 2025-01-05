//
// Created by ENDERZOMBI102 on 21/11/2024.
//
#include "vstdlib/osversion.h"
#include "dbg.h"
#include "strtools.h"
#include <cstdio>
#include <fstream>
#if IsWindows()
	#include <winnt.h>
#elif IsLinux()
	#include <sys/utsname.h>
#endif
// This must be the final include in a .cpp file!!!
#include "memdbgon.h"


auto GetNameFromOSType( const EOSType eOSType ) -> const char* {
	switch ( eOSType ) {
		case k_eLinuxUnknown:
			return "Linux (unknown)";
		case k_eLinux22:
			return "Linux 2.2";
		case k_eLinux24:
			return "Linux 2.4";
		case k_eLinux26:
			return "Linux 2.6";
		case k_eMacOSUnknown:
			return "Mac OS (unknown)";
		case k_eMacOS104:
			return "MacOS 10.4";
		case k_eMacOS105:
			return "MacOS 10.5";
		case k_eMacOS1058:
			return "MacOS 10.5.8";
		case k_eMacOS106:
			return "MacOS 10.6";
		case k_eMacOS1063:
			return "MacOS 10.6.3";
		case k_eMacOS107:
			return "MacOS 10.7";
		case k_eWinUnknown:
			return "Windows (unknown)";
		case k_eWin311:
			return "Windows 3.11";
		case k_eWin95:
			return "Windows 95";
		case k_eWin98:
			return "Windows 98";
		case k_eWinME:
			return "Windows ME";
		case k_eWinNT:
			return "Windows NT";
		case k_eWin2000:
			return "Windows 2000";
		case k_eWinXP:
			return "Windows XP";
		case k_eWin2003:
			return "Windows 2003";
		case k_eWinVista:
			return "Windows Vista";
		case k_eWindows7:
			return "Windows 7";
		case k_eWin2008:
			return "Windows 2008";
		default:
			return "Unknown";
	}
}
auto GetOSDetailString( char* oBuffer, const int pBufferSize ) -> const char* {
	#if IsLinux()
		// successfully tested on: Arch, openSUSE
		std::fstream stream{ "/etc/os-release", std::ios_base::in };

		std::string line{};
		line.reserve( 256 );
		while ( std::getline( stream, line ) ) {
			if ( stream.eof() or line.empty() ) {
				return nullptr;
			}
			if ( line.starts_with( "NAME=" ) ) {
				line[line.length() - 1] = '\0';
				V_strncpy( oBuffer, line.c_str() + 6, pBufferSize );
				return oBuffer;
			}
		}
		return nullptr;
	#elif IsWindows()
		return nullptr;
	#endif
}
auto GetOSType() -> EOSType {
	#if IsWindows()
		OSVERSIONINFOEXA verinfo{};
		verinfo.dwOSVersionInfoSize = sizeof( verinfo );
		// early riser :3
		if ( not GetVersionExA( reinterpret_cast<LPOSVERSIONINFOA>( &verinfo ) ) ) {
			return EOSType::k_eWinUnknown;
		}
		if ( verinfo.wProductType == VER_PLATFORM_WIN32s ) {
		    return EOSType::k_eWin311;
		}
		// http://vbnet.mvps.org/index.html?code/system/getversionex.htm
	    if ( verinfo.wProductType == VER_PLATFORM_WIN32_WINDOWS ) {
	    	if ( verinfo.dwMinorVersion == 0 ) { return EOSType::k_eWin95; }
	    	if ( verinfo.dwMinorVersion == 10 ) { return EOSType::k_eWin98; }
	    	if ( verinfo.dwMinorVersion == 90 ) { return EOSType::k_eWinME; }
	    }
	    if ( verinfo.dwMajorVersion == 4 ) {
	        return EOSType::k_eWinNT;
	    }
	    if ( verinfo.dwMajorVersion == 5 and verinfo.dwMinorVersion == 0 ) {
	        return EOSType::k_eWin2000;
	    }
	    if ( verinfo.dwMajorVersion == 5 and verinfo.dwMinorVersion == 1 ) {
	        return EOSType::k_eWinXP;
	    }
	    if ( verinfo.dwMajorVersion == 5 and verinfo.dwMinorVersion == 2 ) {
	        return EOSType::k_eWin2003;
	    }
	    if ( verinfo.dwMajorVersion == 6 and verinfo.dwMinorVersion == 0 ) {
	        return EOSType::k_eWinVista;
	    }
	    if ( verinfo.dwMajorVersion == 6 and verinfo.dwMinorVersion == 1 and verinfo.wProductType == VER_NT_WORKSTATION ) {
	        return EOSType::k_eWindows7;
	    }
	    if ( verinfo.dwMajorVersion == 6 and verinfo.dwMinorVersion == 1 and verinfo.wProductType != VER_NT_WORKSTATION ) {
	        return EOSType::k_eWin2008;
	    }
		return EOSType::k_eWinUnknown;
	#elif IsLinux()
		utsname data{};
		uname( &data );
		// TODO: Parsing the string is not needed, can just compare the chars directly
		short major{};
		short minor{};
		std::sscanf( data.release, "%hu.%hu", &major, &minor );

		if ( major == 2 and minor == 2 ) { return EOSType::k_eLinux22; }
		if ( major == 2 and minor == 4 ) { return EOSType::k_eLinux24; }
		if ( major == 2 and minor == 6 ) { return EOSType::k_eLinux26; }
		return EOSType::k_eLinuxUnknown;
	// TODO: Macos?
	#elif IsPosix() && false
		return EOSType::k_eMacOSUnknown;
	#endif
	return EOSType::k_eOSUnknown;
}
auto OSTypesAreCompatible( const EOSType eOSTypeDetected, const EOSType eOSTypeRequired ) -> bool {
	// unknown is only compatible with unknown
	if ( eOSTypeRequired == EOSType::k_eOSUnknown ) {
		return eOSTypeDetected == EOSType::k_eOSUnknown;
	}
	// FIXME: Not checking actual breaking changes

	// windows range: 0..
	if ( eOSTypeDetected >= EOSType::k_eWinUnknown ) {
	    // is the required OS windows?
		return eOSTypeRequired >= EOSType::k_eWinUnknown;
	}
	// macos range: -2..=-102
	if ( eOSTypeDetected < k_eOSUnknown and eOSTypeDetected >= EOSType::k_eMacOSUnknown ) {
	    // is the required OS macOS?
		return eOSTypeRequired < k_eOSUnknown and eOSTypeRequired >= EOSType::k_eMacOSUnknown;
	}
	// linux range: -104..=-203
	if ( eOSTypeDetected < k_eMacOSUnknown and eOSTypeDetected >= EOSType::k_eLinuxUnknown ) {
	    // is the required OS linux?
		return eOSTypeRequired < k_eMacOSUnknown and eOSTypeRequired >= EOSType::k_eLinuxUnknown;
	}
	// nothing left...
	return false;
}
auto GetPlatformName( bool* pIs64Bit ) -> const char* {
	if ( pIs64Bit ) {
		*pIs64Bit = Is64BitOS();
	}

	const auto type{ GetOSType() };
	if ( type >= EOSType::k_eWinUnknown ) {  // 0..
		return "windows";
	} else if ( type < k_eOSUnknown and type >= EOSType::k_eMacOSUnknown ) {  // -2..=-102
		return "macos";
	} else if ( type < k_eMacOSUnknown and type >= EOSType::k_eLinuxUnknown ) {  // -104..=-203
		return "linux";
	}
	return "unknown";
}
