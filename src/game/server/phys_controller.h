//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//
#pragma once


CBaseEntity* CreateKeepUpright( const Vector& vecOrigin, const QAngle& vecAngles, CBaseEntity* pOwner, float flAngularLimit, bool bActive );

AngularImpulse ComputeRotSpeedToAlignAxes( const Vector& testAxis, const Vector& alignAxis, const AngularImpulse& currentSpeed,
										   float damping, float scale, float maxSpeed );
