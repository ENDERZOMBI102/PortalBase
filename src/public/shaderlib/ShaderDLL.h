//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $Header: $
// $NoKeywords: $
//=============================================================================//
#pragma once

class IShader;

//-----------------------------------------------------------------------------
// The standard implementation of CShaderDLL
//-----------------------------------------------------------------------------
class IShaderDLL {
public:
	// Adds a shader to the list of shaders
	virtual void InsertShader( IShader* pShader ) = 0;
};


//-----------------------------------------------------------------------------
// Singleton interface
//-----------------------------------------------------------------------------
IShaderDLL* GetShaderDLL();
