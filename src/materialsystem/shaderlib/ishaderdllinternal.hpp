//
// Created by ENDERZOMBI102 on 03/12/2024.
// 
#pragma once

class IShader;


class IShaderDLLInternal {
	virtual bool Connect( CreateInterfaceFn factory ) = 0;
	virtual void Disconnect() = 0;
	virtual int ShaderCount() const = 0;
	virtual IShader* GetShader( int nShader ) = 0;
};
