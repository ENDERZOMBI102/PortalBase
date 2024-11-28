//
// Created by ENDERZOMBI102 on 21/11/2024.
//
#include "vstdlib/osversion.h"
#include "dbg.h"
#include "strtools.h"
#include <cstdio>
#include <fstream>
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
auto GetOSDetailString( char* pchOutBuf, const int cchOutBuf ) -> const char* {
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
				V_strncpy( pchOutBuf, line.c_str() + 6, cchOutBuf );
				return pchOutBuf;
			}
		}
		return nullptr;
	#elif IsWindows()
		AssertUnreachable();
		return nullptr;
	#endif
}
auto GetOSType() -> EOSType {
	if constexpr ( IsLinux() ) {
		return EOSType::k_eLinuxUnknown;
	}
	if constexpr ( IsWindows() ) {
		return EOSType::k_eWinUnknown;
	}
	if constexpr ( IsLinux() ) {
		return EOSType::k_eLinuxUnknown;
	}
	return EOSType::k_eOSUnknown;
}
auto OSTypesAreCompatible( EOSType eOSTypeDetected, EOSType eOSTypeRequired ) -> bool {
	AssertUnreachable();
}
auto GetPlatformName( bool* pbIs64Bit ) -> const char* {
	*pbIs64Bit = Is64BitOS();
	AssertUnreachable();
}


