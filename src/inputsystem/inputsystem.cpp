//
// Created by ENDERZOMBI102 on 06/09/2023.
//
#include "inputsystem.hpp"
#include "SDL3/SDL.h"
#include "icommandline.h"
#include "inputmaps.hpp"

#include <chrono>

ConVar joy_gamecontroller_config{ "joy_gamecontroller_config", "", FCVAR_ARCHIVE, "Game controller mapping (passed to SDL with SDL_HINT_GAMECONTROLLERCONFIG), can also be configured in Steam Big Picture mode." };

InitReturnVal_t CInputSystem::Init() {
	int res{0};

	res = SDL_InitSubSystem( SDL_INIT_EVENTS | SDL_INIT_VIDEO );
	if ( res != 0 ) {
		Error( "[AuroraSource|InputSystem] Failed to initialize SDL events/video subsystem(s) (%s)\n", SDL_GetError() );
		return InitReturnVal_t::INIT_FAILED;
	}

	// m_ConsoleTextMode makes us not init the joystick system
	if ( not m_ConsoleTextMode ) {
		// ensure we respect the `joy_gamecontroller_config` convar
		joy_gamecontroller_config.InstallChangeCallback( []( IConVar*, const char*, float ) -> void {
			const auto cfg{ joy_gamecontroller_config.GetString() };

			if ( cfg and cfg[0] != '\0' ) {
				DevMsg( "Passing joy_gamecontroller_config to SDL ('%s').\n", cfg );
				SDL_SetHint( "SDL_GAMECONTROLLERCONFIG", cfg );
			} else {
				DevMsg( "Reset SDL hint as joy_gamecontroller_config is empty.\n" );
				SDL_ResetHint( "SDL_GAMECONTROLLERCONFIG" );
			}
		});
		res = SDL_InitSubSystem( SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC );
		if ( res != 0 ) {
			Error( "[AuroraSource|InputSystem] Failed to initialize SDL joystick subsystem (%s)\n", SDL_GetError() );
			return InitReturnVal_t::INIT_FAILED;
		}

		// init controller if any
		int32 joyNum;
		const auto* joys{ SDL_GetJoysticks( &joyNum ) };
		if ( joyNum != 0 ) [[unlikely]] {
			for ( int i = 0; i != joyNum; i += 1 ) {
				if ( not SDL_IsGamepad( joys[i] ) ) {
					Warning( "[AuroraSource|InputSystem] Joystick is not recognized by the game controller system. You can configure the controller in Steam Big Picture mode.\n" );
					continue;
				}
				const auto pad{ SDL_OpenGamepad( joys[i] ) };
				if ( pad == nullptr ) {
					Warning( "[AuroraSource|InputSystem] Failed to open controller %d: %s\n", i, SDL_GetError() );
					continue;
				}
				// TODO: Finish this
			}
		} else {
			Msg( "[AuroraSource|InputSystem] Did not detect any valid joysticks.\n" );
		}
	}

	return BaseClass::Init();
}

void CInputSystem::Shutdown() {
	// quit systems in the reverse order of init
	if ( not m_ConsoleTextMode ) {
		SDL_QuitSubSystem( SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC );
	}

	SDL_QuitSubSystem( SDL_INIT_EVENTS | SDL_INIT_VIDEO );

	// TODO: Finish this
	BaseClass::Shutdown();
}

// IInputSystem
void CInputSystem::AttachToWindow( void* hWnd ) {
	// WHY DOES ERROR CRASH
	if ( hWnd == nullptr ) {
		Error( "Called `CInputSystem::AttachToWindow` with a `nullptr`!" );
	}

	if ( m_SdlWindow ) {
		Error( "`CInputSystem::AttachToWindow`: Cannot attach to two windows at once!" );
	}

	auto props { SDL_CreateProperties() };
	#if IsWindows()
		SDL_SetNumberProperty( props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, reinterpret_cast<int>( hWnd ) );
	#elif IsLinux()
		// TODO: When we move to wayland, this should change!
		SDL_SetNumberProperty( props, SDL_PROPERTY_WINDOW_X11_WINDOW_NUMBER, reinterpret_cast<int>( hWnd ) );
	#endif

		m_SdlWindow = SDL_CreateWindowWithProperties( props );
	AssertMsg( m_SdlWindow != nullptr, "%s", SDL_GetError() );
}

void CInputSystem::DetachFromWindow() {
	if ( m_SdlWindow == nullptr ) {
		DevWarning( "Called `CInputSystem::DetachFromWindow` when not attached to a window!" );
		return;
	}

	SDL_DestroyWindow( m_SdlWindow );
	m_SdlWindow = nullptr;
}

void CInputSystem::EnableInput( const bool bEnable ) {
	m_Enabled = bEnable;
}

void CInputSystem::EnableMessagePump( const bool bEnable ) {
	// asked to disable
	if ( bEnable ) {
		// if we are already disabled, skip
		if (! m_Running ) {
			return;
		}
		// disable the pump
		m_Running = false;
		m_EventPump.Join();
	} else {
		// if are we already enabled skip
		if ( m_Running ) {
			return;
		}
		// enable the pump
		m_Running = true;
		m_EventPump.Start();
	}
}

void CInputSystem::PollInputState() {
	if ( not m_Enabled ) {
		return;
	}
	m_PoolCount += 1;
	AssertMsg( false, "TODO: `CInputSystem::PollInputState()` not implemented" );
}

int CInputSystem::GetPollTick() const {
	AssertMsg( false, "TODO: `CInputSystem::GetPollTick()` not implemented" );
	SDL_GetTicks();
	return 0;
}

bool CInputSystem::IsButtonDown( const ButtonCode_t pCode ) const {
	if ( pCode >= ButtonCode_t::BUTTON_CODE_LAST ) {
		AssertMsg( false, "Given ButtonCode_t is too high! (%d > %d)", pCode, ButtonCode_t::BUTTON_CODE_LAST );
		return false;
	}

	return m_Buttons[ pCode ].pressed;
}

int CInputSystem::GetButtonPressedTick( const ButtonCode_t pCode ) const {
	return m_Buttons[ pCode ].pressTick;
}

int CInputSystem::GetButtonReleasedTick( const ButtonCode_t pCode ) const {
	return m_Buttons[ pCode ].releaseTick;
}

int CInputSystem::GetAnalogValue( AnalogCode_t code ) const {
	AssertMsg( false, "TODO: `CInputSystem::GetAnalogValue( %d )` not implemented", code );
	return 0;
}

int CInputSystem::GetAnalogDelta( AnalogCode_t code ) const {
	AssertMsg( false, "TODO: `CInputSystem::GetAnalogDelta( %d )` not implemented", code );
	return 0;
}

int CInputSystem::GetEventCount() const {
	return m_EventQueue.Count();
}

const InputEvent_t* CInputSystem::GetEventData() const {
	if ( m_EventQueue.IsEmpty() ) {
		return nullptr;
	}
	return &m_EventQueue.Head();
}

void CInputSystem::PostUserEvent( const InputEvent_t& event ) {
	m_EventQueue.Insert( event );
}

int CInputSystem::GetJoystickCount() const {
	int count{0};
	if ( SDL_GetJoysticks( &count ) == nullptr ) {
		DevWarning( "[AuroraSource|InputSystem] Failed to enumerate joysticks: %s", SDL_GetError() );
		return 0;
	}

	return count;
}

void CInputSystem::EnableJoystickInput( int nJoystick, bool bEnable ) {
	AssertMsg( false, "TODO: `CInputSystem::EnableJoystickInput( %d, %d )` not implemented", nJoystick, bEnable );
}

void CInputSystem::EnableJoystickDiagonalPOV( int nJoystick, bool bEnable ) {
	AssertMsg( false, "TODO: `CInputSystem::EnableJoystickDiagonalPOV( %d, %d )` not implemented", nJoystick, bEnable );
}

void CInputSystem::SampleDevices() {
	AssertMsg( false, "TODO: `CInputSystem::SampleDevices()` not implemented" );
}

void CInputSystem::SetRumble( const float fLeftMotor, const float fRightMotor, const int userId ) {
	for ( auto& stick : m_Gamepads ) {
		if ( stick.handle != nullptr and SDL_GetJoystickPlayerIndex( stick.handle ) == userId ) {
			const int32 res = SDL_RumbleJoystick(
				stick.handle,
				static_cast<uint16>( fLeftMotor * SDL_MAX_UINT16 ),
				static_cast<uint16>( fRightMotor * SDL_MAX_UINT16 ),
				-1
			);
			stick.rumbling = res == 0;
		}
	}
}

void CInputSystem::StopRumble() {
	for ( auto& stick : m_Gamepads ) {
		if ( stick.rumbling ) {  // only valid joysticks may have this set to `true`
			AssertMsg( stick.handle, "Invalid SDL_Joystick handle in rumbling joystick!!" );
			SDL_RumbleJoystick( stick.handle, 0, 0, 0 );
			stick.rumbling = false;
		}
	}
}

void CInputSystem::ResetInputState() {
	// reset button states
	V_memset( &m_Buttons, 0, sizeof(ButtonState) * ButtonCode_t::BUTTON_CODE_COUNT );
	// reset mouse motion accumulators
	m_MouseAccX = 0;
	m_MouseAccY = 0;
	m_PoolCount = 0;
}

void CInputSystem::SetPrimaryUserId( int pUserId ) {
	if ( pUserId > 2 ) {
		pUserId = -1;
	}
	m_PrimaryPadUserId = pUserId;
	ConMsg( "[AuroraSource|InputSystem] Primary gamepad UserId is now %d\n", pUserId );
}

const char* CInputSystem::ButtonCodeToString( const ButtonCode_t pCode ) const {
	for ( const auto [name, code] : BUTTON_MAP ) {
		if ( code == pCode ) {
			return name;
		}
	}
	return "";
}

const char* CInputSystem::AnalogCodeToString( AnalogCode_t pCode ) const {
	for ( const auto& [name, code] : ANALOG_MAP ) {
		if ( code == pCode ) {
			return name;
		}
	}

	// TODO: Implement missing entries (joystick)
	AssertMsg( false, "TODO: `CInputSystem::AnalogCodeToString(%d)` not implemented", pCode );
	return nullptr;
}

ButtonCode_t CInputSystem::StringToButtonCode( const char* pString ) const {
	for ( const auto& [name, code] : BUTTON_MAP ) {
		if ( Q_strcmp( name, pString ) == 0 ) {
			return code;
		}
	}
	return BUTTON_CODE_INVALID;
}

AnalogCode_t CInputSystem::StringToAnalogCode( const char* pString ) const {
	for ( const auto& [name, code] : ANALOG_MAP ) {
		if ( Q_strcmp( name, pString ) == 0 ) {
			return code;
		}
	}

	AssertMsg( false, "TODO: `CInputSystem::StringToAnalogCode(%s)` not implemented", pString );
	return ANALOG_CODE_INVALID;
}

void CInputSystem::SleepUntilInput( const int nMaxSleepTimeMS ) {
	// AssertMsg( false, "TODO: `CInputSystem::SleepUntilInput( %d )` not implemented", nMaxSleepTimeMS );
	const auto end{ std::chrono::high_resolution_clock::now() + std::chrono::milliseconds{ nMaxSleepTimeMS } };
	while ( m_EventQueue.IsEmpty() ) {
		if ( std::chrono::high_resolution_clock::now() > end ) {
			break;
		}
	}
}

ButtonCode_t CInputSystem::VirtualKeyToButtonCode( int nVirtualKey ) const {
	AssertMsg( false, "TODO: `CInputSystem::VirtualKeyToButtonCode( %d )` not implemented", nVirtualKey );
	return KEY_CAPSLOCK;
}

int CInputSystem::ButtonCodeToVirtualKey( ButtonCode_t pCode ) const {
	AssertMsg( false, "TODO: `CInputSystem::ButtonCodeToVirtualKey( %d )` not implemented", pCode );
	return 0;
}

ButtonCode_t CInputSystem::ScanCodeToButtonCode( int lParam ) const {
	AssertMsg( false, "TODO: `CInputSystem::ScanCodeToButtonCode( %d )` not implemented", lParam );
	return KEY_CAPSLOCK;
}

int CInputSystem::GetPollCount() const {
	return m_PoolCount;
}

void CInputSystem::SetCursorPosition( const int x, const int y ) {
	// FIXME: This doesn't sound good
	if ( SDL_WarpMouseGlobal( x, y ) < 0 ) {
		DevWarning( "[AuroraSource|InputSystem] Failed to warp mouse pointer: %s", SDL_GetError() );
	}
}

void* CInputSystem::GetHapticsInterfaceAddress() const {
	AssertMsg( false, "TODO: `CInputSystem::GetHapticsInterfaceAddress()` not implemented" );
	return nullptr;
}

void CInputSystem::SetNovintPure( bool pPure ) {
	AssertMsg( false, "TODO: `CInputSystem::SetNovintPure( %d )` not implemented", pPure );
}

bool CInputSystem::GetRawMouseAccumulators( int& accumX, int& accumY ) {
	accumX = m_MouseAccX;
	m_MouseAccX = 0;

	accumY = m_MouseAccY;
	m_MouseAccY = 0;

	return true;
}

void CInputSystem::SetConsoleTextMode( const bool pConsoleTextMode ) {
	m_ConsoleTextMode = pConsoleTextMode;
}

namespace { CInputSystem s_InputSystem{}; }
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CInputSystem, IInputSystem, INPUTSYSTEM_INTERFACE_VERSION, s_InputSystem )

int CInputSystem::CMessagePumpThread::Run() {
	SDL_Event sdlEvent;
	while ( s_InputSystem.m_Running ) {
		while ( SDL_PollEvent( &sdlEvent ) ) {
			switch ( sdlEvent.type ) {
			case SDL_EVENT_QUIT:
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
				s_InputSystem.m_EventQueue.Insert({
					.m_nType = InputEventType_t::IE_Quit
				});
				break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				s_InputSystem.m_EventQueue.Insert({
					.m_nType = InputEventType_t::IE_ButtonPressed,
					.m_nTick = static_cast<int32>( SDL_GetTicks() ),
					.m_nData = sdlEvent.button.button,
				});
				break;
			case SDL_EVENT_MOUSE_BUTTON_UP:
				s_InputSystem.m_EventQueue.Insert({
					.m_nType = InputEventType_t::IE_ButtonReleased,
					.m_nTick = static_cast<int32>( SDL_GetTicks() ),
					.m_nData = sdlEvent.button.button,
				});
				break;
			case SDL_EVENT_MOUSE_MOTION:
				s_InputSystem.m_EventQueue.Insert({
					.m_nType = InputEventType_t::IE_AnalogValueChanged,
					.m_nTick = static_cast<int32>( SDL_GetTicks() ),
					.m_nData = AnalogCode_t::MOUSE_X,
					.m_nData2 = 0, // sdlEvent.motion.x,
				});
				s_InputSystem.m_EventQueue.Insert({
					.m_nType = InputEventType_t::IE_AnalogValueChanged,
					.m_nTick = static_cast<int32>( SDL_GetTicks() ),
					.m_nData = AnalogCode_t::MOUSE_Y,
					.m_nData2 = 0, // sdlEvent.motion.y,
				});
				break;
			default:
				Warning( "[AuroraSource|InputSystem] Ignored SDL event: 0x%x\n", sdlEvent.type );
			}
		}
	}
	return 0;
}
