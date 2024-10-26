//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//===========================================================================//
#pragma once
#include "inputsystem/InputEnums.h"
#include "mathlib/mathlib.h"

//-----------------------------------------------------------------------------
// Button enum. "Buttons" are binary-state input devices (mouse buttons, keyboard keys)
//-----------------------------------------------------------------------------
enum {
	JOYSTICK_MAX_BUTTON_COUNT = 32,
	JOYSTICK_POV_BUTTON_COUNT = 4,
	JOYSTICK_AXIS_BUTTON_COUNT = MAX_JOYSTICK_AXES * 2,
};

#define JOYSTICK_BUTTON_INTERNAL( _joystick, _button ) ( JOYSTICK_FIRST_BUTTON + ( ( _joystick ) * JOYSTICK_MAX_BUTTON_COUNT ) + ( _button ) )
#define JOYSTICK_POV_BUTTON_INTERNAL( _joystick, _button ) ( JOYSTICK_FIRST_POV_BUTTON + ( ( _joystick ) * JOYSTICK_POV_BUTTON_COUNT ) + ( _button ) )
#define JOYSTICK_AXIS_BUTTON_INTERNAL( _joystick, _button ) ( JOYSTICK_FIRST_AXIS_BUTTON + ( ( _joystick ) * JOYSTICK_AXIS_BUTTON_COUNT ) + ( _button ) )

#define JOYSTICK_BUTTON( _joystick, _button ) ( (ButtonCode_t) JOYSTICK_BUTTON_INTERNAL( _joystick, _button ) )
#define JOYSTICK_POV_BUTTON( _joystick, _button ) ( (ButtonCode_t) JOYSTICK_POV_BUTTON_INTERNAL( _joystick, _button ) )
#define JOYSTICK_AXIS_BUTTON( _joystick, _button ) ( (ButtonCode_t) JOYSTICK_AXIS_BUTTON_INTERNAL( _joystick, _button ) )

enum ButtonCode_t {
	BUTTON_CODE_INVALID = -1,
	BUTTON_CODE_NONE = 0,

	KEY_FIRST = 0,

	KEY_NONE = KEY_FIRST,
	KEY_0,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_PAD_0,
	KEY_PAD_1,
	KEY_PAD_2,
	KEY_PAD_3,
	KEY_PAD_4,
	KEY_PAD_5,
	KEY_PAD_6,
	KEY_PAD_7,
	KEY_PAD_8,
	KEY_PAD_9,
	KEY_PAD_DIVIDE,
	KEY_PAD_MULTIPLY,
	KEY_PAD_MINUS,
	KEY_PAD_PLUS,
	KEY_PAD_ENTER,
	KEY_PAD_DECIMAL,
	KEY_LBRACKET,
	KEY_RBRACKET,
	KEY_SEMICOLON,
	KEY_APOSTROPHE,
	KEY_BACKQUOTE,
	KEY_COMMA,
	KEY_PERIOD,
	KEY_SLASH,
	KEY_BACKSLASH,
	KEY_MINUS,
	KEY_EQUAL,
	KEY_ENTER,
	KEY_SPACE,
	KEY_BACKSPACE,
	KEY_TAB,
	KEY_CAPSLOCK,
	KEY_NUMLOCK,
	KEY_ESCAPE,
	KEY_SCROLLLOCK,
	KEY_INSERT,
	KEY_DELETE,
	KEY_HOME,
	KEY_END,
	KEY_PAGEUP,
	KEY_PAGEDOWN,
	KEY_BREAK,
	KEY_LSHIFT,
	KEY_RSHIFT,
	KEY_LALT,
	KEY_RALT,
	KEY_LCONTROL,
	KEY_RCONTROL,
	KEY_LWIN,
	KEY_RWIN,
	KEY_APP,
	KEY_UP,
	KEY_LEFT,
	KEY_DOWN,
	KEY_RIGHT,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_CAPSLOCKTOGGLE,
	KEY_NUMLOCKTOGGLE,
	KEY_SCROLLLOCKTOGGLE,

	KEY_LAST = KEY_SCROLLLOCKTOGGLE,
	KEY_COUNT = KEY_LAST - KEY_FIRST + 1,

	// Mouse
	MOUSE_FIRST = KEY_LAST + 1,

	MOUSE_LEFT = MOUSE_FIRST,
	MOUSE_RIGHT,
	MOUSE_MIDDLE,
	MOUSE_4,
	MOUSE_5,
	MOUSE_WHEEL_UP,  // A fake button which is 'pressed' and 'released' when the wheel is moved up
	MOUSE_WHEEL_DOWN,// A fake button which is 'pressed' and 'released' when the wheel is moved down

	MOUSE_LAST = MOUSE_WHEEL_DOWN,
	MOUSE_COUNT = MOUSE_LAST - MOUSE_FIRST + 1,

	// Joystick
	JOYSTICK_FIRST = MOUSE_LAST + 1,

	JOYSTICK_FIRST_BUTTON = JOYSTICK_FIRST,
	JOYSTICK_LAST_BUTTON = JOYSTICK_BUTTON_INTERNAL( MAX_JOYSTICKS - 1, JOYSTICK_MAX_BUTTON_COUNT - 1 ),
	JOYSTICK_FIRST_POV_BUTTON,
	JOYSTICK_LAST_POV_BUTTON = JOYSTICK_POV_BUTTON_INTERNAL( MAX_JOYSTICKS - 1, JOYSTICK_POV_BUTTON_COUNT - 1 ),
	JOYSTICK_FIRST_AXIS_BUTTON,
	JOYSTICK_LAST_AXIS_BUTTON = JOYSTICK_AXIS_BUTTON_INTERNAL( MAX_JOYSTICKS - 1, JOYSTICK_AXIS_BUTTON_COUNT - 1 ),

	JOYSTICK_LAST = JOYSTICK_LAST_AXIS_BUTTON,

	NOVINT_FIRST = JOYSTICK_LAST + 2,// plus 1 missing key. +1 seems to cause issues on the first button.

	NOVINT_LOGO_0 = NOVINT_FIRST,
	NOVINT_TRIANGLE_0,
	NOVINT_BOLT_0,
	NOVINT_PLUS_0,
	NOVINT_LOGO_1,
	NOVINT_TRIANGLE_1,
	NOVINT_BOLT_1,
	NOVINT_PLUS_1,

	NOVINT_LAST = NOVINT_PLUS_1,

	BUTTON_CODE_LAST,
	BUTTON_CODE_COUNT = BUTTON_CODE_LAST - KEY_FIRST + 1,

	// Helpers for XBox 360
	KEY_XBUTTON_UP = JOYSTICK_FIRST_POV_BUTTON,// POV buttons
	KEY_XBUTTON_RIGHT,
	KEY_XBUTTON_DOWN,
	KEY_XBUTTON_LEFT,

	KEY_XBUTTON_A = JOYSTICK_FIRST_BUTTON,// Buttons
	KEY_XBUTTON_B,
	KEY_XBUTTON_X,
	KEY_XBUTTON_Y,
	KEY_XBUTTON_LEFT_SHOULDER,
	KEY_XBUTTON_RIGHT_SHOULDER,
	KEY_XBUTTON_BACK,
	KEY_XBUTTON_START,
	KEY_XBUTTON_STICK1,
	KEY_XBUTTON_STICK2,

	KEY_XSTICK1_RIGHT = JOYSTICK_FIRST_AXIS_BUTTON,// XAXIS POSITIVE
	KEY_XSTICK1_LEFT,                              // XAXIS NEGATIVE
	KEY_XSTICK1_DOWN,                              // YAXIS POSITIVE
	KEY_XSTICK1_UP,                                // YAXIS NEGATIVE
	KEY_XBUTTON_LTRIGGER,                          // ZAXIS POSITIVE
	KEY_XBUTTON_RTRIGGER,                          // ZAXIS NEGATIVE
	KEY_XSTICK2_RIGHT,                             // UAXIS POSITIVE
	KEY_XSTICK2_LEFT,                              // UAXIS NEGATIVE
	KEY_XSTICK2_DOWN,                              // VAXIS POSITIVE
	KEY_XSTICK2_UP,                                // VAXIS NEGATIVE
};

inline bool IsAlpha( const ButtonCode_t code ) {
	return code >= KEY_A and code <= KEY_Z;
}

inline bool IsAlphaNumeric( const ButtonCode_t code ) {
	return code >= KEY_0 and code <= KEY_Z;
}

inline bool IsSpace( const ButtonCode_t code ) {
	return code == KEY_ENTER or code == KEY_TAB or code == KEY_SPACE;
}

inline bool IsKeypad( const ButtonCode_t code ) {
	return code >= MOUSE_FIRST and code <= KEY_PAD_DECIMAL;
}

inline bool IsPunctuation( const ButtonCode_t code ) {
	return code >= KEY_0 and code <= KEY_SPACE and !IsAlphaNumeric( code ) and !IsSpace( code ) and !IsKeypad( code );
}

inline bool IsKeyCode( const ButtonCode_t code ) {
	return code >= KEY_FIRST and code <= KEY_LAST;
}

inline bool IsMouseCode( const ButtonCode_t code ) {
	return code >= MOUSE_FIRST and code <= MOUSE_LAST;
}

inline bool IsNovintCode( const ButtonCode_t code ) {
	return code >= NOVINT_FIRST and code <= NOVINT_LAST;
}

inline bool IsNovintButtonCode( const ButtonCode_t code ) {
	return IsNovintCode( code );
}

inline bool IsJoystickCode( const ButtonCode_t code ) {
	return ( code >= JOYSTICK_FIRST and code <= JOYSTICK_LAST ) or IsNovintCode( code );
}

inline bool IsJoystickButtonCode( const ButtonCode_t code ) {
	return code >= JOYSTICK_FIRST_BUTTON and code <= JOYSTICK_LAST_BUTTON or IsNovintButtonCode( code );
}

inline bool IsJoystickPOVCode( const ButtonCode_t code ) {
	return code >= JOYSTICK_FIRST_POV_BUTTON and code <= JOYSTICK_LAST_POV_BUTTON;
}

inline bool IsJoystickAxisCode( const ButtonCode_t code ) {
	return code >= JOYSTICK_FIRST_AXIS_BUTTON and code <= JOYSTICK_LAST_AXIS_BUTTON;
}

inline ButtonCode_t GetBaseButtonCode( const ButtonCode_t code ) {
	if ( IsJoystickButtonCode( code ) ) {
		const int offset = ( code - JOYSTICK_FIRST_BUTTON ) % JOYSTICK_MAX_BUTTON_COUNT;
		return static_cast<ButtonCode_t>( JOYSTICK_FIRST_BUTTON + offset );
	}

	if ( IsJoystickPOVCode( code ) ) {
		const int offset = ( code - JOYSTICK_FIRST_POV_BUTTON ) % JOYSTICK_POV_BUTTON_COUNT;
		return static_cast<ButtonCode_t>( JOYSTICK_FIRST_POV_BUTTON + offset );
	}

	if ( IsJoystickAxisCode( code ) ) {
		const int offset = ( code - JOYSTICK_FIRST_AXIS_BUTTON ) % JOYSTICK_AXIS_BUTTON_COUNT;
		return static_cast<ButtonCode_t>( JOYSTICK_FIRST_AXIS_BUTTON + offset );
	}

	return code;
}

inline int GetJoystickForCode( const ButtonCode_t code ) {
	if ( not IsJoystickCode( code ) ) {
	  return 0;
	}

	if ( IsJoystickButtonCode( code ) ) {
		return ( code - JOYSTICK_FIRST_BUTTON ) / JOYSTICK_MAX_BUTTON_COUNT;
	}
	if ( IsJoystickPOVCode( code ) ) {
		return ( code - JOYSTICK_FIRST_POV_BUTTON ) / JOYSTICK_POV_BUTTON_COUNT;
	}
	if ( IsJoystickAxisCode( code ) ) {
		return ( code - JOYSTICK_FIRST_AXIS_BUTTON ) / JOYSTICK_AXIS_BUTTON_COUNT;
	}

	return 0;
}

inline ButtonCode_t ButtonCodeToJoystickButtonCode( ButtonCode_t code, int nDesiredJoystick ) {
	if ( not IsJoystickCode( code ) or nDesiredJoystick == 0 ) {
		return code;
	}

	nDesiredJoystick = ::clamp<int>( nDesiredJoystick, 0, MAX_JOYSTICKS - 1 );

	code = GetBaseButtonCode( code );

	// Now upsample it
	if ( IsJoystickButtonCode( code ) ) {
		const int nOffset = code - JOYSTICK_FIRST_BUTTON;
		return JOYSTICK_BUTTON( nDesiredJoystick, nOffset );
	}

	if ( IsJoystickPOVCode( code ) ) {
		const int nOffset = code - JOYSTICK_FIRST_POV_BUTTON;
		return JOYSTICK_POV_BUTTON( nDesiredJoystick, nOffset );
	}

	if ( IsJoystickAxisCode( code ) ) {
		const int nOffset = code - JOYSTICK_FIRST_AXIS_BUTTON;
		return JOYSTICK_AXIS_BUTTON( nDesiredJoystick, nOffset );
	}

	return code;
}
