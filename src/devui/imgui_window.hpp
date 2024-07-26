/*********************************************************************************
*  MIT License
*  
*  Copyright (c) 2023 Strata Source Contributors
*  
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*  
*  The above copyright notice and this permission notice shall be included in all
*  copies or substantial portions of the Software.
*  
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*  SOFTWARE.
*********************************************************************************/
#pragma once

#include "imgui.h"

#include "imgui_system.h"
#include "utlmap.h"

// Implemented by devui_static
extern void RegisterImGuiWindowFactory( IImguiWindow* pWindow );

//--------------------------------------------------------------------------------//
// Purpose: Interface implemented by each window. Use with DEFINE_DEVUI_WINDOW.
//--------------------------------------------------------------------------------//
class IImguiWindow {
public:
	IImguiWindow() = delete;

	IImguiWindow( const char* pszName, const char* pszTitle ) : m_pName( pszName ),
																m_pTitle( pszTitle ) {
		RegisterImGuiWindowFactory( this );
	}

	virtual ~IImguiWindow() = default;

	// Must be implemented by subclasses
	// Returns true to indicate if this window should continue to stay open
	virtual bool Draw() = 0;

	// Default implementations provided
	virtual bool ShouldDraw() { return m_bEnabled; }

	virtual void ToggleDraw() {
		m_bEnabled = !m_bEnabled;
		OnChangeVisibility();
	}

	virtual void SetDraw( bool bState ) {
		bool old = m_bEnabled;
		m_bEnabled = bState;
		if ( old != m_bEnabled )
			OnChangeVisibility();
	}

	// Returns window flags for this window, override this if you want (Or use DECLARE_DEVUI_WINDOW_F)
	virtual ImGuiWindowFlags GetFlags() const { return ImGuiWindowFlags_None; }

	// Returns the internal reference name of the window
	const char* GetName() const { return m_pName; }

	// Returns the window title. This will be passed to ImGui::Begin and displayed in the menu
	virtual const char* GetWindowTitle() const { return m_pTitle; }

	virtual void OnChangeVisibility() {}

protected:
	bool m_bEnabled = false;
	const char* m_pName;
	const char* m_pTitle;
};

// Defines an imgui window
#define DEFINE_IMGUI_WINDOW( _className ) \
	static _className __s_##_className##_registrar;


// Helper macro for simpler windows
#define DECLARE_IMGUI_WINDOW_F( name, _title, _flags )                \
	class CDearImGuiWindow##name : public IImguiWindow {              \
	public:                                                           \
		CDearImGuiWindow##name() : IImguiWindow( #name, _title ) {    \
		}                                                             \
		bool Draw() override;                                         \
		ImGuiWindowFlags GetFlags() const override { return _flags; } \
	};                                                                \
	static CDearImGuiWindow##name _s_dearimguiwindow##name;           \
	bool CDearImGuiWindow##name::Draw()


#define DECLARE_IMGUI_WINDOW( _name, _title ) DECLARE_IMGUI_WINDOW_F( _name, _title, ImGuiWindowFlags_None )