//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================
#pragma once
#include "hud.h"
#include "hudelement.h"
#include <game/client/iviewport.h>
#include <vgui/IScheme.h>
#include <vgui_controls/EditablePanel.h>

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Replay reminder panel
//-----------------------------------------------------------------------------
class CReplayReminderPanel : public EditablePanel, public CHudElement {
	DECLARE_CLASS_SIMPLE( CReplayReminderPanel, vgui::EditablePanel );

public:
	CReplayReminderPanel( const char* pElementName );

	void Hide();// To be used by HUD only
	void Show();// To be used by HUD only

	// CHudElement overrides
	virtual bool ShouldDraw();
	virtual void OnThink();
	virtual int HudElementKeyInput( int down, ButtonCode_t keynum, const char* pszCurrentBinding );

	// EditablePanel overrides
	virtual void ApplySchemeSettings( IScheme* pScheme );
	virtual void SetVisible( bool bState );

private:
	void SetupText();

	float m_flShowTime;// Used by the HUD only, to display the panel only for a certain period of time
	bool m_bShouldDraw;// Store this state for ShouldDraw(), which allows us to use a single panel for
					   // both the post-win reminder and the freezepanel reminder.
};
