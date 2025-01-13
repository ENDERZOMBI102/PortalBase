//
// Created by ENDERZOMBI102 on 22/02/2024.
//
#pragma once
#include "basefilesystem.hpp"
#include "driver/fsdriver.hpp"
#include "tier1/utldict.h"


#undef AsyncRead
class CFileSystemStdio : public IFileSystem {
public: // IAppSystem
	// Here's where the app systems get to learn about each other
	 bool Connect( CreateInterfaceFn factory ) override;
	 void Disconnect() override;

	 // Here's where systems can access other interfaces implemented by this object
	 // Returns NULL if it doesn't implement the requested interface
	 void* QueryInterface( const char* pInterfaceName ) override;

	 // Init, shutdown
	 InitReturnVal_t Init() override;
	 void Shutdown() override;
public: // IBaseFileSystem
	int Read( void* pOutput, int size, FileHandle_t file ) override;
	int Write( void const* pInput, int size, FileHandle_t file ) override;

	// if pathID is nullptr, all paths will be searched for the file
	FileHandle_t Open( const char* pFileName, const char* pOptions, const char* pathID = nullptr ) override;
	void Close( FileHandle_t file ) override;

	void Seek( FileHandle_t file, int pos, FileSystemSeek_t seekType ) override;
	unsigned int Tell( FileHandle_t file ) override;
	unsigned int Size( FileHandle_t file ) override;
	unsigned int Size( const char* pFileName, const char* pPathID = nullptr ) override;

	void Flush( FileHandle_t file ) override;
	bool Precache( const char* pFileName, const char* pPathID = nullptr ) override;

	bool FileExists( const char* pFileName, const char* pPathID = nullptr ) override;
	bool IsFileWritable( char const* pFileName, const char* pPathID = nullptr ) override;
	bool SetFileWritable( char const* pFileName, bool writable, const char* pPathID = nullptr ) override;

	long GetFileTime( const char* pFileName, const char* pPathID = nullptr ) override;

	//--------------------------------------------------------
	// Reads/writes files to utlbuffers. Use this for optimal read performance when doing open/read/close
	//--------------------------------------------------------
	bool ReadFile( const char* pFileName, const char* pPath, CUtlBuffer& buf, int nMaxBytes = 0, int nStartingByte = 0, FSAllocFunc_t pfnAlloc = nullptr ) override;
	bool WriteFile( const char* pFileName, const char* pPath, CUtlBuffer& buf ) override;
	bool UnzipFile( const char* pFileName, const char* pPath, const char* pDestination ) override;
public: // IFileSystem
	//--------------------------------------------------------
	// Steam operations
	//--------------------------------------------------------

	[[nodiscard]]
	bool IsSteam() const override;

	// Supplying an extra app id will mount this app in addition
	// to the one specified in the environment variable "steamappid"
	//
	// If nExtraAppId is < -1, then it will mount that app ID only.
	// (Was needed by the dedicated server b/c the "SteamAppId" env var only gets passed to steam.dll
	// at load time, so the dedicated couldn't pass it in that way).
	FilesystemMountRetval_t MountSteamContent( int nExtraAppId = -1 ) override;

	//--------------------------------------------------------
	// Search path manipulation
	//--------------------------------------------------------

	// Add paths in priority order (mod dir, game dir, ....)
	// If one or more .pak files are in the specified directory, then they are
	//  added after the file system path
	// If the path is the relative path to a .bsp file, then any previous .bsp file
	//  override is cleared and the current .bsp is searched for an embedded PAK file
	//  and this file becomes the highest priority search path ( i.e., it's looked at first
	//   even before the mod's file system path ).
	void AddSearchPath( const char* pPath, const char* pathID, SearchPathAdd_t addType = PATH_ADD_TO_TAIL ) override;
	bool RemoveSearchPath( const char* pPath, const char* pathID = nullptr ) override;

	// Remove all search paths (including write path?)
	void RemoveAllSearchPaths() override;

	// Remove search paths associated with a given pathID
	void RemoveSearchPaths( const char* szPathID ) override;

	// This is for optimization. If you mark a path ID as "by request only", then files inside it
	// will only be accessed if the path ID is specifically requested. Otherwise, it will be ignored.
	// If there are currently no search paths with the specified path ID, then it will still
	// remember it in case you add search paths with this path ID.
	void MarkPathIDByRequestOnly( const char* pPathID, bool bRequestOnly ) override;

	// converts a partial path into a full path
	// Prefer using the RelativePathToFullPath_safe template wrapper to calling this directly
	const char* RelativePathToFullPath( const char* pFileName, const char* pPathID, OUT_Z_CAP( maxLenInChars ) char* pDest, int maxLenInChars, PathTypeFilter_t pathFilter = FILTER_NONE, PathTypeQuery_t* pPathType = nullptr ) override;


	// Returns the search path, each path is separated by ;s. Returns the length of the string returned
	// Prefer using the GetSearchPath_safe template wrapper to calling this directly
	int GetSearchPath( const char* pathID, bool bGetPackFiles, OUT_Z_CAP( maxLenInChars ) char* pDest, int maxLenInChars ) override;

	// interface for custom pack files > 4Gb
	bool AddPackFile( const char* fullpath, const char* pathID ) override;

	//--------------------------------------------------------
	// File manipulation operations
	//--------------------------------------------------------

	// Deletes a file (on the WritePath)
	void RemoveFile( char const* pRelativePath, const char* pathID = nullptr ) override;

	// Renames a file (on the WritePath)
	bool RenameFile( char const* pOldPath, char const* pNewPath, const char* pathID = nullptr ) override;

	// create a local directory structure
	void CreateDirHierarchy( const char* path, const char* pathID = nullptr ) override;

	// File I/O and info
	bool IsDirectory( const char* pFileName, const char* pPathID = nullptr ) override;

	void FileTimeToString( char* pStrip, int maxCharsIncludingTerminator, long fileTime ) override;

	//--------------------------------------------------------
	// Open file operations
	//--------------------------------------------------------

	void SetBufferSize( FileHandle_t file, unsigned nBytes ) override;

	bool IsOk( FileHandle_t file ) override;

	bool EndOfFile( FileHandle_t file ) override;

	char* ReadLine( char* pOutput, int maxChars, FileHandle_t file ) override;
	int FPrintf( FileHandle_t file, PRINTF_FORMAT_STRING const char* pFormat, ... ) override;

	//--------------------------------------------------------
	// Dynamic library operations
	//--------------------------------------------------------

	// load/unload modules
	CSysModule* LoadModule( const char* pFileName, const char* pPathID = nullptr, bool bValidatedDllOnly = true ) override;
	void UnloadModule( CSysModule * pModule ) override;

	//--------------------------------------------------------
	// File searching operations
	//--------------------------------------------------------

	// FindFirst/FindNext. Also see FindFirstEx.
	const char* FindFirst( const char* pWildCard, FileFindHandle_t* pHandle ) override;
	const char* FindNext( FileFindHandle_t handle ) override;
	bool FindIsDirectory( FileFindHandle_t handle ) override;
	void FindClose( FileFindHandle_t handle ) override;

	// Same as FindFirst, but you can filter by path ID, which can make it faster.
	const char* FindFirstEx( const char* pWildCard, const char* pPathID, FileFindHandle_t* pHandle ) override;

	//--------------------------------------------------------
	// File name and directory operations
	//--------------------------------------------------------

	// FIXME: This method is obsolete! Use RelativePathToFullPath instead!
	// converts a partial path into a full path
	// Prefer using the GetLocalPath_safe template wrapper to calling this directly
	[[deprecated("Use RelativePathToFullPath instead!")]]
	const char* GetLocalPath( const char* pFileName, OUT_Z_CAP( maxLenInChars ) char* pDest, int maxLenInChars ) override;

	// Returns true on success ( based on current list of search paths, otherwise false if
	//  it can't be resolved )
	// Prefer using the FullPathToRelativePath_safe template wrapper to calling this directly
	bool FullPathToRelativePath( const char* pFullpath, OUT_Z_CAP( maxLenInChars ) char* pDest, int maxLenInChars ) override;

	// Gets the current working directory
	bool GetCurrentDirectory( char* pDirectory, int maxlen ) override;

	//--------------------------------------------------------
	// Filename dictionary operations
	//--------------------------------------------------------

	FileNameHandle_t FindOrAddFileName( char const* pFileName ) override;
	bool String( const FileNameHandle_t& pHandle, char* pBuf, int pBufLen ) override;

	//--------------------------------------------------------
	// Asynchronous file operations
	//--------------------------------------------------------

	//------------------------------------
	// Global operations
	//------------------------------------
	FSAsyncStatus_t AsyncRead( const FileAsyncRequest_t& request, FSAsyncControl_t* phControl = nullptr ) { return AsyncReadMultiple( &request, 1, phControl ); }
	FSAsyncStatus_t AsyncReadMultiple( const FileAsyncRequest_t* pRequests, int nRequests, FSAsyncControl_t* phControls = nullptr ) override;
	FSAsyncStatus_t AsyncAppend( const char* pFileName, const void* pSrc, int nSrcBytes, bool bFreeMemory, FSAsyncControl_t* pControl = nullptr ) override;
	FSAsyncStatus_t AsyncAppendFile( const char* pAppendToFileName, const char* pAppendFromFileName, FSAsyncControl_t* pControl = nullptr ) override;
	void AsyncFinishAll( int iToPriority = 0 ) override;
	void AsyncFinishAllWrites() override;
	FSAsyncStatus_t AsyncFlush() override;
	bool AsyncSuspend() override;
	bool AsyncResume() override;

	/// Add async fetcher interface.  This gives apps a hook to intercept async requests and
	/// pull the data from a source of their choosing.  The immediate use case is to load
	/// assets from the CDN via HTTP.
	void AsyncAddFetcher( IAsyncFileFetch* pFetcher ) override;
	void AsyncRemoveFetcher( IAsyncFileFetch* pFetcher ) override;

	//------------------------------------
	// Functions to hold a file open if planning on doing mutiple reads. Use is optional,
	// and is taken only as a hint
	//------------------------------------
	FSAsyncStatus_t AsyncBeginRead( const char* pszFile, FSAsyncFile_t* phFile ) override;
	FSAsyncStatus_t AsyncEndRead( FSAsyncFile_t hFile ) override;

	//------------------------------------
	// Request management
	//------------------------------------
	FSAsyncStatus_t AsyncFinish( FSAsyncControl_t hControl, bool wait = true ) override;
	FSAsyncStatus_t AsyncGetResult( FSAsyncControl_t hControl, void** ppData, int* pSize ) override;
	FSAsyncStatus_t AsyncAbort( FSAsyncControl_t hControl ) override;
	FSAsyncStatus_t AsyncStatus( FSAsyncControl_t hControl ) override;
	// set a new priority for a file already in the queue
	FSAsyncStatus_t AsyncSetPriority( FSAsyncControl_t hControl, int newPriority ) override;
	void AsyncAddRef( FSAsyncControl_t hControl ) override;
	void AsyncRelease( FSAsyncControl_t hControl ) override;

	//--------------------------------------------------------
	// Remote resource management
	//--------------------------------------------------------

	// starts waiting for resources to be available
	// returns FILESYSTEM_INVALID_HANDLE if there is nothing to wait on
	WaitForResourcesHandle_t WaitForResources( const char* resourcelist ) override;
	// get progress on waiting for resources; progress is a float [0, 1], complete is true on the waiting being done
	// returns false if no progress is available
	// any calls after complete is true or on an invalid handle will return false, 0.0f, true
	bool GetWaitForResourcesProgress( WaitForResourcesHandle_t handle, float* progress /* out */, bool* complete /* out */ ) override;
	// cancels a progress call
	void CancelWaitForResources( WaitForResourcesHandle_t handle ) override;

	// hints that a set of files will be loaded in near future
	// HintResourceNeed() is not to be confused with resource precaching.
	int HintResourceNeed( const char* hintlist, int forgetEverything ) override;
	// returns true if a file is on disk
	bool IsFileImmediatelyAvailable( const char* pFileName ) override;

	// copies file out of pak/bsp/steam cache onto disk (to be accessible by third-party code)
	void GetLocalCopy( const char* pFileName ) override;

	//--------------------------------------------------------
	// Debugging operations
	//--------------------------------------------------------

	// Dump to printf/OutputDebugString the list of files that have not been closed
	void PrintOpenedFiles() override;
	void PrintSearchPaths() override;

	// output
	void SetWarningFunc( FileWarningFunc_t pWarning ) override;
	void SetWarningLevel( FileWarningLevel_t level ) override;
	void AddLoggingFunc( FileSystemLoggingFunc_t pLogFunc ) override;
	void RemoveLoggingFunc( FileSystemLoggingFunc_t logFunc ) override;

	// Returns the file system statistics retreived by the implementation.  Returns nullptr if not supported.
	const FileSystemStatistics* GetFilesystemStatistics() override;

	//--------------------------------------------------------
	// Start of new functions after Lost Coast release (7/05)
	//--------------------------------------------------------

	FileHandle_t OpenEx( const char* pFileName, const char* pOptions, unsigned flags = 0, const char* pathID = nullptr, char** ppszResolvedFilename = nullptr ) override;

	// Extended version of read provides more context to allow for more optimal reading
	int ReadEx( void* pOutput, int sizeDest, int size, FileHandle_t file ) override;
	int ReadFileEx( const char* pFileName, const char* pPath, void** ppBuf, bool bNullTerminate = false, bool bOptimalAlloc = false, int nMaxBytes = 0, int nStartingByte = 0, FSAllocFunc_t pfnAlloc = nullptr ) override;

	FileNameHandle_t FindFileName( char const* pFileName ) override;

	#if defined( TRACK_BLOCKING_IO )
		void EnableBlockingFileAccessTracking( bool state ) override;
		bool IsBlockingFileAccessEnabled() const override;

		IBlockingFileItemList* RetrieveBlockingFileAccessInfo() override;
	#endif

	void SetupPreloadData() override;
	void DiscardPreloadData() override;

	void LoadCompiledKeyValues( KeyValuesPreloadType_t type, char const* archiveFile ) override;

	// If the "PreloadedData" hasn't been purged, then this'll try and instance the KeyValues using the fast path of compiled keyvalues loaded during startup.
	// Otherwise, it'll just fall through to the regular KeyValues loading routines
	KeyValues* LoadKeyValues( KeyValuesPreloadType_t type, char const* filename, char const* pPathID = nullptr ) override;
	bool LoadKeyValues( KeyValues& head, KeyValuesPreloadType_t type, char const* filename, char const* pPathID = nullptr ) override;
	bool ExtractRootKeyName( KeyValuesPreloadType_t type, char* outbuf, size_t bufsize, char const* filename, char const* pPathID = nullptr ) override;

	FSAsyncStatus_t AsyncWrite( const char* pFileName, const void* pSrc, int nSrcBytes, bool bFreeMemory, bool bAppend = false, FSAsyncControl_t* pControl = nullptr ) override;
	FSAsyncStatus_t AsyncWriteFile( const char* pFileName, const CUtlBuffer* pSrc, int nSrcBytes, bool bFreeMemory, bool bAppend = false, FSAsyncControl_t* pControl = nullptr ) override;
	// Async read functions with memory blame
	FSAsyncStatus_t AsyncReadCreditAlloc( const FileAsyncRequest_t& request, const char* pszFile, int line, FSAsyncControl_t* phControl = nullptr ) { return AsyncReadMultipleCreditAlloc( &request, 1, pszFile, line, phControl ); }
	FSAsyncStatus_t AsyncReadMultipleCreditAlloc( const FileAsyncRequest_t* pRequests, int nRequests, const char* pszFile, int line, FSAsyncControl_t* phControls = nullptr ) override;

	bool GetFileTypeForFullPath( char const* pFullPath, OUT_Z_BYTECAP( bufSizeInBytes ) wchar_t* buf, size_t bufSizeInBytes ) override;

	//--------------------------------------------------------
	//--------------------------------------------------------
	bool ReadToBuffer( FileHandle_t hFile, CUtlBuffer& buf, int nMaxBytes = 0, FSAllocFunc_t pfnAlloc = nullptr ) override;

	//--------------------------------------------------------
	// Optimal IO operations
	//--------------------------------------------------------
	bool GetOptimalIOConstraints( FileHandle_t hFile, unsigned* pOffsetAlign, unsigned* pSizeAlign, unsigned* pBufferAlign ) override;
	void* AllocOptimalReadBuffer( FileHandle_t hFile, unsigned nSize = 0, unsigned nOffset = 0 ) override;
	void FreeOptimalReadBuffer( void* ) override;

	//--------------------------------------------------------
	//
	//--------------------------------------------------------
	void BeginMapAccess() override;
	void EndMapAccess() override;

	// Returns true on success, otherwise false if it can't be resolved
	// Prefer using the FullPathToRelativePathEx_safe template wrapper to calling this directly
	bool FullPathToRelativePathEx( const char* pFullpath, const char* pPathId, OUT_Z_CAP( maxLenInChars ) char* pDest, int maxLenInChars ) override;

	int GetPathIndex( const FileNameHandle_t& handle ) override;
	long GetPathTime( const char* pPath, const char* pPathID ) override;

	DVDMode_t GetDVDMode() override;

	//--------------------------------------------------------
	// Whitelisting for pure servers.
	//--------------------------------------------------------

	// This should be called ONCE at startup. Multiplayer games (gameinfo.txt does not contain singleplayer_only)
	// want to enable this so sv_pure works.
	void EnableWhitelistFileTracking( bool bEnable, bool bCacheAllVPKHashes, bool bRecalculateAndCheckHashes ) override;

	// This is called when the client connects to a server using a pure_server_whitelist.txt file.
	void RegisterFileWhitelist( IPureServerWhitelist* pWhiteList, IFileList** pFilesToReload ) override;

	// Called when the client logs onto a server. Any files that came off disk should be marked as
	// unverified because this server may have a different set of files it wants to guarantee.
	void MarkAllCRCsUnverified() override;

	// As the server loads whitelists when it transitions maps, it calls this to calculate CRCs for any files marked
	// with check_crc.   Then it calls CheckCachedFileCRC later when it gets client requests to verify CRCs.
	void CacheFileCRCs( const char* pPathname, ECacheCRCType eType, IFileList* pFilter ) override;
	EFileCRCStatus CheckCachedFileHash( const char* pPathID, const char* pRelativeFilename, int nFileFraction, FileHash_t* pFileHash ) override;

	// Fills in the list of files that have been loaded off disk and have not been verified.
	// Returns the number of files filled in (between 0 and nMaxFiles).
	//
	// This also removes any files it's returning from the unverified CRC list, so they won't be
	// returned from here again.
	// The client sends batches of these to the server to verify.
	int GetUnverifiedFileHashes( CUnverifiedFileHash * pFiles, int nMaxFiles ) override;

	// Control debug message output.
	// Pass a combination of WHITELIST_SPEW_ flags.
	int GetWhitelistSpewFlags() override;
	void SetWhitelistSpewFlags( int flags ) override;

	// Installs a callback used to display a dirty disk dialog
	void InstallDirtyDiskReportFunc( FSDirtyDiskReportFunc_t func ) override;

	//--------------------------------------------------------
	// Low-level file caching. Cached files are loaded into memory and used
	// to satisfy read requests (sync and async) until the cache is destroyed.
	// NOTE: this could defeat file whitelisting, if a file were loaded in
	// a non-whitelisted environment and then reused. Clients should not cache
	// files across moves between pure/non-pure environments.
	//--------------------------------------------------------
	FileCacheHandle_t CreateFileCache() override;
	void AddFilesToFileCache( FileCacheHandle_t cacheId, const char** ppFileNames, int nFileNames, const char* pPathID ) override;
	bool IsFileCacheFileLoaded( FileCacheHandle_t cacheId, const char* pFileName ) override;
	bool IsFileCacheLoaded( FileCacheHandle_t cacheId ) override;
	void DestroyFileCache( FileCacheHandle_t cacheId ) override;

	// XXX For now, we assume that all path IDs are "GAME", never cache files
	// outside of the game search path, and preferentially return those files
	// whenever anyone searches for a match even if an on-disk file in another
	// folder would have been found first in a traditional search. extending
	// the memory cache to cover non-game files isn't necessary right now, but
	// should just be a matter of defining a more complex key type. (henryg)

	// Register a CMemoryFileBacking; must balance with UnregisterMemoryFile.
	// Returns false and outputs an ref-bumped pointer to the existing entry
	// if the same file has already been registered by someone else; this must
	// be Unregistered to maintain the balance.
	bool RegisterMemoryFile( CMemoryFileBacking* pFile, CMemoryFileBacking** ppExistingFileWithRef ) override;

	// Unregister a CMemoryFileBacking; must balance with RegisterMemoryFile.
	void UnregisterMemoryFile( CMemoryFileBacking* pFile ) override;

	void CacheAllVPKFileHashes( bool bCacheAllVPKHashes, bool bRecalculateAndCheckHashes ) override;
	bool CheckVPKFileHash( int PackFileID, int nPackFileNumber, int nFileFraction, MD5Value_t& md5Value ) override;

	// Called when we unload a file, to remove that file's info for pure server purposes.
	void NotifyFileUnloaded( const char* pszFilename, const char* pPathId ) override;

	// Returns true on successfully retrieve case-sensitive full path, otherwise false
	// Prefer using the GetCaseCorrectFullPath template wrapper to calling this directly
	bool GetCaseCorrectFullPath_Ptr( const char* pFullPath, OUT_Z_CAP( maxLenInChars ) char* pDest, int maxLenInChars ) override;
private:
	struct SearchPath {
		SearchPath() = default;
		~SearchPath() {
			Warning( "~SearchPath called\n" );
		}
		SearchPath( const SearchPath& other ) {// copy-constructor-but-actually-move
			m_Drivers = other.m_Drivers;
			m_ClientIDs = other.m_ClientIDs;
			m_RequestOnly = other.m_RequestOnly;
		}

		CUtlVector<CFsDriver*> m_Drivers{};
		CUtlVector<int> m_ClientIDs{};
		bool m_RequestOnly{ false };
	};
	struct FindState {
		FindState() = default;
		~FindState() {
			Warning( "~FindState called\n" );
		}
		FindState( const FindState& other ) {  // copy-constructor-but-actually-move
			m_Paths = other.m_Paths;
			m_Current = other.m_Current;
		}

		CUtlVector<const char*> m_Paths{};
		int m_Current{0};
	};

	// Whether we were initialized
	bool m_Initialized{ false };
	// The last-used driver ID, 0 is reserved for the root
	int m_LastId{ 1 };
	// The named search paths
	CUtlDict<SearchPath*> m_SearchPaths{};
	// All open descriptors
	CUtlVector<FileDescriptor*> m_Descriptors{ 10 };
	// Open `FindFile*` states
	CUtlVector<FindState> m_FindStates{ 10 };
	// The logging functions which were registered
	CUtlVector<FileSystemLoggingFunc_t> m_LoggingFuncs{ 5 };
	// Used to display dirty disk error
	FSDirtyDiskReportFunc_t m_DirtyDiskReporter{ nullptr };
	// Filesystem stats, mostly read/write related
	FileSystemStatistics m_Stats{};
	// The log level for the `m_Warning` output
	FileWarningLevel_t m_WarningLevel{ FileWarningLevel_t::FILESYSTEM_WARNING_QUIET };
	// Warnings output
	FileWarningFunc_t m_Warning{ nullptr };
	// Filename dictionary
	CUtlFilenameSymbolTable m_Filenames{};
};
