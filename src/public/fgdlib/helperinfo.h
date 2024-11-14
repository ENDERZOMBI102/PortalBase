//========= Copyright Valve Corporation, All rights reserved. ============//
//
//=============================================================================
#pragma once
#include <tier0/dbg.h>
#include <utlvector.h>


#define MAX_HELPER_NAME_LEN 256


typedef CUtlVector<char*> CParameterList;


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CHelperInfo {
public:
	inline CHelperInfo();
	inline ~CHelperInfo();

	[[nodiscard]]
	inline const char* GetName() const { return m_szName; }
	inline void SetName( const char* pszName );

	inline bool AddParameter( const char* pszParameter );

	[[nodiscard]]
	inline int GetParameterCount() const { return m_Parameters.Count(); }
	[[nodiscard]]
	inline const char* GetParameter( int nIndex ) const;

protected:
	char m_szName[MAX_HELPER_NAME_LEN];
	CParameterList m_Parameters;
};


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
inline CHelperInfo::CHelperInfo() {
	m_szName[0] = '\0';
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
inline CHelperInfo::~CHelperInfo() {
	int nCount = m_Parameters.Count();
	for ( int i = 0; i < nCount; i++ ) {
		char* pszParam = m_Parameters.Element( i );
		Assert( pszParam != nullptr );
		if ( pszParam != nullptr ) {
			delete[] pszParam;
		}
	}

	m_Parameters.RemoveAll();
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : char *pszParameter -
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
inline bool CHelperInfo::AddParameter( const char* pszParameter ) {
	if ( pszParameter != nullptr and pszParameter[0] != '\0' ) {
		const int nLen = strlen( pszParameter );

		if ( nLen > 0 ) {
			const auto pszNew = new char[nLen + 1];
			if ( pszNew != nullptr ) {
				strcpy( pszNew, pszParameter );
				m_Parameters.AddToTail( pszNew );
				return true;
			}
		}
	}

	return false;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline const char* CHelperInfo::GetParameter( int nIndex ) const {
	if ( nIndex >= m_Parameters.Count() ) {
		return nullptr;
	}

	return m_Parameters.Element( nIndex );
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : char *pszName -
//-----------------------------------------------------------------------------
inline void CHelperInfo::SetName( const char* pszName ) {
	if ( pszName != nullptr ) {
		strcpy( m_szName, pszName );
	}
}


typedef CUtlVector<CHelperInfo*> CHelperInfoList;
