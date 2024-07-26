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
#include "cbase.h"

#include "imgui_system.hpp"
#include "imgui_window.hpp"

#include "filesystem.h"
#include "fmtstr.h"
#include "imgui.h"
#include "imgui_impl_source.hpp"
#include "imgui_internal.h"
#include "inputsystem/iinputsystem.h"
#include "materialsystem/imaterialsystem.h"
#include "strtools.h"
#include "tier2/tier2.h"
#include "tier3/tier3.h"
#include "utldict.h"

#include "ienginevgui.h"
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Panel.h>

#include "tier0/memdbgon.h"

class CDummyOverlayPanel;

CUtlVector<IImguiWindow*>& ImGuiWindows();

static ConVar imgui_font_scale( "imgui_font_scale", "1", FCVAR_ARCHIVE, "Global scale applied to Imgui fonts" );
static ConVar imgui_display_scale( "imgui_display_scale", "1", FCVAR_ARCHIVE, "Global imgui scale, usually used for Hi-DPI displays" );

void* ImGui_MemAlloc( size_t sz, void* user_data ) {
	return MemAlloc_Alloc( sz, "Dear ImGui", 0 );
}

static void ImGui_MemFree( void* ptr, void* user_data ) {
	MemAlloc_Free( ptr );
}

//---------------------------------------------------------------------------------------//
// Global helpers
//---------------------------------------------------------------------------------------//

CUtlVector<IImguiWindow*>& ImGuiWindows() {
	static CUtlVector<IImguiWindow*> s_DearImGuiWindows;
	return s_DearImGuiWindows;
}

void RegisterImGuiWindowFactory( IImguiWindow* pWindow ) {
	ImGuiWindows().AddToTail( pWindow );
}


//---------------------------------------------------------------------------------------//
// Purpose: Implementation of the imgui system
//---------------------------------------------------------------------------------------//
class CDearImGuiSystem : public IImguiSystem {
	using BaseClass = IImguiSystem;

public:
	// IAppSystem
	bool Init() override;
	void Shutdown() override;

	// IDearImGuiSystem
	DearImGuiSysData_t GetData() override;
	void Render() override;
	void RegisterWindowFactories( IImguiWindow** arrpWindows, int nCount ) override;
	IImguiWindow* FindWindow( const char* szName ) override;
	void UnregisterWindowFactories( IImguiWindow** ppWindows, int nCount ) override;
	void GetAllWindows( CUtlVector<IImguiWindow*>& windows ) override;
	void SetWindowVisible( IImguiWindow* pWindow, bool bVisible, bool bEnableInput ) override;

	bool DrawWindow( IImguiWindow* pWindow );

	void PushInputContext();
	void PopInputContext();

	// True if our input context is enabled in any capacity
	bool IsInputContextEnabled() const { return m_bInputEnabled; }

	void SetStyle();
	void DrawMenuBar();

	void SetDrawMenuBar( bool bEnable ) { m_bDrawMenuBar = bEnable; }
	bool IsDrawingMenuBar() const { return m_bDrawMenuBar; }
	void ToggleMenuBar() { m_bDrawMenuBar = !m_bDrawMenuBar; }

public:
	CUtlDict<IImguiWindow*> m_ImGuiWindows;

	double m_flLastFrameTime;
	bool m_bInputEnabled = false;

	bool m_bDrawMenuBar = false;
	bool m_bDrawMetrics = false;
	bool m_bDrawDemo = false;
	CDummyOverlayPanel* m_pInputOverlay = nullptr;
};

static CDearImGuiSystem g_ImguiSystem;
IImguiSystem* g_pImguiSystem = &g_ImguiSystem;

//---------------------------------------------------------------------------------------//
// Purpose: Dummy overlay panel for capturing input
//---------------------------------------------------------------------------------------//
class CDummyOverlayPanel : public vgui::Panel {
public:
	CDummyOverlayPanel() {
		SetVisible( false );
		SetParent( enginevgui->GetPanel( PANEL_GAMEUIDLL ) );
		SetPaintEnabled( true );
		SetCursor( vgui::dc_arrow );

		SetPaintBorderEnabled( false );
		SetPaintBackgroundEnabled( false );
		MakePopup();
	}

	void OnMousePressed( ButtonCode_t code ) override {
		auto& io = ImGui::GetIO();
		if ( io.WantCaptureMouse )
			io.AddMouseButtonEvent( code - MOUSE_FIRST, true );
	}

	void OnMouseReleased( ButtonCode_t code ) override {
		auto& io = ImGui::GetIO();
		if ( io.WantCaptureMouse ) {
			io.AddMouseButtonEvent( code - MOUSE_FIRST, false );
		}
	}

	void OnMouseWheeled( int delta ) override {
		auto& io = ImGui::GetIO();
		io.AddMouseWheelEvent( 0, delta );
	}

	void OnCursorMoved( int x, int y ) override {
		vgui::Panel::OnCursorMoved( x, y );
		ImGui::GetIO().AddMousePosEvent( x, y );
	}

	void OnMouseDoublePressed( ButtonCode_t code ) override {
	}

	void OnKeyTyped( wchar_t code ) override {
		auto& io = ImGui::GetIO();
		io.AddInputCharacter( code );
	}

	// always pass keycodes to imgui, WantCaptureKeyboard should NOT affect whether or not we do this
	void OnKeyCodePressed( vgui::KeyCode code ) override {
		if ( code > KEY_LAST || code < KEY_FIRST )
			return;

		auto& io = ImGui::GetIO();
		io.AddKeyEvent( IMGUI_KEY_TABLE[ code ], true );
	}

	void OnKeyCodeReleased( vgui::KeyCode code ) override {
		if ( code > KEY_LAST || code < KEY_FIRST )
			return;

		auto& io = ImGui::GetIO();
		io.AddKeyEvent( IMGUI_KEY_TABLE[ code ], false );

		// hack: input fields are fucked
		if ( io.WantTextInput ) {
			io.AddInputCharacter( ' ' );
			io.AddKeyEvent( ImGuiKey_Backspace, true );
			io.AddKeyEvent( ImGuiKey_Backspace, false );
		}
	}

	void Paint() override {
		// hack: bad dumb stupid, enter is a bitch make it false by the end of the frame please
		ImGui::GetIO().AddKeyEvent( ImGuiKey_Enter, false );

		g_pImguiSystem->Render();
	}

	void Activate( bool bActive ) {
		SetVisible( bActive );
		SetEnabled( bActive );
		SetKeyBoardInputEnabled( bActive );
		SetMouseInputEnabled( bActive );

		if ( bActive ) {
			MoveToFront();
			RequestFocus();
			vgui::input()->SetMouseCapture( NULL );
		}
	}
};

//---------------------------------------------------------------------------------------//
// Purpose: Init our dear friend
//---------------------------------------------------------------------------------------//
bool CDearImGuiSystem::Init() {
	ImGui::SetAllocatorFunctions( ImGui_MemAlloc, ImGui_MemFree, nullptr );
	ImFontAtlas* atlas = new ImFontAtlas();
	ImGui::CreateContext( atlas );
	ImGui_ImplSource_Init();

	m_flLastFrameTime = Plat_FloatTime();

	DearImGuiSysData_t data = g_pImguiSystem->GetData();

	ImGui::SetAllocatorFunctions( (ImGuiMemAllocFunc) data.memallocfn, (ImGuiMemFreeFunc) data.memfreefn, nullptr );
	ImGui::SetCurrentContext( (ImGuiContext*) data.context );

	g_pImguiSystem->RegisterWindowFactories( ImGuiWindows().Base(), ImGuiWindows().Count() );

	// moved to fix weird skin color issues???
	SetStyle();

	return true;
}

void CDearImGuiSystem::Shutdown() {
	g_pImguiSystem->UnregisterWindowFactories( ImGuiWindows().Base(), ImGuiWindows().Count() );
	ImGui_ImplSource_Shutdown();

	ImGui::DestroyContext();
}

DearImGuiSysData_t CDearImGuiSystem::GetData() {
	DearImGuiSysData_t data;
	data.context = ImGui::GetCurrentContext();
	data.memallocfn = ImGui_MemAlloc;
	data.memfreefn = ImGui_MemFree;
	return data;
}

//---------------------------------------------------------------------------------------//
// Purpose: Render all imgui windows
//---------------------------------------------------------------------------------------//
void CDearImGuiSystem::Render() {
	// Update the IO
	auto& io = ImGui::GetIO();

	// Create input overlay helper if it doesn't yet exist
	if ( !m_pInputOverlay ) {
		m_pInputOverlay = new CDummyOverlayPanel();
	}

	// Update the screen size before drawing
	CMatRenderContextPtr pRenderContext( materials );
	int w, h;
	pRenderContext->GetWindowSize( w, h );
	if ( !w || !h )
		return;

	m_pInputOverlay->SetSize( w, h );

	io.DisplaySize.x = static_cast<float>( w );
	io.DisplaySize.y = static_cast<float>( h );
	io.DisplayFramebufferScale.x = io.DisplayFramebufferScale.y = imgui_display_scale.GetFloat();
	io.FontGlobalScale = imgui_font_scale.GetFloat();

	// Create new frame
	ImGui::NewFrame();

	// Draw menubar first
	if ( m_bDrawMenuBar )
		DrawMenuBar();

	// Draw imgui-specific debug menus
	if ( m_bDrawDemo )
		ImGui::ShowDemoWindow( &m_bDrawDemo );

	if ( m_bDrawMetrics )
		ImGui::ShowMetricsWindow( &m_bDrawMetrics );

	// Draw everything else
	bool bDrawn = false;
	FOR_EACH_DICT( m_ImGuiWindows, i ) {
		auto* pWindow = m_ImGuiWindows[ i ];
		if ( pWindow->ShouldDraw() ) {
			DrawWindow( pWindow );
			bDrawn = true;
		}
	}

	ImGui::Render();
	ImDrawData* drawdata = ImGui::GetDrawData();
	if ( drawdata )
		ImGui_ImplSource_RenderDrawData( drawdata );

	// Post render, update deltas
	auto curtime = Plat_FloatTime();
	auto dt = curtime - m_flLastFrameTime;
	m_flLastFrameTime = curtime;

	io.DeltaTime = static_cast<float>( dt );

	// Deactivate our overlay if nothing is being drawn anymore
	if ( !bDrawn && !m_bDrawDemo && !m_bDrawMetrics && !m_bDrawMenuBar ) {
		PopInputContext();
	}
}

//---------------------------------------------------------------------------------------//
// Purpose: Draws a window
//---------------------------------------------------------------------------------------//
bool CDearImGuiSystem::DrawWindow( IImguiWindow* pWindow ) {
	bool closeButton = pWindow->ShouldDraw();
	ImGui::Begin( pWindow->GetWindowTitle(), &closeButton, pWindow->GetFlags() );
	pWindow->SetDraw( closeButton );

	bool stayOpen = pWindow->Draw();
	ImGui::End();
	return stayOpen;
}

//---------------------------------------------------------------------------------------//
// Purpose: Register all window factories from another DLL
//---------------------------------------------------------------------------------------//
void CDearImGuiSystem::RegisterWindowFactories( IImguiWindow** arrpWindows, int nCount ) {
	for ( int i = 0; i < nCount; i++ ) {
		auto* pWindow = arrpWindows[ i ];
		m_ImGuiWindows.Insert( pWindow->GetName(), pWindow );
	}
}

//---------------------------------------------------------------------------------------//
// Purpose: Unregister window factories, usually called on DLL shutdown
//---------------------------------------------------------------------------------------//
void CDearImGuiSystem::UnregisterWindowFactories( IImguiWindow** ppWindows, int nCount ) {
	for ( int i = 0; i < nCount; ++i ) {
		m_ImGuiWindows.Remove( ppWindows[ i ]->GetName() );
	}
}

//---------------------------------------------------------------------------------------//
// Purpose: find window by name
//---------------------------------------------------------------------------------------//
IImguiWindow* CDearImGuiSystem::FindWindow( const char* szName ) {
	// note: We don't have C++17 in this file apparently so we have to init outside of if
	auto it = m_ImGuiWindows.Find( szName );
	if ( it != m_ImGuiWindows.InvalidIndex() )
		return m_ImGuiWindows[ it ];
	return nullptr;
}

//---------------------------------------------------------------------------------------//
// Purpose: Returns a list of all windows into `windows`
//---------------------------------------------------------------------------------------//
void CDearImGuiSystem::GetAllWindows( CUtlVector<IImguiWindow*>& windows ) {
	windows.Purge();
	FOR_EACH_DICT( m_ImGuiWindows, i )
	windows.AddToTail( m_ImGuiWindows[ i ] );
}

//---------------------------------------------------------------------------------------//
// Purpose: Make a window visible
//---------------------------------------------------------------------------------------//
void CDearImGuiSystem::SetWindowVisible( IImguiWindow* pWindow, bool bVisible, bool bEnableInput ) {
	Assert( pWindow );
	pWindow->SetDraw( bVisible );
	if ( bVisible && bEnableInput )
		PushInputContext();
	else if ( !bVisible && bEnableInput )
		PopInputContext();
}

//---------------------------------------------------------------------------------------//
// Purpose: Push a new input context so we can show the mouse cursor
//---------------------------------------------------------------------------------------//
void CDearImGuiSystem::PushInputContext() {
	if ( m_bInputEnabled )
		return;

	if ( !m_pInputOverlay )
		m_pInputOverlay = new CDummyOverlayPanel();

	m_pInputOverlay->Activate( true );
	m_bInputEnabled = true;
}

//---------------------------------------------------------------------------------------//
// Purpose: Pop input context, returning mouse control to whatever needed it last
//---------------------------------------------------------------------------------------//
void CDearImGuiSystem::PopInputContext() {
	m_pInputOverlay->Activate( false );
	m_bInputEnabled = false;
}

//---------------------------------------------------------------------------------------//
// Purpose: Update Imgui style
//  TODO: User-configurable styles
//---------------------------------------------------------------------------------------//
void CDearImGuiSystem::SetStyle() {
	// fixme: this causes really weird results???????
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ ImGuiCol_Text ] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
	style.Colors[ ImGuiCol_TextDisabled ] = ImVec4( 0.50f, 0.50f, 0.50f, 1.00f );
	style.Colors[ ImGuiCol_WindowBg ] = ImVec4( 0.13f, 0.14f, 0.15f, 1.00f );
	style.Colors[ ImGuiCol_ChildBg ] = ImVec4( 0.13f, 0.14f, 0.15f, 1.00f );
	style.Colors[ ImGuiCol_PopupBg ] = ImVec4( 0.13f, 0.14f, 0.15f, 1.00f );
	style.Colors[ ImGuiCol_Border ] = ImVec4( 0.43f, 0.43f, 0.50f, 0.50f );
	style.Colors[ ImGuiCol_BorderShadow ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
	style.Colors[ ImGuiCol_FrameBg ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
	style.Colors[ ImGuiCol_FrameBgHovered ] = ImVec4( 0.38f, 0.38f, 0.38f, 1.00f );
	style.Colors[ ImGuiCol_FrameBgActive ] = ImVec4( 0.67f, 0.67f, 0.67f, 0.39f );
	style.Colors[ ImGuiCol_TitleBg ] = ImVec4( 0.08f, 0.08f, 0.09f, 1.00f );
	style.Colors[ ImGuiCol_TitleBgActive ] = ImVec4( 0.08f, 0.08f, 0.09f, 1.00f );
	style.Colors[ ImGuiCol_TitleBgCollapsed ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.51f );
	style.Colors[ ImGuiCol_MenuBarBg ] = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
	style.Colors[ ImGuiCol_ScrollbarBg ] = ImVec4( 0.02f, 0.02f, 0.02f, 0.53f );
	style.Colors[ ImGuiCol_ScrollbarGrab ] = ImVec4( 0.31f, 0.31f, 0.31f, 1.00f );
	style.Colors[ ImGuiCol_ScrollbarGrabHovered ] = ImVec4( 0.41f, 0.41f, 0.41f, 1.00f );
	style.Colors[ ImGuiCol_ScrollbarGrabActive ] = ImVec4( 0.51f, 0.51f, 0.51f, 1.00f );
	style.Colors[ ImGuiCol_CheckMark ] = ImVec4( 0.11f, 0.64f, 0.92f, 1.00f );
	style.Colors[ ImGuiCol_SliderGrab ] = ImVec4( 0.11f, 0.64f, 0.92f, 1.00f );
	style.Colors[ ImGuiCol_SliderGrabActive ] = ImVec4( 0.08f, 0.50f, 0.72f, 1.00f );
	style.Colors[ ImGuiCol_Button ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
	style.Colors[ ImGuiCol_ButtonHovered ] = ImVec4( 0.38f, 0.38f, 0.38f, 1.00f );
	style.Colors[ ImGuiCol_ButtonActive ] = ImVec4( 0.67f, 0.67f, 0.67f, 0.39f );
	style.Colors[ ImGuiCol_Header ] = ImVec4( 0.22f, 0.22f, 0.22f, 1.00f );
	style.Colors[ ImGuiCol_HeaderHovered ] = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
	style.Colors[ ImGuiCol_HeaderActive ] = ImVec4( 0.67f, 0.67f, 0.67f, 0.39f );
	style.Colors[ ImGuiCol_Separator ] = style.Colors[ ImGuiCol_Border ];
	style.Colors[ ImGuiCol_SeparatorHovered ] = ImVec4( 0.41f, 0.42f, 0.44f, 1.00f );
	style.Colors[ ImGuiCol_SeparatorActive ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.95f );
	style.Colors[ ImGuiCol_ResizeGrip ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
	style.Colors[ ImGuiCol_ResizeGripHovered ] = ImVec4( 0.29f, 0.30f, 0.31f, 0.67f );
	style.Colors[ ImGuiCol_ResizeGripActive ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.95f );
	style.Colors[ ImGuiCol_Tab ] = ImVec4( 0.08f, 0.08f, 0.09f, 0.83f );
	style.Colors[ ImGuiCol_TabHovered ] = ImVec4( 0.33f, 0.34f, 0.36f, 0.83f );
	style.Colors[ ImGuiCol_TabActive ] = ImVec4( 0.23f, 0.23f, 0.24f, 1.00f );
	style.Colors[ ImGuiCol_TabUnfocused ] = ImVec4( 0.08f, 0.08f, 0.09f, 1.00f );
	style.Colors[ ImGuiCol_TabUnfocusedActive ] = ImVec4( 0.13f, 0.14f, 0.15f, 1.00f );
	style.Colors[ ImGuiCol_PlotLines ] = ImVec4( 0.61f, 0.61f, 0.61f, 1.00f );
	style.Colors[ ImGuiCol_PlotLinesHovered ] = ImVec4( 1.00f, 0.43f, 0.35f, 1.00f );
	style.Colors[ ImGuiCol_PlotHistogram ] = ImVec4( 0.90f, 0.70f, 0.00f, 1.00f );
	style.Colors[ ImGuiCol_PlotHistogramHovered ] = ImVec4( 1.00f, 0.60f, 0.00f, 1.00f );
	style.Colors[ ImGuiCol_TextSelectedBg ] = ImVec4( 0.26f, 0.59f, 0.98f, 0.35f );
	style.Colors[ ImGuiCol_DragDropTarget ] = ImVec4( 0.11f, 0.64f, 0.92f, 1.00f );
	style.Colors[ ImGuiCol_NavHighlight ] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
	style.Colors[ ImGuiCol_NavWindowingHighlight ] = ImVec4( 1.00f, 1.00f, 1.00f, 0.70f );
	style.Colors[ ImGuiCol_NavWindowingDimBg ] = ImVec4( 0.80f, 0.80f, 0.80f, 0.20f );
	style.Colors[ ImGuiCol_ModalWindowDimBg ] = ImVec4( 0.80f, 0.80f, 0.80f, 0.35f );
	style.GrabRounding = style.FrameRounding = 2.3f;
}

//---------------------------------------------------------------------------------------//
// Purpose: Draws the menu bar as a top-level window
//---------------------------------------------------------------------------------------//
void CDearImGuiSystem::DrawMenuBar() {
	if ( ImGui::BeginMainMenuBar() ) {
		if ( ImGui::BeginMenu( "File" ) ) {
			if ( ImGui::MenuItem( "Close Menu" ) ) {
				this->m_bDrawMenuBar = false;
				this->PopInputContext();
			}
			ImGui::EndMenu();
		}
		if ( ImGui::BeginMenu( "Windows" ) ) {
			CUtlVector<IImguiWindow*> windows;
			g_pImguiSystem->GetAllWindows( windows );

			for ( auto& window: windows ) {
				bool bToggle = window->ShouldDraw();
				if ( ImGui::MenuItem( window->GetWindowTitle(), "", &bToggle ) )
					window->SetDraw( bToggle );
			}

			ImGui::EndMenu();
		}

		if ( ImGui::BeginMenu( "Debug" ) ) {
			ImGui::MenuItem( "Show Demo Window", "", &m_bDrawDemo );
			ImGui::MenuItem( "Show Metrics Window", "", &m_bDrawMetrics );

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

//---------------------------------------------------------------------------------------//
// Purpose: Implementation of imgui_show command
//---------------------------------------------------------------------------------------//
class CImGuiShowAutoCompletionFunctor : public ICommandCallback,
										public ICommandCompletionCallback {
public:
	void CommandCallback( const CCommand& command ) override {
		if ( command.ArgC() < 2 ) {
			Msg( "Format: imgui_show <window name>\n" );
			return;
		}

		// Get our window
		const char* pKey = command.Arg( 1 );
		auto* pWindow = g_ImguiSystem.FindWindow( pKey );

		if ( !pWindow ) {
			Warning( "Failed to find ImGUI window called %s\n", pKey );
			return;
		}

		pWindow->ToggleDraw();
	}

	int CommandCompletionCallback( const char* partial, CUtlVector<CUtlString>& commands ) override {
		auto& windows = g_ImguiSystem.m_ImGuiWindows;
		FOR_EACH_DICT( windows, i ) {
			commands.AddToTail( CFmtStr( "%s %s", "imgui_show", windows[ i ]->GetName() ).Get() );
		}

		return commands.Count();
	}
};

static CImGuiShowAutoCompletionFunctor g_ImGuiShowAutoComplete;
static ConCommand imgui_show( "imgui_show", &g_ImGuiShowAutoComplete, "Toggles the specified imgui window", FCVAR_CLIENTDLL, &g_ImGuiShowAutoComplete );

//---------------------------------------------------------------------------------------//
// Purpose: Toggle input for imgui windows
//---------------------------------------------------------------------------------------//
CON_COMMAND_F( imgui_toggle_input, "Toggles the mouse cursor for imgui windows", FCVAR_CLIENTDLL ) {
	if ( g_ImguiSystem.IsInputContextEnabled() )
		g_ImguiSystem.PopInputContext();
	else
		g_ImguiSystem.PushInputContext();
}

static ConCommand imgui_input_start(
	"+imgui_input", []( const CCommand& args ) {
		g_ImguiSystem.PushInputContext();
	},
	"Toggles the mouse cursor for imgui windows, same as imgui_toggle_input", FCVAR_CLIENTDLL );

static ConCommand imgui_input_end(
	"-imgui_input", []( const CCommand& args ) {
		g_ImguiSystem.PopInputContext();
	},
	"Toggles the mouse cursor for imgui windows, same as imgui_toggle_input", FCVAR_CLIENTDLL );

//---------------------------------------------------------------------------------------//
// Purpose: Toggles the built-in menu bar (and input)
//---------------------------------------------------------------------------------------//

CON_COMMAND_F( imgui_toggle_menu, "Toggles the imgui menu bar", FCVAR_CLIENTDLL ) {
	g_ImguiSystem.ToggleMenuBar();
	if ( g_ImguiSystem.IsDrawingMenuBar() )
		g_ImguiSystem.PushInputContext();
	else
		g_ImguiSystem.PopInputContext();
}

// hack: fuck you I can't template this
void CC_EnableMenu( const CCommand& args ) {
	g_ImguiSystem.SetDrawMenuBar( true );
	g_ImguiSystem.PushInputContext();
}

void CC_DisableMenu( const CCommand& args ) {
	g_ImguiSystem.SetDrawMenuBar( false );
	g_ImguiSystem.PopInputContext();
}

static ConCommand imgui_menu_start( "+imgui_menu", CC_EnableMenu, "Toggles the menu bar and input", FCVAR_CLIENTDLL );
static ConCommand imgui_menu_end( "-imgui_menu", CC_DisableMenu, "Toggles the menu bar and input", FCVAR_CLIENTDLL );

CON_COMMAND_F( imgui_show_demo, "Shows the imgui demo", FCVAR_CLIENTDLL ) {
	g_ImguiSystem.m_bDrawDemo = true;
}