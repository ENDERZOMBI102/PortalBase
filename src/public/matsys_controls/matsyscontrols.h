//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#pragma once
#include "tier1/interface.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class IMaterialSystem;
class IMaterialSystemHardwareConfig;
class IMDLCache;
class IMatSystemSurface;
class IStudioRender;

namespace vgui {

	//-----------------------------------------------------------------------------
	// handles the initialization of the vgui interfaces.
	// NOTE: Calls into VGui_InitInterfacesList
	// interfaces (listed below) are first attempted to be loaded from primaryProvider, then secondaryProvider
	// moduleName should be the name of the module that this instance of the vgui_controls has been compiled into
	//-----------------------------------------------------------------------------
	bool VGui_InitMatSysInterfacesList( const char* moduleName, CreateInterfaceFn* factoryList, int numFactories );


	//-----------------------------------------------------------------------------
	// set of accessor functions to matsys interfaces
	// the appropriate header file for each is listed above the item
	//-----------------------------------------------------------------------------

	// #include <materialsystem/imaterialsystem.h>
	IMaterialSystem* MaterialSystem();

	// #include <datacache/imdlcache.h>
	IMDLCache* MDLCache();

	// #include <VGuiMatSurface/IMatSystemSurface.h>
	IMatSystemSurface* MatSystemSurface();

	// #include <istudiorender.h"
	IStudioRender* StudioRender();

	// #include <materialsystem/imaterialsystemhardwareconfig.h>
	IMaterialSystemHardwareConfig* MaterialSystemHardwareConfig();

}// end namespace vgui


//-----------------------------------------------------------------------------
// predeclare all the matsys control class names
//-----------------------------------------------------------------------------
class CMDLPanel;
class CMDLSequencePicker;
class CMDLPicker;
class CSequencePicker;
class CGameFileTreeView;
