//=========== Copyright © 2024, Team Fennec, All rights reserved. =============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#include <stdio.h>

#include "gameconsole.hpp"

#include "convar.h"
#include "convar_serverbounded.h"
#include "icvar.h"
#include "imgui.h"
#include "KeyValues.h"
#include "cdll_client_int.h"

// memdbgon must be the last include file in a .cpp file!!!
// literally 1984
#include "tier0/memdbgon.h"

static ImGameConsole g_GameConsole;
//-----------------------------------------------------------------------------
// Purpose: singleton accessor
//-----------------------------------------------------------------------------
ImGameConsole &GameConsole()
{
	return g_GameConsole;
}
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( ImGameConsole, IGameConsole, GAMECONSOLE_INTERFACE_VERSION, g_GameConsole );

ImGameConsoleDialog::ImGameConsoleDialog()
	: IImguiWindow( "imGameConsole", "Game Console" )
{
}

ImGameConsoleDialog::~ImGameConsoleDialog()
{
	// just fucking leak it tbh, I don't give a shit dude, when the engine closes it gets freed anyways.
	//m_pLogItems.PurgeAndDeleteElements();
}

bool ImGameConsoleDialog::Draw()
{
	static char g_gcInput[2048] = "";
	// assemble the size for the history child
	ImVec2 historySize = ImGui::GetContentRegionAvail();
	historySize.y -= 30;

	if ( ImGui::BeginChild( "gameconsole_history", historySize ) )
	{
		// render log
		ImGui::PushTextWrapPos( ImGui::GetContentRegionAvail().x );
		for ( size_t i = 0; i < m_pLogItems.Count(); i++ )
		{
			ImGui::PushStyleColor( ImGuiCol_Text, m_pLogItems[i].m_cColor);
			// what the fuck
			Assert( m_pLogItems[i].m_sMsg != null );

			ImGui::TextUnformatted( m_pLogItems[i].m_sMsg );
			ImGui::PopStyleColor();
		}
		ImGui::PopTextWrapPos();
		
		// auto scroll
		if ( ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 50 )
			ImGui::SetScrollY( ImGui::GetScrollMaxY() );

		ImGui::EndChild();
	}

	ImGui::InputText( "##gameconsole_inputfield", g_gcInput, 2047, ImGuiInputTextFlags_EnterReturnsTrue );
	ImGui::SameLine();
	bool handleInput2 = ImGui::Button( "Submit##gameconsole_submit" );
	

	if ( handleInput2 )
	{
		// submit this as a console command
		engine->ExecuteClientCmd( g_gcInput );
		// clear input
		// hack: cstrings dude I fucking hate them
		char *inp = &g_gcInput[0];
		inp = "\0";
	}

	// idfk
	return ShouldDraw();
}

void ImGameConsoleDialog::Clear()
{
	m_pLogItems.RemoveAll();
}

void ImGameConsoleDialog::ColorPrint( const Color &clr, const char *msg )
{
	// convert rgb color to a proper color imvec4
	ImVec4 imColor;
	imColor.x = ( float )( clr.r() / 255 );
	imColor.y = ( float )( clr.g() / 255 );
	imColor.z = ( float )( clr.b() / 255 );
	imColor.w = ( float )( clr.a() / 255 );

	im_ConLogItem logItem;
	logItem.m_cColor = imColor;
	// if it's longer than that god help you because I won't
	V_strncpy( logItem.m_sMsg, msg, 1023 );

	m_pLogItems.AddToTail( logItem );
}

void ImGameConsoleDialog::Print( const char *msg )
{
	ColorPrint( Color( 255, 255, 255, 255 ), msg );
}

void ImGameConsoleDialog::DPrint( const char *msg )
{
	ColorPrint( Color( 150, 150, 150, 255 ), msg );
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
ImGameConsole::ImGameConsole()
{
	m_bInitialized = false;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
ImGameConsole::~ImGameConsole()
{
	m_bInitialized = false;
}

//-----------------------------------------------------------------------------
// Purpose: sets up the console for use
//-----------------------------------------------------------------------------
void ImGameConsole::Initialize()
{
	// register the console
	g_pCVar->InstallConsoleDisplayFunc( &__s_ImGameConsoleDialog_registrar );

#if 0 // todo: revisit this someday, maybe add the ability to set the position and size of an imgui window
	// set the console to taking up most of the right-half of the screen
	int swide, stall;
	vgui::surface()->GetScreenSize( swide, stall );
	// lmao we use vgui to set the position and scaling of an imgui window maybe we shouldn't lol
	int offset = vgui::scheme()->GetProportionalScaledValue( 16 );

	m_pConsole->SetBounds(
		swide / 2 - ( offset * 4 ),
		offset,
		( swide / 2 ) + ( offset * 3 ),
		stall - ( offset * 8 ) );
#endif

	m_bInitialized = true;
}

//-----------------------------------------------------------------------------
// Purpose: activates the console, makes it visible and brings it to the foreground
//-----------------------------------------------------------------------------
void ImGameConsole::Activate()
{
	if ( !m_bInitialized )
		return;

	g_pImguiSystem->SetWindowVisible( g_pImguiSystem->FindWindow( "imGameConsole" ), true );
}

//-----------------------------------------------------------------------------
// Purpose: hides the console
//-----------------------------------------------------------------------------
void ImGameConsole::Hide()
{
	if ( !m_bInitialized )
		return;

	g_pImguiSystem->SetWindowVisible( g_pImguiSystem->FindWindow( "imGameConsole" ), false );
}

//-----------------------------------------------------------------------------
// Purpose: clears the console
//-----------------------------------------------------------------------------
void ImGameConsole::Clear()
{
	if ( !m_bInitialized )
		return;

	ImGameConsoleDialog *gameConsole = ( ImGameConsoleDialog* )g_pImguiSystem->FindWindow( "imGameConsole" );
	gameConsole->Clear();
}


//-----------------------------------------------------------------------------
// Purpose: returns true if the console is currently in focus
//-----------------------------------------------------------------------------
bool ImGameConsole::IsConsoleVisible()
{
	if ( !m_bInitialized )
		return false;

	return false;
}

void ImGameConsole::SetParent( int parent )
{
	// absolutely fuck all: we don't need this
}
