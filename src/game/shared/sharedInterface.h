//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Exposes client-server neutral interfaces implemented in both places
//
// $NoKeywords: $
//=============================================================================//
#pragma once

class IFileSystem;
class CGaussianRandomStream;
class IEngineSound;
class IMapData;

extern IFileSystem				*filesystem;
extern CGaussianRandomStream *randomgaussian;
extern IEngineSound				*enginesound;
extern IMapData					*g_pMapData; // TODO: current implementations of the interface are in TF2, should probably move to TF2/HL2 neutral territory
