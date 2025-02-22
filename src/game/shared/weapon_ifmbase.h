//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//===========================================================================//
#pragma once


#if defined( CLIENT_DLL )
	#define CWeaponIFMBase C_WeaponIFMBase
#endif

#if defined( DOD_DLL )
	#include "weapon_dodbase.h"
	#define CWeaponModBaseClass CWeaponDODBase
#elif defined( TF_CLIENT_DLL ) || defined( TF_DLL )
	#include "tf_weaponbase.h"
	#define CWeaponModBaseClass CTFWeaponBase
#endif

class CWeaponIFMBase : public CWeaponModBaseClass {
public:
	DECLARE_CLASS( CWeaponIFMBase, CWeaponModBaseClass );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponIFMBase();

	#ifdef GAME_DLL
		DECLARE_DATADESC();
	#endif

	// All predicted weapons need to implement and return true
	virtual bool IsPredicted() const;

	//	virtual void	FallInit( void );

public:
	#if defined( CLIENT_DLL )
		virtual bool ShouldPredict();
		virtual void OnDataChanged( DataUpdateType_t type );
	#else
		virtual void Spawn();

		// FIXME: How should this work? This is a hack to get things working
		virtual const unsigned char* GetEncryptionKey( void ) { return NULL; }
	#endif

private:
	CWeaponIFMBase( const CWeaponIFMBase& );
};
