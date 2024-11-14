//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================


#include "fgdlib/inputoutput.h"
#include "tier0/dbg.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


struct TypeMap_t {
	InputOutputType_t eType;  // The enumeration of this type.
	const char* pszName;      // The name of this type.
};


const char* CClassInputOutputBase::g_pszEmpty = "";


//-----------------------------------------------------------------------------
// Maps type names to type enums for inputs and outputs.
//-----------------------------------------------------------------------------
static TypeMap_t TypeMap[] {
	{ iotVoid, "void" },
	{ iotInt, "integer" },
	{ iotBool, "bool" },
	{ iotString, "string" },
	{ iotFloat, "float" },
	{ iotVector, "vector" },
	{ iotEHandle, "target_destination" },
	{ iotColor, "color255" },
	{ iotEHandle, "ehandle" },  // for backwards compatibility
};


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CClassInputOutputBase::CClassInputOutputBase() {
	m_eType = iotInvalid;
	m_pszDescription = nullptr;
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : pszName -
//			eType -
//-----------------------------------------------------------------------------
CClassInputOutputBase::CClassInputOutputBase( const char* pszName, InputOutputType_t eType ) {
	m_pszDescription = nullptr;
}


//-----------------------------------------------------------------------------
// Purpose: Destructor.
//-----------------------------------------------------------------------------
CClassInputOutputBase::~CClassInputOutputBase() {
	delete m_pszDescription;
	m_pszDescription = nullptr;
}


//-----------------------------------------------------------------------------
// Purpose: Returns a string representing the type of this I/O, eg. "integer".
//-----------------------------------------------------------------------------
const char* CClassInputOutputBase::GetTypeText() const {
	for ( const auto& type : TypeMap ) {
		if ( type.eType == m_eType ) {
			return type.pszName;
		}
	}

	return "unknown";
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : szType -
// Output : InputOutputType_t
//-----------------------------------------------------------------------------
InputOutputType_t CClassInputOutputBase::SetType( const char* szType ) {
	for ( const auto& type : TypeMap ) {
		if ( not stricmp( type.pszName, szType ) ) {
			m_eType = type.eType;
			return m_eType;
		}
	}

	return iotInvalid;
}


//-----------------------------------------------------------------------------
// Purpose: Assignment operator.
//-----------------------------------------------------------------------------
CClassInputOutputBase& CClassInputOutputBase::operator=( const CClassInputOutputBase& pOther ) {
	if ( &pOther == this ) {
		return *this;
	}
	strcpy( m_szName, pOther.m_szName );
	m_eType = pOther.m_eType;

	//
	// Copy the description.
	//
	delete m_pszDescription;
	if ( pOther.m_pszDescription != nullptr ) {
		m_pszDescription = new char[strlen( pOther.m_pszDescription ) + 1];
		strcpy( m_pszDescription, pOther.m_pszDescription );
	} else {
		m_pszDescription = nullptr;
	}

	return *this;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CClassInput::CClassInput() { }


//-----------------------------------------------------------------------------
// Purpose:
// Input  : pszName -
//			eType -
//-----------------------------------------------------------------------------
CClassInput::CClassInput( const char* pszName, const InputOutputType_t eType )
	: CClassInputOutputBase( pszName, eType ) { }


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CClassOutput::CClassOutput() = default;


//-----------------------------------------------------------------------------
// Purpose:
// Input  : pszName -
//			eType -
//-----------------------------------------------------------------------------
CClassOutput::CClassOutput( const char* pszName, const InputOutputType_t eType )
	: CClassInputOutputBase( pszName, eType ) { }
