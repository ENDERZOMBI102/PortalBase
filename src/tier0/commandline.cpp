//
// Created by ENDERZOMBI102 on 09/02/2024.
//
#include "commandline.hpp"
#include "tier0/dbg.h"

static CCommandLine* g_pCommandLine{ nullptr };


const char* tokenize( const char* line, std::string& buffer ) {
	// if we're already at the end, do nothing
	if ( not line or *line == '\0' ) {
		return nullptr;
	}

	// ignore leading space
	while ( *line == ' ' or *line == '\t' ) {
		line += 1;
	}

	size_t offset;
	const char* start;
	if ( ( offset = *line == '"' ) ) {  // parse a quoted string
		start = ++line;
		while ( *line != '"' and *line != '\0' ) {
			line += 1;
		}
	} else {  // parse a single token
		start = line;
		while ( *line != ' ' and *line != '\t' and *line != '\0' ) {
			line += 1;
		}
	}

	buffer.clear();
	buffer.append( start, line - start );
	// return the new initial position
	return line + offset;
}


void CCommandLine::CreateCmdLine( const char* pCommandLine ) {
	this->Reset();
	// allocates new cmdline
	const char* line{ pCommandLine };
	std::string token;
	while ( (line = tokenize( line, token )) ) {
		m_Params.emplace_back( token );
		m_sCmdLine += token + " ";
	}

	m_sCmdLine.resize( m_sCmdLine.length() - 1 );
	m_sCmdLine.shrink_to_fit();
}
void CCommandLine::CreateCmdLine( const int argc, char** argv ) {
	using namespace std::string_literals;
	this->Reset();
	// allocates new cmdline, wrapping every token in `"`
	for ( int i{ 0 }; i < argc; i += 1 ) {
		m_Params.emplace_back( argv[ i ] );
		m_sCmdLine += "\""s + argv[i] + "\" ";
	}
	m_sCmdLine.resize( m_sCmdLine.length() - 1 );
	m_sCmdLine.shrink_to_fit();
}
const char* CCommandLine::GetCmdLine() const {
	// returns our version of the cmdline
	return m_sCmdLine.c_str();
}

const char* CCommandLine::CheckParm( const char* psz, const char** ppszValue ) const {
	const auto index{ this->FindParm( psz ) };
	if ( ppszValue ) {
		*ppszValue = nullptr;
	}

	if ( not index ) {
		return nullptr;
	}

	if ( ppszValue and index + 1 < m_Params.size() and m_Params[index + 1][0] != '-' ) {
		*ppszValue = m_Params[index + 1].c_str();
	}
	return m_Params[index].c_str();
}
void CCommandLine::RemoveParm( const char* parm ) {
	// remove `-key` and its `value`
	AssertUnreachable();
}
void CCommandLine::AppendParm( const char* pszParm, const char* pszValues ) {
	// appends to the cmdline string too, without wrapping ( unless pre-wrapped by caller )
	m_Params.emplace_back( pszParm );
	m_sCmdLine.append( " " ).append( pszParm );

	// FIXME: This has invalid behavior, it doesn't parse the param correctly.
	if ( pszValues ) {
		m_Params.emplace_back( pszValues );
		m_sCmdLine.append( " " ).append( pszValues );
	}
}

const char* CCommandLine::ParmValue( const char* psz, const char* pDefaultVal ) const {
	for ( int i{ 0 }; i < m_Params.size(); i += 1 ) {
		if ( m_Params[i] == psz and i + 1 < m_Params.size() ) {
			return m_Params[ i + 1 ].c_str();
		}
	}
	return pDefaultVal;
}
int CCommandLine::ParmValue( const char* psz, const int nDefaultVal ) const {
	for ( int i{ 0 }; i < m_Params.size(); i += 1 ) {
		if ( m_Params[i] == psz && i + 1 < m_Params.size() ) {
			char* invalid;
			const auto value{ strtol( m_Params[ i + 1 ].c_str(), &invalid, 10 ) };
			if ( invalid ) {
				break;
			}

			return value;
		}
	}
	return nDefaultVal;
}
float CCommandLine::ParmValue( const char* psz, const float flDefaultVal ) const {
	for ( int i{ 0 }; i < m_Params.size(); i += 1 ) {
		if ( m_Params[i] == psz && i + 1 < m_Params.size() ) {
			char* invalid;
			const auto value{ strtof( m_Params[ i + 1 ].c_str(), &invalid ) };
			if ( invalid ) {
				break;
			}

			return value;
		}
	}
	return flDefaultVal;
}

int CCommandLine::ParmCount() const {
	// counting both `-key`s and `value`s
	return static_cast<int>( m_Params.size() );
}
int CCommandLine::FindParm( const char* psz ) const {
	// ignores wrapping
	for ( int i{ 0 }; i < m_Params.size(); i += 1 ) {
		if ( m_Params[i] == psz ) {
			return i;
		}
	}
	return 0;
}
const char* CCommandLine::GetParm( const int nIndex ) const {
	if ( m_Params.size() < nIndex ) {
		return "";
	}

	return m_Params[nIndex].c_str();
}

void CCommandLine::SetParm( const int nIndex, const char* pNewParm ) {
	if ( m_Params.size() < nIndex ) {
		return;
	}

	m_Params[nIndex] = pNewParm;
}

const char* CCommandLine::ParmValueByIndex( const int nIndex, const char* pDefaultVal ) const {
	if ( nIndex == 0 or nIndex >= m_Params.size() ) {
		return pDefaultVal;
	}

	return m_Params[nIndex + 1].c_str();
}

void CCommandLine::Reset() {
	m_sCmdLine.clear();
	m_Params.clear();
}


ICommandLine* CommandLine_Tier0() {
	if ( not g_pCommandLine ) {
		g_pCommandLine = new CCommandLine();
	}

	return g_pCommandLine;
}
