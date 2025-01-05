//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $Header: $
// $NoKeywords: $
//=============================================================================//
#include "shaderdll.hpp"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/materialsystem_config.h"
#include "mathlib/mathlib.h"
#include "shaderlib_cvar.h"
#include "tier0/dbg.h"
#include "tier1/tier1.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


IMaterialSystemHardwareConfig* g_pHardwareConfig{ nullptr };
const MaterialSystem_Config_t* g_pConfig{ nullptr };
bool g_shaderConfigDumpEnable{ false };
// local to shaderlib
IShaderSystem* g_pSLShaderSystem{nullptr};
namespace { CShaderDLL* s_pShaderDLL{ nullptr }; }


CShaderDLL::CShaderDLL() {
	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f );
}

//-----------------------------------------------------------------------------
// Connect, disconnect...
//-----------------------------------------------------------------------------
bool CShaderDLL::Connect( const CreateInterfaceFn pFactory, const bool pIsMaterialSystem ) {
	g_pHardwareConfig = static_cast<IMaterialSystemHardwareConfig*>( pFactory( MATERIALSYSTEM_HARDWARECONFIG_INTERFACE_VERSION, nullptr ) );
	g_pConfig = static_cast<const MaterialSystem_Config_t*>( pFactory( MATERIALSYSTEM_CONFIG_VERSION, nullptr ) );
	g_pSLShaderSystem = static_cast<IShaderSystem*>( pFactory( SHADERSYSTEM_INTERFACE_VERSION, nullptr ) );

	if ( not pIsMaterialSystem ) {
		ConnectTier1Libraries( &pFactory, 1 );
		InitShaderLibCVars( pFactory );
	}

	return g_pConfig != nullptr and g_pHardwareConfig != nullptr and g_pSLShaderSystem != nullptr;
}

void CShaderDLL::Disconnect( const bool pIsMaterialSystem ) {
	if ( not pIsMaterialSystem ) {
		ConVar_Unregister();
		DisconnectTier1Libraries();
	}

	g_pHardwareConfig = nullptr;
	g_pConfig = nullptr;
	g_pSLShaderSystem = nullptr;
}

bool CShaderDLL::Connect( const CreateInterfaceFn pFactory ) {
	return Connect( pFactory, false );
}
void CShaderDLL::Disconnect() {
	Disconnect( false );
}

int CShaderDLL::ShaderCount() const {
	return m_ShaderList.Count();
}

IShader* CShaderDLL::GetShader( const int pShader ) {
	if ( pShader < 0 or pShader >= m_ShaderList.Count() ) {
		return nullptr;
	}

	return m_ShaderList[pShader];
}


void CShaderDLL::InsertShader( IShader* pShader ) {
	Assert( pShader );
	m_ShaderList.AddToTail( pShader );
}

//-----------------------------------------------------------------------------
// Global accessor
//-----------------------------------------------------------------------------
IShaderDLL* GetShaderDLL() {
	// Pattern necessary because shaders register themselves in global constructors
	if ( not s_pShaderDLL ) {
		s_pShaderDLL = new CShaderDLL{};
	}

	return s_pShaderDLL;
}

IShaderDLLInternal* GetShaderDLLInternal() {
	// Pattern necessary because shaders register themselves in global constructors
	if ( not s_pShaderDLL ) {
		s_pShaderDLL = new CShaderDLL{};
	}

	return s_pShaderDLL;
}

//-----------------------------------------------------------------------------
// Singleton interface
//-----------------------------------------------------------------------------
EXPOSE_INTERFACE_FN( GetShaderDLLInternal, IShaderDLLInternal, SHADER_DLL_INTERFACE_VERSION );