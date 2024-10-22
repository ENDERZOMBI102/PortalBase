//
// Created by ENDERZOMBI102 on 26/10/2023.
//
#pragma once
#include "inputsystem/ButtonCode.h"
#include <array>


struct ButtonMapEntry {
	const char* name;
	const ButtonCode_t code;
};

extern std::array<ButtonMapEntry, 106> BUTTON_MAP;
