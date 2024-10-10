//
// Created by ENDERZOMBI102 on 26/10/2023.
//
#pragma once
#include <array>
#include "inputsystem/ButtonCode.h"


struct ButtonEntry {
	const char* name;
	const ButtonCode_t code;
};

extern std::array<ButtonEntry, 106> BUTTON_MAP;
