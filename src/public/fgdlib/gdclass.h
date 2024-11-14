//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Defines the interface to a game class as defined by the game data
//			file (FGD). Each class is type of entity that can be placed in
//			the editor and has attributes such as: keys that may be set,
//			the default size and color of the entity, inputs and outputs,
//			and any default models that are created when the entity is placed.
//
//			The game classes support multiple inheritance through aggregation
//			of properties.
//
//=============================================================================//
#pragma once
#include "gdvar.h"
#include "helperinfo.h"
#include "inputoutput.h"
#include "mathlib/vector.h"
#include "tokenreader.h"

class CHelperInfo;
class GameData;
class GDinputvariable;

constexpr int GD_MAX_VARIABLES = 128;


class GDclass {
public:
	GDclass();
	~GDclass();

	//
	// Interface to class information:
	//
	[[nodiscard]]
	inline const char* GetName() const { return m_szName; }
	[[nodiscard]]
	inline const char* GetDescription() const;

	//
	// Reading a class from the game data file:
	//
	BOOL InitFromTokens( TokenReader& tr, GameData* );

	//
	// Interface to variable information (keys):
	//
	[[nodiscard]]
	inline int GetVariableCount() const { return m_nVariables; }
	GDinputvariable* GetVariableAt( int iIndex );
	void GetHelperForGDVar( const GDinputvariable* pVar, CUtlVector<const char*>* pHelperName );
	GDinputvariable* VarForName( const char* pName, int* piIndex = nullptr );
	BOOL AddVariable( GDinputvariable* pVar, GDclass* pBase, int pBaseIndex, int pVarIndex );
	void AddBase( GDclass* pBase );

	//
	// Interface to input information:
	//
	inline void AddInput( CClassInput* pInput );
	CClassInput* FindInput( const char* szName );
	[[nodiscard]]
	inline int GetInputCount() const { return m_Inputs.Count(); }
	CClassInput* GetInput( int pIndex );

	//
	// Interface to output information:
	//
	inline void AddOutput( CClassOutput* pOutput );
	CClassOutput* FindOutput( const char* szName );
	[[nodiscard]]
	inline int GetOutputCount() const { return m_Outputs.Count(); }
	CClassOutput* GetOutput( int pIndex );

	GameData* Parent{ nullptr };

	//
	// Interface to class attributes:
	//
	inline bool IsClass( const char* pszClass ) const;
	[[nodiscard]]
	inline bool IsSolidClass() const { return m_bSolid; }
	[[nodiscard]]
	inline bool IsBaseClass() const { return m_bBase; }
	[[nodiscard]]
	inline bool IsMoveClass() const { return m_bMove; }
	[[nodiscard]]
	inline bool IsKeyFrameClass() const { return m_bKeyFrame; }
	[[nodiscard]]
	inline bool IsPointClass() const { return m_bPoint; }
	[[nodiscard]]
	inline bool IsNPCClass() const { return m_bNPC; }
	[[nodiscard]]
	inline bool IsFilterClass() const { return m_bFilter; }
	[[nodiscard]]
	inline bool IsNodeClass() const;
	static inline bool IsNodeClass( const char* pszClassName );

	[[nodiscard]]
	inline bool ShouldSnapToHalfGrid() const { return m_bHalfGridSnap; }

	inline void SetNPCClass( const bool bNPC ) { m_bNPC = bNPC; }
	inline void SetFilterClass( const bool bFilter ) { m_bFilter = bFilter; }
	inline void SetPointClass( const bool bPoint ) { m_bPoint = bPoint; }
	inline void SetSolidClass( const bool bSolid ) { m_bSolid = bSolid; }
	inline void SetBaseClass( const bool bBase ) { m_bBase = bBase; }
	inline void SetMoveClass( const bool bMove ) { m_bMove = bMove; }
	inline void SetKeyFrameClass( const bool bKeyFrame ) { m_bKeyFrame = bKeyFrame; }

	inline const Vector& GetMins() { return m_bmins; }
	inline const Vector& GetMaxs() { return m_bmaxs; }

	BOOL GetBoundBox( Vector& pMins, Vector& pMaxs );
	[[nodiscard]]
	bool HasBoundBox() const { return m_bGotSize; }

	[[nodiscard]]
	inline color32 GetColor() const;

	//
	// Interface to helper information:
	//
	inline void AddHelper( CHelperInfo* pHelper );
	[[nodiscard]]
	inline int GetHelperCount() const { return m_Helpers.Count(); }
	CHelperInfo* GetHelper( int pIndex );

protected:
	//
	// Parsing the game data file:
	//
	bool ParseInput( TokenReader& tr );
	bool ParseInputOutput( TokenReader& tr, CClassInputOutputBase* pInputOutput );
	bool ParseOutput( TokenReader& tr );
	bool ParseVariable( TokenReader& tr );

private:
	bool ParseBase( TokenReader& tr );
	bool ParseColor( TokenReader& tr );
	bool ParseHelper( TokenReader& tr, const char* pHelperName );
	bool ParseSize( TokenReader& tr );
	bool ParseSpecifiers( TokenReader& tr );
	bool ParseVariables( TokenReader& tr );

	color32 m_rgbColor{ .r = 220, .g = 30, .b = 220, .a = 0 };    // Color of entity.

	bool m_bBase{ false };          // Base only - not available to user.
	bool m_bSolid{ false };         // Tied to solids only.
	bool m_bModel{ false };         // Properties of a single model.
	bool m_bMove{ false };          // Animatable
	bool m_bKeyFrame{ false };      // Animation keyframe
	bool m_bPoint{ false };         // Point class, not tied to solids.
	bool m_bNPC{ false };           // NPC class - used for populating lists of NPC classes.
	bool m_bFilter{ false };        // filter class - used for populating lists of filters.
	bool m_bHalfGridSnap{ false };  // Snaps to a 1/2 grid so it can be centered on any geometry. Used for hinges, etc.

	bool m_bGotSize{ false };       // Just for loading.
	bool m_bGotColor{ false };

	char m_szName[ MAX_IDENT ] { };     // Name of this class.
	char* m_pszDescription{ nullptr };  // Description of this class, dynamically allocated.

	CUtlVector<GDinputvariable*> m_Variables;  // Variables for this class.
	int m_nVariables{ 0 };                          // Count of base & local variables combined.
	CUtlVector<GDclass*> m_Bases;              // List of base classes this class inherits from.

	CClassInputList m_Inputs;
	CClassOutputList m_Outputs;

	CHelperInfoList m_Helpers;  // Helpers for this class.

	//
	//	[0] = base number from Bases, or -1 if not in a base.
	//	[1] = index into base's variables
	//
	int16 m_VariableMap[GD_MAX_VARIABLES][2] {};

	Vector m_bmins;  // 3D minima of object (pointclass).
	Vector m_bmaxs;  // 3D maxima of object (pointclass).
};


void GDclass::AddInput( CClassInput* pInput ) {
	Assert( pInput != nullptr );
	if ( pInput != nullptr ) {
		m_Inputs.AddToTail( pInput );
	}
}


inline void GDclass::AddOutput( CClassOutput* pOutput ) {
	Assert( pOutput != nullptr );
	if ( pOutput != nullptr ) {
		m_Outputs.AddToTail( pOutput );
	}
}


inline void GDclass::AddHelper( CHelperInfo* pHelper ) {
	Assert( pHelper != nullptr );
	if ( pHelper != nullptr ) {
		m_Helpers.AddToTail( pHelper );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Returns the render color of this entity class.
//-----------------------------------------------------------------------------
color32 GDclass::GetColor() const {
	return m_rgbColor;
}


//-----------------------------------------------------------------------------
// Purpose: Returns a description of this entity class, or the entity class name
//			if no description exists.
//-----------------------------------------------------------------------------
const char* GDclass::GetDescription() const {
	if ( m_pszDescription == nullptr ) {
		return ( m_szName );
	}

	return ( m_pszDescription );
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : pszClass -
//-----------------------------------------------------------------------------
bool GDclass::IsClass( const char* pszClass ) const {
	Assert( pszClass != nullptr );
	return not stricmp( pszClass, m_szName );
}


//-----------------------------------------------------------------------------
// Purpose: Returns true if the given classname represents an AI node class, false if not.
//-----------------------------------------------------------------------------
bool GDclass::IsNodeClass( const char* pszClassName ) {
	return strnicmp( pszClassName, "info_node", 9 ) == 0 and stricmp( pszClassName, "info_node_link" ) != 0;
}


//-----------------------------------------------------------------------------
// Purpose: Returns true if this is an AI node class, false if not.
//
// HACK: if this is necessary, we should have a new @NodeClass FGD specifier (or something)
//-----------------------------------------------------------------------------
bool GDclass::IsNodeClass() const {
	return IsNodeClass( m_szName );
}
