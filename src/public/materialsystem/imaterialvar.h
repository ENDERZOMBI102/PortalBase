//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
//===========================================================================//
#pragma once
#include "mathlib/vector4d.h"
#include "tier0/platform.h"
#include "tier1/utlsymbol.h"
class IMaterial;
class VMatrix;
class ITexture;

#define MAKE_MATERIALVAR_FOURCC( ch0, ch1, ch2, ch3 ) \
	( (uint32) ( ch0 ) | ( (uint32) ( ch1 ) << 8 ) | ( (uint32) ( ch2 ) << 16 ) | ( (uint32) ( ch3 ) << 24 ) )

// This fourcc is reserved.
#define FOURCC_UNKNOWN MAKE_MATERIALVAR_FOURCC( 'U', 'N', 'K', 'N' )


//-----------------------------------------------------------------------------
// Various material var types
//-----------------------------------------------------------------------------
enum MaterialVarType_t {
	MATERIAL_VAR_TYPE_FLOAT = 0,
	MATERIAL_VAR_TYPE_STRING,
	MATERIAL_VAR_TYPE_VECTOR,
	MATERIAL_VAR_TYPE_TEXTURE,
	MATERIAL_VAR_TYPE_INT,
	MATERIAL_VAR_TYPE_FOURCC,
	MATERIAL_VAR_TYPE_UNDEFINED,
	MATERIAL_VAR_TYPE_MATRIX,
	MATERIAL_VAR_TYPE_MATERIAL,
};

using MaterialVarSym_t = uint16;

class IMaterialVar {
public:
	using FourCC = uint32;

protected:
	// base data and accessors
	char* m_pStringVal;
	int m_intVal;
	Vector4D m_VecVal;

	// member data. total = 4 bytes
	uint8 m_Type : 4;
	uint8 m_nNumVectorComps : 3;
	uint8 m_bFakeMaterialVar : 1;
	uint8 m_nTempIndex;
	CUtlSymbol m_Name;

public:
	// class factory methods
	static IMaterialVar* Create( IMaterial* pMaterial, char const* pKey, VMatrix const& matrix );
	static IMaterialVar* Create( IMaterial* pMaterial, char const* pKey, char const* pVal );
	static IMaterialVar* Create( IMaterial* pMaterial, char const* pKey, float* pVal, int numcomps );
	static IMaterialVar* Create( IMaterial* pMaterial, char const* pKey, float val );
	static IMaterialVar* Create( IMaterial* pMaterial, char const* pKey, int val );
	static IMaterialVar* Create( IMaterial* pMaterial, char const* pKey );
	static void Destroy( IMaterialVar* pVar );
	static MaterialVarSym_t GetSymbol( char const* pName );
	static MaterialVarSym_t FindSymbol( char const* pName );
	static bool SymbolMatches( char const* pName, MaterialVarSym_t symbol );
	static void DeleteUnreferencedTextures( bool enable );

	virtual ITexture* GetTextureValue() = 0;

	[[nodiscard]]
	virtual char const* GetName() const = 0;
	[[nodiscard]]
	virtual MaterialVarSym_t GetNameAsSymbol() const = 0;

	virtual void SetFloatValue( float val ) = 0;

	virtual void SetIntValue( int val ) = 0;

	virtual void SetStringValue( char const* val ) = 0;
	[[nodiscard]]
	virtual char const* GetStringValue() const = 0;

	// Use FourCC values to pass app-defined data structures between
	// the proxy and the shader. The shader should ignore the data if
	// its FourCC type not correct.
	virtual void SetFourCCValue( FourCC type, void* pData ) = 0;
	virtual void GetFourCCValue( FourCC* type, void** ppData ) = 0;

	// Vec (dim 2-4)
	virtual void SetVecValue( float const* val, int numcomps ) = 0;
	virtual void SetVecValue( float x, float y ) = 0;
	virtual void SetVecValue( float x, float y, float z ) = 0;
	virtual void SetVecValue( float x, float y, float z, float w ) = 0;
	virtual void GetLinearVecValue( float* val, int numcomps ) const = 0;

	// revisit: is this a good interface for textures?
	virtual void SetTextureValue( ITexture* ) = 0;

	virtual IMaterial* GetMaterialValue() = 0;
	virtual void SetMaterialValue( IMaterial* ) = 0;

	[[nodiscard]]
	virtual bool IsDefined() const = 0;
	virtual void SetUndefined() = 0;

	// Matrix
	virtual void SetMatrixValue( VMatrix const& matrix ) = 0;
	virtual const VMatrix& GetMatrixValue() = 0;
	[[nodiscard]]
	virtual bool MatrixIsIdentity() const = 0;

	// Copy....
	virtual void CopyFrom( IMaterialVar* pMaterialVar ) = 0;

	virtual void SetValueAutodetectType( char const* val ) = 0;

	virtual IMaterial* GetOwningMaterial() = 0;

	//set just 1 component
	virtual void SetVecComponentValue( float fVal, int nComponent ) = 0;

protected:
	[[nodiscard]]
	virtual int GetIntValueInternal() const = 0;
	[[nodiscard]]
	virtual float GetFloatValueInternal() const = 0;
	[[nodiscard]]
	virtual float const* GetVecValueInternal() const = 0;
	virtual void GetVecValueInternal( float* val, int numcomps ) const = 0;
	[[nodiscard]]
	virtual int VectorSizeInternal() const = 0;

public:
	ALWAYS_INLINE MaterialVarType_t GetType() const {
		return static_cast<MaterialVarType_t>( m_Type );
	}

	ALWAYS_INLINE bool IsTexture() const {
		return m_Type == MATERIAL_VAR_TYPE_TEXTURE;
	}

	ALWAYS_INLINE operator ITexture*() {
		return GetTextureValue();
	}

	// NOTE: Fast methods should only be called in thread-safe situations
	ALWAYS_INLINE int GetIntValueFast() const {
		// Set methods for float and vector update this
		return m_intVal;
	}

	ALWAYS_INLINE float GetFloatValueFast() const {
		return m_VecVal[ 0 ];
	}

	ALWAYS_INLINE float const* GetVecValueFast() const {
		return m_VecVal.Base();
	}

	ALWAYS_INLINE void GetVecValueFast( float* val, int numcomps ) const {
		Assert( ( numcomps > 0 ) && ( numcomps <= 4 ) );
		for ( int i = 0; i < numcomps; i++ ) {
			val[ i ] = m_VecVal[ i ];
		}
	}

	ALWAYS_INLINE int VectorSizeFast() const {
		return m_nNumVectorComps;
	}

	#ifdef FAST_MATERIALVAR_ACCESS
		ALWAYS_INLINE int GetIntValue() const {
			return GetIntValueFast();
		}

		ALWAYS_INLINE float GetFloatValue() const {
			return GetFloatValueFast();
		}

		ALWAYS_INLINE float const* GetVecValue() const {
			return GetVecValueFast();
		}

		ALWAYS_INLINE void GetVecValue( float* val, int numcomps ) const {
			GetVecValueFast( val, numcomps );
		}

		ALWAYS_INLINE int VectorSize() const {
			return VectorSizeFast();
		}
	#else// !FAST_MATERIALVAR_ACCESS
		ALWAYS_INLINE int GetIntValue() const {
			return GetIntValueInternal();
		}

		ALWAYS_INLINE float GetFloatValue() const {
			return GetFloatValueInternal();
		}

		ALWAYS_INLINE float const* GetVecValue() const {
			return GetVecValueInternal();
		}

		ALWAYS_INLINE void GetVecValue( float* val, int numcomps ) const {
			GetVecValueInternal( val, numcomps );
		}

		ALWAYS_INLINE int VectorSize() const {
			return VectorSizeInternal();
		}
	#endif

private:
	ALWAYS_INLINE void SetTempIndex( int nIndex ) {
		m_nTempIndex = nIndex;
	}

	friend void EnableThreadedMaterialVarAccess( bool bEnable, IMaterialVar** ppParams, int nVarCount );
};
