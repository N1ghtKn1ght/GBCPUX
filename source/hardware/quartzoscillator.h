#pragma once

// std 
#include <iostream>
#include <thread>

// project
#include "memory.h"
#include "ppu.h"
#include "timer.h"
#include "cpu.h"

namespace GBCPUX {
namespace Hardware {

class QuartzOscillator {

public:
    QuartzOscillator();
    ~QuartzOscillator();

    const uint8_t* buffer() const { return m_buffer.data(); };
    bool start(const std::string& path);
    void stop();

private: 
    CPU m_cpu;
    Memory m_memory;
    PPU m_ppu;
    Timer m_timer;
    std::thread m_thread;
    std::array<uint8_t, 0x5A00> m_buffer;
    std::atomic<bool> m_isRunning;

private:
    void run();
};

}
}

