#include "GamepadController.h"

#include <stdexcept>
#include <ViGEm/Client.h>

GamepadController::GamepadController() {
	client_ = vigem_alloc();
	if (vigem_connect(client_) != VIGEM_ERROR_NONE) {
		throw std::runtime_error("Failed to connect to ViGEmBus");
	}

	target_ = vigem_target_x360_alloc();
	if (vigem_target_add(client_, target_) != VIGEM_ERROR_NONE) {
		throw std::runtime_error("Failed to add virtual controller");
	}

	ZeroMemory(&report_, sizeof(report_));
}

GamepadController::~GamepadController() {
	vigem_target_remove(client_, target_);
	vigem_target_free(target_);
	vigem_disconnect(client_);
	vigem_free(client_);
}

void GamepadController::update() const {
    const auto err = vigem_target_x360_update(client_, target_, report_);
    if (err != VIGEM_ERROR_NONE) {
        throw std::runtime_error("vigem_target_x360_update failed");
    }
}

void GamepadController::press(WORD btn_mask) {
    report_.wButtons |= btn_mask;
}

void GamepadController::release(WORD btn_mask) {
    report_.wButtons &= ~btn_mask;
}

bool GamepadController::is_pressed(WORD btn_mask) const {
    return (report_.wButtons & btn_mask) != 0;
}

void GamepadController::set_dpad(DPad dir) {
    report_.wButtons &= ~(XUSB_GAMEPAD_DPAD_UP | XUSB_GAMEPAD_DPAD_DOWN |
        XUSB_GAMEPAD_DPAD_LEFT | XUSB_GAMEPAD_DPAD_RIGHT);
    switch (dir) {
    case DPad::UP:    report_.wButtons |= XUSB_GAMEPAD_DPAD_UP;    break;
    case DPad::DOWN:  report_.wButtons |= XUSB_GAMEPAD_DPAD_DOWN;  break;
    case DPad::LEFT:  report_.wButtons |= XUSB_GAMEPAD_DPAD_LEFT;  break;
    case DPad::RIGHT: report_.wButtons |= XUSB_GAMEPAD_DPAD_RIGHT; break;
    case DPad::NEUTRAL: default: break;
    }
}

void GamepadController::set_left_stick(SHORT x, SHORT y) {
    report_.sThumbLX = x;
    report_.sThumbLY = y;
}

void GamepadController::set_right_stick(SHORT x, SHORT y) {
    report_.sThumbRX = x;
    report_.sThumbRY = y;
}

void GamepadController::set_triggers(uint8_t left, uint8_t right) {
    report_.bLeftTrigger = left;
    report_.bRightTrigger = right;
}

void GamepadController::clear_all() {
    ZeroMemory(&report_, sizeof(report_));
}