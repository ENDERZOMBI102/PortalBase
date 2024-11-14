//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//===========================================================================//
#include "tokenreader.h"
#include "tier0/dbg.h"
#include "tier0/platform.h"
#include "tier1/strtools.h"
#include <cctype>
#include <cstdio>
#include <cstring>

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *pFilename -
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool TokenReader::Open( const char* pFilename ) {
	open( pFilename, std::ios::in | std::ios::binary );
	Q_strncpy( m_szFilename, pFilename, sizeof( m_szFilename ) );
	m_nLine = 1;
	m_nErrorCount = 0;
	m_bStuffed = false;
	return is_open() != 0;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void TokenReader::Close() {
	close();
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : *error -
// Output : const char
//-----------------------------------------------------------------------------
const char* TokenReader::Error( const char* error, ... ) {
	static char errorBuf[256];
	Q_snprintf( errorBuf, sizeof( errorBuf ), "File %s, line %d: ", m_szFilename, m_nLine );
	Q_strncat( errorBuf, error, sizeof( errorBuf ), COPY_ALL_CHARACTERS );
	m_nErrorCount += 1;
	return errorBuf;
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : pStore -
//			nSize -
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
trtoken_t TokenReader::GetString( char* pStore, int nSize ) {
	if ( nSize <= 0 ) {
		return TOKENERROR;
	}

	//
	// Until we reach the end of this string or run out of room in
	// the destination buffer...
	//
	while ( true ) {
		char buf[1024];

		//
		// Fetch the next batch of text from the file.
		//
		get( buf, sizeof( buf ), '\"' );
		if ( eof() ) {
			return TOKENEOF;
		}

		if ( fail() ) {
			// Just means nothing was read (empty string probably "")
			clear();
		}

		//
		// Transfer the text to the destination buffer.
		//
		const char* src = buf;
		while ( *src != '\0' and nSize > 1 ) {
			if ( *src == 0x0d ) {
				//
				// Newline encountered before closing quote -- unterminated string.
				//
				*pStore = '\0';
				return TOKENSTRINGTOOLONG;
			}
			if ( *src != '\\' ) {
				*pStore = *src;
				src += 1;
			} else {
				//
				// Backslash sequence - replace with the appropriate character.
				//
				src += 1;

				if ( *src == 'n' ) {
					*pStore = '\n';
				}

				src += 1;
			}

			pStore += 1;
			nSize -= 1;
		}

		if ( *src != '\0' ) {
			//
			// Ran out of room in the destination buffer. Skip to the close-quote,
			// terminate the string, and exit.
			//
			ignore( 1024, '\"' );
			*pStore = '\0';
			return TOKENSTRINGTOOLONG;
		}

		//
		// Check for closing quote.
		//
		if ( peek() == '\"' ) {
			//
			// Eat the close quote and any whitespace.
			//
			get();

			const bool bCombineStrings = SkipWhiteSpace();

			//
			// Combine consecutive quoted strings if the combine strings character was
			// encountered between the two strings.
			//
			if ( bCombineStrings and peek() == '\"' ) {
				//
				// Eat the open quote and keep parsing this string.
				//
				get();
			} else {
				//
				// Done with this string, terminate the string and exit.
				//
				*pStore = '\0';
				return STRING;
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Returns the next token, allocating enough memory to store the token
//			plus a terminating NULL.
// Input  : pStore - Pointer to a string that will be allocated.
// Output : Returns the type of token that was read, or TOKENERROR.
//-----------------------------------------------------------------------------
trtoken_t TokenReader::NextTokenDynamic( char** pStore ) {
	char szTempBuffer[8192];
	trtoken_t eType = NextToken( szTempBuffer, sizeof( szTempBuffer ) );

	int len = Q_strlen( szTempBuffer ) + 1;
	*pStore = new char[len];
	Assert( *pStore );
	Q_strncpy( *pStore, szTempBuffer, len );

	return eType;
}


//-----------------------------------------------------------------------------
// Purpose: Returns the next token.
// Input  : pszStore - Pointer to a string that will receive the token.
// Output : Returns the type of token that was read, or TOKENERROR.
//-----------------------------------------------------------------------------
trtoken_t TokenReader::NextToken( char* pStore, const int pSize ) {
	const char* pStart = pStore;

	if ( !is_open() ) {
		return TOKENEOF;
	}

	//
	// If they stuffed a token, return that token.
	//
	if ( m_bStuffed ) {
		m_bStuffed = false;
		Q_strncpy( pStore, m_szStuffed, pSize );
		return m_eStuffed;
	}

	SkipWhiteSpace();

	if ( eof() ) {
		return TOKENEOF;
	}

	if ( fail() ) {
		return TOKENEOF;
	}

	char ch = get();

	//
	// Look for all the valid operators.
	//
	switch ( ch ) {
		case '@':
		case ',':
		case '!':
		case '+':
		case '&':
		case '*':
		case '$':
		case '.':
		case '=':
		case ':':
		case '[':
		case ']':
		case '(':
		case ')':
		case '{':
		case '}':
		case '\\': {
			pStore[0] = ch;
			pStore[1] = 0;
			return OPERATOR;
		}
		default:;
	}

	//
	// Look for the start of a quoted string.
	//
	if ( ch == '\"' ) {
		return GetString( pStore, pSize );
	}

	//
	// Integers consist of numbers with an optional leading minus sign.
	//
	if ( isdigit( ch ) or ch == '-' ) {
		do {
			if ( pStore - pStart + 1 < pSize ) {
				*pStore = ch;
				pStore += 1;
			}

			ch = get();
			if ( ch == '-' ) {
				return TOKENERROR;
			}
		} while ( isdigit( ch ) );

		//
		// No identifier characters are allowed contiguous with numbers.
		//
		if ( isalpha( ch ) or ch == '_' ) {
			return TOKENERROR;
		}

		//
		// Put back the non-numeric character for the next call.
		//
		putback( ch );
		*pStore = '\0';
		return INTEGER;
	}

	//
	// Identifiers consist of a consecutive string of alphanumeric
	// characters and underscores.
	//
	while ( isalpha( ch ) or isdigit( ch ) or ch == '_' ) {
		if ( pStore - pStart + 1 < pSize ) {
			*pStore = ch;
			pStore += 1;
		}

		ch = get();
	}

	//
	// Put back the non-identifier character for the next call.
	//
	putback( ch );
	*pStore = '\0';
	return IDENT;
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : ttype -
//			*pToken -
//-----------------------------------------------------------------------------
void TokenReader::IgnoreTill( const trtoken_t ttype, const char* pToken ) {
	while ( true ) {
		char szBuf[1024];
		const trtoken_t _ttype = NextToken( szBuf, sizeof( szBuf ) );

		if ( _ttype == TOKENEOF ) {
			return;
		}
		if ( _ttype == ttype ) {
			if ( IsToken( pToken, szBuf ) ) {
				Stuff( ttype, pToken );
				return;
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : ttype -
//			pToken -
//-----------------------------------------------------------------------------
void TokenReader::Stuff( const trtoken_t etype, const char* pToken ) {
	m_eStuffed = etype;
	Q_strncpy( m_szStuffed, pToken, sizeof( m_szStuffed ) );
	m_bStuffed = true;
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : ttype -
//			pszToken -
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool TokenReader::Expecting( const trtoken_t ttype, const char* pszToken ) {
	char szBuf[1024];
	if ( NextToken( szBuf, sizeof( szBuf ) ) != ttype or not IsToken( pszToken, szBuf ) ) {
		return false;
	}
	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : pStore -
// Output :
//-----------------------------------------------------------------------------
trtoken_t TokenReader::PeekTokenType( char* pStore, const int pMaxlen ) {
	if ( not m_bStuffed ) {
		m_eStuffed = NextToken( m_szStuffed, sizeof( m_szStuffed ) );
		m_bStuffed = true;
	}

	if ( pStore ) {
		Q_strncpy( pStore, m_szStuffed, pMaxlen );
	}

	return m_eStuffed;
}


//-----------------------------------------------------------------------------
// Purpose: Gets the next non-whitespace character from the file.
// Input  : ch - Receives the character.
// Output : Returns true if the whitespace contained the combine strings
//			character '\', which is used to merge consecutive quoted strings.
//-----------------------------------------------------------------------------
bool TokenReader::SkipWhiteSpace() {
	bool bCombineStrings = false;

	while ( true ) {
		const char ch = get();

		if ( ch == ' ' or ch == '\t' or ch == '\r' or ch == 0 ) {
			continue;
		}

		if ( ch == '+' ) {
			bCombineStrings = true;
			continue;
		}

		if ( ch == '\n' ) {
			m_nLine += 1;
			continue;
		}

		if ( eof() ) {
			return bCombineStrings;
		}

		//
		// Check for the start of a comment.
		//
		if ( ch == '/' ) {
			if ( peek() == '/' ) {
				ignore( 1024, '\n' );
				m_nLine += 1;
			}
		} else {
			//
			// It is a worthy character. Put it back.
			//
			putback( ch );
			return bCombineStrings;
		}
	}
}
