#include "CupheadController.h"

#include <thread>

void CupheadController::sleep_ms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void CupheadController::tap_button(WORD btn, int ms) {
    pad_.press(btn);
    pad_.update();
    sleep_ms(ms);
    pad_.release(btn);
    pad_.update();
}

void CupheadController::move_left() {
    pad_.set_dpad(GamepadController::DPad::LEFT);
    pad_.update();
}

void CupheadController::move_right() {
    pad_.set_dpad(GamepadController::DPad::RIGHT);
    pad_.update();
}
void CupheadController::move_neutral() {
    pad_.set_dpad(GamepadController::DPad::NEUTRAL);
    pad_.update();
}

void CupheadController::jump_tap() { tap_button(XUSB_GAMEPAD_A, cuphead_timings::tap_ms); }
void CupheadController::dash_tap() { tap_button(XUSB_GAMEPAD_B, cuphead_timings::tap_ms); }
void CupheadController::shoot_tap() { tap_button(XUSB_GAMEPAD_X, cuphead_timings::tap_ms); }
void CupheadController::ex_tap() { tap_button(XUSB_GAMEPAD_Y, cuphead_timings::ex_ms); }
void CupheadController::lock_tap() { tap_button(XUSB_GAMEPAD_RIGHT_SHOULDER, cuphead_timings::lock_toggle_ms); }
void CupheadController::switch_weapon_tap() { tap_button(XUSB_GAMEPAD_LEFT_SHOULDER, cuphead_timings::weapon_switch_ms); }

void CupheadController::shoot_hold(bool down) {
    if (down) pad_.press(XUSB_GAMEPAD_X); else pad_.release(XUSB_GAMEPAD_X);
    pad_.update();
}
void CupheadController::lock_hold(bool down) {
    if (down) pad_.press(XUSB_GAMEPAD_RIGHT_SHOULDER); else pad_.release(XUSB_GAMEPAD_RIGHT_SHOULDER);
    pad_.update();
}

void CupheadController::all_neutral() {
    pad_.clear_all();
    pad_.update();
}

void CupheadController::step_neutral_frame() {
    move_neutral();
    sleep_ms(cuphead_timings::frame_ms);
}
