#pragma once

#include "GamepadController.h"

namespace cuphead_timings {
    static constexpr int tap_ms = 60;
    static constexpr int frame_ms = 33;
    static constexpr int weapon_switch_ms = 80;
    static constexpr int ex_ms = 120;
    static constexpr int lock_toggle_ms = 40;
    static constexpr int parry_window_ms = 120;
};

class CupheadController {
public:
    void move_left();
    void move_right();
    void move_neutral();

    void jump_tap();
    void dash_tap();
    void shoot_tap();
    void ex_tap();
    void lock_tap();
    void switch_weapon_tap();

    void shoot_hold(bool down);
    void lock_hold(bool down);

    void all_neutral();

    void step_neutral_frame();

private:
    static void sleep_ms(int ms);
    void tap_button(WORD btn, int ms);

    GamepadController pad_;
};
