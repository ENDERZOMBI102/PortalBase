//
// Created by ENDERZOMBI102 on 23/02/2024.
//
#pragma once
#include "refcount.h"
#include "tier0/platform.h"
#include "utlvector.h"


enum class FileType {
	Regular = 1,
	Socket,
	Directory,

	Unknown = 0
};

/**
 * Stats of a "file".
 */
struct StatData {
	FileType m_Type;    // Type of file
	uint64_t m_AccTime; // Access time in ns (unix time)
	uint64_t m_ModTime; // Modification time in ns (unix time)
	uint64_t m_Length;  // File Length in bytes
};

/**
 * How to open a file.
 */
struct OpenMode {
	bool read     : 1 { false };
	bool write    : 1 { false };
	bool append   : 1 { false };
	bool update   : 1 { false };
	bool binary   : 1 { false };
	bool truncate : 1 { false };
	bool close    : 1 { false };
	bool unused   : 1 { false };

	ALWAYS_INLINE
	explicit operator bool() const { return *reinterpret_cast<const uint8_t*>( this ) != 0; }
};
static_assert( sizeof( OpenMode ) == sizeof( uint8_t ) );

/**
 * Internal representation of an open file.
 * Uses a memory arena to avoid sparse allocations.
 */
struct FileDescriptor { // NOLINT(*-pro-type-member-init)
	class CFsDriver* m_Driver;
	const char* m_Path{nullptr};
	uintptr_t m_Handle;
	uint64 m_Offset{0};
	int64 m_Size{ -1 };

	/**
	 * Allocates a new instance of a descriptor.
	 * @return
	 */
	static auto Make() -> FileDescriptor*;
	/**
	 * Frees an existing descriptor.
	 * @return
	 */
	static auto Free( FileDescriptor* ) -> void;
	/**
	 * Cleans the arena, removing all descriptors.
	 * @return
	 */
	static auto CleanupArena() -> void;
};


class CFsDriver : public CRefCounted<> {
public:
	CFsDriver();
	~CFsDriver() override;
	// metadata
	[[nodiscard]]
	virtual auto GetNativePath() const -> const char* = 0;
	[[nodiscard]]
	virtual auto GetNativeAbsolutePath() const -> const char* = 0;
	[[nodiscard]]
	virtual auto GetIdentifier() const -> int32 = 0;
	[[nodiscard]]
	virtual auto GetType() const -> const char* = 0;
	virtual auto Shutdown() -> void = 0;
	auto operator==( CFsDriver& pOther ) const -> bool { return this == &pOther || this->GetIdentifier() == pOther.GetIdentifier(); }
	auto operator==( const CFsDriver& pOther ) const -> bool { return this == &pOther || this->GetIdentifier() == pOther.GetIdentifier(); }
	// file ops
	virtual auto Open ( const char* pPath, OpenMode pMode ) -> FileDescriptor* = 0;
	virtual auto Read ( const FileDescriptor* pDesc, void* pBuffer, uint32 pCount ) -> int32 = 0;
	virtual auto Write( const FileDescriptor* pDesc, const void* pBuffer, uint32 pCount ) -> int32 = 0;
	virtual auto Flush( const FileDescriptor* pDesc ) -> bool = 0;
	virtual auto Close( const FileDescriptor* pDesc ) -> void = 0;
	// generic ops
	virtual auto ListDir( const char* pPattern, CUtlVector<const char*>& pResult ) -> bool = 0;
	virtual auto Create ( const char* pPath, FileType pType, OpenMode pMode ) -> FileDescriptor* = 0;
	virtual auto Remove ( const FileDescriptor* pDesc ) -> void = 0;
	virtual auto Stat   ( const FileDescriptor* pDesc ) -> std::optional<StatData> = 0;
};
