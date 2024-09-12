//
// Created by ENDERZOMBI102 on 23/02/2024.
//
#include "plainfsdriver.hpp"
#include <dirent.h>
#include <fcntl.h>
#include <filesystem>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>
#include "strtools.h"
#include "dbg.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


CPlainFsDriver::CPlainFsDriver( int32 pId, const char* pAbsolute, const char* pPath )
	: m_iId( pId ), m_szNativePath( V_strdup( pPath ) ), m_szNativeAbsolutePath( V_strdup( pAbsolute ) ), CFsDriver() { }
auto CPlainFsDriver::GetNativePath() const -> const char* {
	return this->m_szNativePath;
}
auto CPlainFsDriver::GetNativeAbsolutePath() const -> const char* {
	return this->m_szNativeAbsolutePath.c_str();
}
auto CPlainFsDriver::GetIdentifier() const -> int32 {
	return this->m_iId;
}
auto CPlainFsDriver::GetType() const -> const char* {
	return "plain";
}
auto CPlainFsDriver::Shutdown() -> void {}

// FS interaction
auto CPlainFsDriver::Open( const char* pPath, OpenMode pMode ) -> FileDescriptor* {
	AssertFatalMsg( pPath, "Was given a `NULL` file path!" );
	AssertFatalMsg( pMode, "Was given an empty open mode!" );

	// create full path
	char buffer[1024];
	V_ComposeFileName( m_szNativeAbsolutePath.c_str(), pPath, buffer, 1024 );

	#if IsLinux()
		int32_t mode2{ 0 };
		// read/write combos
		if ( pMode.read && !pMode.write ) {
			mode2 |= O_RDONLY;
		}
		if ( pMode.write && !pMode.read ) {
			mode2 |= O_WRONLY;
		}
		if ( pMode.read && pMode.write ) {
			mode2 |= O_RDWR;
		}

		// other modes
		if ( pMode.truncate ) {
			mode2 |= O_TRUNC;
		}
		if ( pMode.close ) {
			mode2 |= O_CLOEXEC;
		}
		if ( pMode.append ) {
			mode2 |= O_APPEND;
		}

		int file{ open( buffer, mode2 ) };

		// Check if we got a valid handle, TODO: Actual error handling
		if ( file == -1 ) {
			return nullptr;
		}
	#elif IsWindows()
		#error "Not implemented yet"
	#endif

	auto desc{ FileDescriptor::Make() };
	desc->m_Handle = file;
	return desc;
}
auto CPlainFsDriver::Read( const FileDescriptor* pDesc, void* pBuffer, uint32 pCount ) -> int32 {
	AssertFatalMsg( pDesc, "Was given a `NULL` file handle!" );
	AssertFatalMsg( pBuffer, "Was given a `NULL` buffer ptr!" );

	return pread64( static_cast<int>( pDesc->m_Handle ), pBuffer, pCount, static_cast<__off64_t>( pDesc->m_Offset ) );
}
auto CPlainFsDriver::Write( const FileDescriptor* pDesc, const void* pBuffer, uint32 pCount ) -> int32 {
	AssertFatalMsg( pDesc, "Was given a `NULL` file handle!" );
	AssertFatalMsg( pBuffer, "Was given a `NULL` buffer ptr!" );

	return pwrite64( static_cast<int>( pDesc->m_Handle ), pBuffer, pCount, static_cast<__off64_t>( pDesc->m_Offset ) );
}
auto CPlainFsDriver::Flush( const FileDescriptor* pDesc ) -> bool {
	AssertFatalMsg( pDesc, "Was given a `NULL` file handle!" );

	#if IsWindows()
		AssertUnreachable();
	#endif

	return true;
}
auto CPlainFsDriver::Close( const FileDescriptor* pDesc ) -> void {
	close( static_cast<int>( pDesc->m_Handle ) );
}

auto CPlainFsDriver::ListDir( const char* pWildcard, CUtlVector<const char*>& pResult ) -> void {
	auto path{ V_strdup( pWildcard ) };
	V_StripFilename( path );
	const auto dir{ opendir( path ) }; // FIXME: This errors for globs like `path/*/nom?.txt`
	dirent* entry{ nullptr };
	while ( (entry = readdir( dir )) != nullptr ) {  // TODO: Finish impl this
		Log( "%s: %s", __FUNCTION__, entry->d_name );
	}
	closedir( dir );
}
auto CPlainFsDriver::Create( const char* pPath, FileType pType, OpenMode pMode ) -> FileDescriptor* { return {}; }
auto CPlainFsDriver::Remove( const FileDescriptor* pDesc ) -> void { }
auto CPlainFsDriver::Stat( const FileDescriptor* pDesc ) -> std::optional<StatData> {
	// get stat data
	struct stat64 it {};
	if ( fstat64( static_cast<int>( pDesc->m_Handle ), &it ) != 0 ) {
		return {};// error happened :/
	}

		// find the type
	auto fileType{ FileType::Unknown };
	if ( S_ISDIR( it.st_mode ) ) {
		fileType = FileType::Directory;
	} else if ( S_ISREG( it.st_mode ) ) {
		fileType = FileType::Regular;
	} else if ( S_ISSOCK( it.st_mode ) ) {
		fileType = FileType::Socket;
	}

	// return value
	return { StatData{ fileType, static_cast<uint64>( it.st_atim.tv_nsec ), static_cast<uint64>( it.st_mtim.tv_nsec ), static_cast<uint64>( it.st_size ) } };
}
