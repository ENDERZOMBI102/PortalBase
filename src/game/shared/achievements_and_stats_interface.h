//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================
#pragma once
#include "vgui/ISurface.h"
#include "vgui_controls/PHandle.h"
#include "vgui_controls/Panel.h"

class AchievementsAndStatsInterface {
public:
	AchievementsAndStatsInterface() = default;

	virtual void CreatePanel( vgui::Panel* pParent ) { }
	virtual void DisplayPanel() { }
	virtual void ReleasePanel() { }
	[[nodiscard]]
	virtual int GetAchievementsPanelMinWidth() const { return 0; }

protected:
	//-----------------------------------------------------------------------------
	// Purpose: Positions a dialog on screen.
	//-----------------------------------------------------------------------------
	void PositionDialog( vgui::PHandle dlg ) {
		if ( !dlg.Get() ) {
			return;
		}

		int x, y, ww, wt, wide, tall;
		vgui::surface()->GetWorkspaceBounds( x, y, ww, wt );
		dlg->GetSize( wide, tall );

		// Center it, keeping requested size
		dlg->SetPos( x + ( ww - wide ) / 2, y + ( wt - tall ) / 2 );
	}
};
