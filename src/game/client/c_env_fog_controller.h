//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#pragma once
#include "playernet_vars.h"
#define CFogController C_FogController

//=============================================================================
//
// Class Fog Controller:
// Compares a set of integer inputs to the one main input
// Outputs true if they are all equivalent, false otherwise
//
class C_FogController : public C_BaseEntity {
public:
	DECLARE_NETWORKCLASS();
	DECLARE_CLASS( C_FogController, C_BaseEntity );

	C_FogController();

public:
	fogparams_t m_fog;
};
