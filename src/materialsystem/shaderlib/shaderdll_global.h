//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $Header: $
// $NoKeywords: $
//=============================================================================//
#pragma once

class IShaderSystem;


inline IShaderSystem* GetShaderSystem() {
	extern IShaderSystem* g_pSLShaderSystem;
	return g_pSLShaderSystem;
}
