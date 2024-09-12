//
// Created by ENDERZOMBI102 on 23/02/2024.
//
#include "packfsdriver.hpp"
#include "utlvector.h"
#include "strtools.h"
#include "wildcard/wildcard.hpp"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


CPackFsDriver::CPackFsDriver( int32 pId, const char* pAbsolute, const char* pPath )
	: m_iId{ pId }, m_szNativePath{ V_strdup( pPath ) }, m_PackFile{ vpkpp::PackFile::open( pAbsolute, {} ) }, CFsDriver() { }
auto CPackFsDriver::GetNativePath() const -> const char* {
	return this->m_szNativePath;
}
auto CPackFsDriver::GetNativeAbsolutePath() const -> const char* {
	return this->m_PackFile->getFilepath().data();
}
auto CPackFsDriver::GetIdentifier() const -> int32 {
	return this->m_iId;
}
auto CPackFsDriver::GetType() const -> const char* {
	return "pack";
}
auto CPackFsDriver::Shutdown() -> void { }

// FS interaction
auto CPackFsDriver::Open( const char* pPath, OpenMode pMode ) -> FileDescriptor* {
	AssertFatalMsg( pPath, "Was given a `NULL` file path!" );
	AssertFatalMsg( pMode, "Was given an empty open mode!" );

	auto maybeEntry{ m_PackFile->findEntry( pPath ) };
	if (! maybeEntry ) {
		return nullptr;
	}
	const auto& entry{ *maybeEntry };

	auto desc{ FileDescriptor::Make() };
	desc->m_Handle = reinterpret_cast<uintptr_t>( V_strdup( pPath ) );
	desc->m_Size = static_cast<int64>( entry.length );
	return desc;
}
auto CPackFsDriver::Read( const FileDescriptor* pDesc, void* pBuffer, uint32 pCount ) -> int32 {
	AssertFatalMsg( pDesc, "Was given a `NULL` file handle!" );
	AssertFatalMsg( pBuffer, "Was given a `NULL` buffer ptr!" );

	const auto maybeData{ m_PackFile->readEntry( reinterpret_cast<const char*>( pDesc->m_Handle ) ) };
	if (! maybeData ) {
		return -1;
	}

	const auto size{ std::min( static_cast<int32>( pCount ), static_cast<int32>( pDesc->m_Size ) ) };
	const auto& data{ *maybeData };
	V_memcpy( pBuffer,  data.data(), size );
	return size;
}
auto CPackFsDriver::Write( const FileDescriptor* pDesc, void const* pBuffer, uint32 pCount ) -> int32 {
	AssertFatalMsg( false, "Not supported!!" );
	std::unreachable();
}
auto CPackFsDriver::Flush( const FileDescriptor* pDesc ) -> bool {
	AssertFatalMsg( false, "Not supported!!" );
	std::unreachable();
}
auto CPackFsDriver::Close( const FileDescriptor* pDesc ) -> void {
	delete[] reinterpret_cast<const char*>( pDesc->m_Handle );
}

auto CPackFsDriver::ListDir( const char* pWildcard, CUtlVector<const char*>& pResult ) -> void {
	const auto& entries{ m_PackFile->getBakedEntries() };
	const auto patternLen{ V_strlen( pWildcard ) };
	std::string key;
	for ( auto entry{ entries.cbegin() }; entry != entries.cend(); ++entry ) {
		entry.key( key );
		if ( Wildcard::SimpleMatch( key.c_str(), pWildcard, static_cast<int32>( key.size() ), patternLen ) ) {
			pResult.AddToTail( V_strdup( key.c_str() ) );
		}
	}
}
auto CPackFsDriver::Create( const char* pPath, FileType pType, OpenMode pMode ) -> FileDescriptor* {
	AssertFatalMsg( false, "Not supported!!" );
	return nullptr;
}
auto CPackFsDriver::Remove( const FileDescriptor* pDesc ) -> void {
	AssertFatalMsg( false, "Not supported!!" );
	std::unreachable();
}
auto CPackFsDriver::Stat( const FileDescriptor* pDesc ) -> std::optional<StatData> {
	return {};
}
