//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
//=============================================================================//
//
// Purpose:
//
//=============================================================================
#pragma once
#include "physics_bone_follower.h"
#include "physobj.h"
#include "rope.h"
#include "rope_shared.h"

#define CRANE_EXTENSION_RATE_MAX 0.01
#define CRANE_TURN_RATE_MAX 1.2
#define MAXIMUM_CRANE_PICKUP_MASS 10000
#define MINIMUM_CRANE_PICKUP_MASS 500

#define MAX_CRANE_FLAT_REACH 1400.0
#define MIN_CRANE_FLAT_REACH 700.0
#define CRANE_EXTENSION_ACCEL 0.006
#define CRANE_EXTENSION_DECEL 0.02
#define CRANE_TURN_ACCEL 0.2
#define CRANE_DECEL 0.5

#define CRANE_SLOWRAISE_TIME 5.0

// Turning stats
enum {
	TURNING_NOT,
	TURNING_LEFT,
	TURNING_RIGHT,
};

class CPropCrane;

//-----------------------------------------------------------------------------
// Purpose: This is the entity we attach to the tip of the crane and dangle the cable from
//-----------------------------------------------------------------------------
class CCraneTip : public CBaseAnimating {
	DECLARE_CLASS( CCraneTip, CBaseAnimating );

public:
	DECLARE_DATADESC();

	~CCraneTip() {
		if ( m_pSpring ) {
			physenv->DestroySpring( m_pSpring );
		}
	}

	void Spawn();
	void Precache();

	bool CreateConstraint( CBaseAnimating* pMagnet, IPhysicsConstraintGroup* pGroup );
	static CCraneTip* Create( CBaseAnimating* pCraneMagnet, IPhysicsConstraintGroup* pGroup, const Vector& vecOrigin, const QAngle& vecAngles );

public:
	IPhysicsSpring* m_pSpring;
};

//-----------------------------------------------------------------------------
// Purpose: Crane vehicle server
//-----------------------------------------------------------------------------
class CCraneServerVehicle : public CBaseServerVehicle {
	typedef CBaseServerVehicle BaseClass;
	// IServerVehicle
public:
	void GetVehicleViewPosition( int nRole, Vector* pAbsOrigin, QAngle* pAbsAngles, float* pFOV = NULL );

	// NPC Driving
	void NPC_SetDriver( CNPC_VehicleDriver* pDriver );
	void NPC_DriveVehicle();

	virtual bool IsPassengerEntering() { return false; }// NOTE: This mimics the scenario HL2 would have seen
	virtual bool IsPassengerExiting() { return false; }

protected:
	CPropCrane* GetCrane();
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CPropCrane : public CBaseProp, public IDrivableVehicle {
	DECLARE_CLASS( CPropCrane, CBaseProp );

public:
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CPropCrane() {
		m_ServerVehicle.SetVehicle( this );
	}

	~CPropCrane() {
		physenv->DestroyConstraintGroup( m_pConstraintGroup );
	}

	// CBaseEntity
	virtual void Precache();
	void Spawn();
	void Activate();
	void UpdateOnRemove();
	bool CreateVPhysics();
	void InitCraneSpeeds();
	void Think();
	virtual int ObjectCaps() { return BaseClass::ObjectCaps() | FCAP_IMPULSE_USE; };
	virtual void Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value );
	virtual void DrawDebugGeometryOverlays();

	virtual bool PassengerShouldReceiveDamage( CTakeDamageInfo& info ) {
		if ( info.GetDamageType() & DMG_VEHICLE )
			return true;

		return ( info.GetDamageType() & ( DMG_RADIATION | DMG_BLAST ) ) == 0;
	}

	virtual Vector BodyTarget( const Vector& posSrc, bool bNoisy = true );
	virtual void TraceAttack( const CTakeDamageInfo& info, const Vector& vecDir, trace_t* ptr, CDmgAccumulator* pAccumulator );
	virtual int OnTakeDamage( const CTakeDamageInfo& info );

	void PlayerControlInit( CBasePlayer* pPlayer );
	void PlayerControlShutdown();
	void ResetUseKey( CBasePlayer* pPlayer );

	void DriveCrane( int iDriverButtons, int iButtonsPressed, float flNPCSteering = 0.0 );
	void RunCraneMovement( float flTime );

	void TurnMagnetOn();
	void TurnMagnetOff();
	const Vector& GetCraneTipPosition();
	float GetExtensionRate() { return m_flExtensionRate; }
	float GetTurnRate() { return m_flTurn; }
	float GetMaxTurnRate() { return m_flMaxTurnSpeed; }
	CPhysMagnet* GetMagnet() { return m_hCraneMagnet; }
	float GetTotalMassOnCrane() { return m_hCraneMagnet->GetTotalMassAttachedObjects(); }
	bool IsDropping() { return m_bDropping; }

	// Inputs
	void InputLock( inputdata_t& inputdata );
	void InputUnlock( inputdata_t& inputdata );
	void InputForcePlayerIn( inputdata_t& inputdata );

	// Crane handling
	void GetCraneTipPosition( Vector* vecOrigin, QAngle* vecAngles );
	void RecalculateCraneTip();
	void GetVectors( Vector* pForward, Vector* pRight, Vector* pUp ) const;

	void SetNPCDriver( CNPC_VehicleDriver* pDriver );

	// IDrivableVehicle
public:
	virtual CBaseEntity* GetDriver();
	virtual void ItemPostFrame( CBasePlayer* pPlayer );
	virtual void SetupMove( CBasePlayer* player, CUserCmd* ucmd, IMoveHelper* pHelper, CMoveData* move );
	virtual void ProcessMovement( CBasePlayer* pPlayer, CMoveData* pMoveData ) { return; }
	virtual void FinishMove( CBasePlayer* player, CUserCmd* ucmd, CMoveData* move ) { return; }
	virtual bool CanEnterVehicle( CBaseEntity* pEntity );
	virtual bool CanExitVehicle( CBaseEntity* pEntity );
	virtual void SetVehicleEntryAnim( bool bOn ) { m_bEnterAnimOn = bOn; }
	virtual void SetVehicleExitAnim( bool bOn, Vector vecEyeExitEndpoint ) {
		m_bExitAnimOn = bOn;
		if ( bOn ) m_vecEyeExitEndpoint = vecEyeExitEndpoint;
	}
	virtual void EnterVehicle( CBaseCombatCharacter* pPassenger );

	virtual bool AllowBlockedExit( CBaseCombatCharacter* pPassenger, int nRole ) { return true; }
	virtual bool AllowMidairExit( CBaseCombatCharacter* pPassenger, int nRole ) { return false; }
	virtual void PreExitVehicle( CBaseCombatCharacter* pPassenger, int nRole );
	virtual void ExitVehicle( int nRole );
	virtual string_t GetVehicleScriptName() { return m_vehicleScript; }

	// If this is a vehicle, returns the vehicle interface
	virtual IServerVehicle* GetServerVehicle() { return &m_ServerVehicle; }

protected:
	// Contained IServerVehicle
	CCraneServerVehicle m_ServerVehicle;
	// Contained Bone Follower manager
	CBoneFollowerManager m_BoneFollowerManager;

private:
	CNetworkHandle( CBasePlayer, m_hPlayer );
	CNetworkVar( bool, m_bMagnetOn );

	// NPC Driving
	CHandle<CNPC_VehicleDriver> m_hNPCDriver;
	int m_nNPCButtons;

	// Entering / Exiting
	bool m_bLocked;
	CNetworkVar( bool, m_bEnterAnimOn );
	CNetworkVar( bool, m_bExitAnimOn );
	CNetworkVector( m_vecEyeExitEndpoint );
	COutputEvent m_playerOn;
	COutputEvent m_playerOff;

	// Turning
	int m_iTurning;
	bool m_bStartSoundAtCrossover;
	float m_flTurn;

	// Crane arm extension / retraction
	bool m_bExtending;
	float m_flExtension;
	float m_flExtensionRate;

	// Magnet movement
	bool m_bDropping;
	float m_flNextDangerSoundTime;
	float m_flNextCreakSound;
	float m_flNextDropAllowedTime;
	float m_flSlowRaiseTime;

	// Speeds
	float m_flMaxExtensionSpeed;
	float m_flMaxTurnSpeed;
	float m_flExtensionAccel;
	float m_flExtensionDecel;
	float m_flTurnAccel;
	float m_flTurnDecel;

	// Cable Tip & Magnet
	string_t m_iszMagnetName;
	CHandle<CPhysMagnet> m_hCraneMagnet;
	CHandle<CCraneTip> m_hCraneTip;
	CHandle<CRopeKeyframe> m_hRope;
	IPhysicsConstraintGroup* m_pConstraintGroup;

	// Vehicle script filename
	string_t m_vehicleScript;
};
