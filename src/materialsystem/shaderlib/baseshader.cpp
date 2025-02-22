//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $Header: $
// $NoKeywords: $
//=============================================================================//
#include "shaderlib/BaseShader.h"
#include "convar.h"
#include "ishadersystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/ishaderapi.h"
#include "materialsystem/itexture.h"
#include "materialsystem/materialsystem_config.h"
#include "mathlib/vmatrix.h"
#include "shaderdll_global.h"
#include "shaderlib/ShaderDLL.h"
#include "shaderlib/cshader.h"
#include "tier0/dbg.h"
#include "tier0/vprof.h"
#include "tier1/strtools.h"
// NOTE: This must be the last include file in a .cpp file!
#include "tier0/memdbgon.h"


//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
const char* CBaseShader::s_pTextureGroupName = nullptr;
IMaterialVar** CBaseShader::s_ppParams;
IShaderShadow* CBaseShader::s_pShaderShadow;
IShaderDynamicAPI* CBaseShader::s_pShaderAPI;
IShaderInit* CBaseShader::s_pShaderInit;
int CBaseShader::s_nModulationFlags;
CMeshBuilder* CBaseShader::s_pMeshBuilder;
static ConVar mat_fullbright( "mat_fullbright", "0", FCVAR_CHEAT );

//-----------------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------------
CBaseShader::CBaseShader() {
	GetShaderDLL()->InsertShader( this );
}


//-----------------------------------------------------------------------------
// Shader parameter info
//-----------------------------------------------------------------------------
// Look in BaseShader.h for the enumeration for these.
// Update there if you update here.
static ShaderParamInfo_t s_StandardParams[NUM_SHADER_MATERIAL_VARS] {
	{ "$flags", "flags", SHADER_PARAM_TYPE_INTEGER, "0", SHADER_PARAM_NOT_EDITABLE },
	{ "$flags_defined", "flags_defined", SHADER_PARAM_TYPE_INTEGER, "0", SHADER_PARAM_NOT_EDITABLE },
	{ "$flags2", "flags2", SHADER_PARAM_TYPE_INTEGER, "0", SHADER_PARAM_NOT_EDITABLE },
	{ "$flags_defined2", "flags2_defined", SHADER_PARAM_TYPE_INTEGER, "0", SHADER_PARAM_NOT_EDITABLE },
	{ "$color", "color", SHADER_PARAM_TYPE_COLOR, "[1 1 1]", 0 },
	{ "$alpha", "alpha", SHADER_PARAM_TYPE_FLOAT, "1.0", 0 },
	{ "$basetexture", "Base Texture with lighting built in", SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", 0 },
	{ "$frame", "Animation Frame", SHADER_PARAM_TYPE_INTEGER, "0", 0 },
	{ "$basetexturetransform", "Base Texture Texcoord Transform", SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", 0 },
	{ "$flashlighttexture", "flashlight spotlight shape texture", SHADER_PARAM_TYPE_TEXTURE, "effects/flashlight001", SHADER_PARAM_NOT_EDITABLE },
	{ "$flashlighttextureframe", "Animation Frame for $flashlight", SHADER_PARAM_TYPE_INTEGER, "0", SHADER_PARAM_NOT_EDITABLE },
	{ "$color2", "color2", SHADER_PARAM_TYPE_COLOR, "[1 1 1]", 0 },
	{ "$srgbtint", "tint value to be applied when running on new-style srgb parts", SHADER_PARAM_TYPE_COLOR, "[1 1 1]", 0 },
};


//-----------------------------------------------------------------------------
// Gets the standard shader parameter names
// FIXME: Turn this into one function?
//-----------------------------------------------------------------------------
int CBaseShader::GetNumParams() const {
	return NUM_SHADER_MATERIAL_VARS;
}

char const* CBaseShader::GetParamName( int pParamIndex ) const {
	Assert( pParamIndex < NUM_SHADER_MATERIAL_VARS );
	return s_StandardParams[pParamIndex].m_pName;
}

const char* CBaseShader::GetParamHelp( int pParamIndex ) const {
	Assert( pParamIndex < NUM_SHADER_MATERIAL_VARS );
	return s_StandardParams[pParamIndex].m_pHelp;
}

ShaderParamType_t CBaseShader::GetParamType( int pParamIndex ) const {
	Assert( pParamIndex < NUM_SHADER_MATERIAL_VARS );
	return s_StandardParams[pParamIndex].m_Type;
}

const char* CBaseShader::GetParamDefault( int pParamIndex ) const {
	Assert( pParamIndex < NUM_SHADER_MATERIAL_VARS );
	return s_StandardParams[pParamIndex].m_pDefaultValue;
}

int CBaseShader::GetParamFlags( int pParamIndex ) const {
	Assert( pParamIndex < NUM_SHADER_MATERIAL_VARS );
	return s_StandardParams[pParamIndex].m_nFlags;
}

//-----------------------------------------------------------------------------
// Necessary to snag ahold of some important data for the helper methods
//-----------------------------------------------------------------------------
void CBaseShader::InitShaderParams( IMaterialVar** pParams, const char* pMaterialName ) {
	// Re-entrance check
	Assert( not s_ppParams );

	s_ppParams = pParams;

	OnInitShaderParams( pParams, pMaterialName );

	s_ppParams = nullptr;
}

void CBaseShader::InitShaderInstance( IMaterialVar** pParams, IShaderInit* pShaderInit, const char* pMaterialName, const char* pTextureGroupName ) {
	// Re-entrance check
	Assert( not s_ppParams );

	s_ppParams = pParams;
	s_pShaderInit = pShaderInit;
	s_pTextureGroupName = pTextureGroupName;

	OnInitShaderInstance( pParams, pShaderInit, pMaterialName );

	s_pTextureGroupName = nullptr;
	s_ppParams = nullptr;
	s_pShaderInit = nullptr;
}

void CBaseShader::DrawElements( IMaterialVar** pParams, int pModulationFlags, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, VertexCompressionType_t pVertexCompression, CBasePerMaterialContextData** pContext ) {
	VPROF( "CBaseShader::DrawElements" );
	// Re-entrance check
	Assert( not s_ppParams );

	s_ppParams = pParams;
	s_pShaderAPI = pShaderAPI;
	s_pShaderShadow = pShaderShadow;
	s_nModulationFlags = pModulationFlags;
	s_pMeshBuilder = pShaderAPI ? pShaderAPI->GetVertexModifyBuilder() : nullptr;

	if ( IsSnapshotting() ) {
		// Set up the shadow state
		SetInitialShadowState();
	}

	OnDrawElements( pParams, pShaderShadow, pShaderAPI, pVertexCompression, pContext );

	s_nModulationFlags = 0;
	s_ppParams = nullptr;
	s_pShaderAPI = nullptr;
	s_pShaderShadow = nullptr;
	s_pMeshBuilder = nullptr;
}


//-----------------------------------------------------------------------------
// Sets the default shadow state
//-----------------------------------------------------------------------------
void CBaseShader::SetInitialShadowState() {
	// Set the default state
	s_pShaderShadow->SetDefaultState();

	// Init the standard states...
	int flags = s_ppParams[FLAGS]->GetIntValue();
	if ( flags & MATERIAL_VAR_IGNOREZ ) {
		s_pShaderShadow->EnableDepthTest( false );
		s_pShaderShadow->EnableDepthWrites( false );
	}

	if ( flags & MATERIAL_VAR_DECAL ) {
		s_pShaderShadow->EnablePolyOffset( SHADER_POLYOFFSET_DECAL );
		s_pShaderShadow->EnableDepthWrites( false );
	}

	if ( flags & MATERIAL_VAR_NOCULL ) {
		s_pShaderShadow->EnableCulling( false );
	}

	if ( flags & MATERIAL_VAR_ZNEARER ) {
		s_pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_NEARER );
	}

	if ( flags & MATERIAL_VAR_WIREFRAME ) {
		s_pShaderShadow->PolyMode( SHADER_POLYMODEFACE_FRONT_AND_BACK, SHADER_POLYMODE_LINE );
	}

	// Set alpha to coverage
	if ( flags & MATERIAL_VAR_ALLOWALPHATOCOVERAGE ) {
		// Force the bit on and then check against alpha blend and test states in CShaderShadowDX8::ComputeAggregateShadowState()
		s_pShaderShadow->EnableAlphaToCoverage( true );
	}
}


//-----------------------------------------------------------------------------
// Draws a snapshot
//-----------------------------------------------------------------------------
void CBaseShader::Draw( const bool pMakeActualDrawCall ) {
	if ( IsSnapshotting() ) {
		// Turn off transparency if we're asked to....
		if ( g_pConfig->bNoTransparency and ( s_ppParams[FLAGS]->GetIntValue() & MATERIAL_VAR_NO_DEBUG_OVERRIDE ) == 0 ) {
			s_pShaderShadow->EnableDepthWrites( true );
			s_pShaderShadow->EnableBlending( false );
		}

		GetShaderSystem()->TakeSnapshot();
	} else {
		GetShaderSystem()->DrawSnapshot( pMakeActualDrawCall );
	}
}


//-----------------------------------------------------------------------------
// Finds a particular parameter	(works because the lowest parameters match the shader)
//-----------------------------------------------------------------------------
int CBaseShader::FindParamIndex( const char* pName ) const {
	const int numParams = GetNumParams();
	for ( int i = 0; i < numParams; i++ ) {
		if ( Q_strnicmp( GetParamName( i ), pName, 64 ) == 0 ) {
			return i;
		}
	}
	return -1;
}


//-----------------------------------------------------------------------------
// Are we using graphics?
//-----------------------------------------------------------------------------
bool CBaseShader::IsUsingGraphics() {
	return GetShaderSystem()->IsUsingGraphics();
}


//-----------------------------------------------------------------------------
// Are we using graphics?
//-----------------------------------------------------------------------------
bool CBaseShader::CanUseEditorMaterials() {
	return GetShaderSystem()->CanUseEditorMaterials();
}


//-----------------------------------------------------------------------------
// Gets the builder...
//-----------------------------------------------------------------------------
CMeshBuilder* CBaseShader::MeshBuilder() {
	return s_pMeshBuilder;
}


//-----------------------------------------------------------------------------
// Loads a texture
//-----------------------------------------------------------------------------
void CBaseShader::LoadTexture( const int pTextureVar, const int pAdditionalCreationFlags ) {
	if ( not s_ppParams or pTextureVar == -1 ) {
		return;
	}

	IMaterialVar* pNameVar = s_ppParams[pTextureVar];
	if ( pNameVar and pNameVar->IsDefined() ) {
		s_pShaderInit->LoadTexture( pNameVar, s_pTextureGroupName, pAdditionalCreationFlags );
	}
}


//-----------------------------------------------------------------------------
// Loads a bumpmap
//-----------------------------------------------------------------------------
void CBaseShader::LoadBumpMap( const int pTextureVar ) {
	if ( not s_ppParams or pTextureVar == -1 ) {
		return;
	}

	IMaterialVar* pNameVar = s_ppParams[pTextureVar];
	if ( pNameVar and pNameVar->IsDefined() ) {
		s_pShaderInit->LoadBumpMap( pNameVar, s_pTextureGroupName );
	}
}


//-----------------------------------------------------------------------------
// Loads a cubemap
//-----------------------------------------------------------------------------
void CBaseShader::LoadCubeMap( const int pTextureVar, const int pAdditionalCreationFlags ) {
	if ( not s_ppParams or pTextureVar == -1 ) {
		return;
	}

	IMaterialVar* pNameVar = s_ppParams[pTextureVar];
	if ( pNameVar and pNameVar->IsDefined() ) {
		s_pShaderInit->LoadCubeMap( s_ppParams, pNameVar, pAdditionalCreationFlags );
	}
}


ShaderAPITextureHandle_t CBaseShader::GetShaderAPITextureBindHandle( int pTextureVar, int pFrameVar, int pTextureChannel ) {
	Assert( not IsSnapshotting() );
	Assert( pTextureVar != -1 );
	Assert( s_ppParams );

	IMaterialVar* textureVar = s_ppParams[pTextureVar];
	IMaterialVar* frameVar = pFrameVar != -1 ? s_ppParams[pFrameVar] : nullptr;
	int nFrame = frameVar ? frameVar->GetIntValue() : 0;
	return GetShaderSystem()->GetShaderAPITextureBindHandle( textureVar->GetTextureValue(), nFrame, pTextureChannel );
}


//-----------------------------------------------------------------------------
// Four different flavors of BindTexture(), handling the two-sampler
// case as well as ITexture* versus textureVar forms
//-----------------------------------------------------------------------------

void CBaseShader::BindTexture( const Sampler_t pSampler1, const int pTextureVar, const int pFrameVar /* = -1 */ ) {
	BindTexture( pSampler1, static_cast<Sampler_t>( -1 ), pTextureVar, pFrameVar );
}


void CBaseShader::BindTexture( Sampler_t pSampler1, Sampler_t pSampler2, int pTextureVar, int pFrameVar /* = -1 */ ) {
	Assert( not IsSnapshotting() );
	Assert( pTextureVar != -1 );
	Assert( s_ppParams );

	IMaterialVar* textureVar = s_ppParams[pTextureVar];
	IMaterialVar* frameVar = pFrameVar != -1 ? s_ppParams[pFrameVar] : nullptr;
	if ( textureVar ) {
		int nFrame = frameVar ? frameVar->GetIntValue() : 0;

		if ( pSampler2 == -1 ) {
			GetShaderSystem()->BindTexture( pSampler1, textureVar->GetTextureValue(), nFrame );
		} else {
			GetShaderSystem()->BindTexture( pSampler1, pSampler2, textureVar->GetTextureValue(), nFrame );
		}
	}
}


void CBaseShader::BindTexture( const Sampler_t pSampler1, ITexture* pTexture, const int pFrame /* = 0 */ ) {
	BindTexture( pSampler1, static_cast<Sampler_t>( -1 ), pTexture, pFrame );
}

void CBaseShader::BindTexture( Sampler_t pSampler1, Sampler_t pSampler2, ITexture* pTexture, int pFrame /* = 0 */ ) {
	Assert( not IsSnapshotting() );

	if ( pSampler2 == -1 ) {
		GetShaderSystem()->BindTexture( pSampler1, pTexture, pFrame );
	} else {
		GetShaderSystem()->BindTexture( pSampler1, pSampler2, pTexture, pFrame );
	}
}


//-----------------------------------------------------------------------------
// Does the texture store translucency in its alpha channel?
//-----------------------------------------------------------------------------
bool CBaseShader::TextureIsTranslucent( const int pTextureVar, const bool pIsBaseTexture ) {
	if ( pTextureVar < 0 ) {
		return false;
	}

	IMaterialVar** params = s_ppParams;
	if ( params[pTextureVar]->GetType() == MATERIAL_VAR_TYPE_TEXTURE ) {
		if ( not pIsBaseTexture ) {
			return params[pTextureVar]->GetTextureValue()->IsTranslucent();
		}

		// Override translucency settings if this flag is set.
		if ( IS_FLAG_SET( MATERIAL_VAR_OPAQUETEXTURE ) ) {
			return false;
		}

		if ( ( CurrentMaterialVarFlags() & ( MATERIAL_VAR_SELFILLUM | MATERIAL_VAR_BASEALPHAENVMAPMASK ) ) == 0 ) {
			if ( CurrentMaterialVarFlags() & MATERIAL_VAR_TRANSLUCENT or CurrentMaterialVarFlags() & MATERIAL_VAR_ALPHATEST ) {
				return params[pTextureVar]->GetTextureValue()->IsTranslucent();
			}
		}
	}

	return false;
}


//-----------------------------------------------------------------------------
//
// Helper methods for color modulation
//
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Are we alpha or color modulating?
//-----------------------------------------------------------------------------
bool CBaseShader::IsAlphaModulating() {
	return ( s_nModulationFlags & SHADER_USING_ALPHA_MODULATION ) != 0;
}

bool CBaseShader::IsColorModulating() {
	return ( s_nModulationFlags & SHADER_USING_COLOR_MODULATION ) != 0;
}


void CBaseShader::GetColorParameter( IMaterialVar** params, float* pColorOut ) const {
	float32 flColor2[3];
	params[COLOR]->GetVecValue( pColorOut, 3 );
	params[COLOR2]->GetVecValue( flColor2, 3 );

	pColorOut[0] *= flColor2[0];
	pColorOut[1] *= flColor2[1];
	pColorOut[2] *= flColor2[2];

	if ( g_pHardwareConfig->UsesSRGBCorrectBlending() ) {
		float32 flSRGBTint[3];
		params[SRGBTINT]->GetVecValue( flSRGBTint, 3 );

		pColorOut[0] *= flSRGBTint[0];
		pColorOut[1] *= flSRGBTint[1];
		pColorOut[2] *= flSRGBTint[2];
	}
}

//-----------------------------------------------------------------------------
// FIXME: Figure out a better way to do this?
//-----------------------------------------------------------------------------
int CBaseShader::ComputeModulationFlags( IMaterialVar** params, IShaderDynamicAPI* pShaderAPI ) {
	s_pShaderAPI = pShaderAPI;

	int mod = 0;
	if ( GetAlpha( params ) < 1.0f ) {
		mod |= SHADER_USING_ALPHA_MODULATION;
	}

	float color[3];
	GetColorParameter( params, color );

	if ( color[0] != 1.0 or color[1] != 1.0 or color[2] != 1.0 ) {
		mod |= SHADER_USING_COLOR_MODULATION;
	}

	if ( UsingFlashlight( params ) ) {
		mod |= SHADER_USING_FLASHLIGHT;
	}

	if ( UsingEditor( params ) ) {
		mod |= SHADER_USING_EDITOR;
	}

	if ( IS_FLAG2_SET( MATERIAL_VAR2_USE_FIXED_FUNCTION_BAKED_LIGHTING ) ) {
		AssertOnce( IS_FLAG2_SET( MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS ) );
		if ( IS_FLAG2_SET( MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS ) ) {
			mod |= SHADER_USING_FIXED_FUNCTION_BAKED_LIGHTING;
		}
	}

	s_pShaderAPI = nullptr;

	return mod;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CBaseShader::NeedsPowerOfTwoFrameBufferTexture( IMaterialVar** params, bool pCheckSpecificToThisFrame ) const {
	return CShader_IsFlag2Set( params, MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE );
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CBaseShader::NeedsFullFrameBufferTexture( IMaterialVar** params, bool pCheckSpecificToThisFrame ) const {
	return CShader_IsFlag2Set( params, MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE );
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CBaseShader::IsTranslucent( IMaterialVar** params ) const {
	return IS_FLAG_SET( MATERIAL_VAR_TRANSLUCENT );
}

//-----------------------------------------------------------------------------
// Returns the translucency...
//-----------------------------------------------------------------------------
float CBaseShader::GetAlpha( IMaterialVar** pParams ) {
	if ( not pParams ) {
		pParams = s_ppParams;
	}

	if ( not pParams ) {
		return 1.0f;
	}

	if ( pParams[FLAGS]->GetIntValue() & MATERIAL_VAR_NOALPHAMOD ) {
		return 1.0f;
	}

	const float32 flAlpha = pParams[ALPHA]->GetFloatValue();
	return clamp( flAlpha, 0.0f, 1.0f );
}


//-----------------------------------------------------------------------------
// Sets the color + transparency
//-----------------------------------------------------------------------------
void CBaseShader::SetColorState( int colorVar, bool setAlpha ) {
	Assert( not IsSnapshotting() );
	if ( not s_ppParams ) {
		return;
	}

	// Use tint instead of color if it was specified...
	IMaterialVar* pColorVar = colorVar != -1 ? s_ppParams[colorVar] : nullptr;

	float32 color[4] = { 1.0, 1.0, 1.0, 1.0 };
	if ( pColorVar ) {
		if ( pColorVar->GetType() == MATERIAL_VAR_TYPE_VECTOR ) {
			pColorVar->GetVecValue( color, 3 );
		} else {
			color[0] = color[1] = color[2] = pColorVar->GetFloatValue();
		}

		if ( not g_pHardwareConfig->SupportsPixelShaders_1_4() )  // Clamp 0..1 for ps_1_1 and below
		{
			color[0] = clamp( color[0], 0.0f, 1.0f );
			color[1] = clamp( color[1], 0.0f, 1.0f );
			color[2] = clamp( color[2], 0.0f, 1.0f );
		} else if ( not g_pHardwareConfig->SupportsPixelShaders_2_0() )  // Clamp 0..8 for ps_1_4
		{
			color[0] = clamp( color[0], 0.0f, 8.0f );
			color[1] = clamp( color[1], 0.0f, 8.0f );
			color[2] = clamp( color[2], 0.0f, 8.0f );
		}
	}
	ApplyColor2Factor( color );
	color[3] = setAlpha ? GetAlpha() : 1.0f;
	s_pShaderAPI->Color4fv( color );
}


void CBaseShader::SetModulationShadowState( const int pTintVar ) {
	// Have have no control over the tint var...
	bool doModulation = pTintVar != -1;

	// We activate color modulating when we're alpha or color modulating
	doModulation = doModulation or IsAlphaModulating() or IsColorModulating();

	s_pShaderShadow->EnableConstantColor( doModulation );
}

void CBaseShader::SetModulationDynamicState( const int pTintVar ) {
	if ( pTintVar != -1 ) {
		SetColorState( pTintVar, true );
	} else {
		SetColorState( COLOR, true );
	}
}

void CBaseShader::ApplyColor2Factor( float* pColorOut ) const {  // (*pColorOut) *= COLOR2
	const IMaterialVar* color2Var = s_ppParams[COLOR2];
	if ( color2Var->GetType() == MATERIAL_VAR_TYPE_VECTOR ) {
		float32 flColor2[3];
		color2Var->GetVecValue( flColor2, 3 );

		pColorOut[0] *= flColor2[0];
		pColorOut[1] *= flColor2[1];
		pColorOut[2] *= flColor2[2];
	}
	if ( g_pHardwareConfig->UsesSRGBCorrectBlending() ) {
		const IMaterialVar* SRGBVar = s_ppParams[SRGBTINT];
		if ( SRGBVar->GetType() == MATERIAL_VAR_TYPE_VECTOR ) {
			float32 flSRGB[3];
			SRGBVar->GetVecValue( flSRGB, 3 );

			pColorOut[0] *= flSRGB[0];
			pColorOut[1] *= flSRGB[1];
			pColorOut[2] *= flSRGB[2];
		}
	}
}

void CBaseShader::ComputeModulationColor( float* color ) {
	Assert( not IsSnapshotting() );
	if ( not s_ppParams ) {
		return;
	}

	IMaterialVar* pColorVar = s_ppParams[COLOR];
	if ( pColorVar->GetType() == MATERIAL_VAR_TYPE_VECTOR ) {
		pColorVar->GetVecValue( color, 3 );
	} else {
		color[0] = color[1] = color[2] = pColorVar->GetFloatValue();
	}

	ApplyColor2Factor( color );

	if ( not g_pConfig->bShowDiffuse ) {
		color[0] = color[1] = color[2] = 0.0f;
	}
	if ( mat_fullbright.GetInt() == 2 ) {
		color[0] = color[1] = color[2] = 1.0f;
	}
	color[3] = GetAlpha();
}


//-----------------------------------------------------------------------------
//
// Helper methods for alpha blending....
//
//-----------------------------------------------------------------------------
void CBaseShader::EnableAlphaBlending( ShaderBlendFactor_t src, ShaderBlendFactor_t dst ) {
	Assert( IsSnapshotting() );
	s_pShaderShadow->EnableBlending( true );
	s_pShaderShadow->BlendFunc( src, dst );
	s_pShaderShadow->EnableDepthWrites( false );
}

void CBaseShader::DisableAlphaBlending() {
	Assert( IsSnapshotting() );
	s_pShaderShadow->EnableBlending( false );
}

void CBaseShader::SetNormalBlendingShadowState( int textureVar, bool isBaseTexture ) {
	Assert( IsSnapshotting() );

	// Either we've got a constant modulation
	bool isTranslucent = IsAlphaModulating();

	// Or we've got a vertex alpha
	isTranslucent = isTranslucent or CurrentMaterialVarFlags() & MATERIAL_VAR_VERTEXALPHA;

	// Or we've got a texture alpha
	isTranslucent = isTranslucent or ( TextureIsTranslucent( textureVar, isBaseTexture ) and not ( CurrentMaterialVarFlags() & MATERIAL_VAR_ALPHATEST ) );

	if ( isTranslucent ) {
		EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
	} else {
		DisableAlphaBlending();
	}
}

//ConVar mat_debug_flashlight_only( "mat_debug_flashlight_only", "0" );
void CBaseShader::SetAdditiveBlendingShadowState( int textureVar, bool isBaseTexture ) {
	Assert( IsSnapshotting() );

	// Either we've got a constant modulation
	bool isTranslucent = IsAlphaModulating();

	// Or we've got a vertex alpha
	isTranslucent = isTranslucent or CurrentMaterialVarFlags() & MATERIAL_VAR_VERTEXALPHA;

	// Or we've got a texture alpha
	isTranslucent = isTranslucent or ( TextureIsTranslucent( textureVar, isBaseTexture ) and not ( CurrentMaterialVarFlags() & MATERIAL_VAR_ALPHATEST ) );

	/*
	if ( mat_debug_flashlight_only.GetBool() )
	{
		if (isTranslucent)
		{
			EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);
			//s_pShaderShadow->EnableAlphaTest( true );
			//s_pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GREATER, 0.99f );
		}
		else
		{
			EnableAlphaBlending( SHADER_BLEND_ONE, SHADER_BLEND_ZERO);
		}
	}
	else
	*/
	{
		if ( isTranslucent ) {
			EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE );
		} else {
			EnableAlphaBlending( SHADER_BLEND_ONE, SHADER_BLEND_ONE );
		}
	}
}

void CBaseShader::SetDefaultBlendingShadowState( int textureVar, bool isBaseTexture ) {
	if ( CurrentMaterialVarFlags() & MATERIAL_VAR_ADDITIVE ) {
		SetAdditiveBlendingShadowState( textureVar, isBaseTexture );
	} else {
		SetNormalBlendingShadowState( textureVar, isBaseTexture );
	}
}

void CBaseShader::SetBlendingShadowState( BlendType_t nMode ) {
	switch ( nMode ) {
		case BT_NONE:
			DisableAlphaBlending();
			break;

		case BT_BLEND:
			EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
			break;

		case BT_ADD:
			EnableAlphaBlending( SHADER_BLEND_ONE, SHADER_BLEND_ONE );
			break;

		case BT_BLENDADD:
			EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE );
			break;
	}
}


//-----------------------------------------------------------------------------
// Sets lightmap blending mode for single texturing
//-----------------------------------------------------------------------------
void CBaseShader::SingleTextureLightmapBlendMode() {
	Assert( IsSnapshotting() );

	s_pShaderShadow->EnableBlending( true );
	s_pShaderShadow->BlendFunc( SHADER_BLEND_DST_COLOR, SHADER_BLEND_SRC_COLOR );
}


//-----------------------------------------------------------------------------
// Loads the identity transform into a matrix
//-----------------------------------------------------------------------------
void CBaseShader::LoadIdentity( MaterialMatrixMode_t matrixMode ) {
	Assert( not IsSnapshotting() );

	s_pShaderAPI->MatrixMode( matrixMode );
	s_pShaderAPI->LoadIdentity();
}


//-----------------------------------------------------------------------------
// Loads the camera to world transform into a matrix
//-----------------------------------------------------------------------------
void CBaseShader::LoadCameraToWorldTransform( MaterialMatrixMode_t matrixMode ) {
	s_pShaderAPI->MatrixMode( matrixMode );
	s_pShaderAPI->LoadCameraToWorld();
}

void CBaseShader::LoadCameraSpaceSphereMapTransform( MaterialMatrixMode_t matrixMode ) {
	static float mat[4][4] =
		{
			{ 0.5f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, -0.5f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0.5f, -0.5f, 0.0f, 1.0f },
		};

	s_pShaderAPI->MatrixMode( matrixMode );
	s_pShaderAPI->LoadMatrix( (float*) mat );
}


//-----------------------------------------------------------------------------
//
// Sets a texture translation transform
//
//-----------------------------------------------------------------------------
void CBaseShader::SetFixedFunctionTextureTranslation( MaterialMatrixMode_t textureTransform, int translationVar ) {
	Assert( not IsSnapshotting() );

	// handle scrolling of base texture
	Vector2D vDelta( 0, 0 );

	if ( translationVar != -1 ) {
		s_ppParams[translationVar]->GetVecValue( vDelta.Base(), 2 );
	}

	if ( vDelta[0] != 0.0f or vDelta[1] != 0.0f ) {
		s_pShaderAPI->MatrixMode( textureTransform );

		// only do the upper 3x3 since this is a 2D matrix
		float mat[16];
		mat[0] = 1.0f;
		mat[1] = 0.0f;
		mat[2] = 0.0f;
		mat[4] = 0.0f;
		mat[5] = 1.0f;
		mat[6] = 0.0f;
		mat[8] = vDelta[0];
		mat[9] = vDelta[1];
		mat[10] = 1.0f;

		// Better set the stuff we don't set with some sort of value!
		mat[3] = mat[7] = mat[11] = 0;
		mat[12] = mat[13] = mat[14] = 0;
		mat[15] = 1;

		s_pShaderAPI->LoadMatrix( mat );
	} else {
		LoadIdentity( textureTransform );
	}
}

void CBaseShader::SetFixedFunctionTextureScale( MaterialMatrixMode_t textureTransform, int scaleVar ) {
	Assert( not IsSnapshotting() );

	// handle scrolling of base texture
	Vector2D vScale;
	s_ppParams[scaleVar]->GetVecValue( vScale.Base(), 2 );
	if ( vScale[0] != 0.0f or vScale[1] != 0.0f ) {
		s_pShaderAPI->MatrixMode( textureTransform );

		// only do the upper 3x3 since this is a 2D matrix
		float mat[16];
		mat[0] = vScale[0];
		mat[1] = 0.0f;
		mat[2] = 0.0f;
		mat[4] = 0.0f;
		mat[5] = vScale[1];
		mat[6] = 0.0f;
		mat[8] = 0.0f;
		mat[9] = 0.0f;
		mat[10] = 1.0f;

		// Better set the stuff we don't set with some sort of value!
		mat[3] = mat[7] = mat[11] = 0;
		mat[12] = mat[13] = mat[14] = 0;
		mat[15] = 1;

		s_pShaderAPI->LoadMatrix( mat );
	} else {
		LoadIdentity( textureTransform );
	}
}

void CBaseShader::SetFixedFunctionTextureTransform( MaterialMatrixMode_t textureTransform, int transformVar ) {
	Assert( not IsSnapshotting() );

	IMaterialVar* pTransformationVar = s_ppParams[transformVar];
	if ( pTransformationVar and pTransformationVar->GetType() == MATERIAL_VAR_TYPE_MATRIX ) {
		s_pShaderAPI->MatrixMode( textureTransform );

		const VMatrix& transformation = pTransformationVar->GetMatrixValue();

		// only do the upper 3x3 since this is a 2D matrix
		float mat[16];
		mat[0] = transformation[0][0];
		mat[1] = transformation[1][0];
		mat[2] = transformation[3][0];
		mat[4] = transformation[0][1];
		mat[5] = transformation[1][1];
		mat[6] = transformation[3][1];
		mat[8] = transformation[0][3];
		mat[9] = transformation[1][3];
		mat[10] = transformation[3][3];

		// Better set the stuff we don't set with some sort of value!
		mat[3] = mat[7] = mat[11] = 0;
		mat[12] = mat[13] = mat[14] = 0;
		mat[15] = 1;

		s_pShaderAPI->LoadMatrix( mat );
	} else {
		LoadIdentity( textureTransform );
	}
}

void CBaseShader::SetFixedFunctionTextureScaledTransform( MaterialMatrixMode_t textureTransform, int transformVar, int scaleVar ) {
	Assert( not IsSnapshotting() );

	float mat[16];
	IMaterialVar* pTransformationVar = s_ppParams[transformVar];
	if ( pTransformationVar and ( pTransformationVar->GetType() == MATERIAL_VAR_TYPE_MATRIX ) ) {
		Vector2D scale( 1, 1 );
		IMaterialVar* pScaleVar = s_ppParams[scaleVar];
		if ( pScaleVar ) {
			if ( pScaleVar->GetType() == MATERIAL_VAR_TYPE_VECTOR )
				pScaleVar->GetVecValue( scale.Base(), 2 );
			else if ( pScaleVar->IsDefined() )
				scale[0] = scale[1] = pScaleVar->GetFloatValue();
		}

		s_pShaderAPI->MatrixMode( textureTransform );

		const VMatrix& transformation = pTransformationVar->GetMatrixValue();

		// only do the upper 3x3 since this is a 2D matrix
		mat[0] = transformation[0][0] * scale[0];
		mat[1] = transformation[1][0] * scale[0];
		mat[2] = transformation[3][0] * scale[0];
		mat[4] = transformation[0][1] * scale[1];
		mat[5] = transformation[1][1] * scale[1];
		mat[6] = transformation[3][1] * scale[1];
		mat[8] = transformation[0][3];
		mat[9] = transformation[1][3];
		mat[10] = transformation[3][3];

		// Better set the stuff we don't set with some sort of value!
		mat[3] = mat[7] = mat[11] = 0;
		mat[12] = mat[13] = mat[14] = 0;
		mat[15] = 1;

		s_pShaderAPI->LoadMatrix( mat );
	} else {
		SetFixedFunctionTextureScale( textureTransform, scaleVar );
	}
}


//-----------------------------------------------------------------------------
//
// Helper methods for fog
//
//-----------------------------------------------------------------------------
void CBaseShader::FogToOOOverbright() {
	Assert( IsSnapshotting() );
	if ( ( CurrentMaterialVarFlags() & MATERIAL_VAR_NOFOG ) == 0 ) {
		s_pShaderShadow->FogMode( SHADER_FOGMODE_OO_OVERBRIGHT );
	} else {
		s_pShaderShadow->FogMode( SHADER_FOGMODE_DISABLED );
	}
}

void CBaseShader::FogToWhite() {
	Assert( IsSnapshotting() );
	if ( ( CurrentMaterialVarFlags() & MATERIAL_VAR_NOFOG ) == 0 ) {
		s_pShaderShadow->FogMode( SHADER_FOGMODE_WHITE );
	} else {
		s_pShaderShadow->FogMode( SHADER_FOGMODE_DISABLED );
	}
}
void CBaseShader::FogToBlack() {
	Assert( IsSnapshotting() );
	if ( ( CurrentMaterialVarFlags() & MATERIAL_VAR_NOFOG ) == 0 ) {
		s_pShaderShadow->FogMode( SHADER_FOGMODE_BLACK );
	} else {
		s_pShaderShadow->FogMode( SHADER_FOGMODE_DISABLED );
	}
}

void CBaseShader::FogToGrey() {
	Assert( IsSnapshotting() );
	if ( ( CurrentMaterialVarFlags() & MATERIAL_VAR_NOFOG ) == 0 ) {
		s_pShaderShadow->FogMode( SHADER_FOGMODE_GREY );
	} else {
		s_pShaderShadow->FogMode( SHADER_FOGMODE_DISABLED );
	}
}

void CBaseShader::FogToFogColor() {
	Assert( IsSnapshotting() );
	if ( ( CurrentMaterialVarFlags() & MATERIAL_VAR_NOFOG ) == 0 ) {
		s_pShaderShadow->FogMode( SHADER_FOGMODE_FOGCOLOR );
	} else {
		s_pShaderShadow->FogMode( SHADER_FOGMODE_DISABLED );
	}
}

void CBaseShader::DisableFog() {
	Assert( IsSnapshotting() );
	s_pShaderShadow->FogMode( SHADER_FOGMODE_DISABLED );
}

void CBaseShader::DefaultFog() {
	if ( CurrentMaterialVarFlags() & MATERIAL_VAR_ADDITIVE ) {
		FogToBlack();
	} else {
		FogToFogColor();
	}
}


//-----------------------------------------------------------------------------
// Fixed function multiply by detail texture pass
//-----------------------------------------------------------------------------
void CBaseShader::FixedFunctionMultiplyByDetailPass( int baseTextureVar, int frameVar, int textureTransformVar, int detailVar, int detailScaleVar ) {
	IMaterialVar** params = s_ppParams;

	if ( not params[detailVar]->IsDefined() )
		return;

	if ( IsSnapshotting() ) {
		SetInitialShadowState();

		s_pShaderShadow->EnableAlphaTest( IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) );

		bool translucentTexture = TextureIsTranslucent( baseTextureVar, true ) or IS_FLAG_SET( MATERIAL_VAR_ALPHATEST );
		s_pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
		s_pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
		s_pShaderShadow->EnableTexGen( SHADER_TEXTURE_STAGE0, false );
		s_pShaderShadow->EnableTexGen( SHADER_TEXTURE_STAGE1, false );

		// Mod 2x blend here
		EnableAlphaBlending( SHADER_BLEND_DST_COLOR, SHADER_BLEND_SRC_COLOR );

		s_pShaderShadow->EnableCustomPixelPipe( true );
		s_pShaderShadow->CustomTextureStages( 2 );

		// We need to blend towards grey based on alpha...
		// We can never get the perfect alpha (vertex alpha * cc alpha * texture alpha)
		// so we'll just choose to use cc alpha * texture alpha

		int flags = SHADER_DRAW_POSITION | SHADER_DRAW_TEXCOORD1;

		// Compute alpha, stage 0 is used, stage 1 isn't.
		if ( translucentTexture ) {
			s_pShaderShadow->CustomTextureOperation( SHADER_TEXTURE_STAGE0,
													 SHADER_TEXCHANNEL_ALPHA, SHADER_TEXOP_MODULATE,
													 SHADER_TEXARG_TEXTURE, SHADER_TEXARG_CONSTANTCOLOR );
			flags |= SHADER_DRAW_TEXCOORD0;
		} else {
			bool hasVertexAlpha = ( CurrentMaterialVarFlags() & MATERIAL_VAR_VERTEXALPHA ) != 0;
			if ( hasVertexAlpha ) {
				flags |= SHADER_DRAW_COLOR;
			}

			s_pShaderShadow->CustomTextureOperation( SHADER_TEXTURE_STAGE0,
													 SHADER_TEXCHANNEL_ALPHA, hasVertexAlpha ? SHADER_TEXOP_MODULATE : SHADER_TEXOP_SELECTARG1,
													 SHADER_TEXARG_CONSTANTCOLOR, SHADER_TEXARG_VERTEXCOLOR );
		}

		s_pShaderShadow->CustomTextureOperation( SHADER_TEXTURE_STAGE1,
												 SHADER_TEXCHANNEL_ALPHA, SHADER_TEXOP_SELECTARG1,
												 SHADER_TEXARG_PREVIOUSSTAGE, SHADER_TEXARG_NONE );

		// This here will perform color = vertex light * alpha + 0.5f * (1 - alpha)
		// Stage 0 really doesn't do anything
		s_pShaderShadow->CustomTextureOperation( SHADER_TEXTURE_STAGE0,
												 SHADER_TEXCHANNEL_COLOR, SHADER_TEXOP_SELECTARG1,
												 SHADER_TEXARG_TEXTURE, SHADER_TEXARG_CONSTANTCOLOR );

		s_pShaderShadow->CustomTextureOperation( SHADER_TEXTURE_STAGE1,
												 SHADER_TEXCHANNEL_COLOR, SHADER_TEXOP_BLEND_PREVIOUSSTAGEALPHA,
												 SHADER_TEXARG_TEXTURE, SHADER_TEXARG_CONSTANTCOLOR );

		s_pShaderShadow->DrawFlags( flags );
		FogToGrey();
		Draw();

		s_pShaderShadow->EnableCustomPixelPipe( false );
		DisableAlphaBlending();
	} else {
		if ( TextureIsTranslucent( baseTextureVar, true ) ) {
			SetFixedFunctionTextureTransform( MATERIAL_TEXTURE0, textureTransformVar );
			BindTexture( SHADER_SAMPLER0, baseTextureVar, frameVar );
		} else {
			// Unnecessary... but we get strange colors if we don't put something on stage 0
			BindTexture( SHADER_SAMPLER0, detailVar, frameVar );
		}

		BindTexture( SHADER_SAMPLER1, detailVar, frameVar );
		SetFixedFunctionTextureScaledTransform( MATERIAL_TEXTURE1, textureTransformVar, detailScaleVar );
		float alpha = GetAlpha();
		s_pShaderAPI->Color4ub( 128, 128, 128, 255 * alpha );

		Draw();
	}
}


//-----------------------------------------------------------------------------
// Multiply by lightmap pass
//-----------------------------------------------------------------------------
void CBaseShader::FixedFunctionMultiplyByLightmapPass( int baseTextureVar, int frameVar, int baseTextureTransformVar, float alphaOverride ) {
	if ( IsSnapshotting() ) {
		SetInitialShadowState();

		s_pShaderShadow->EnableAlphaTest( false );

		s_pShaderShadow->EnableBlending( true );
		SingleTextureLightmapBlendMode();

		s_pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
		s_pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
		s_pShaderShadow->EnableTexGen( SHADER_TEXTURE_STAGE0, false );
		s_pShaderShadow->EnableTexGen( SHADER_TEXTURE_STAGE1, false );

		s_pShaderShadow->EnableCustomPixelPipe( true );
		s_pShaderShadow->CustomTextureStages( 2 );

		// Stage zero color is not used, this op doesn't matter
		s_pShaderShadow->CustomTextureOperation( SHADER_TEXTURE_STAGE0,
												 SHADER_TEXCHANNEL_COLOR, SHADER_TEXOP_SELECTARG1,
												 SHADER_TEXARG_CONSTANTCOLOR, SHADER_TEXARG_CONSTANTCOLOR );

		// This here will perform color = lightmap * (cc alpha) + 1 * (1- cc alpha)
		s_pShaderShadow->CustomTextureOperation( SHADER_TEXTURE_STAGE1,
												 SHADER_TEXCHANNEL_COLOR, SHADER_TEXOP_BLEND_PREVIOUSSTAGEALPHA,
												 SHADER_TEXARG_TEXTURE, SHADER_TEXARG_CONSTANTCOLOR );

		int flags = SHADER_DRAW_POSITION | SHADER_DRAW_LIGHTMAP_TEXCOORD1;

		// Multiply the constant alpha by the texture alpha for total alpha
		if ( TextureIsTranslucent( baseTextureVar, true ) ) {
			s_pShaderShadow->CustomTextureOperation( SHADER_TEXTURE_STAGE0,
													 SHADER_TEXCHANNEL_ALPHA, SHADER_TEXOP_MODULATE,
													 SHADER_TEXARG_TEXTURE, SHADER_TEXARG_CONSTANTCOLOR );

			flags |= SHADER_DRAW_TEXCOORD0;
		} else {
			s_pShaderShadow->EnableTexture( SHADER_SAMPLER0, false );
			s_pShaderShadow->CustomTextureOperation( SHADER_TEXTURE_STAGE0,
													 SHADER_TEXCHANNEL_ALPHA, SHADER_TEXOP_SELECTARG2,
													 SHADER_TEXARG_TEXTURE, SHADER_TEXARG_CONSTANTCOLOR );
		}

		// Alpha isn't used, it doesn't matter what we set it to.
		s_pShaderShadow->CustomTextureOperation( SHADER_TEXTURE_STAGE1,
												 SHADER_TEXCHANNEL_ALPHA, SHADER_TEXOP_SELECTARG1,
												 SHADER_TEXARG_PREVIOUSSTAGE, SHADER_TEXARG_NONE );

		s_pShaderShadow->DrawFlags( flags );

		FogToOOOverbright();
		Draw();

		s_pShaderShadow->EnableCustomPixelPipe( false );
	} else {
		s_pShaderAPI->SetDefaultState();

		// Put the alpha in the color channel to modulate the color down....
		float alpha = ( alphaOverride < 0 ) ? GetAlpha() : alphaOverride;

		// NOTE: 128 is a more exact OO_OVERBRIGHT; it prevents some artifacts
		//		s_pShaderAPI->Color4f( OO_OVERBRIGHT, OO_OVERBRIGHT, OO_OVERBRIGHT, alpha );
		s_pShaderAPI->Color4ub( 128, 128, 128, (int) ( alpha * 255 ) );

		if ( TextureIsTranslucent( baseTextureVar, true ) ) {
			SetFixedFunctionTextureTransform( MATERIAL_TEXTURE0, baseTextureTransformVar );
			BindTexture( SHADER_SAMPLER0, baseTextureVar, frameVar );
		}

		LoadIdentity( MATERIAL_TEXTURE1 );
		s_pShaderAPI->BindStandardTexture( SHADER_SAMPLER1, TEXTURE_LIGHTMAP );

		Draw();
	}
}


//-----------------------------------------------------------------------------
// Fixed function Self illumination pass
//-----------------------------------------------------------------------------
void CBaseShader::FixedFunctionSelfIlluminationPass( Sampler_t sampler,
													 int baseTextureVar, int frameVar, int baseTextureTransformVar, int selfIllumTintVar ) {
	//	IMaterialVar** params = s_ppParams;

	if ( IsSnapshotting() ) {
		SetInitialShadowState();

		// A little setup for self illum here...
		SetModulationShadowState( selfIllumTintVar );

		s_pShaderShadow->EnableTexture( sampler, true );

		// No overbrighting
		s_pShaderShadow->OverbrightValue( SHADER_TEXTURE_STAGE0, 1.0f );
		s_pShaderShadow->OverbrightValue( SHADER_TEXTURE_STAGE1, 1.0f );

		// Don't bother with z writes here...
		s_pShaderShadow->EnableDepthWrites( false );

		// We're always blending
		EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );

		int flags = SHADER_DRAW_POSITION;
		if ( sampler == SHADER_SAMPLER0 )
			flags |= SHADER_DRAW_TEXCOORD0;
		else
			flags |= SHADER_DRAW_TEXCOORD1;

		s_pShaderShadow->DrawFlags( flags );
		FogToFogColor();
	} else {
		s_pShaderAPI->SetDefaultState();

		SetFixedFunctionTextureTransform( sampler == SHADER_SAMPLER0 ? MATERIAL_TEXTURE0 : MATERIAL_TEXTURE1, baseTextureTransformVar );
		BindTexture( sampler, baseTextureVar, frameVar );

		// NOTE: Texture + texture offset are set from BaseTimesLightmap
		SetModulationDynamicState( selfIllumTintVar );
	}
	Draw();
}


//-----------------------------------------------------------------------------
// Fixed function Base * detail pass
//-----------------------------------------------------------------------------
void CBaseShader::FixedFunctionBaseTimesDetailPass( int baseTextureVar, int frameVar, int baseTextureTransformVar, int detailVar, int detailScaleVar ) {
	IMaterialVar** params = s_ppParams;

	// We can't do this one one pass if CC and VC are both active...
	bool hasDetail = detailVar != -1 and params[detailVar]->IsDefined();
	bool detailInSecondPass = hasDetail and IsColorModulating() and ( IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR ) or IS_FLAG_SET( MATERIAL_VAR_VERTEXALPHA ) );

	if ( IsSnapshotting() ) {
		IMaterialVar** params = s_ppParams;

		s_pShaderShadow->EnableTexGen( SHADER_TEXTURE_STAGE0, false );
		s_pShaderShadow->EnableTexGen( SHADER_TEXTURE_STAGE1, false );

		// alpha test
		s_pShaderShadow->EnableAlphaTest( IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) );

		// Alpha blending
		SetDefaultBlendingShadowState( baseTextureVar, true );

		// independently configure alpha and color
		s_pShaderShadow->EnableAlphaPipe( true );

		// Here's the color	states (NOTE: SHADER_DRAW_COLOR == use Vertex Color)
		s_pShaderShadow->EnableConstantColor( IsColorModulating() );
		s_pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );

		int flags = SHADER_DRAW_POSITION | SHADER_DRAW_TEXCOORD0;

		// Detail texture..
		if ( hasDetail and ( not detailInSecondPass ) ) {
			s_pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );

			// Force mod2x
			s_pShaderShadow->OverbrightValue( SHADER_TEXTURE_STAGE1, 2.0f );

			flags |= SHADER_DRAW_TEXCOORD1;
		}

		// Here's the alpha states
		s_pShaderShadow->EnableConstantAlpha( IsAlphaModulating() );
		s_pShaderShadow->EnableVertexAlpha( IS_FLAG_SET( MATERIAL_VAR_VERTEXALPHA ) );
		s_pShaderShadow->EnableTextureAlpha( SHADER_TEXTURE_STAGE0, TextureIsTranslucent( baseTextureVar, true ) );

		if ( IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR ) )
			flags |= SHADER_DRAW_COLOR;
		s_pShaderShadow->DrawFlags( flags );

		DefaultFog();

		Draw();

		s_pShaderShadow->EnableAlphaPipe( false );
	} else {
		SetFixedFunctionTextureTransform( MATERIAL_TEXTURE0, baseTextureTransformVar );
		BindTexture( SHADER_SAMPLER0, baseTextureVar, frameVar );

		// Detail texture..
		if ( hasDetail and ( not detailInSecondPass ) ) {
			BindTexture( SHADER_SAMPLER1, detailVar, frameVar );
			SetFixedFunctionTextureScaledTransform( MATERIAL_TEXTURE1, baseTextureTransformVar, detailScaleVar );
		}

		SetModulationDynamicState();

		Draw();
	}

	if ( detailInSecondPass ) {
		FixedFunctionMultiplyByDetailPass( baseTextureVar, frameVar, baseTextureTransformVar, detailVar, detailScaleVar );
	}
}


//-----------------------------------------------------------------------------
// Helpers for environment mapping...
//-----------------------------------------------------------------------------
int CBaseShader::SetShadowEnvMappingState( int envMapMaskVar, int tintVar ) {
	Assert( IsSnapshotting() );
	IMaterialVar** params = s_ppParams;

	int varFlags = params[FLAGS]->GetIntValue();

	s_pShaderShadow->EnableAlphaTest( false );

	// envmap on stage 0
	s_pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
	s_pShaderShadow->EnableTexGen( SHADER_TEXTURE_STAGE0, true );
	if ( ( varFlags & MATERIAL_VAR_ENVMAPSPHERE ) == 0 )
		s_pShaderShadow->TexGen( SHADER_TEXTURE_STAGE0, SHADER_TEXGENPARAM_CAMERASPACEREFLECTIONVECTOR );
	else
		s_pShaderShadow->TexGen( SHADER_TEXTURE_STAGE0, SHADER_TEXGENPARAM_SPHERE_MAP );

	int flags = SHADER_DRAW_POSITION | SHADER_DRAW_NORMAL;

	// mask on stage 1
	if ( params[envMapMaskVar]->IsDefined() or varFlags & MATERIAL_VAR_BASEALPHAENVMAPMASK ) {
		s_pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
		flags |= SHADER_DRAW_TEXCOORD1;
	} else {
		s_pShaderShadow->EnableTexture( SHADER_SAMPLER1, false );
	}

	if ( varFlags & MATERIAL_VAR_BASEALPHAENVMAPMASK ) {
		s_pShaderShadow->EnableCustomPixelPipe( true );
		s_pShaderShadow->CustomTextureStages( 2 );

		// Color = base texture * envmaptint * (1 - mask alpha)
		s_pShaderShadow->CustomTextureOperation( SHADER_TEXTURE_STAGE0,
												 SHADER_TEXCHANNEL_COLOR, SHADER_TEXOP_MODULATE, SHADER_TEXARG_TEXTURE, SHADER_TEXARG_CONSTANTCOLOR );
		s_pShaderShadow->CustomTextureOperation( SHADER_TEXTURE_STAGE1,
												 SHADER_TEXCHANNEL_COLOR, SHADER_TEXOP_MODULATE, SHADER_TEXARG_PREVIOUSSTAGE, SHADER_TEXARG_INVTEXTUREALPHA );

		// Use alpha modulation * vertex alpha * env map alpha
		s_pShaderShadow->CustomTextureOperation( SHADER_TEXTURE_STAGE0,
												 SHADER_TEXCHANNEL_ALPHA, SHADER_TEXOP_MODULATE, SHADER_TEXARG_VERTEXCOLOR, SHADER_TEXARG_TEXTURE );
		s_pShaderShadow->CustomTextureOperation( SHADER_TEXTURE_STAGE1,
												 SHADER_TEXCHANNEL_ALPHA, SHADER_TEXOP_SELECTARG1, SHADER_TEXARG_PREVIOUSSTAGE, SHADER_TEXARG_CONSTANTCOLOR );
	} else {
		s_pShaderShadow->EnableAlphaPipe( true );

		// Color = base texture * envmaptint * mask
		s_pShaderShadow->EnableConstantColor( tintVar >= 0 );

		// Alpha = vertex alpha * constant alpha * env map alpha * mask alpha (only if it's not a base alpha mask)
		s_pShaderShadow->EnableConstantAlpha( IsAlphaModulating() );
		s_pShaderShadow->EnableVertexAlpha( ( varFlags & MATERIAL_VAR_VERTEXALPHA ) != 0 );
		s_pShaderShadow->EnableTextureAlpha( SHADER_TEXTURE_STAGE0, true );
		s_pShaderShadow->EnableTextureAlpha( SHADER_TEXTURE_STAGE1, params[envMapMaskVar]->IsTexture() );
	}

	return flags;
}

void CBaseShader::SetDynamicEnvMappingState( int envMapVar, int envMapMaskVar, int baseTextureVar, int envMapFrameVar, int envMapMaskFrameVar, int frameVar, int maskOffsetVar, int maskScaleVar, int tintVar ) {
	Assert( not IsSnapshotting() );

	IMaterialVar** params = s_ppParams;
	int varFlags = params[FLAGS]->GetIntValue();

	if ( ( varFlags & MATERIAL_VAR_ENVMAPSPHERE ) == 0 ) {
		if ( ( varFlags & MATERIAL_VAR_ENVMAPCAMERASPACE ) == 0 ) {
			LoadCameraToWorldTransform( MATERIAL_TEXTURE0 );
		} else {
			LoadIdentity( MATERIAL_TEXTURE0 );
		}
	} else {
		LoadCameraSpaceSphereMapTransform( MATERIAL_TEXTURE0 );
	}

	BindTexture( SHADER_SAMPLER0, envMapVar, envMapFrameVar );

	if ( params[envMapMaskVar]->IsTexture() ) {
		SetFixedFunctionTextureScaledTransform( MATERIAL_TEXTURE1,
												maskOffsetVar, maskScaleVar );
		BindTexture( SHADER_SAMPLER1, envMapMaskVar, envMapMaskFrameVar );
	} else if ( varFlags & MATERIAL_VAR_BASEALPHAENVMAPMASK ) {
		SetFixedFunctionTextureScaledTransform( MATERIAL_TEXTURE1,
												maskOffsetVar, maskScaleVar );
		BindTexture( SHADER_SAMPLER1, baseTextureVar, frameVar );
	}

	SetModulationDynamicState( tintVar );
}


//-----------------------------------------------------------------------------
// Masked environment map
//-----------------------------------------------------------------------------
void CBaseShader::FixedFunctionMaskedEnvmapPass( int envMapVar, int envMapMaskVar, int baseTextureVar, int envMapFrameVar, int envMapMaskFrameVar, int frameVar, int maskOffsetVar, int maskScaleVar, int envMapTintVar ) {
	//	IMaterialVar** params = ShaderState().m_ppParams;

	if ( IsSnapshotting() ) {
		// Alpha blending
		SetDefaultBlendingShadowState( envMapMaskVar, false );

		// Disable overbright
		s_pShaderShadow->OverbrightValue( SHADER_TEXTURE_STAGE0, 1.0f );
		s_pShaderShadow->OverbrightValue( SHADER_TEXTURE_STAGE1, 1.0f );

		int flags = SetShadowEnvMappingState( envMapMaskVar, envMapTintVar );
		s_pShaderShadow->DrawFlags( flags );

		DefaultFog();
		Draw();

		s_pShaderShadow->EnableCustomPixelPipe( false );
		s_pShaderShadow->EnableAlphaPipe( false );
	} else {
		SetDynamicEnvMappingState( envMapVar, envMapMaskVar, baseTextureVar, envMapFrameVar, envMapMaskFrameVar, frameVar, maskOffsetVar, maskScaleVar, envMapTintVar );

		Draw();
	}
}


//-----------------------------------------------------------------------------
// Add masked environment map
//-----------------------------------------------------------------------------
void CBaseShader::FixedFunctionAdditiveMaskedEnvmapPass( int envMapVar, int envMapMaskVar, int baseTextureVar, int envMapFrameVar, int envMapMaskFrameVar, int frameVar, int maskOffsetVar, int maskScaleVar, int envMapTintVar ) {
	//	IMaterialVar** params = ShaderState().m_ppParams;

	if ( IsSnapshotting() ) {
		SetInitialShadowState();

		// Alpha blending
		SetAdditiveBlendingShadowState( envMapMaskVar, false );

		// Disable overbright
		s_pShaderShadow->OverbrightValue( SHADER_TEXTURE_STAGE0, 1.0f );
		s_pShaderShadow->OverbrightValue( SHADER_TEXTURE_STAGE1, 1.0f );

		// Don't bother with z writes here...
		s_pShaderShadow->EnableDepthWrites( false );

		int flags = SetShadowEnvMappingState( envMapMaskVar, envMapTintVar );
		s_pShaderShadow->DrawFlags( flags );

		FogToBlack();
		Draw();

		s_pShaderShadow->EnableCustomPixelPipe( false );
		s_pShaderShadow->EnableAlphaPipe( false );
	} else {
		SetDynamicEnvMappingState( envMapVar, envMapMaskVar, baseTextureVar, envMapFrameVar, envMapMaskFrameVar, frameVar, maskOffsetVar, maskScaleVar, envMapTintVar );

		Draw();
	}
}


void CBaseShader::CleanupDynamicStateFixedFunction() {
	Assert( not IsSnapshotting() );
	LoadIdentity( MATERIAL_TEXTURE0 );
}

bool CBaseShader::UsingFlashlight( IMaterialVar** params ) const {
	if ( IsSnapshotting() ) {
		return CShader_IsFlag2Set( params, MATERIAL_VAR2_USE_FLASHLIGHT );
	}
	return s_pShaderAPI->InFlashlightMode();
}

bool CBaseShader::UsingEditor( IMaterialVar** params ) const {
	if ( IsSnapshotting() ) {
		return CShader_IsFlag2Set( params, MATERIAL_VAR2_USE_EDITOR );
	}
	return s_pShaderAPI->InEditorMode();
}

void CBaseShader::DrawFlashlight_dx70( IMaterialVar** params, IShaderDynamicAPI* pShaderAPI, IShaderShadow* pShaderShadow, int flashlightTextureVar, int flashlightTextureFrameVar, bool suppress_lighting ) {
	SHADOW_STATE {
		SET_FLAGS2( MATERIAL_VAR2_NEEDS_FIXED_FUNCTION_FLASHLIGHT );
		pShaderShadow->EnableDepthWrites( false );
		pShaderShadow->EnableAlphaWrites( false );

		// Alpha test
		//		pShaderShadow->EnableAlphaTest( IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) );
		bool bIsAlphaTested = IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) != 0;
		if ( bIsAlphaTested ) {
			// disable alpha test and use the zfunc zequals since alpha isn't guaranteed to
			// be the same on both the regular pass and the flashlight pass.
			s_pShaderShadow->EnableAlphaTest( false );
			s_pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_EQUAL );
		}

		// Alpha blend
		SetAdditiveBlendingShadowState( BASETEXTURE, true );

		int flags = SHADER_DRAW_POSITION | SHADER_DRAW_TEXCOORD1 | SHADER_DRAW_COLOR | SHADER_DRAW_NORMAL;
		pShaderShadow->DrawFlags( flags );
		FogToBlack();

		if ( not suppress_lighting )
			pShaderShadow->EnableLighting( true );

		pShaderShadow->EnableCustomPixelPipe( true );
		pShaderShadow->CustomTextureStages( 2 );

		// color stage 0
		// projected texture * vertex color (lighting)
		pShaderShadow->CustomTextureOperation( SHADER_TEXTURE_STAGE0,
											   SHADER_TEXCHANNEL_COLOR,
											   SHADER_TEXOP_MODULATE,
											   SHADER_TEXARG_TEXTURE,
											   SHADER_TEXARG_VERTEXCOLOR );

		// color stage 1
		// * base texture
		pShaderShadow->CustomTextureOperation( SHADER_TEXTURE_STAGE1,
											   SHADER_TEXCHANNEL_COLOR,
											   SHADER_TEXOP_MODULATE,
											   SHADER_TEXARG_TEXTURE, SHADER_TEXARG_PREVIOUSSTAGE );

		// alpha stage 0
		// get alpha from constant alpha
		pShaderShadow->CustomTextureOperation( SHADER_TEXTURE_STAGE0,
											   SHADER_TEXCHANNEL_ALPHA,
											   SHADER_TEXOP_SELECTARG1,
											   SHADER_TEXARG_CONSTANTCOLOR, SHADER_TEXARG_NONE );

		// alpha stage 1
		// get alpha from $basetexture
		pShaderShadow->CustomTextureOperation( SHADER_TEXTURE_STAGE1,
											   SHADER_TEXCHANNEL_ALPHA,
											   SHADER_TEXOP_MODULATE,
											   SHADER_TEXARG_TEXTURE, SHADER_TEXARG_PREVIOUSSTAGE );

		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
		pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );

		// Shove the view position into texcoord 0 before the texture matrix.
		pShaderShadow->TexGen( SHADER_TEXTURE_STAGE0, SHADER_TEXGENPARAM_EYE_LINEAR );
		pShaderShadow->EnableTexGen( SHADER_TEXTURE_STAGE0, true );
	}
	DYNAMIC_STATE {
		SetFlashlightFixedFunctionTextureTransform( MATERIAL_TEXTURE0 );

		// NOTE: This has to come after the loadmatrix since the loadmatrix screws with the
		// transform flags!!!!!!
		// Specify that we have XYZ texcoords that need to be divided by W before the pixel shader.
		// NOTE Tried to divide XY by Z, but doesn't work.
		pShaderAPI->SetTextureTransformDimension( SHADER_TEXTURE_STAGE0, 3, true );

		BindTexture( SHADER_SAMPLER0, flashlightTextureVar, flashlightTextureFrameVar );
		if ( params[BASETEXTURE]->IsTexture() ) {
			BindTexture( SHADER_SAMPLER1, BASETEXTURE, FRAME );
		} else {
			pShaderAPI->BindStandardTexture( SHADER_SAMPLER1, TEXTURE_GREY );
		}

		SetModulationDynamicState();
	}
	Draw();
}

void CBaseShader::SetFlashlightFixedFunctionTextureTransform( MaterialMatrixMode_t matrix ) {
	VMatrix worldToTexture;
	s_pShaderAPI->GetFlashlightState( worldToTexture );

	VMatrix worldToView, viewToWorld, viewToTexture;
	s_pShaderAPI->GetMatrix( MATERIAL_VIEW, &worldToView[0][0] );
	// The matrix that we get back from the shader api is transposed. . . yuck.
	MatrixTranspose( worldToView, worldToView );
	MatrixInverseGeneral( worldToView, viewToWorld );
	MatrixMultiply( worldToTexture, viewToWorld, viewToTexture );

	s_pShaderAPI->MatrixMode( matrix );
	// tranpose before going into the shaderapi. . . suck
	MatrixTranspose( viewToTexture, viewToTexture );
	s_pShaderAPI->LoadMatrix( &viewToTexture[0][0] );
}

bool CBaseShader::IsHDREnabled() {
	// FIXME!  Need to fix this for vgui materials
	HDRType_t hdr_mode = g_pHardwareConfig->GetHDRType();
	switch ( hdr_mode ) {
		case HDR_TYPE_NONE:
			return false;

		case HDR_TYPE_INTEGER:
			return true;

		case HDR_TYPE_FLOAT: {
			ITexture* pRT = s_pShaderAPI->GetRenderTargetEx( 0 );
			if ( pRT and pRT->GetImageFormat() == IMAGE_FORMAT_RGBA16161616F ) {
				return true;
			}
		}
	}
	return false;
}
