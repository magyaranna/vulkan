#pragma once

namespace v {

#include <chrono>

    class FrameTimer {
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_lastFrameTime;

    public:
        FrameTimer() {
            m_lastFrameTime = std::chrono::high_resolution_clock::now();
        }

        float getFrameSeconds() {
            auto currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> deltaTime = currentTime - m_lastFrameTime;
            m_lastFrameTime = currentTime;
            return deltaTime.count();
        }
    };
}