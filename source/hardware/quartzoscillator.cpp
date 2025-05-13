
//std
#include <fstream>

// project
#include "quartzoscillator.h"

namespace GBCPUX {
namespace Hardware {

QuartzOscillator::QuartzOscillator()
	: m_cpu(m_memory), m_timer(m_memory), m_ppu(m_memory, m_buffer)
{
}

QuartzOscillator::~QuartzOscillator()
{
}

bool QuartzOscillator::start(const std::string& path)
{
    if (m_thread.joinable()) {
        stop();
    }

    std::ifstream file;
    std::string line;

    file.open(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string buffer(size, ' ');
    file.seekg(0);
    file.read(&buffer[0], size);
    file.close();

    if (!m_memory.load(buffer)) {
        return false;
    }
    m_isRunning.store(true, std::memory_order_release);
    m_thread = std::thread(&QuartzOscillator::run, this);

    return true;
}

void QuartzOscillator::stop()
{
    m_isRunning.store(false, std::memory_order_release);
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void QuartzOscillator::run()
{
    constexpr auto cycle_duration = std::chrono::nanoseconds(954);
    using clock = std::chrono::high_resolution_clock;

    auto next_cycle = clock::now();

    while (m_isRunning.load(std::memory_order_acquire)) {
        auto now = clock::now();

        if (now >= next_cycle) {
            m_cpu.update(1);
            m_ppu.update(1);
            m_timer.update(1);

            next_cycle += cycle_duration;
        }
        else {
            std::this_thread::yield();
        }
    }
}

}
}