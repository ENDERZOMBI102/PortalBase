//
// Created by ENDERZOMBI102 on 03/12/2024.
//
#pragma once
#include "interface.h"
#include "ishadersystem.h"
#include "shaderlib/ShaderDLL.h"
#include "utlvector.h"


class CShaderDLL : public IShaderDLLInternal, public IShaderDLL {
public:
	CShaderDLL();

public:  // IShaderDLL
	virtual bool Connect( CreateInterfaceFn pFactory );
	virtual void Disconnect();
	virtual int ShaderCount() const;
	virtual IShader* GetShader( int pShader );

public:  // IShaderDLLInternal
	virtual bool Connect( CreateInterfaceFn pFactory, bool pIsMaterialSystem );
	virtual void Disconnect( bool pIsMaterialSystem );
	virtual void InsertShader( IShader* pShader );

private:
	CUtlVector<IShader*> m_ShaderList;
};


// Singleton interface
IShaderDLL* GetShaderDLL();
