//
// Created by ENDERZOMBI102 on 23/02/2024.
//
#pragma once
#include "fsdriver.hpp"


// TODO: Reunify with `CPlainFsDriver`
class CRootFsDriver final : public CFsDriver {
public:
	CRootFsDriver();
	~CRootFsDriver() override = default;
	// metadata
	[[nodiscard]]
	auto GetNativePath() const -> const char* override;
	[[nodiscard]]
	auto GetNativeAbsolutePath() const -> const char* override;
	[[nodiscard]]
	auto GetIdentifier() const -> int32 override;
	[[nodiscard]]
	auto GetType() const -> const char* override;
	auto Shutdown() -> void override;
	// file ops
	auto Open( const char* pPath, OpenMode pMode ) -> FileDescriptor* override;
	auto Read( const FileDescriptor* pDesc, void* pBuffer, uint32 pCount ) -> int32 override;
	auto Write( const FileDescriptor* pDesc, const void* pBuffer, uint32 pCount ) -> int32 override;
	auto Flush( const FileDescriptor* pDesc ) -> bool override;
	auto Close( const FileDescriptor* pDesc ) -> void override;
	// generic ops
	auto ListDir( const char* pPattern, CUtlVector<const char*>& pResult ) -> bool override;
	auto Create( const char* pPath, FileType pType, OpenMode pMode ) -> FileDescriptor* override;
	auto Remove( const FileDescriptor* pDesc ) -> void override;
	auto Stat( const FileDescriptor* pDesc ) -> std::optional<StatData> override;
};
