//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================
#include "fgdlib/gdclass.h"
#include "fgdlib/gamedata.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
GDclass::GDclass() {
	for ( int i = 0; i < 3; i += 1 ) {
		m_bmins[ i ] = -8;
		m_bmaxs[ i ] = 8;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Destructor. Frees variable and helper lists.
//-----------------------------------------------------------------------------
GDclass::~GDclass() {
	//
	// Free variables.
	//
	int count = m_Variables.Count();
	for ( int i = 0; i < count; i += 1 ) {
		const GDinputvariable* pvi = m_Variables.Element( i );
		delete pvi;
	}
	m_Variables.RemoveAll();

	//
	// Free helpers.
	//
	count = m_Helpers.Count();
	for ( int i = 0; i < count; i += 1 ) {
		const CHelperInfo* helper = m_Helpers.Element( i );
		delete helper;
	}
	m_Helpers.RemoveAll();

	//
	// Free inputs.
	//
	count = m_Inputs.Count();
	for ( int i = 0; i < count; i += 1 ) {
		const CClassInput* input = m_Inputs.Element( i );
		delete input;
	}
	m_Inputs.RemoveAll();

	//
	// Free outputs.
	//
	count = m_Outputs.Count();
	for ( int i = 0; i < count; i += 1 ) {
		const CClassOutput* output = m_Outputs.Element( i );
		delete output;
	}
	m_Outputs.RemoveAll();

	delete m_pszDescription;
}


//-----------------------------------------------------------------------------
// Purpose: Adds the base class's variables to our variable list. Acquires the
//			base class's bounding box and color, if any.
// Input  : pszBase - Name of base class to add.
//-----------------------------------------------------------------------------
void GDclass::AddBase( GDclass* pBase ) {
	int iBaseIndex;
	Parent->ClassForName( pBase->GetName(), &iBaseIndex );

	//
	// Add variables from base - update variable table
	//
	for ( int i = 0; i < pBase->GetVariableCount(); i += 1 ) {
		GDinputvariable* pVar = pBase->GetVariableAt( i );
		AddVariable( pVar, pBase, iBaseIndex, i );
	}

	//
	// Add inputs from the base.
	// UNDONE: need to use references to inputs & outputs to conserve memory
	//
	int nCount = pBase->GetInputCount();
	for ( int i = 0; i < nCount; i += 1 ) {
		const CClassInput* pInput = pBase->GetInput( i );

		auto* pNew = new CClassInput;
		*pNew = *pInput;
		AddInput( pNew );
	}

	//
	// Add outputs from the base.
	//
	nCount = pBase->GetOutputCount();
	for ( int i = 0; i < nCount; i += 1 ) {
		const CClassOutput* pOutput = pBase->GetOutput( i );

		auto* pNew = new CClassOutput;
		*pNew = *pOutput;
		AddOutput( pNew );
	}

	//
	// If we don't have a bounding box, try to get the base's box.
	//
	if ( not m_bGotSize ) {
		if ( pBase->GetBoundBox( m_bmins, m_bmaxs ) ) {
			m_bGotSize = true;
		}
	}

	//
	// If we don't have a color, use the base's color.
	//
	if ( not m_bGotColor ) {
		m_rgbColor = pBase->GetColor();
		m_bGotColor = true;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Adds the given GDInputVariable to this GDClass's list of variables.
// Input  : pVar -
//			pBase -
//			iBaseIndex -
//			iVarIndex -
// Output : Returns true if the pVar pointer was copied directly into this GDClass,
//			false if not. If this function returns true, pVar should not be
//			deleted by the caller.
//-----------------------------------------------------------------------------
BOOL GDclass::AddVariable( GDinputvariable* pVar, GDclass* pBase, int pBaseIndex, int pVarIndex ) {
	int iThisIndex;
	GDinputvariable* pThisVar = VarForName( pVar->GetName(), &iThisIndex );

	//
	// Check to see if we are overriding an existing variable definition.
	//
	if ( pThisVar != nullptr ) {
		//
		// Same name, different type. Flag this as an error.
		//
		if ( pThisVar->GetType() != pVar->GetType() ) {
			return false;
		}

		GDinputvariable* pAddVar;
		bool bReturn;

		//
		// Check to see if we need to combine a choices/flags array.
		//
		if ( pVar->GetType() == ivFlags or pVar->GetType() == ivChoices ) {
			//
			// Combine two variables' flags into a new variable. Add the new
			// variable to the local variable list and modify the old variable's
			// position in our variable map to reflect the new local variable.
			// This way, we can have multiple inheritance.
			//
			auto* pNewVar = new GDinputvariable;
			*pNewVar = *pVar;
			pNewVar->Merge( *pThisVar );

			pAddVar = pNewVar;
			bReturn = false;
		} else {
			pAddVar = pVar;
			bReturn = true;
		}

		if ( m_VariableMap[ iThisIndex ][ 0 ] == -1 ) {
			//
			// "pThisVar" is a leaf variable - we can remove since it is overridden.
			//
			int nIndex = m_Variables.Find( pThisVar );
			Assert( nIndex != -1 );
			delete pThisVar;

			m_Variables.Element( nIndex ) = pAddVar;

			//
			// No need to modify variable map - we just replaced
			// the pointer in the local variable list.
			//
		} else {
			//
			// "pThisVar" was declared in a base class - we can replace the reference in
			// our variable map with the new variable.
			//
			m_VariableMap[ iThisIndex ][0] = static_cast<int16>( pBaseIndex );

			if ( pBaseIndex == -1 ) {
				m_Variables.AddToTail( pAddVar );
				m_VariableMap[ iThisIndex ][1] = static_cast<int16>( m_Variables.Count() - 1 );
			} else {
				m_VariableMap[ iThisIndex ][1] = static_cast<int16>( pVarIndex );
			}
		}

		return bReturn;
	}

	//
	// New variable.
	//
	if ( pBaseIndex == -1 ) {
		//
		// Variable declared in the leaf class definition - add it to the list.
		//
		m_Variables.AddToTail( pVar );
	}

	//
	// Too many variables already declared in this class definition - abort.
	//
	if ( m_nVariables == GD_MAX_VARIABLES ) {
		//CUtlString str;
		//str.Format("Too many gamedata variables for class \"%s\"", m_szName);
		//AfxMessageBox(str);

		return false;
	}

	//
	// Add the variable to our list.
	//
	m_VariableMap[ m_nVariables ][0] = static_cast<int16>( pBaseIndex );
	m_VariableMap[ m_nVariables ][1] = static_cast<int16>( pVarIndex );
	m_nVariables += 1;

	//
	// We added the pointer to our list of items (see Variables.AddToTail, above) so
	// we must return true here.
	//
	return true;
}


//-----------------------------------------------------------------------------
// Finds an input by name.
//-----------------------------------------------------------------------------
CClassInput* GDclass::FindInput( const char* szName ) {
	const int count = GetInputCount();
	for ( int i = 0; i < count; i += 1 ) {
		CClassInput* pInput = GetInput( i );
		if ( not stricmp( pInput->GetName(), szName ) ) {
			return pInput;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------
// Finds an output by name.
//-----------------------------------------------------------------------------
CClassOutput* GDclass::FindOutput( const char* szName ) {
	const int count = GetOutputCount();
	for ( int i = 0; i < count; i += 1 ) {
		CClassOutput* pOutput = GetOutput( i );
		if ( not stricmp( pOutput->GetName(), szName ) ) {
			return pOutput;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------
// Purpose: Gets the mins and maxs of the class's bounding box as read from the
//			FGD file. This controls the onscreen representation of any entities
//			derived from this class.
// Input  : pfMins - Receives minimum X, Y, and Z coordinates for the class.
//			pfMaxs - Receives maximum X, Y, and Z coordinates for the class.
// Output : Returns true if this class has a specified bounding box, false if not.
//-----------------------------------------------------------------------------
BOOL GDclass::GetBoundBox( Vector& pMins, Vector& pMaxs ) {
	if ( m_bGotSize ) {
		pMins[ 0 ] = m_bmins[ 0 ];
		pMins[ 1 ] = m_bmins[ 1 ];
		pMins[ 2 ] = m_bmins[ 2 ];

		pMaxs[ 0 ] = m_bmaxs[ 0 ];
		pMaxs[ 1 ] = m_bmaxs[ 1 ];
		pMaxs[ 2 ] = m_bmaxs[ 2 ];
	}

	return m_bGotSize;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CHelperInfo* GDclass::GetHelper( const int pIndex ) {
	return m_Helpers.Element( pIndex );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CClassInput* GDclass::GetInput( const int pIndex ) {
	return m_Inputs.Element( pIndex );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CClassOutput* GDclass::GetOutput( const int pIndex ) {
	return m_Outputs.Element( pIndex );
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : tr -
//			pGD -
// Output : Returns true if worth continuing, false otherwise.
//-----------------------------------------------------------------------------
BOOL GDclass::InitFromTokens( TokenReader& tr, GameData* pGD ) {
	Parent = pGD;

	//
	// Initialize VariableMap
	//
	for ( int i = 0; i < GD_MAX_VARIABLES; i += 1 ) {
		m_VariableMap[ i ][ 0 ] = -1;
		m_VariableMap[ i ][ 1 ] = -1;
	}

	//
	// Parse all specifiers (base, size, color, etc.)
	//
	if ( not ParseSpecifiers( tr ) ) {
		return false;
	}

	//
	// Specifiers should be followed by an "="
	//
	if ( not GDSkipToken( tr, OPERATOR, "=" ) ) {
		return false;
	}

	//
	// Parse the class name.
	//
	if ( not GDGetToken( tr, m_szName, sizeof( m_szName ), IDENT ) ) {
		return false;
	}

	//
	// Check next operator - if ":", we have a description - if "[",
	// we have no description.
	//
	char szToken[ MAX_TOKEN ];
	if ( tr.PeekTokenType( szToken, sizeof( szToken ) ) == OPERATOR and IsToken( szToken, ":" ) ) {
		// Skip ":"
		tr.NextToken( szToken, sizeof( szToken ) );

		//
		// Free any existing description and set the pointer to nullptr so that GDGetToken
		// allocates memory for us.
		//
		delete m_pszDescription;
		m_pszDescription = nullptr;

		// Load description
		if ( not GDGetTokenDynamic( tr, &m_pszDescription, STRING ) ) {
			return false;
		}
	}

	//
	// Opening square brace.
	//
	if ( not GDSkipToken( tr, OPERATOR, "[" ) ) {
		return false;
	}

	//
	// Get class variables.
	//
	if ( not ParseVariables( tr ) ) {
		return false;
	}

	//
	// Closing square brace.
	//
	if ( not GDSkipToken( tr, OPERATOR, "]" ) ) {
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : &tr -
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool GDclass::ParseBase( TokenReader& tr ) {
	while ( true ) {
		char szToken[MAX_TOKEN];

		if ( not GDGetToken( tr, szToken, sizeof( szToken ), IDENT ) ) {
			return false;
		}

		//
		// Find base class in list of classes.
		//
		GDclass* pBase = Parent->ClassForName( szToken );
		if ( pBase == nullptr ) {
			GDError( tr, "undefined base class '%s", szToken );
			return false;
		}

		AddBase( pBase );

		if ( not GDGetToken( tr, szToken, sizeof( szToken ), OPERATOR ) ) {
			return false;
		}

		if ( IsToken( szToken, ")" ) ) {
			break;
		}
		if ( not IsToken( szToken, "," ) ) {
			GDError( tr, "expecting ',' or ')', but found %s", szToken );
			return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : &tr -
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool GDclass::ParseColor( TokenReader& tr ) {
	char szToken[ MAX_TOKEN ];

	//
	// Red.
	//
	if ( not GDGetToken( tr, szToken, sizeof( szToken ), INTEGER ) ) {
		return false;
	}
	const BYTE r = strtol( szToken, nullptr, 10 );

	//
	// Green.
	//
	if ( not GDGetToken( tr, szToken, sizeof( szToken ), INTEGER ) ) {
		return false;
	}
	const BYTE g = strtol( szToken, nullptr, 10 );

	//
	// Blue.
	//
	if ( not GDGetToken( tr, szToken, sizeof( szToken ), INTEGER ) ) {
		return false;
	}
	const BYTE b = strtol( szToken, nullptr, 10 );

	m_rgbColor.r = r;
	m_rgbColor.g = g;
	m_rgbColor.b = b;
	m_rgbColor.a = 0;

	m_bGotColor = true;

	if ( not GDGetToken( tr, szToken, sizeof( szToken ), OPERATOR, ")" ) ) {
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Parses a helper from the FGD file. Helpers are of the following format:
//
//			<helpername>(<parameter 0> <parameter 1> ... <parameter n>)
//
//			When this function is called, the helper name has already been parsed.
// Input  : tr - Tokenreader to use for parsing.
//			pszHelperName - Name of the helper being declared.
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool GDclass::ParseHelper( TokenReader& tr, const char* pHelperName ) {
	auto* pHelper = new CHelperInfo;
	pHelper->SetName( pHelperName );

	bool bCloseParen = false;
	while ( not bCloseParen ) {
		char szToken[MAX_TOKEN];
		const trtoken_t eType = tr.PeekTokenType( szToken, sizeof( szToken ) );

		if ( eType == OPERATOR ) {
			if ( not GDGetToken( tr, szToken, sizeof( szToken ), OPERATOR ) ) {
				delete pHelper;
				return false;
			}

			if ( IsToken( szToken, ")" ) ) {
				bCloseParen = true;
			} else if ( IsToken( szToken, "=" ) ) {
				delete pHelper;
				return false;
			}
		} else {
			if ( not GDGetToken( tr, szToken, sizeof( szToken ), eType ) ) {
				delete pHelper;
				return false;
			}
			pHelper->AddParameter( szToken );
		}
	}

	m_Helpers.AddToTail( pHelper );

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : &tr -
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool GDclass::ParseSize( TokenReader& tr ) {
	char szToken[ MAX_TOKEN ];

	//
	// Mins.
	//
	for ( int i = 0; i < 3; i += 1 ) {
		if ( not GDGetToken( tr, szToken, sizeof( szToken ), INTEGER ) ) {
			return false;
		}

		m_bmins[ i ] = strtof( szToken, nullptr );
	}

	if ( tr.PeekTokenType( szToken, sizeof( szToken ) ) == OPERATOR and IsToken( szToken, "," ) ) {
		//
		// Skip ","
		//
		tr.NextToken( szToken, sizeof( szToken ) );

		//
		// Get maxes.
		//
		for ( int i = 0; i < 3; i += 1 ) {
			if ( not GDGetToken( tr, szToken, sizeof( szToken ), INTEGER ) ) {
				return false;
			}
			m_bmaxs[ i ] = strtof( szToken, nullptr );
		}
	} else {
		//
		// Split mins across origin.
		//
		for ( int i = 0; i < 3; i += 1 ) {
			const float div2 = m_bmins[ i ] / 2;
			m_bmaxs[ i ] = div2;
			m_bmins[ i ] = -div2;
		}
	}

	m_bGotSize = true;

	if ( not GDGetToken( tr, szToken, sizeof( szToken ), OPERATOR, ")" ) ) {
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : &tr -
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool GDclass::ParseSpecifiers( TokenReader& tr ) {
	while ( tr.PeekTokenType() == IDENT ) {
		char szToken[MAX_TOKEN];
		tr.NextToken( szToken, sizeof( szToken ) );

		//
		// Handle specifiers that don't have any parens after them.
		//
		if ( IsToken( szToken, "halfgridsnap" ) ) {
			m_bHalfGridSnap = true;
		} else {
			//
			// Handle specifiers require parens after them.
			//
			if ( not GDSkipToken( tr, OPERATOR, "(" ) ) {
				return false;
			}

			if ( IsToken( szToken, "base" ) ) {
				if ( not ParseBase( tr ) ) {
					return false;
				}
			} else if ( IsToken( szToken, "size" ) ) {
				if ( not ParseSize( tr ) ) {
					return false;
				}
			} else if ( IsToken( szToken, "color" ) ) {
				if ( not ParseColor( tr ) ) {
					return false;
				}
			} else if ( not ParseHelper( tr, szToken ) ) {
				return false;
			}
		}
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Reads an input using a given token reader. If the input is
//			read successfully, the input is added to this class. If not, a
//			parsing failure is returned.
// Input  : tr - Token reader to use for parsing.
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool GDclass::ParseInput( TokenReader& tr ) {
	char szToken[ MAX_TOKEN ];

	if ( not GDGetToken( tr, szToken, sizeof( szToken ), IDENT, "input" ) ) {
		return false;
	}

	auto* pInput = new CClassInput;

	const bool bReturn = ParseInputOutput( tr, pInput );
	if ( bReturn ) {
		AddInput( pInput );
	} else {
		delete pInput;
	}

	return bReturn;
}


//-----------------------------------------------------------------------------
// Purpose: Reads an input or output using a given token reader.
// Input  : tr - Token reader to use for parsing.
//			pInputOutput - Input or output to fill out.
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
// ReSharper disable once CppMemberFunctionMayBeStatic
bool GDclass::ParseInputOutput( TokenReader& tr, CClassInputOutputBase* pInputOutput ) { // NOLINT(*-convert-member-functions-to-static)
	char szToken[ MAX_TOKEN ];

	//
	// Read the name.
	//
	if ( not GDGetToken( tr, szToken, sizeof( szToken ), IDENT ) ) {
		return false;
	}

	pInputOutput->SetName( szToken );

	//
	// Read the type.
	//
	if ( not GDGetToken( tr, szToken, sizeof( szToken ), OPERATOR, "(" ) ) {
		return false;
	}

	if ( not GDGetToken( tr, szToken, sizeof( szToken ), IDENT ) ) {
		return false;
	}

	const InputOutputType_t eType = pInputOutput->SetType( szToken );
	if ( eType == iotInvalid ) {
		GDError( tr, "bad input/output type '%s'", szToken );
		return false;
	}

	if ( not GDGetToken( tr, szToken, sizeof( szToken ), OPERATOR, ")" ) ) {
		return false;
	}

	//
	// Check the next operator - if ':', we have a description.
	//
	if ( tr.PeekTokenType( szToken, sizeof( szToken ) ) == OPERATOR and IsToken( szToken, ":" ) ) {
		//
		// Skip the ":".
		//
		tr.NextToken( szToken, sizeof( szToken ) );

		//
		// Read the description.
		//
		char* pszDescription;
		if ( not GDGetTokenDynamic( tr, &pszDescription, STRING ) ) {
			return false;
		}

		pInputOutput->SetDescription( pszDescription );
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Reads an output using a given token reader. If the output is
//			read successfully, the output is added to this class. If not, a
//			parsing failure is returned.
// Input  : tr - Token reader to use for parsing.
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool GDclass::ParseOutput( TokenReader& tr ) {
	char szToken[ MAX_TOKEN ];

	if ( not GDGetToken( tr, szToken, sizeof( szToken ), IDENT, "output" ) ) {
		return false;
	}

	auto* pOutput = new CClassOutput;

	const bool bReturn = ParseInputOutput( tr, pOutput );
	if ( bReturn ) {
		AddOutput( pOutput );
	} else {
		delete pOutput;
	}

	return bReturn;
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : &tr -
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool GDclass::ParseVariables( TokenReader& tr ) {
	while ( true ) {
		char szToken[ MAX_TOKEN ];

		if ( tr.PeekTokenType( szToken, sizeof( szToken ) ) == OPERATOR ) {
			break;
		}

		if ( not stricmp( szToken, "input" ) ) {
			if ( not ParseInput( tr ) ) {
				return false;
			}

			continue;
		}

		if ( not stricmp( szToken, "output" ) ) {
			if ( not ParseOutput( tr ) ) {
				return false;
			}

			continue;
		}

		if ( not stricmp( szToken, "key" ) ) {
			GDGetToken( tr, szToken, sizeof( szToken ) );
		}

		auto* var = new GDinputvariable;
		if ( not var->InitFromTokens( tr ) ) {
			delete var;
			return false;
		}

		int nDupIndex;
		const GDinputvariable* pDupVar = VarForName( var->GetName(), &nDupIndex );

		// check for duplicate variable definitions
		if ( pDupVar ) {
			// Same name, different type.
			if ( pDupVar->GetType() != var->GetType() ) {
				char szError[ _MAX_PATH ];

				sprintf( szError, "%s: Variable '%s' is multiply defined with different types.", GetName(), var->GetName() );
				GDError( tr, szError );
			}
		}

		if ( not AddVariable( var, this, -1, m_Variables.Count() ) ) {
			delete var;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : iIndex -
// Output : GDinputvariable *
//-----------------------------------------------------------------------------
GDinputvariable* GDclass::GetVariableAt( const int iIndex ) { // NOLINT(*-no-recursion)
	if ( iIndex < 0 or iIndex >= m_nVariables ) {
		return nullptr;
	}

	if ( m_VariableMap[ iIndex ][ 0 ] == -1 ) {
		return m_Variables.Element( m_VariableMap[ iIndex ][ 1 ] );
	}

	// find var's owner
	GDclass* pVarClass = Parent->GetClass( m_VariableMap[ iIndex ][ 0 ] );

	// find var in pVarClass
	return pVarClass->GetVariableAt( m_VariableMap[ iIndex ][ 1 ] );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
GDinputvariable* GDclass::VarForName( const char* pName, int* piIndex ) {
	for ( int i = 0; i < GetVariableCount(); i += 1 ) {
		GDinputvariable* pVar = GetVariableAt( i );
		if ( not strcmpi( pVar->GetName(), pName ) ) {
			if ( piIndex ) {
				piIndex[ 0 ] = i;
			}
			return pVar;
		}
	}

	return nullptr;
}

void GDclass::GetHelperForGDVar( const GDinputvariable* pVar, CUtlVector<const char*>* pHelperName ) {
	const char* pszName = pVar->GetName();
	for ( int i = 0; i < GetHelperCount(); i += 1 ) {
		const CHelperInfo* pHelper = GetHelper( i );
		const int nParamCount = pHelper->GetParameterCount();
		for ( int j = 0; j < nParamCount; j += 1 ) {
			if ( not strcmpi( pszName, pHelper->GetParameter( j ) ) ) {
				pHelperName->AddToTail( pHelper->GetName() );
			}
		}
	}
}
