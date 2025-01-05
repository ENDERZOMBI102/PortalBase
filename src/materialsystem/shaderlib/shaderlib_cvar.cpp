//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//
#include "shaderlib_cvar.h"
#include "convar.h"
#include "icvar.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// ------------------------------------------------------------------------------------------- //
// ConVar stuff.
// ------------------------------------------------------------------------------------------- //
class CShaderLibConVarAccessor : public IConCommandBaseAccessor {
public:
	bool RegisterConCommandBase( ConCommandBase* pCommand ) override {
		// Link to engine's list instead
		g_pCVar->RegisterConCommand( pCommand );

		const char* pValue = g_pCVar->GetCommandLineValue( pCommand->GetName() );
		if ( pValue and not pCommand->IsCommand() ) {
			static_cast<ConVar*>( pCommand)->SetValue( pValue );
		}
		return true;
	}
};

CShaderLibConVarAccessor g_ConVarAccessor;


void InitShaderLibCVars( CreateInterfaceFn pCvarFactory ) {
	if ( g_pCVar ) {
		ConVar_Register( 0, &g_ConVarAccessor );
	}
}

