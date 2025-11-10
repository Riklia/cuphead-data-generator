#pragma once
#include <string>

class CupheadController;

enum class CupheadAction : int {
	// Movement-only
	IDLE = 0,
	MOVE_LEFT,
	MOVE_RIGHT,

	// Simple taps
	JUMP,
	DASH,
	SHOOT_TAP,
	EX,
	LOCK_TAP,
	SWITCH_WEAPON,

	// Holds (toggle intent via policy)
	SHOOT_HOLD_ON,
	SHOOT_HOLD_OFF,
	LOCK_HOLD_ON,
	LOCK_HOLD_OFF,

	// Useful combos
	JUMP_SHOOT_TAP,
	JUMP_DASH_TAP,

	COUNT
};


std::string to_string(CupheadAction e);

CupheadAction action_from_int(int value);

void execute_action(CupheadAction action, CupheadController& controller);
