#include "CupheadAction.h"

#include <iostream>

#include "CupheadController.h"

std::string to_string(CupheadAction e) {
	switch (e) {
	case CupheadAction::IDLE: return "IDLE";
	case CupheadAction::MOVE_LEFT: return "MOVE_LEFT";
	case CupheadAction::MOVE_RIGHT: return "MOVE_RIGHT";
	case CupheadAction::JUMP: return "JUMP";
	case CupheadAction::DASH: return "DASH";
	case CupheadAction::SHOOT_TAP: return "SHOOT_TAP";
	case CupheadAction::EX: return "EX";
	case CupheadAction::LOCK_TAP: return "LOCK_TAP";
	case CupheadAction::SWITCH_WEAPON: return "SWITCH_WEAPON";
	case CupheadAction::SHOOT_HOLD_ON: return "SHOOT_HOLD_ON";
	case CupheadAction::SHOOT_HOLD_OFF: return "SHOOT_HOLD_OFF";
	case CupheadAction::LOCK_HOLD_ON: return "LOCK_HOLD_ON";
	case CupheadAction::LOCK_HOLD_OFF: return "LOCK_HOLD_OFF";
	case CupheadAction::JUMP_SHOOT_TAP: return "JUMP_SHOOT_TAP";
	case CupheadAction::JUMP_DASH_TAP: return "JUMP_DASH_TAP";
	case CupheadAction::COUNT: return "COUNT";
	default: return "unknown";
	}
}

CupheadAction action_from_int(int value) {
	if (value < 0 || value >= static_cast<int>(CupheadAction::COUNT))
		return CupheadAction::IDLE;
	return static_cast<CupheadAction>(value);
}

void execute_action(CupheadAction action, CupheadController& controller) {
    std::cout << "Executing action: " << to_string(action) << std::endl;

    switch (action) {
    case CupheadAction::IDLE:
        controller.move_neutral();
        break;

    case CupheadAction::MOVE_LEFT:
        controller.move_left();
        break;

    case CupheadAction::MOVE_RIGHT:
        controller.move_right();
        break;

    case CupheadAction::JUMP:
        controller.jump_tap();
        break;

    case CupheadAction::DASH:
        controller.dash_tap();
        break;

    case CupheadAction::SHOOT_TAP:
        controller.shoot_tap();
        break;

    case CupheadAction::EX:
        controller.ex_tap();
        break;

    case CupheadAction::LOCK_TAP:
        controller.lock_tap();
        break;

    case CupheadAction::SWITCH_WEAPON:
        controller.switch_weapon_tap();
        break;

    case CupheadAction::SHOOT_HOLD_ON:
        controller.shoot_hold(true);
        break;

    case CupheadAction::SHOOT_HOLD_OFF:
        controller.shoot_hold(false);
        break;

    case CupheadAction::LOCK_HOLD_ON:
        controller.lock_hold(true);
        break;

    case CupheadAction::LOCK_HOLD_OFF:
        controller.lock_hold(false);
        break;

    case CupheadAction::JUMP_SHOOT_TAP:
        controller.jump_tap();
        controller.shoot_tap();
        break;

    case CupheadAction::JUMP_DASH_TAP:
        controller.jump_tap();
        controller.dash_tap();
        break;

    default:
        break;
    }
}
