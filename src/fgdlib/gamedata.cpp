//========= Copyright Valve Corporation, All rights reserved. ============//
//
//=============================================================================
#include "fgdlib/gamedata.h"
#include "KeyValues.h"
#include "filesystem_tools.h"
#include "tier0/dbg.h"
#include "tier1/strtools.h"
#include "utlmap.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#pragma warning( disable : 4244 )


constexpr int MAX_ERRORS = 5;
static GameDataMessageFunc_t g_pMsgFunc = nullptr;


//-----------------------------------------------------------------------------
// Sets the function used for emitting error messages while loading gamedata files.
//-----------------------------------------------------------------------------
void GDSetMessageFunc( const GameDataMessageFunc_t pFunc ) {
	g_pMsgFunc = pFunc;
}


//-----------------------------------------------------------------------------
// Purpose: Fetches the next token from the file.
// Input  : tr -
//			ppszStore - Destination buffer, one of the following:
//				pointer to nullptr - token will be placed in an allocated buffer
//				pointer to non-nullptr buffer - token will be placed in buffer
//			ttexpecting -
//			pszExpecting -
// Output :
//-----------------------------------------------------------------------------
static bool DoGetToken( TokenReader& tr, char** pStore, const int pSize, const trtoken_t ttexpecting, const char* pExpecting ) {
	trtoken_t ttype;

	if ( *pStore != nullptr ) {
		// Reads the token into the given buffer.
		ttype = tr.NextToken( *pStore, pSize );
	} else {
		// Allocates a buffer to hold the token.
		ttype = tr.NextTokenDynamic( pStore );
	}

	if ( ttype == TOKENSTRINGTOOLONG ) {
		GDError( tr, "unterminated string or string too long" );
		return false;
	}

	//
	// Check for a bad token type.
	//
	char* pszStore = *pStore;
	bool bBadTokenType = false;
	if ( ttype != ttexpecting and ttexpecting != TOKENNONE ) {
		//
		// If we were expecting a string and got an integer, don't worry about it.
		// We can translate from integer to string.
		//
		if ( not ( ttexpecting == STRING and ttype == INTEGER ) ) {
			bBadTokenType = true;
		}
	}

	if ( bBadTokenType and pExpecting == nullptr ) {
		//
		// We didn't get the expected token type but no expected
		// string was specified.
		//
		const char* pszTokenName;
		switch ( ttexpecting ) {
			case IDENT: {
				pszTokenName = "identifier";
				break;
			}

			case INTEGER: {
				pszTokenName = "integer";
				break;
			}

			case STRING: {
				pszTokenName = "string";
				break;
			}

			case OPERATOR:
			default: {
				pszTokenName = "symbol";
				break;
			}
		}

		GDError( tr, "expecting %s", pszTokenName );
		return false;
	}
	if ( bBadTokenType or pExpecting != nullptr and not IsToken( pszStore, pExpecting ) ) {
		//
		// An expected string was specified, and we got either the wrong type or
		// the right type but the wrong string,
		//
		GDError( tr, "expecting '%s', but found '%s'", pExpecting, pszStore );
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : tr -
//			error -
// Output :
//-----------------------------------------------------------------------------
bool GDError( TokenReader& tr, const char* error, ... ) {
	char szBuf[128];
	va_list vl;
	va_start( vl, error );
	vsprintf( szBuf, error, vl );
	va_end( vl );

	if ( g_pMsgFunc ) {
		// HACK: should use an enumeration for error level
		g_pMsgFunc( 1, tr.Error( szBuf ) );
	}

	if ( tr.GetErrorCount() >= MAX_ERRORS ) {
		if ( g_pMsgFunc ) {
			// HACK: should use an enumeration for error level
			g_pMsgFunc( 1, "   - too many errors; aborting." );
		}

		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Fetches the next token from the file.
// Input  : tr - The token reader object with which to fetch the token.
//			pszStore - Buffer in which to place the token, nullptr to discard the token.
//			ttexpecting - The token type that we are expecting. If this is not TOKENNONE
//				and token type read is different, the operation will fail.
//			pszExpecting - The token string that we are expecting. If this string
//				is not nullptr and the token string read is different, the operation will fail.
// Output : Returns true if the operation succeeded, false if there was an error.
//			If there was an error, the error will be reported in the message window.
//-----------------------------------------------------------------------------
bool GDGetToken( TokenReader& tr, char* pszStore, int nSize, trtoken_t ttexpecting, const char* pszExpecting ) {
	Assert( pszStore != nullptr );
	if ( pszStore != nullptr ) {
		return DoGetToken( tr, &pszStore, nSize, ttexpecting, pszExpecting );
	}

	return false;
}


//-----------------------------------------------------------------------------
// Purpose: Fetches the next token from the file.
// Input  : tr - The token reader object with which to fetch the token.
//			pszStore - Buffer in which to place the token, nullptr to discard the token.
//			ttexpecting - The token type that we are expecting. If this is not TOKENNONE
//				and token type read is different, the operation will fail.
//			pszExpecting - The token string that we are expecting. If this string
//				is not nullptr and the token string read is different, the operation will fail.
// Output : Returns true if the operation succeeded, false if there was an error.
//			If there was an error, the error will be reported in the message window.
//-----------------------------------------------------------------------------
bool GDSkipToken( TokenReader& tr, const trtoken_t ttexpecting, const char* pszExpecting ) {
	//
	// Read the next token into a buffer and discard it.
	//
	char szDiscardBuf[MAX_TOKEN];
	char* pszDiscardBuf = szDiscardBuf;
	return DoGetToken( tr, &pszDiscardBuf, std::size( szDiscardBuf ), ttexpecting, pszExpecting );
}


//-----------------------------------------------------------------------------
// Purpose: Fetches the next token from the file, allocating a buffer exactly
//			large enough to hold the token.
// Input  : tr -
//			ppszStore -
//			ttexpecting -
//			pszExpecting -
// Output :
//-----------------------------------------------------------------------------
bool GDGetTokenDynamic( TokenReader& tr, char** const pStore, const trtoken_t ttexpecting, const char* pExpecting ) {
	if ( pStore == nullptr ) {
		return false;
	}

	*pStore = nullptr;
	return DoGetToken( tr, pStore, -1, ttexpecting, pExpecting );
}


//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
GameData::GameData() = default;


//-----------------------------------------------------------------------------
// Purpose: Destructor.
//-----------------------------------------------------------------------------
GameData::~GameData() {
	ClearData();
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void GameData::ClearData() {
	// delete classes.
	const int nCount = m_Classes.Count();
	for ( int i = 0; i < nCount; i += 1 ) {
		const GDclass* pm = m_Classes.Element( i );
		delete pm;
	}
	m_Classes.RemoveAll();
}


//-----------------------------------------------------------------------------
// Purpose: Loads a gamedata (FGD) file into this object.
// Input  : pszFilename -
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool GameData::Load( const char* pFilename ) { // NOLINT(*-no-recursion)
	TokenReader tr;

	#if IsWindows()
		if ( GetFileAttributes( pszFilename ) == 0xffffffff ) {
			return false;
		}
	#endif

	if ( not tr.Open( pFilename ) ) {
		return false;
	}

	while ( true ) {
		char szToken[128];
		if ( tr.GetErrorCount() >= MAX_ERRORS ) {
			break;
		}

		const trtoken_t ttype = tr.NextToken( szToken, std::size( szToken ) );

		if ( ttype == TOKENEOF ) {
			break;
		}

		if ( ttype != OPERATOR or not IsToken( szToken, "@" ) ) {
			if ( not GDError( tr, "expected @" ) ) {
				return false;
			}
		}

		// check what kind it is, and parse a new object
		if ( tr.NextToken( szToken, std::size( szToken ) ) != IDENT ) {
			if ( not GDError( tr, "expected identifier after @" ) ) {
				return false;
			}
		}

		if ( IsToken( szToken, "baseclass" ) or IsToken( szToken, "pointclass" ) or IsToken( szToken, "solidclass" ) or IsToken( szToken, "keyframeclass" ) or
			 IsToken( szToken, "moveclass" ) or IsToken( szToken, "npcclass" ) or IsToken( szToken, "filterclass" ) ) {
			//
			// New class.
			//
			auto pNewClass = new GDclass;
			if ( not pNewClass->InitFromTokens( tr, this ) ) {
				tr.IgnoreTill( OPERATOR, "@" );  // go to next section
				delete pNewClass;
			} else {
				if ( IsToken( szToken, "baseclass" ) ) {  // Not directly available to user.
					pNewClass->SetBaseClass( true );
				} else if ( IsToken( szToken, "pointclass" ) ) {  // Generic point class.
					pNewClass->SetPointClass( true );
				} else if ( IsToken( szToken, "solidclass" ) ) {  // Tied to solids.
					pNewClass->SetSolidClass( true );
				} else if ( IsToken( szToken, "npcclass" ) ) {  // NPC class - can be spawned by npc_maker.
					pNewClass->SetPointClass( true );
					pNewClass->SetNPCClass( true );
				} else if ( IsToken( szToken, "filterclass" ) ) {  // Filter class - can be used as a filter
					pNewClass->SetPointClass( true );
					pNewClass->SetFilterClass( true );
				} else if ( IsToken( szToken, "moveclass" ) ) {  // Animating
					pNewClass->SetMoveClass( true );
					pNewClass->SetPointClass( true );
				} else if ( IsToken( szToken, "keyframeclass" ) ) {  // Animation keyframes
					pNewClass->SetKeyFrameClass( true );
					pNewClass->SetPointClass( true );
				}

				// Check and see if this new class matches an existing one. If so, we will override the previous definition.
				int existingClassIndex = 0;
				const GDclass* pExistingClass = ClassForName( pNewClass->GetName(), &existingClassIndex );
				if ( nullptr != pExistingClass ) {
					m_Classes.InsertAfter( existingClassIndex, pNewClass );
					m_Classes.Remove( existingClassIndex );
				} else {
					m_Classes.AddToTail( pNewClass );
				}
			}
		} else if ( IsToken( szToken, "include" ) ) {
			if ( GDGetToken( tr, szToken, std::size( szToken ), STRING ) ) {
				// Let's assume it's in the same directory.
				char justPath[MAX_PATH], loadFilename[MAX_PATH];
				if ( Q_ExtractFilePath( pFilename, justPath, std::size( justPath ) ) ) {
					Q_snprintf( loadFilename, std::size( loadFilename ), "%s%s", justPath, szToken );
				} else {
					Q_strncpy( loadFilename, szToken, std::size( loadFilename ) );
				}

				// First try our fully specified directory
				if ( not Load( loadFilename ) ) {
					// Failing that, try our start directory
					if ( not Load( szToken ) ) {
						GDError( tr, "error including file: %s", szToken );
					}
				}
			}
		} else if ( IsToken( szToken, "mapsize" ) ) {
			if ( not ParseMapSize( tr ) ) {
				// Error in map size specifier, skip to next @ sign.
				tr.IgnoreTill( OPERATOR, "@" );
			}
		} else if ( IsToken( szToken, "materialexclusion" ) ) {
			if ( not LoadFGDMaterialExclusions( tr ) ) {
				// FGD exclusions weren't defined; skip to next @ sign.
				tr.IgnoreTill( OPERATOR, "@" );
			}
		} else if ( IsToken( szToken, "autovisgroup" ) ) {
			if ( not LoadFGDAutoVisGroups( tr ) ) {
				// FGD AutoVisGroups not defined; skip to next @ sign.
				tr.IgnoreTill( OPERATOR, "@" );
			}
		} else {
			GDError( tr, "unrecognized section name %s", szToken );
			tr.IgnoreTill( OPERATOR, "@" );
		}
	}

	if ( tr.GetErrorCount() > 0 ) {
		return false;
	}

	tr.Close();

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Parses the "mapsize" specifier, which should be of the form:
//
//			mapsize(min, max)
//
//			ex: mapsize(-8192, 8192)
//
// Input  : tr -
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool GameData::ParseMapSize( TokenReader& tr ) {
	if ( not GDSkipToken( tr, OPERATOR, "(" ) ) {
		return false;
	}

	char szToken[128];
	if ( not GDGetToken( tr, szToken, std::size( szToken ), INTEGER ) ) {
		return false;
	}
	const int min = strtol( szToken, nullptr, 10 );

	if ( not GDSkipToken( tr, OPERATOR, "," ) ) {
		return false;
	}

	if ( not GDGetToken( tr, szToken, std::size( szToken ), INTEGER ) ) {
		return false;
	}
	const int max = strtol( szToken, nullptr, 10 );

	if ( min != max ) {
		m_nMinMapCoord = std::min( min, max );
		m_nMaxMapCoord = std::max( min, max );
	}

	if ( not GDSkipToken( tr, OPERATOR, ")" ) ) {
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : pszName -
//			piIndex -
// Output :
//-----------------------------------------------------------------------------
GDclass* GameData::ClassForName( const char* pszName, int* piIndex ) {
	const int count = m_Classes.Count();
	for ( int i = 0; i < count; i += 1 ) {
		GDclass* mp = m_Classes.Element( i );
		if ( not strcmp( mp->GetName(), pszName ) ) {
			if ( piIndex ) {
				piIndex[0] = i;
			}
			return mp;
		}
	}

	return nullptr;
}


// These are 'standard' keys that every entity uses, but they aren't specified that way in the .fgd
static const char* RequiredKeys[] { "Origin", "Angles", nullptr };

//-----------------------------------------------------------------------------
// Purpose: this function will set up the initial class about to be instanced
// Input  : pszClassName - the class name of the entity to be instanced
//			pszInstancePrefix - the prefix to be used for all name fields
//			Origin - the origin offset of the instance
//			Angles - the angle rotation of the instance
// Output : if successful, will return the game data class of the class name
//-----------------------------------------------------------------------------
GDclass* GameData::BeginInstanceRemap( const char* pszClassName, const char* pszInstancePrefix, const Vector& Origin, const QAngle& Angle ) {
	m_InstanceOrigin = Origin;
	m_InstanceAngle = Angle;
	AngleMatrix( m_InstanceAngle, m_InstanceOrigin, m_InstanceMat );

	strcpy( m_InstancePrefix, pszInstancePrefix );

	if ( m_InstanceClass ) {
		delete m_InstanceClass;
		m_InstanceClass = nullptr;
	}

	if ( strcmpi( pszClassName, "info_overlay_accessor" ) == 0 ) {  // yucky hack for a made up entity in the bsp process
		pszClassName = "info_overlay";
	}

	if ( GDclass* BaseClass = ClassForName( pszClassName ) ) {
		m_InstanceClass = new GDclass();
		m_InstanceClass->Parent = this;
		m_InstanceClass->AddBase( BaseClass );

		for ( int i = 0; RequiredKeys[i]; i += 1 ) {
			if ( m_InstanceClass->VarForName( RequiredKeys[i] ) == nullptr ) {
				BaseClass = ClassForName( RequiredKeys[i] );
				if ( BaseClass ) {
					m_InstanceClass->AddBase( BaseClass );
				}
			}
		}
	} else {
		m_InstanceClass = nullptr;
	}

	return m_InstanceClass;
}


enum tRemapOperation {
	REMAP_NAME = 0,
	REMAP_POSITION,
	REMAP_ANGLE,
	REMAP_ANGLE_NEGATIVE_PITCH,
};


static CUtlMap<GDIV_TYPE, tRemapOperation> RemapOperation;


//-----------------------------------------------------------------------------
// Purpose: function to sort the class type for the RemapOperations map
// Input  : type1 - the first type to compare against
//			type2 - the second type to compare against
// Output : returns true if the first type is less than the second one
//-----------------------------------------------------------------------------
static bool CUtlType_LessThan( const GDIV_TYPE& type1, const GDIV_TYPE& type2 ) {
	return type1 < type2;
}


//-----------------------------------------------------------------------------
// Purpose: this function will attempt to remap a key's value
// Input  : pszKey - the name of the key
//			pszInvalue - the original value
//			AllowNameRemapping - only do name remapping if this parameter is true.
//				this is generally only false on the instance level.
// Output : returns true if the value changed
//			pszOutValue - the new value if changed
//-----------------------------------------------------------------------------
bool GameData::RemapKeyValue( const char* pszKey, const char* pszInValue, char* pszOutValue, const TNameFixup NameFixup ) {
	if ( RemapOperation.Count() == 0 ) {
		RemapOperation.SetLessFunc( &CUtlType_LessThan );
		RemapOperation.Insert( ivAngle, REMAP_ANGLE );
		RemapOperation.Insert( ivTargetDest, REMAP_NAME );
		RemapOperation.Insert( ivTargetSrc, REMAP_NAME );
		RemapOperation.Insert( ivOrigin, REMAP_POSITION );
		RemapOperation.Insert( ivAxis, REMAP_ANGLE );
		RemapOperation.Insert( ivAngleNegativePitch, REMAP_ANGLE_NEGATIVE_PITCH );
	}

	if ( not m_InstanceClass ) {
		return false;
	}

	const GDinputvariable* KVVar = m_InstanceClass->VarForName( pszKey );
	if ( not KVVar ) {
		return false;
	}

	const GDIV_TYPE KVType = KVVar->GetType();
	const int KVRemapIndex = RemapOperation.Find( KVType );
	if ( KVRemapIndex == CUtlMap<GDIV_TYPE, tRemapOperation>::InvalidIndex() ) {
		return false;
	}

	strcpy( pszOutValue, pszInValue );

	switch ( RemapOperation[KVRemapIndex] ) {
		case REMAP_NAME:
			if ( KVType != ivInstanceVariable ) {
				RemapNameField( pszInValue, pszOutValue, NameFixup );
			}
			break;

		case REMAP_POSITION: {
			Vector inPoint( 0.0f, 0.0f, 0.0f ), outPoint;

			sscanf( pszInValue, "%f %f %f", &inPoint.x, &inPoint.y, &inPoint.z );
			VectorTransform( inPoint, m_InstanceMat, outPoint );
			sprintf( pszOutValue, "%g %g %g", outPoint.x, outPoint.y, outPoint.z );
		} break;

		case REMAP_ANGLE:
			if ( m_InstanceAngle.x != 0.0f or m_InstanceAngle.y != 0.0f or m_InstanceAngle.z != 0.0f ) {
				QAngle inAngles( 0.0f, 0.0f, 0.0f ), outAngles;
				matrix3x4_t angToWorld, localMatrix;

				sscanf( pszInValue, "%f %f %f", &inAngles.x, &inAngles.y, &inAngles.z );

				AngleMatrix( inAngles, angToWorld );
				MatrixMultiply( m_InstanceMat, angToWorld, localMatrix );
				MatrixAngles( localMatrix, outAngles );

				sprintf( pszOutValue, "%g %g %g", outAngles.x, outAngles.y, outAngles.z );
			}
			break;

		case REMAP_ANGLE_NEGATIVE_PITCH:
			if ( m_InstanceAngle.x != 0.0f or m_InstanceAngle.y != 0.0f or m_InstanceAngle.z != 0.0f ) {
				QAngle inAngles( 0.0f, 0.0f, 0.0f ), outAngles;
				matrix3x4_t angToWorld, localMatrix;

				sscanf( pszInValue, "%f", &inAngles.x );  // just the pitch
				inAngles.x = -inAngles.x;

				AngleMatrix( inAngles, angToWorld );
				MatrixMultiply( m_InstanceMat, angToWorld, localMatrix );
				MatrixAngles( localMatrix, outAngles );

				sprintf( pszOutValue, "%g", -outAngles.x );  // just the pitch
			}
			break;
	}

	return strcmpi( pszInValue, pszOutValue ) != 0;
}


//-----------------------------------------------------------------------------
// Purpose: this function will attempt to remap a name field.
// Input  : pszInvalue - the original value
//			AllowNameRemapping - only do name remapping if this parameter is true.
//				this is generally only false on the instance level.
// Output : returns true if the value changed
//			pszOutValue - the new value if changed
//-----------------------------------------------------------------------------
bool GameData::RemapNameField( const char* pszInValue, char* pszOutValue, TNameFixup NameFixup ) {
	strcpy( pszOutValue, pszInValue );

	// `!` at the start of a value means that it is global and should not be remapped
	if ( pszInValue[0] and pszInValue[0] != '@' ) {
		switch ( NameFixup ) {
			case NAME_FIXUP_PREFIX:
				sprintf( pszOutValue, "%s-%s", m_InstancePrefix, pszInValue );
				break;

			case NAME_FIXUP_POSTFIX:
				sprintf( pszOutValue, "%s-%s", pszInValue, m_InstancePrefix );
				break;
			default:
				AssertUnreachable();
		}
	}

	return strcmpi( pszInValue, pszOutValue ) != 0;
}


//-----------------------------------------------------------------------------
// Purpose: Gathers any FGD-defined material directory exclusions
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool GameData::LoadFGDMaterialExclusions( TokenReader& tr ) {
	if ( not GDSkipToken( tr, OPERATOR, "[" ) ) {
		return false;
	}
	while ( true ) {
		char szToken[128];

		if ( tr.PeekTokenType( szToken, std::size( szToken ) ) == OPERATOR ) {
			break;
		}
		if ( GDGetToken( tr, szToken, std::size( szToken ), STRING ) ) {
			bool matchFound = false;
			// Make sure we haven't loaded this from another FGD
			for ( int i = 0; i < m_FGDMaterialExclusions.Count(); i += 1 ) {
				if ( not stricmp( szToken, m_FGDMaterialExclusions[i].szDirectory ) ) {
					matchFound = true;
					break;
				}
			}

			// Parse the string
			if ( matchFound == false ) {
				const int index = m_FGDMaterialExclusions.AddToTail();
				Q_strncpy( m_FGDMaterialExclusions[index].szDirectory, szToken, static_cast<int>( std::size( m_FGDMaterialExclusions[index].szDirectory ) ) );
				m_FGDMaterialExclusions[index].bUserGenerated = false;
			}
		}
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
// Purpose: Gathers any FGD-defined Auto VisGroups
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool GameData::LoadFGDAutoVisGroups( TokenReader& tr ) {
	int gindex = 0;  // Index of AutoVisGroups

	char szToken[128];

	// Handle the Parent -- World Geometry, Entities, World Detail
	if ( GDSkipToken( tr, OPERATOR, "=" ) ) {
		// We expect a name
		if ( not GDGetToken( tr, szToken, std::size( szToken ), STRING ) ) {
			return false;
		}

		gindex = m_FGDAutoVisGroups.AddToTail();
		Q_strncpy( m_FGDAutoVisGroups[gindex].szParent, szToken, static_cast<int>( std::size( m_FGDAutoVisGroups[gindex].szParent ) ) );

		// We expect a Class
		if ( not GDSkipToken( tr, OPERATOR, "[" ) ) {
			return false;
		}
	}

	// Handle the Class(es) -- Brush Entities, Occluders, Lights
	while ( true ) {
		if ( GDGetToken( tr, szToken, std::size( szToken ), STRING ) ) {
			const int classIndex = m_FGDAutoVisGroups[gindex].m_Classes.AddToTail();  // Index of Classes
			Q_strncpy( m_FGDAutoVisGroups[gindex].m_Classes[classIndex].szClass, szToken, static_cast<int>( std::size( m_FGDAutoVisGroups[gindex].m_Classes[classIndex].szClass ) ) );

			if ( not GDSkipToken( tr, OPERATOR, "[" ) ) {
				return false;
			}

			// Parse objects/entities -- func_detail, point_template, light_spot
			while ( true ) {
				if ( tr.PeekTokenType( szToken, std::size( szToken ) ) == OPERATOR ) {
					break;
				}

				if ( not GDGetToken( tr, szToken, std::size( szToken ), STRING ) ) {
					return false;
				}

				m_FGDAutoVisGroups[gindex].m_Classes[classIndex].szEntities.CopyAndAddToTail( szToken );
			}

			if ( not GDSkipToken( tr, OPERATOR, "]" ) ) {
				return false;
			}

			// See if we have another Class coming up
			if ( tr.PeekTokenType( szToken, std::size( szToken ) ) == STRING ) {
				continue;
			}

			// If no more Classes, we now expect a terminating ']'
			if ( not GDSkipToken( tr, OPERATOR, "]" ) ) {
				return false;
			}

			// We're done
			return true;
		}

		// We don't have another Class; look for a terminating brace
		if ( not GDSkipToken( tr, OPERATOR, "]" ) ) {
			return false;
		}
	}

	// Safety net
	// ReSharper disable once CppDFAUnreachableCode
	GDError( tr, "Malformed AutoVisGroup -- Last processed:  %s", szToken );
	return false;
}
