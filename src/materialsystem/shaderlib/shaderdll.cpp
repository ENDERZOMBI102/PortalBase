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
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


IMaterialSystemHardwareConfig* g_pHardwareConfig;
const MaterialSystem_Config_t* g_pConfig;
namespace { CShaderDLL* s_pShaderDLL; }


CShaderDLL::CShaderDLL() {
	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f );
}

bool CShaderDLL::Connect( CreateInterfaceFn pFactory ) {
	g_pHardwareConfig = static_cast<IMaterialSystemHardwareConfig*>( pFactory( MATERIALSYSTEM_HARDWARECONFIG_INTERFACE_VERSION, NULL ));
	g_pConfig = static_cast<const MaterialSystem_Config_t*>( pFactory( MATERIALSYSTEM_CONFIG_VERSION, NULL ));
	InitShaderLibCVars( pFactory );

	return g_pConfig != nullptr and g_pHardwareConfig != nullptr;
}

void CShaderDLL::Disconnect() {
	g_pHardwareConfig = nullptr;
	g_pConfig = nullptr;
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



