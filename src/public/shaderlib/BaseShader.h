//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: This is what all shaders inherit from.
//
// $NoKeywords: $
//
//===========================================================================//
#pragma once
#include "materialsystem/IShader.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/ishaderapi.h"
#include "shaderlib/BaseShader.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class IMaterialVar;

//-----------------------------------------------------------------------------
// Standard material vars
//-----------------------------------------------------------------------------
// Note: if you add to these, add to s_StandardParams in CBaseShader.cpp
enum ShaderMaterialVars_t {
	FLAGS = 0,
	FLAGS_DEFINED,  // mask indicating if the flag was specified
	FLAGS2,
	FLAGS_DEFINED2,
	COLOR,
	ALPHA,
	BASETEXTURE,
	FRAME,
	BASETEXTURETRANSFORM,
	FLASHLIGHTTEXTURE,
	FLASHLIGHTTEXTUREFRAME,
	COLOR2,
	SRGBTINT,

	NUM_SHADER_MATERIAL_VARS
};


// Alpha belnd mode enums. Moved from basevsshader
enum BlendType_t {
	// no alpha blending
	BT_NONE = 0,


	// src * srcAlpha + dst * (1-srcAlpha)
	// two passes for HDR:
	//		pass 1:
	//			color: src * srcAlpha + dst * (1-srcAlpha)
	//			alpha: srcAlpha * zero + dstAlpha * (1-srcAlpha)
	//		pass 2:
	//			color: none
	//			alpha: srcAlpha * one + dstAlpha * one
	//
	BT_BLEND,


	// src * one + dst * one
	// one pass for HDR
	BT_ADD,


	// Why do we ever use this instead of using premultiplied alpha?
	// src * srcAlpha + dst * one
	// two passes for HDR
	//		pass 1:
	//			color: src * srcAlpha + dst * one
	//			alpha: srcAlpha * one + dstAlpha * one
	//		pass 2:
	//			color: none
	//			alpha: srcAlpha * one + dstAlpha * one
	BT_BLENDADD
};


//-----------------------------------------------------------------------------
// Base class for shaders, contains helper methods.
//-----------------------------------------------------------------------------
class CBaseShader : public IShader {
public:
	// constructor
	CBaseShader();

	// Methods inherited from IShader
	virtual char const* GetFallbackShader( IMaterialVar** params ) const { return 0; }
	virtual int GetNumParams() const;
	virtual char const* GetParamName( int pParamIndex ) const;
	virtual char const* GetParamHelp( int pParamIndex ) const;
	virtual ShaderParamType_t GetParamType( int pParamIndex ) const;
	virtual char const* GetParamDefault( int pParamIndex ) const;
	virtual int GetParamFlags( int pParamIndex ) const;

	virtual void InitShaderParams( IMaterialVar** pParams, const char* pMaterialName );
	virtual void InitShaderInstance( IMaterialVar** pParams, IShaderInit* pShaderInit, const char* pMaterialName, const char* pTextureGroupName );
	virtual void DrawElements( IMaterialVar** pParams, int pModulationFlags, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI,
							   VertexCompressionType_t pVertexCompression, CBasePerMaterialContextData** pContext );

	virtual const SoftwareVertexShader_t GetSoftwareVertexShader() const { return m_SoftwareVertexShader; }

	virtual int ComputeModulationFlags( IMaterialVar** params, IShaderDynamicAPI* pShaderAPI );
	virtual bool NeedsPowerOfTwoFrameBufferTexture( IMaterialVar** params, bool pCheckSpecificToThisFrame = true ) const;
	virtual bool NeedsFullFrameBufferTexture( IMaterialVar** params, bool pCheckSpecificToThisFrame = true ) const;
	virtual bool IsTranslucent( IMaterialVar** params ) const;

public:
	// These functions must be implemented by the shader
	virtual void OnInitShaderParams( IMaterialVar** ppParams, const char* pMaterialName ) {}
	virtual void OnInitShaderInstance( IMaterialVar** ppParams, IShaderInit* pShaderInit, const char* pMaterialName ) = 0;
	virtual void OnDrawElements( IMaterialVar** params, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData** pContextDataPtr ) = 0;

	// Sets the default shadow state
	void SetInitialShadowState();

	// Draws a snapshot
	void Draw( bool pMakeActualDrawCall = true );

	// Are we currently taking a snapshot?
	bool IsSnapshotting() const;

	// Gets at the current materialvar flags
	int CurrentMaterialVarFlags() const;

	// Finds a particular parameter	(works because the lowest parameters match the shader)
	int FindParamIndex( const char* pName ) const;

	// Are we using graphics?
	bool IsUsingGraphics();

	// Are we using editor materials?
	bool CanUseEditorMaterials();

	// Gets the builder...
	CMeshBuilder* MeshBuilder();

	// Loads a texture
	void LoadTexture( int pTextureVar, int pAdditionalCreationFlags = 0 );

	// Loads a bumpmap
	void LoadBumpMap( int pTextureVar );

	// Loads a cubemap
	void LoadCubeMap( int pTextureVar, int pAdditionalCreationFlags = 0 );

	// get the shaderapi handle for a texture. BE CAREFUL WITH THIS.
	ShaderAPITextureHandle_t GetShaderAPITextureBindHandle( int pTextureVar, int pFrameVar, int pTextureChannel = 0 );


	// Binds a texture
	void BindTexture( Sampler_t pSampler1, Sampler_t pSampler2, int pTextureVar, int pFrameVar = -1 );
	void BindTexture( Sampler_t pSampler1, int pTextureVar, int pFrameVar = -1 );
	void BindTexture( Sampler_t pSampler1, ITexture* pTexture, int pFrame = 0 );
	void BindTexture( Sampler_t pSampler1, Sampler_t pSampler2, ITexture* pTexture, int pFrame = 0 );

	void GetTextureDimensions( float* pOutWidth, float* pOutHeight, int nTextureVar );

	// Is the texture translucent?
	bool TextureIsTranslucent( int pTextureVar, bool pIsBaseTexture );

	// Returns the translucency...
	float GetAlpha( IMaterialVar** params = nullptr );

	// Is the color var white?
	bool IsWhite( int colorVar );

	// Helper methods for fog
	void FogToOOOverbright();
	void FogToWhite();
	void FogToBlack();
	void FogToGrey();
	void FogToFogColor();
	void DisableFog();
	void DefaultFog();

	// Helpers for alpha blending
	void EnableAlphaBlending( ShaderBlendFactor_t src, ShaderBlendFactor_t dst );
	void DisableAlphaBlending();

	void SetBlendingShadowState( BlendType_t nMode );

	void SetNormalBlendingShadowState( int textureVar = -1, bool isBaseTexture = true );
	void SetAdditiveBlendingShadowState( int textureVar = -1, bool isBaseTexture = true );
	void SetDefaultBlendingShadowState( int textureVar = -1, bool isBaseTexture = true );
	void SingleTextureLightmapBlendMode();

	// Helpers for color modulation
	void SetColorState( int colorVar, bool setAlpha = false );
	bool IsAlphaModulating();
	bool IsColorModulating();
	void ComputeModulationColor( float* color );
	void SetModulationShadowState( int pTintVar = -1 );
	void SetModulationDynamicState( int pTintVar = -1 );

	// Helpers for HDR
	bool IsHDREnabled();

	// Loads the identity matrix into the texture
	void LoadIdentity( MaterialMatrixMode_t matrixMode );

	// Loads the camera to world transform
	void LoadCameraToWorldTransform( MaterialMatrixMode_t matrixMode );
	void LoadCameraSpaceSphereMapTransform( MaterialMatrixMode_t matrixMode );

	// Sets a texture translation transform in fixed function
	void SetFixedFunctionTextureTranslation( MaterialMatrixMode_t mode, int translationVar );
	void SetFixedFunctionTextureScale( MaterialMatrixMode_t mode, int scaleVar );
	void SetFixedFunctionTextureScaledTransform( MaterialMatrixMode_t textureTransform, int transformVar, int scaleVar );
	void SetFixedFunctionTextureTransform( MaterialMatrixMode_t textureTransform, int transformVar );

	void CleanupDynamicStateFixedFunction();

	// Fixed function Base * detail pass
	void FixedFunctionBaseTimesDetailPass( int baseTextureVar, int frameVar,
										   int baseTextureTransformVar, int detailVar, int detailScaleVar );

	// Fixed function Self illumination pass
	void FixedFunctionSelfIlluminationPass( Sampler_t sampler,
											int baseTextureVar, int frameVar, int baseTextureTransformVar, int selfIllumTintVar );

	// Masked environment map
	void FixedFunctionMaskedEnvmapPass( int envMapVar, int envMapMaskVar,
										int baseTextureVar, int envMapFrameVar, int envMapMaskFrameVar,
										int frameVar, int maskOffsetVar, int maskScaleVar, int tintVar = -1 );

	// Additive masked environment map
	void FixedFunctionAdditiveMaskedEnvmapPass( int pEnvMapVar, int pEnvMapMaskVar,
												int pBaseTextureVar, int pEnvMapFrameVar, int pEnvMapMaskFrameVar,
												int pFrameVar, int pMaskOffsetVar, int pMaskScaleVar, int pEnvMapTintVar = -1 );

	// Modulate by detail texture pass
	void FixedFunctionMultiplyByDetailPass( int baseTextureVar, int frameVar,
											int textureOffsetVar, int detailVar, int detailScaleVar );

	// Multiply by lightmap pass
	void FixedFunctionMultiplyByLightmapPass( int baseTextureVar, int frameVar,
											  int baseTextureTransformVar, float alphaOverride = -1 );

	// Helper methods for environment mapping
	int SetShadowEnvMappingState( int envMapMaskVar, int tintVar = -1 );
	void SetDynamicEnvMappingState( int envMapVar, int envMapMaskVar,
									int baseTextureVar, int envMapFrameVar, int envMapMaskFrameVar,
									int frameVar, int maskOffsetVar, int maskScaleVar, int tintVar = -1 );

	bool UsingFlashlight( IMaterialVar** params ) const;
	bool UsingEditor( IMaterialVar** params ) const;

	void DrawFlashlight_dx70( IMaterialVar** params, IShaderDynamicAPI* pShaderAPI,
							  IShaderShadow* pShaderShadow,
							  int pFlashlightTextureVar, int pFlashlightTextureFrameVar,
							  bool pSuppressLighting = false );

	void SetFlashlightFixedFunctionTextureTransform( MaterialMatrixMode_t pMatrix );

	void GetColorParameter( IMaterialVar** params, float* pColorOut ) const;  // return tint color (color*color2)
	void ApplyColor2Factor( float* pColorOut ) const;                         // (*pColorOut) *= COLOR2

	static IMaterialVar** s_ppParams;

protected:
	SoftwareVertexShader_t m_SoftwareVertexShader;

	static const char* s_pTextureGroupName;  // Current material's texture group name.
	static IShaderShadow* s_pShaderShadow;
	static IShaderDynamicAPI* s_pShaderAPI;
	static IShaderInit* s_pShaderInit;

private:
	static int s_nModulationFlags;
	static CMeshBuilder* s_pMeshBuilder;
};


//-----------------------------------------------------------------------------
// Gets at the current materialvar flags
//-----------------------------------------------------------------------------
inline int CBaseShader::CurrentMaterialVarFlags() const {
	return s_ppParams[FLAGS]->GetIntValue();
}

//-----------------------------------------------------------------------------
// Are we currently taking a snapshot?
//-----------------------------------------------------------------------------
inline bool CBaseShader::IsSnapshotting() const {
	return s_pShaderShadow != nullptr;
}

//-----------------------------------------------------------------------------
// Is the color var white?
//-----------------------------------------------------------------------------
inline bool CBaseShader::IsWhite( int colorVar ) {
	if ( colorVar < 0 ) {
		return true;
	}

	if ( not s_ppParams[colorVar]->IsDefined() ) {
		return true;
	}

	float color[3];
	s_ppParams[colorVar]->GetVecValue( color, 3 );
	return color[0] >= 1.0f and color[1] >= 1.0f and color[2] >= 1.0f;
}

/**
 * Shaders can keep per material data in classes descended from this
 */
class CBasePerMaterialContextData {
public:
	CBasePerMaterialContextData() = default;

	// virtual destructor so that derived classes can have their own data to be cleaned up on
	// delete of material
	virtual ~CBasePerMaterialContextData() = default;

public:
	uint32 m_nVarChangeID{ 0xffffffff };
	/**
	 * Set by mat system when material vars change. Shader should rethink and then clear the var.
	 */
	bool m_bMaterialVarsChanged{ true };
};
