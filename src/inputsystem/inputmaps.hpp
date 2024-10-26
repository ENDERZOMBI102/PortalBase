//
// Created by ENDERZOMBI102 on 26/10/2023.
//
#pragma once
#include "inputsystem/ButtonCode.h"
#include "inputsystem/AnalogCode.h"
#include <array>


struct ButtonMapEntry {
	const char* name;
	const ButtonCode_t code;
};

struct AnalogMapEntry {
	const char* name;
	const AnalogCode_t code;
};

extern std::array<ButtonMapEntry, 113> BUTTON_MAP;
extern std::array<AnalogMapEntry, 4> ANALOG_MAP;
