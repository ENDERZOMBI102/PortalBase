//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//
#pragma once
#if defined( COMPILER_MSVC )
	#pragma warning( push, 1 )
	#pragma warning( disable : 4701 4702 4530 )
#endif
#include <fstream>
#if defined( COMPILER_MSVC )
	#pragma warning( pop )
#endif
#include "gdclass.h"
#include "tokenreader.h"
#include "utlstring.h"
#include "utlvector.h"


class MDkeyvalue;
class GameData;
class KeyValues;


using GameDataMessageFunc_t = void (*)( int level, PRINTF_FORMAT_STRING const char* fmt, ... );

// FGD-based AutoMaterialExclusion data

struct FGDMatExlcusions_s {
	char szDirectory[MAX_PATH];  // Where we store the material exclusion directories
	bool bUserGenerated;         // If the user specified this ( default:  false -- FGD defined )
};

// FGD-based AutoVisGroup data

struct FGDVisGroupsBaseClass_s {
	char szClass[MAX_PATH];     // i.e. Scene Logic, Sounds, etc   "Custom\Point Entities\Lights"
	CUtlStringList szEntities;  // i.e. func_viscluster
};

struct FGDAutoVisGroups_s {
	char szParent[MAX_PATH];                        // i.e. Custom, SFM, etc
	CUtlVector<FGDVisGroupsBaseClass_s> m_Classes;  // i.e. Scene Logic, Sounds, etc
};

#define MAX_DIRECTORY_SIZE 32


//-----------------------------------------------------------------------------
// Purpose: Contains the set of data that is loaded from a single FGD file.
//-----------------------------------------------------------------------------
class GameData {
public:
	enum TNameFixup {
		NAME_FIXUP_PREFIX = 0,
		NAME_FIXUP_POSTFIX,
		NAME_FIXUP_NONE
	};

	GameData();
	~GameData();

	bool Load( const char* pFilename );

	GDclass* ClassForName( const char* pszName, int* piIndex = nullptr );

	void ClearData();

	[[nodiscard]]
	inline int GetMaxMapCoord() const;
	[[nodiscard]]
	inline int GetMinMapCoord() const;

	[[nodiscard]]
	inline int GetClassCount() const;
	inline GDclass* GetClass( int pIndex );

	GDclass* BeginInstanceRemap( const char* pszClassName, const char* pszInstancePrefix, const Vector& Origin, const QAngle& Angle );
	bool RemapKeyValue( const char* pszKey, const char* pszInValue, char* pszOutValue, TNameFixup NameFixup );
	bool RemapNameField( const char* pszInValue, char* pszOutValue, TNameFixup NameFixup );
	bool LoadFGDMaterialExclusions( TokenReader& tr );
	bool LoadFGDAutoVisGroups( TokenReader& tr );


	CUtlVector<FGDMatExlcusions_s> m_FGDMaterialExclusions;

	CUtlVector<FGDAutoVisGroups_s> m_FGDAutoVisGroups;

private:
	bool ParseMapSize( TokenReader& tr );

	CUtlVector<GDclass*> m_Classes;

	int m_nMinMapCoord{ -8192 };  // Min & max map bounds as defined by the FGD.
	int m_nMaxMapCoord{ 8192 };

	// Instance Remapping
	Vector m_InstanceOrigin;              // the origin offset of the instance
	QAngle m_InstanceAngle;               // the rotation of the instance
	matrix3x4_t m_InstanceMat;            // matrix of the origin and rotation of rendering
	char m_InstancePrefix[128] { };       // the prefix used for the instance name remapping
	GDclass* m_InstanceClass{ nullptr };  // the entity class that is being remapped
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline int GameData::GetClassCount() const {
	return m_Classes.Count();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline GDclass* GameData::GetClass( const int pIndex ) {
	if ( pIndex >= m_Classes.Count() ) {
		return nullptr;
	}

	return m_Classes.Element( pIndex );
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int GameData::GetMinMapCoord() const {
	return m_nMinMapCoord;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int GameData::GetMaxMapCoord() const {
	return m_nMaxMapCoord;
}


void GDSetMessageFunc( GameDataMessageFunc_t pFunc );
bool GDError( TokenReader& tr, PRINTF_FORMAT_STRING const char* error, ... );
bool GDSkipToken( TokenReader& tr, trtoken_t ttexpecting = TOKENNONE, const char* pszExpecting = nullptr );
bool GDGetToken( TokenReader& tr, char* pszStore, int nSize, trtoken_t ttexpecting = TOKENNONE, const char* pszExpecting = nullptr );
bool GDGetTokenDynamic( TokenReader& tr, char** pStore, trtoken_t ttexpecting, const char* pExpecting = nullptr );
