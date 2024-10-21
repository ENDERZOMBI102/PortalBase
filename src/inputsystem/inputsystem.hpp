//
// Created by ENDERZOMBI102 on 11/09/2023.
//
#pragma once
#include "inputsystem/iinputsystem.h"
#include "tier1/tier1.h"
#include "utlqueue.h"

#include <SDL_joystick.h>


class CInputSystem : public CTier1AppSystem<IInputSystem> {
	using BaseClass = CTier1AppSystem<IInputSystem>;
public:  // IAppSystem
	InitReturnVal_t Init() override;
	void Shutdown() override;
public:  // IInputSystem
	void AttachToWindow( void* hWnd ) override;
	void DetachFromWindow() override;

	void EnableInput( bool bEnable ) override;

	void EnableMessagePump( bool bEnable ) override;

	void PollInputState() override;

	int GetPollTick() const override;

	bool IsButtonDown( ButtonCode_t pCode ) const override;

	int GetButtonPressedTick( ButtonCode_t pCode ) const override;
	int GetButtonReleasedTick( ButtonCode_t pCode ) const override;

	int GetAnalogValue( AnalogCode_t code ) const override;

	int GetAnalogDelta( AnalogCode_t code ) const override;

	int GetEventCount() const override;
	const InputEvent_t* GetEventData( ) const override;

	void PostUserEvent( const InputEvent_t &event ) override;

	int GetJoystickCount() const override;

	void EnableJoystickInput( int nJoystick, bool bEnable ) override;

	void EnableJoystickDiagonalPOV( int nJoystick, bool bEnable ) override;

	void SampleDevices() override;

	void SetRumble( float fLeftMotor, float fRightMotor, int userId = INVALID_USER_ID ) override;
	void StopRumble() override;

	void ResetInputState() override;

	void SetPrimaryUserId( int userId ) override;

	const char* ButtonCodeToString( ButtonCode_t pCode ) const override;
	const char* AnalogCodeToString( AnalogCode_t pCode ) const override;
	ButtonCode_t StringToButtonCode( const char *pString ) const override;
	AnalogCode_t StringToAnalogCode( const char *pString ) const override;

	void SleepUntilInput( int nMaxSleepTimeMS = -1 ) override;

	ButtonCode_t VirtualKeyToButtonCode( int nVirtualKey ) const override;
	int ButtonCodeToVirtualKey( ButtonCode_t pCode ) const override;
	ButtonCode_t ScanCodeToButtonCode( int lParam ) const override;

	int GetPollCount() const override;

	void SetCursorPosition( int x, int y ) override;

	void* GetHapticsInterfaceAddress() const override;

	void SetNovintPure( bool pPure ) override;

	bool GetRawMouseAccumulators( int& accumX, int& accumY ) override;

	void SetConsoleTextMode( bool pConsoleTextMode ) override;
private:
	class CMessagePumpThread : public CThread {
		int Run() override;
	};
	struct GamepadState {
		bool rumbling{};
		SDL_Joystick* handle{};
	};
	struct ButtonState {
		bool pressed{ false };
		int32 pressTick{};
		int32 releaseTick{};
	};
private:
	SDL_Window* m_SdlWindow{nullptr};

	bool m_Enabled{false};
	bool m_Running{false};
	bool m_ConsoleTextMode{false};
	int32 m_PoolCount{0};

	int32 m_MouseAccX{};
	int32 m_MouseAccY{};
	int32 m_PrimaryPadUserId{};
	GamepadState m_Gamepads[4] { };
	ButtonState m_Buttons[ButtonCode_t::BUTTON_CODE_COUNT] { };

	CUtlQueue<InputEvent_t> m_EventQueue{};
	CMessagePumpThread m_EventPump{};
};
