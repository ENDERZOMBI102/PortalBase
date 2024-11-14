//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#pragma once
#include <fstream>
#include "tier0/platform.h"
#include "tier0/basetypes.h"


enum trtoken_t {
	TOKENSTRINGTOOLONG = -4,
	TOKENERROR = -3,
	TOKENNONE = -2,
	TOKENEOF = -1,
	OPERATOR,
	INTEGER,
	STRING,
	IDENT
};


#define IsToken( s1, s2 ) (not strcmpi( s1, s2 ))

#define MAX_TOKEN (128 + 1)
#define MAX_IDENT (64 + 1)
#define MAX_STRING (128 + 1)


class TokenReader : private std::ifstream {
public:
	TokenReader() = default;

	bool Open( const char* pFilename );
	trtoken_t NextToken( char* pStore, int pSize );
	trtoken_t NextTokenDynamic( char** pStore );
	void Close();

	void IgnoreTill( trtoken_t ttype, const char* pToken );
	void Stuff( trtoken_t etype, const char* pToken );
	bool Expecting( trtoken_t ttype, const char* pszToken );
	const char* Error( const char* error, ... );
	trtoken_t PeekTokenType( char* = nullptr, int pMaxlen = 0 );

	inline int GetErrorCount() const;

	// compiler can't generate an assignment operator since descended from std::ifstream
	TokenReader( TokenReader const& ) = delete;
	int operator=( TokenReader const& ) = delete;
private:
	trtoken_t GetString( char* pStore, int nSize );
	bool SkipWhiteSpace();

	int m_nLine{1};
	int m_nErrorCount{0};

	char m_szFilename[128] { };
	char m_szStuffed[128] { };
	bool m_bStuffed{ false };
	trtoken_t m_eStuffed{};
};


//-----------------------------------------------------------------------------
// Purpose: Returns the total number of parsing errors since this file was opened.
//-----------------------------------------------------------------------------
int TokenReader::GetErrorCount() const {
	return m_nErrorCount;
}
