//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#pragma once
#include "baseanimating.h"

class CActAnimating : public CBaseAnimating {
public:
	DECLARE_CLASS( CActAnimating, CBaseAnimating );

	void SetActivity( Activity act );
	inline Activity GetActivity( void ) { return m_Activity; }

	virtual int ObjectCaps( void ) { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	DECLARE_DATADESC();

private:
	Activity m_Activity;
};
