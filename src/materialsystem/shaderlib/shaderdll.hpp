//
// Created by ENDERZOMBI102 on 03/12/2024.
// 
#pragma once
#include "interface.h"
#include "ishaderdllinternal.hpp"
#include "shaderlib/ShaderDLL.h"
#include "utlvector.h"


//-----------------------------------------------------------------------------
// The standard implementation of CShaderDLL
//-----------------------------------------------------------------------------
class CShaderDLL : public IShaderDLLInternal, public IShaderDLL {
public:
	CShaderDLL();
public:  // IShaderDLLInternal
	bool Connect( CreateInterfaceFn pFactory ) override;
	void Disconnect() override;
	// Iterates over all shaders
	int ShaderCount() const override;
	IShader* GetShader( int pShader ) override;
public:  // IShaderDLL
	// Adds a shader to the list of shaders
	void InsertShader( IShader* pShader ) override;
private:
	CUtlVector<IShader*> m_ShaderList{};
};

// Singleton interface
IShaderDLL* GetShaderDLL();

// Singleton interface for CVars
ICvar* GetCVar();


