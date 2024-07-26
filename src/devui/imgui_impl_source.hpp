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

struct ImDrawData;
bool ImGui_ImplSource_Init();
void ImGui_ImplSource_Shutdown();
void ImGui_ImplSource_RenderDrawData( ImDrawData* draw_data );

// Use if you want to reset your rendering device without losing Dear ImGui state.
bool ImGui_ImplSource_CreateDeviceObjects();
void ImGui_ImplSource_InvalidateDeviceObjects();

// Translation table for imgui keys
constexpr ImGuiKey IMGUI_KEY_TABLE[] = {

	/*KEY_NONE*/ ImGuiKey_None,
	/*KEY_0*/ ImGuiKey_0,
	/*KEY_1*/ ImGuiKey_1,
	/*KEY_2*/ ImGuiKey_2,
	/*KEY_3*/ ImGuiKey_3,
	/*KEY_4*/ ImGuiKey_4,
	/*KEY_5*/ ImGuiKey_5,
	/*KEY_6*/ ImGuiKey_6,
	/*KEY_7*/ ImGuiKey_7,
	/*KEY_8*/ ImGuiKey_8,
	/*KEY_9*/ ImGuiKey_9,
	/*KEY_A*/ ImGuiKey_A,
	/*KEY_B*/ ImGuiKey_B,
	/*KEY_C*/ ImGuiKey_C,
	/*KEY_D*/ ImGuiKey_D,
	/*KEY_E*/ ImGuiKey_E,
	/*KEY_F*/ ImGuiKey_F,
	/*KEY_G*/ ImGuiKey_G,
	/*KEY_H*/ ImGuiKey_H,
	/*KEY_I*/ ImGuiKey_I,
	/*KEY_J*/ ImGuiKey_J,
	/*KEY_K*/ ImGuiKey_K,
	/*KEY_L*/ ImGuiKey_L,
	/*KEY_M*/ ImGuiKey_M,
	/*KEY_N*/ ImGuiKey_N,
	/*KEY_O*/ ImGuiKey_O,
	/*KEY_P*/ ImGuiKey_P,
	/*KEY_Q*/ ImGuiKey_Q,
	/*KEY_R*/ ImGuiKey_R,
	/*KEY_S*/ ImGuiKey_S,
	/*KEY_T*/ ImGuiKey_T,
	/*KEY_U*/ ImGuiKey_U,
	/*KEY_V*/ ImGuiKey_V,
	/*KEY_W*/ ImGuiKey_W,
	/*KEY_X*/ ImGuiKey_X,
	/*KEY_Y*/ ImGuiKey_Y,
	/*KEY_Z*/ ImGuiKey_Z,
	/*KEY_PAD_0*/ ImGuiKey_Keypad0,
	/*KEY_PAD_1*/ ImGuiKey_Keypad1,
	/*KEY_PAD_2*/ ImGuiKey_Keypad2,
	/*KEY_PAD_3*/ ImGuiKey_Keypad3,
	/*KEY_PAD_4*/ ImGuiKey_Keypad4,
	/*KEY_PAD_5*/ ImGuiKey_Keypad5,
	/*KEY_PAD_6*/ ImGuiKey_Keypad6,
	/*KEY_PAD_7*/ ImGuiKey_Keypad7,
	/*KEY_PAD_8*/ ImGuiKey_Keypad8,
	/*KEY_PAD_9*/ ImGuiKey_Keypad9,
	/*KEY_PAD_DIVIDE*/ ImGuiKey_KeypadDivide,
	/*KEY_PAD_MULTIPLY*/ ImGuiKey_KeypadMultiply,
	/*KEY_PAD_MINUS*/ ImGuiKey_KeypadSubtract,
	/*KEY_PAD_PLUS*/ ImGuiKey_KeypadAdd,
	/*KEY_PAD_ENTER*/ ImGuiKey_KeypadEnter,
	/*KEY_PAD_DECIMAL*/ ImGuiKey_KeypadDecimal,
	/*KEY_LBRACKET*/ ImGuiKey_LeftBracket,
	/*KEY_RBRACKET*/ ImGuiKey_RightBracket,
	/*KEY_SEMICOLON*/ ImGuiKey_Semicolon,
	/*KEY_APOSTROPHE*/ ImGuiKey_Apostrophe,
	/*KEY_BACKQUOTE*/ ImGuiKey_None,// ?
	/*KEY_COMMA*/ ImGuiKey_Comma,
	/*KEY_PERIOD*/ ImGuiKey_Period,
	/*KEY_SLASH*/ ImGuiKey_Slash,
	/*KEY_BACKSLASH*/ ImGuiKey_Backslash,
	/*KEY_MINUS*/ ImGuiKey_Minus,
	/*KEY_EQUAL*/ ImGuiKey_Equal,
	/*KEY_ENTER*/ ImGuiKey_Enter,
	/*KEY_SPACE*/ ImGuiKey_Space,
	/*KEY_BACKSPACE*/ ImGuiKey_Backspace,
	/*KEY_TAB*/ ImGuiKey_Tab,
	/*KEY_CAPSLOCK*/ ImGuiKey_CapsLock,
	/*KEY_NUMLOCK*/ ImGuiKey_NumLock,
	/*KEY_ESCAPE*/ ImGuiKey_Escape,
	/*KEY_SCROLLLOCK*/ ImGuiKey_ScrollLock,
	/*KEY_INSERT*/ ImGuiKey_Insert,
	/*KEY_DELETE*/ ImGuiKey_Delete,
	/*KEY_HOME*/ ImGuiKey_Home,
	/*KEY_END*/ ImGuiKey_End,
	/*KEY_PAGEUP*/ ImGuiKey_PageUp,
	/*KEY_PAGEDOWN*/ ImGuiKey_PageDown,
	/*KEY_BREAK*/ ImGuiKey_None,// ?
	/*KEY_LSHIFT*/ ImGuiKey_LeftShift,
	/*KEY_RSHIFT*/ ImGuiKey_RightShift,
	/*KEY_LALT*/ ImGuiKey_LeftAlt,
	/*KEY_RALT*/ ImGuiKey_RightAlt,
	/*KEY_LCONTROL*/ ImGuiKey_LeftCtrl,
	/*KEY_RCONTROL*/ ImGuiKey_RightCtrl,
	/*KEY_LWIN*/ ImGuiKey_LeftSuper,
	/*KEY_RWIN*/ ImGuiKey_RightSuper,
	/*KEY_APP*/ ImGuiKey_None,// ?
	/*KEY_UP*/ ImGuiKey_UpArrow,
	/*KEY_LEFT*/ ImGuiKey_LeftArrow,
	/*KEY_DOWN*/ ImGuiKey_DownArrow,
	/*KEY_RIGHT*/ ImGuiKey_RightArrow,
	/*KEY_F1*/ ImGuiKey_F1,
	/*KEY_F2*/ ImGuiKey_F2,
	/*KEY_F3*/ ImGuiKey_F3,
	/*KEY_F4*/ ImGuiKey_F4,
	/*KEY_F5*/ ImGuiKey_F5,
	/*KEY_F6*/ ImGuiKey_F6,
	/*KEY_F7*/ ImGuiKey_F7,
	/*KEY_F8*/ ImGuiKey_F8,
	/*KEY_F9*/ ImGuiKey_F9,
	/*KEY_F10*/ ImGuiKey_F10,
	/*KEY_F11*/ ImGuiKey_F11,
	/*KEY_F12*/ ImGuiKey_F12,
	/*KEY_CAPSLOCKTOGGLE*/ ImGuiKey_None,  // ?
	/*KEY_NUMLOCKTOGGLE*/ ImGuiKey_None,   // ?
	/*KEY_SCROLLLOCKTOGGLE*/ ImGuiKey_None,// ?
};