//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//===========================================================================//
#pragma once
#include "appframework/IAppSystem.h"
#include "tier0/platform.h"

#include "inputsystem/AnalogCode.h"
#include "inputsystem/ButtonCode.h"
#include "inputsystem/InputEnums.h"

//-----------------------------------------------------------------------------
// Main interface for input. This is a low-level interface
//-----------------------------------------------------------------------------
#define INPUTSYSTEM_INTERFACE_VERSION "InputSystemVersion001"
abstract_class IInputSystem : public IAppSystem {
public:
	// Attach, detach input system from a particular window
	// This window should be the root window for the application
	// Only 1 window should be attached at any given time.
	virtual void AttachToWindow( void* hWnd ) = 0;
	virtual void DetachFromWindow() = 0;

	// Enables/disables input. PollInputState will not update current
	// button/analog states when it is called if the system is disabled.
	virtual void EnableInput( bool bEnable ) = 0;

	// Enables/disables the Windows message pump. PollInputState will not
	// Peek/Dispatch messages if this is disabled
	virtual void EnableMessagePump( bool bEnable ) = 0;

	// Polls the current input state
	virtual void PollInputState() = 0;

	// Gets the time of the last polling in ms
	[[nodiscard]]
    virtual int GetPollTick() const = 0;

	// Is a button down? "Buttons" are binary-state input devices (mouse buttons, keyboard keys)
	[[nodiscard]]
    virtual bool IsButtonDown( ButtonCode_t pCode ) const = 0;

	// Returns the tick at which the button was pressed and released
	[[nodiscard]]
    virtual int GetButtonPressedTick( ButtonCode_t pCode ) const = 0;
	[[nodiscard]]
    virtual int GetButtonReleasedTick( ButtonCode_t pCode ) const = 0;

	// Gets the value of an analog input device this frame
	// Includes joysticks, mousewheel, mouse
	[[nodiscard]]
    virtual int GetAnalogValue( AnalogCode_t pCode ) const = 0;

	// Gets the change in a particular analog input device this frame
	// Includes joysticks, mousewheel, mouse
	[[nodiscard]]
    virtual int GetAnalogDelta( AnalogCode_t pCode ) const = 0;

	// Returns the input events since the last poll
	[[nodiscard]]
    virtual int GetEventCount() const = 0;
	[[nodiscard]]
    virtual const InputEvent_t* GetEventData() const = 0;

	// Posts a user-defined event into the event queue; this is expected
	// to be called in overridden wndprocs connected to the root panel.
	virtual void PostUserEvent( const InputEvent_t& event ) = 0;

	// Returns the number of joysticks
	[[nodiscard]]
    virtual int GetJoystickCount() const = 0;

	// Enable/disable joystick, it has perf costs
	virtual void EnableJoystickInput( int nJoystick, bool bEnable ) = 0;

	// Enable/disable diagonal joystick POV (simultaneous POV buttons down)
	virtual void EnableJoystickDiagonalPOV( int nJoystick, bool bEnable ) = 0;

	// Sample the joystick and append events to the input queue
	virtual void SampleDevices() = 0;

	// FIXME: Currently force-feedback is only supported on the Xbox 360
	virtual void SetRumble( float fLeftMotor, float fRightMotor, int userId = INVALID_USER_ID ) = 0;
	virtual void StopRumble() = 0;

	// Resets the input state
	virtual void ResetInputState() = 0;

	// Sets a player as the primary user - all other controllers will be ignored.
	virtual void SetPrimaryUserId( int userId ) = 0;

	// Convert back + forth between ButtonCode/AnalogCode + strings
	[[nodiscard]]
    virtual const char* ButtonCodeToString( ButtonCode_t code ) const = 0;
	[[nodiscard]]
    virtual const char* AnalogCodeToString( AnalogCode_t code ) const = 0;
	[[nodiscard]]
    virtual ButtonCode_t StringToButtonCode( const char* pString ) const = 0;
	[[nodiscard]]
    virtual AnalogCode_t StringToAnalogCode( const char* pString ) const = 0;

	// Sleeps until input happens. Pass a negative number to sleep infinitely
	virtual void SleepUntilInput( int nMaxSleepTimeMS = -1 ) = 0;

	// Convert back + forth between virtual codes + button codes
	// FIXME: This is a temporary piece of code
	[[nodiscard]]
    virtual ButtonCode_t VirtualKeyToButtonCode( int nVirtualKey ) const = 0;
	[[nodiscard]]
    virtual int ButtonCodeToVirtualKey( ButtonCode_t code ) const = 0;
	[[nodiscard]]
    virtual ButtonCode_t ScanCodeToButtonCode( int lParam ) const = 0;

	// How many times have we called PollInputState?
	[[nodiscard]]
    virtual int GetPollCount() const = 0;

	// Sets the cursor position
	virtual void SetCursorPosition( int x, int y ) = 0;

	// NVNT get address to haptics interface
	[[nodiscard]]
    virtual void* GetHapticsInterfaceAddress() const = 0;

	virtual void SetNovintPure( bool pPure ) = 0;

	// read and clear accumulated raw input values
	virtual bool GetRawMouseAccumulators( int& accumX, int& accumY ) = 0;

	// tell the input system that we're not a game, we're console text mode.
	// this is used for dedicated servers to not initialize joystick system.
	// this needs to be called before CInputSystem::Init (e.g. in PreInit of
	// some system) if you want ot prevent the joystick system from ever
	// being initialized.
	virtual void SetConsoleTextMode( bool pConsoleTextMode ) = 0;
};
