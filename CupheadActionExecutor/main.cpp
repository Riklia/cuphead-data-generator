#include <iostream>
#include <thread>
#include <zmq.hpp>
#include <Windows.h>

#include "CupheadAction.h"
#include "CupheadController.h"

//int main() {
//    zmq::context_t context(1);
//    zmq::socket_t socket(context, ZMQ_REP);
//    socket.bind("tcp://*:5554");
//
//    CupheadController controller;
//    std::cout << "ActionExecutor waiting for actions..." << "\n";
//
//    while (true) {
//        zmq::message_t request;
//
//        if (socket.recv(request, zmq::recv_flags::none)) {
//            if (request.size() != sizeof
// ) {
//                std::cerr << "Invalid action size: " << request.size() << "\n";
//                continue;
//            }
//
//            const int action_int = *static_cast<int*>(request.data());
//            execute_action(action_from_int(action_int), controller);
//
//            zmq::message_t reply(2);
//            memcpy(reply.data(), "OK", 2);
//            socket.send(reply, zmq::send_flags::none);
//        }
//
//        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
//            break;
//    }
//
//    std::cout << "Exiting on ESC..." << "\n";
//    return 0;
//}

// Send a few quick A-button taps so the virtual pad becomes P1
static void claim_p1(CupheadController& cc, int duration_ms = 2500, int period_ms = 250) {
    auto t0 = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - t0).count() < duration_ms) {
        cc.jump_tap(); // A button in our mapping
        std::this_thread::sleep_for(std::chrono::milliseconds(period_ms));
    }
}

int main() {
    try {
        CupheadController cc;
        // Simple action script to test
        auto run_action = [&](CupheadAction a, int wait_ms) {
            std::cout << "Executing action: " << static_cast<int>(a) << std::endl;
            execute_action(a, cc);
            std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
            };


        int shift_num = 0;
        while (shift_num < 2) {
            if (GetAsyncKeyState(VK_SHIFT)) {
                ++shift_num;
                run_action(CupheadAction::JUMP, 400);
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        claim_p1(cc, /*duration_ms=*/2500, /*period_ms=*/250);
        std::cout << "Virtual controller initialized. "
            "Make sure Cuphead window is focused.\n";


        // Sequence test
        run_action(CupheadAction::MOVE_RIGHT, 1000);   // walk right for 1s
        run_action(CupheadAction::IDLE, 500);         // stop
        run_action(CupheadAction::JUMP, 400);         // short jump
        run_action(CupheadAction::DASH, 400);         // dash
        run_action(CupheadAction::SHOOT_HOLD_ON, 0);    // hold shoot
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        run_action(CupheadAction::SHOOT_HOLD_OFF, 200); // release
        run_action(CupheadAction::EX, 800);           // EX attack
        run_action(CupheadAction::SWITCH_WEAPON, 500); // switch weapon

        cc.all_neutral();
        std::cout << "Test sequence finished.\n";

    }
    catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}