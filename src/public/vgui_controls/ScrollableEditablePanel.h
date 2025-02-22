//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================
#pragma once
#include "vgui_controls/EditablePanel.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
namespace vgui {
	class ScrollBar;
}

namespace vgui {

	//-----------------------------------------------------------------------------
	// An editable panel that has a scrollbar
	//-----------------------------------------------------------------------------
	class ScrollableEditablePanel : public vgui::EditablePanel {
		DECLARE_CLASS_SIMPLE( ScrollableEditablePanel, vgui::EditablePanel );

	public:
		ScrollableEditablePanel( vgui::Panel* pParent, vgui::EditablePanel* pChild, const char* pName );
		virtual ~ScrollableEditablePanel() {}

		virtual void ApplySettings( KeyValues* pInResourceData );
		virtual void PerformLayout();

		vgui::ScrollBar* GetScrollbar( void ) { return m_pScrollBar; }

		MESSAGE_FUNC( OnScrollBarSliderMoved, "ScrollBarSliderMoved" );
		virtual void OnMouseWheeled( int delta );// respond to mouse wheel events

	private:
		vgui::ScrollBar* m_pScrollBar;
		vgui::EditablePanel* m_pChild;
	};
}// end namespace vgui