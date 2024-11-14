//========= Copyright Valve Corporation, All rights reserved. ============//
//
//=============================================================================
#include "fgdlib/gdvar.h"
#include "fgdlib/fgdlib.h"
#include "fgdlib/gamedata.h"
#include "fgdlib/wckeyvalues.h"
#include <cstdlib>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


struct TypeMap_t {
	GDIV_TYPE eType;      // The enumeration of this type.
	const char* pszName;  // The name of this type.
	trtoken_t eStoreAs;   // How this type is stored (STRING, INTEGER, etc).
};


//-----------------------------------------------------------------------------
// Maps type names to type enums and parsing logic for values.
//-----------------------------------------------------------------------------
static TypeMap_t TypeMap[] {
	{ ivAngle, "angle", STRING },
	{ ivChoices, "choices", STRING },
	{ ivColor1, "color1", STRING },
	{ ivColor255, "color255", STRING },
	{ ivDecal, "decal", STRING },
	{ ivFlags, "flags", INTEGER },
	{ ivInteger, "integer", INTEGER },
	{ ivSound, "sound", STRING },
	{ ivSprite, "sprite", STRING },
	{ ivString, "string", STRING },
	{ ivStudioModel, "studio", STRING },
	{ ivTargetDest, "target_destination", STRING },
	{ ivTargetSrc, "target_source", STRING },
	{ ivTargetNameOrClass, "target_name_or_class", STRING },  // Another version of `target_destination` that accepts class names
	{ ivVector, "vector", STRING },
	{ ivNPCClass, "npcclass", STRING },
	{ ivFilterClass, "filterclass", STRING },
	{ ivFloat, "float", STRING },
	{ ivMaterial, "material", STRING },
	{ ivScene, "scene", STRING },
	{ ivSide, "side", STRING },
	{ ivSideList, "sidelist", STRING },
	{ ivOrigin, "origin", STRING },
	{ ivAxis, "axis", STRING },
	{ ivVecLine, "vecline", STRING },
	{ ivPointEntityClass, "pointentityclass", STRING },
	{ ivNodeDest, "node_dest", INTEGER },
	{ ivInstanceFile, "instance_file", STRING },
	{ ivAngleNegativePitch, "angle_negative_pitch", STRING },
	{ ivInstanceVariable, "instance_variable", STRING },
	{ ivInstanceParm, "instance_parm", STRING },
};


const char* GDinputvariable::m_pszEmpty = "";


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
GDinputvariable::GDinputvariable() {
	m_szDefault[0] = 0;
	m_nDefault = 0;
	m_szValue[0] = 0;
	m_bReportable = false;
	m_bReadOnly = false;
	m_pszDescription = nullptr;
}


//-----------------------------------------------------------------------------
// Purpose: construct generally used for creating a temp instance parm type
// Input  : szType - the textual type of this variable
//			szName - the name description of this variable
//-----------------------------------------------------------------------------
GDinputvariable::GDinputvariable( const char* szType, const char* szName ) {
	m_szDefault[0] = 0;
	m_nDefault = 0;
	m_szValue[0] = 0;
	m_bReportable = false;
	m_bReadOnly = false;
	m_pszDescription = nullptr;

	m_eType = GetTypeFromToken( szType );
	strcpy( m_szName, szName );
}


//-----------------------------------------------------------------------------
// Purpose: Destructor.
//-----------------------------------------------------------------------------
GDinputvariable::~GDinputvariable() {
	delete[] m_pszDescription;
	m_Items.RemoveAll();
}


//-----------------------------------------------------------------------------
// Purpose: Implements the copy operator.
//-----------------------------------------------------------------------------
GDinputvariable& GDinputvariable::operator=( const GDinputvariable& pOther ) {
	if ( &pOther == this ) {
		return *this;
	}
	m_eType = pOther.GetType();
	strcpy( m_szName, pOther.m_szName );
	strcpy( m_szLongName, pOther.m_szLongName );
	strcpy( m_szDefault, pOther.m_szDefault );

	//
	// Copy the description.
	//
	delete[] m_pszDescription;
	if ( pOther.m_pszDescription != nullptr ) {
		m_pszDescription = new char[strlen( pOther.m_pszDescription ) + 1];
		strcpy( m_pszDescription, pOther.m_pszDescription );
	} else {
		m_pszDescription = nullptr;
	}

	m_nDefault = pOther.m_nDefault;
	m_bReportable = pOther.m_bReportable;
	m_bReadOnly = pOther.m_bReadOnly;

	m_Items.RemoveAll();

	const int nCount = pOther.m_Items.Count();
	for ( int i = 0; i < nCount; i += 1 ) {
		m_Items.AddToTail( pOther.m_Items.Element( i ) );
	}

	return *this;
}


//-----------------------------------------------------------------------------
// Purpose: Returns the storage format of a given variable type.
// Input  : pszToken - Sting containing the token.
// Output : GDIV_TYPE corresponding to the token in the string, ivBadType if the
//			string does not correspond to a valid type.
//-----------------------------------------------------------------------------
trtoken_t GDinputvariable::GetStoreAsFromType( GDIV_TYPE eType ) {
	for ( int i = 0; i < std::size( TypeMap ); i += 1 ) {
		if ( TypeMap[i].eType == eType ) {
			return TypeMap[i].eStoreAs;
		}
	}

	Assert( false );
	return STRING;
}


//-----------------------------------------------------------------------------
// Purpose: Returns the enumerated type of a string token.
// Input  : pszToken - Sting containing the token.
// Output : GDIV_TYPE corresponding to the token in the string, ivBadType if the
//			string does not correspond to a valid type.
//-----------------------------------------------------------------------------
GDIV_TYPE GDinputvariable::GetTypeFromToken( const char* pszToken ) {
	for ( int i = 0; i < std::size( TypeMap ); i += 1 ) {
		if ( IsToken( pszToken, TypeMap[i].pszName ) ) {
			return TypeMap[i].eType;
		}
	}

	return ivBadType;
}


//-----------------------------------------------------------------------------
// Purpose: Returns a string representing the type of this variable, eg. "integer".
//-----------------------------------------------------------------------------
const char* GDinputvariable::GetTypeText() const {
	for ( int i = 0; i < std::size( TypeMap ); i += 1 ) {
		if ( TypeMap[i].eType == m_eType ) {
			return TypeMap[i].pszName;
		}
	}

	return "unknown";
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : tr -
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
BOOL GDinputvariable::InitFromTokens( TokenReader& tr ) {
	char szToken[128];

	if ( not GDGetToken( tr, m_szName, sizeof( m_szName ), IDENT ) ) {
		return false;
	}

	if ( not GDSkipToken( tr, OPERATOR, "(" ) ) {
		return false;
	}

	// check for "reportable" marker
	trtoken_t ttype = tr.NextToken( szToken, sizeof( szToken ) );
	if ( ttype == OPERATOR ) {
		if ( not strcmp( szToken, "*" ) ) {
			m_bReportable = true;
		}
	} else {
		tr.Stuff( ttype, szToken );
	}

	// get type
	if ( not GDGetToken( tr, szToken, sizeof( szToken ), IDENT ) ) {
		return false;
	}

	if ( not GDSkipToken( tr, OPERATOR, ")" ) ) {
		return false;
	}

	//
	// Check for known variable types.
	//
	m_eType = GetTypeFromToken( szToken );
	if ( m_eType == ivBadType ) {
		GDError( tr, "'%s' is not a valid variable type", szToken );
		return false;
	}

	//
	// Look ahead at the next token.
	//
	ttype = tr.PeekTokenType( szToken, sizeof( szToken ) );

	//
	// Check for the "readonly" specifier.
	//
	if ( ttype == IDENT and IsToken( szToken, "readonly" ) ) {
		tr.NextToken( szToken, sizeof( szToken ) );
		m_bReadOnly = true;

		//
		// Look ahead at the next token.
		//
		ttype = tr.PeekTokenType( szToken, sizeof( szToken ) );
	}

	//
	// Check for the ':' indicating a long name.
	//
	if ( ttype == OPERATOR and IsToken( szToken, ":" ) ) {
		//
		// Eat the ':'.
		//
		tr.NextToken( szToken, sizeof( szToken ) );

		if ( m_eType == ivFlags ) {
			GDError( tr, "flag sets do not have long names" );
			return false;
		}

		//
		// Get the long name.
		//
		if ( not GDGetToken( tr, m_szLongName, sizeof( m_szLongName ), STRING ) ) {
			return ( false );
		}

		//
		// Look ahead at the next token.
		//
		ttype = tr.PeekTokenType( szToken, sizeof( szToken ) );

		//
		// Check for the ':' indicating a default value.
		//
		if ( ttype == OPERATOR and IsToken( szToken, ":" ) ) {
			//
			// Eat the ':'.
			//
			tr.NextToken( szToken, sizeof( szToken ) );

			//
			// Look ahead at the next token.
			//
			ttype = tr.PeekTokenType( szToken, sizeof( szToken ) );
			if ( ttype == OPERATOR and IsToken( szToken, ":" ) ) {
				//
				// No default value provided, skip to the description.
				//
			} else {
				//
				// Determine how to parse the default value. If this is a choices field, the
				// default could either be a string or an integer, so we must look ahead and
				// use whichever is there.
				//
				const trtoken_t eStoreAs = GetStoreAsFromType( m_eType );

				if ( eStoreAs == STRING ) {
					if ( not GDGetToken( tr, m_szDefault, sizeof( m_szDefault ), STRING ) ) {
						return false;
					}
				} else if ( eStoreAs == INTEGER ) {
					if ( not GDGetToken( tr, szToken, sizeof( szToken ), INTEGER ) ) {
						return false;
					}

					m_nDefault = atoi( szToken );
				}

				//
				// Look ahead at the next token.
				//
				ttype = tr.PeekTokenType( szToken, sizeof( szToken ) );
			}
		}

		//
		// Check for the ':' indicating a description.
		//
		if ( ttype == OPERATOR and IsToken( szToken, ":" ) ) {
			//
			// Eat the ':'.
			//
			tr.NextToken( szToken, sizeof( szToken ) );

			//
			// Read the description.
			//

			// If we've already read a description then free it to avoid memory leaks.
			if ( m_pszDescription ) {
				delete[] m_pszDescription;
				m_pszDescription = nullptr;
			}
			if ( not GDGetTokenDynamic( tr, &m_pszDescription, STRING ) ) {
				return false;
			}

			//
			// Look ahead at the next token.
			//
			ttype = tr.PeekTokenType( szToken, sizeof( szToken ) );
		}
	} else {
		//
		// Default long name is short name.
		//
		strcpy( m_szLongName, m_szName );
	}

	//
	// Check for the ']' indicating the end of the class definition.
	//
	if ( ( ttype == OPERATOR and IsToken( szToken, "]" ) ) or ttype != OPERATOR ) {
		if ( m_eType == ivFlags or m_eType == ivChoices ) {
			//
			// Can't define a flags or choices variable without providing any flags or choices.
			//
			GDError( tr, "no %s specified", m_eType == ivFlags ? "flags" : "choices" );
			return false;
		}
		return true;
	}

	if ( not GDSkipToken( tr, OPERATOR, "=" ) ) {
		return false;
	}

	if ( m_eType != ivFlags and m_eType != ivChoices ) {
		GDError( tr, "didn't expect '=' here" );
		return ( false );
	}

	// should be '[' to start flags/choices
	if ( not GDSkipToken( tr, OPERATOR, "[" ) ) {
		return false;
	}

	// get flags?
	if ( m_eType == ivFlags ) {
		GDIVITEM ivi{};

		while ( true ) {
			ttype = tr.PeekTokenType();
			if ( ttype != INTEGER ) {
				break;
			}

			// store bitflag value
			GDGetToken( tr, szToken, sizeof( szToken ), INTEGER );
			sscanf( szToken, "%lu", &ivi.iValue );

			// colon..
			if ( not GDSkipToken( tr, OPERATOR, ":" ) ) {
				return false;
			}

			// get description
			if ( not GDGetToken( tr, szToken, sizeof( szToken ), STRING ) ) {
				return false;
			}
			strcpy( ivi.szCaption, szToken );

			// colon..
			if ( not GDSkipToken( tr, OPERATOR, ":" ) ) {
				return false;
			}

			// get default setting
			if ( not GDGetToken( tr, szToken, sizeof( szToken ), INTEGER ) ) {
				return false;
			}
			ivi.bDefault = atoi( szToken ) ? true : false;

			// add item to array of items
			m_Items.AddToTail( ivi );
		}

		// Set the default value.
		uint32 nDefault = 0;
		for ( int i = 0; i < m_Items.Count(); i += 1 ) {
			if ( m_Items[i].bDefault ) {
				nDefault |= m_Items[i].iValue;
			}
		}
		m_nDefault = static_cast<int>( nDefault );
		Q_snprintf( m_szDefault, sizeof( m_szDefault ), "%d", m_nDefault );
	} else if ( m_eType == ivChoices ) {
		GDIVITEM ivi{};

		while ( true ) {
			ttype = tr.PeekTokenType();
			if ( ttype != INTEGER and ttype != STRING ) {
				break;
			}

			// store choice value
			GDGetToken( tr, szToken, sizeof( szToken ), ttype );
			ivi.iValue = 0;
			strcpy( ivi.szValue, szToken );

			// colon
			if ( not GDSkipToken( tr, OPERATOR, ":" ) ) {
				return false;
			}

			// get description
			if ( not GDGetToken( tr, szToken, sizeof( szToken ), STRING ) ) {
				return false;
			}

			strcpy( ivi.szCaption, szToken );

			m_Items.AddToTail( ivi );
		}
	}

	if ( not GDSkipToken( tr, OPERATOR, "]" ) ) {
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Decodes a key value from a string.
// Input  : pkv - Pointer to the key value object containing string encoded value.
//-----------------------------------------------------------------------------
void GDinputvariable::FromKeyValue( MDkeyvalue* pkv ) {
	const trtoken_t eStoreAs = GetStoreAsFromType( m_eType );

	if ( eStoreAs == STRING ) {
		strcpy( m_szValue, pkv->szValue );
	} else if ( eStoreAs == INTEGER ) {
		m_nValue = atoi( pkv->szValue );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Determines whether the given flag is set (assuming this is an ivFlags).
// Input  : uCheck - Flag to check.
// Output : Returns true if flag is set, false if not.
//-----------------------------------------------------------------------------
BOOL GDinputvariable::IsFlagSet( unsigned int uCheck ) const {
	Assert( m_eType == ivFlags );
	return ( static_cast<uint32>( m_nValue ) & uCheck ) == uCheck;
}


//-----------------------------------------------------------------------------
// Purpose: Combines the flags or choices items from another variable into our
//			list of flags or choices. Ours take priority if collisions occur.
// Input  : Other - The variable whose items are being merged with ours.
//-----------------------------------------------------------------------------
void GDinputvariable::Merge( GDinputvariable& Other ) {
	//
	// Only valid if we are of the same type.
	//
	if ( Other.GetType() != GetType() ) {
		return;
	}

	//
	// Add Other's items to this ONLY if there is no same-value entry
	// for a specific item.
	//
	bool bFound = false;
	const int nOurItems = m_Items.Count();
	for ( int i = 0; i < Other.m_Items.Count(); i += 1 ) {
		GDIVITEM& TheirItem = Other.m_Items[i];
		for ( int j = 0; j < nOurItems; j += 1 ) {
			const GDIVITEM& OurItem = m_Items[j];
			if ( TheirItem.iValue == OurItem.iValue ) {
				bFound = true;
				break;
			}
		}

		if ( not bFound ) {
			//
			// Not found in our list - add their item to our list.
			//
			m_Items.AddToTail( TheirItem );
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Determines whether the given flag is set (assuming this is an ivFlags).
// Input  : uFlags - Flags to set.
//			bSet - true to set the flags, false to clear them.
//-----------------------------------------------------------------------------
void GDinputvariable::SetFlag( unsigned int uFlags, BOOL bSet ) {
	Assert( m_eType == ivFlags );
	if ( bSet ) {
		m_nValue |= uFlags;
	} else {
		m_nValue &= ~uFlags;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Sets this keyvalue to its default value.
//-----------------------------------------------------------------------------
void GDinputvariable::ResetDefaults() {
	if ( m_eType == ivFlags ) {
		m_nValue = 0;

		//
		// Run thru flags and set any default flags.
		//
		const int nCount = m_Items.Count();
		for ( int i = 0; i < nCount; i += 1 ) {
			if ( m_Items[i].bDefault ) {
				m_nValue |= GetFlagMask( i );
			}
		}
	} else {
		m_nValue = m_nDefault;
		strcpy( m_szValue, m_szDefault );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Encodes a key value as a string.
// Input  : pkv - Pointer to the key value object to receive the encoded string.
//-----------------------------------------------------------------------------
void GDinputvariable::ToKeyValue( MDkeyvalue* pkv ) {
	strcpy( pkv->szKey, m_szName );

	const trtoken_t eStoreAs = GetStoreAsFromType( m_eType );

	if ( eStoreAs == STRING ) {
		strcpy( pkv->szValue, m_szValue );
	} else if ( eStoreAs == INTEGER ) {
		sprintf( pkv->szValue, "%d", m_nValue );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Returns the description string that corresponds to a value string
//			for a choices list.
// Input  : pszString - The choices value string.
// Output : Returns the description string.
//-----------------------------------------------------------------------------
const char* GDinputvariable::ItemStringForValue( const char* szValue ) {
	const int nCount = m_Items.Count();
	for ( int i = 0; i < nCount; i += 1 ) {
		if ( !stricmp( m_Items[i].szValue, szValue ) ) {
			return m_Items[i].szCaption;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------
// Purpose: Returns the value string that corresponds to a description string
//			for a choices list.
// Input  : pszString - The choices description string.
// Output : Returns the value string.
//-----------------------------------------------------------------------------
const char* GDinputvariable::ItemValueForString( const char* szString ) {
	const int nCount = m_Items.Count();
	for ( int i = 0; i < nCount; i += 1 ) {
		if ( !strcmpi( m_Items[i].szCaption, szString ) ) {
			return m_Items[i].szValue;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------
// Purpose: this function will let you iterate through the text names of the variable types
// Input  : eType - the type to get the text of
// Output : returns the textual name
//-----------------------------------------------------------------------------
const char* GDinputvariable::GetVarTypeName( const GDIV_TYPE eType ) {
	return TypeMap[eType].pszName;
}
