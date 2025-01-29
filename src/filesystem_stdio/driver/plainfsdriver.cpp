//
// Created by ENDERZOMBI102 on 23/02/2024.
//
#include "plainfsdriver.hpp"
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "strtools.h"
#include "dbg.h"
#include "wildcard/wildcard.hpp"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


CPlainFsDriver::CPlainFsDriver( int32 pId, const char* pAbsolute, const char* pPath )
	: m_iId( pId ), m_szNativePath( V_strdup( pPath ) ), m_szNativeAbsolutePath( V_strdup( pAbsolute ) ), CFsDriver() { }
auto CPlainFsDriver::GetNativePath() const -> const char* {
	return m_szNativePath;
}
auto CPlainFsDriver::GetNativeAbsolutePath() const -> const char* {
	return m_szNativeAbsolutePath.c_str();
}
auto CPlainFsDriver::GetIdentifier() const -> int32 {
	return m_iId;
}
auto CPlainFsDriver::GetType() const -> const char* {
	return "plain";
}
auto CPlainFsDriver::Shutdown() -> void {}

// FS interaction
auto CPlainFsDriver::Open( const char* pPath, OpenMode pMode ) -> FileDescriptor* {
	AssertFatalMsg( pPath, "Was given a `NULL` file path!" );
	AssertFatalMsg( pMode, "Was given an empty open mode!" );

	// create the full path
	char buffer[1024];
	V_ComposeFileName( m_szNativeAbsolutePath.c_str(), pPath, buffer, 1024 );

	#if IsLinux()
		int32_t mode2{ 0 };
		// read/write combos
		if ( pMode.read and not pMode.write ) {
			mode2 |= O_RDONLY;
		}
		if ( pMode.write and not pMode.read ) {
			mode2 |= O_WRONLY;
		}
		if ( pMode.read and pMode.write ) {
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

		if ( pMode.write or pMode.append ) {
			mode2 |= O_CREAT;
		}

		int file{ open( pPath, mode2 ) };

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

// TODO: Verify if this is feature-complete
auto CPlainFsDriver::ListDir( const char* pPattern, CUtlVector<const char*>& pResult ) -> bool {
	auto path{ V_strdup( pPattern ) };
	V_StripFilename( path );

	// first check if we can open the dir
	const auto dir{ opendir( path ) };
	if ( dir == nullptr ) {
		return false;
	}
	char buffer[1024];
	// iterate in it
	for ( const auto* entry{ readdir( dir ) }; entry != nullptr; entry = readdir( dir ) ) {
		V_ComposeFileName( path, entry->d_name, buffer, 1024 );
		if ( Wildcard::Match( buffer, pPattern, true ) ) {
			// printf( "%s: %s -> %s | %s\n", __FUNCTION__, pPattern, buffer, entry->d_name );
			pResult.AddToTail( V_strdup( entry->d_name ) );
		}
	}
	closedir( dir );
	delete[] path;
	return true;
}
auto CPlainFsDriver::Create( const char* pPath, FileType pType, OpenMode pMode ) -> FileDescriptor* { return {}; }
auto CPlainFsDriver::Remove( const FileDescriptor* pDesc ) -> void { }
auto CPlainFsDriver::Stat( const FileDescriptor* pDesc ) -> std::optional<StatData> {
	// get stat data
	struct stat64 it {};
	if ( fstat64( static_cast<int>( pDesc->m_Handle ), &it ) != 0 ) {
		return {};  // error happened :/
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
