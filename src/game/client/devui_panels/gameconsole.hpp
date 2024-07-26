//=========== Copyright © 2024, Team Fennec, All rights reserved. =============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef IMGAMECONSOLE_H
#define IMGAMECONSOLE_H
#ifdef _WIN32
#pragma once
#endif

#include "GameUI/IGameConsole.h"
#include "imgui_window.h"

class im_ConLogItem
{
public:
	ImVec4 m_cColor;
	char m_sMsg[1024];
};

class ImGameConsoleDialog : public IImguiWindow, public IConsoleDisplayFunc
{
public:
	ImGameConsoleDialog();
	~ImGameConsoleDialog();

	bool Draw();

	void Clear();
	void ColorPrint( const Color &clr, const char *msg );
	void Print( const char *msg );
	void DPrint( const char *msg );

private:
	CUtlVector<im_ConLogItem> m_pLogItems;
};

DEFINE_IMGUI_WINDOW( ImGameConsoleDialog );

class ImGameConsole : public IGameConsole
{
public:
	ImGameConsole();
	~ImGameConsole();

	// sets up the console for use
	void Initialize();

	// activates the console, makes it visible and brings it to the foreground
	virtual void Activate();
	// hides the console
	virtual void Hide();
	// clears the console
	virtual void Clear();

	// returns true if the console is currently in focus
	virtual bool IsConsoleVisible();

	virtual void SetParent( int parent );
private:

	bool m_bInitialized;
};

extern ImGameConsole &GameConsole();

#endif // IMGAMECONSOLE_H
