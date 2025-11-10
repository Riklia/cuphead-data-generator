#pragma once

#include <cstdint>
#include <Windows.h> 
#include <ViGEm/Client.h>

class GamepadController {
public:
    GamepadController();

    GamepadController(GamepadController&) = delete;
    GamepadController& operator=(GamepadController&) = delete;
    GamepadController(GamepadController&&) = delete;
    GamepadController& operator=(GamepadController&&) = delete;

    ~GamepadController();

    void update() const;

    void press(WORD btn_mask);
    void release(WORD btn_mask);
    bool is_pressed(WORD btn_mask) const;

    enum class DPad { NEUTRAL, UP, DOWN, LEFT, RIGHT };
    void set_dpad(DPad dir);

    void set_left_stick(SHORT x, SHORT y);
    void set_right_stick(SHORT x, SHORT y);

    void set_triggers(uint8_t left, uint8_t right);

    void clear_all();

private:
    PVIGEM_CLIENT client_;
    PVIGEM_TARGET target_;
    XUSB_REPORT report_;
};
