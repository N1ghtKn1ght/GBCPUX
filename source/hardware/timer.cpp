
// project
#include "timer.h"
#include "memory.h"
#include "tools/commontoolkit.h"

namespace GBCPUX {
namespace Hardware {
Timer::Timer(Memory& memory) 
    : m_memory(memory)
{
    reset();
}

Timer::~Timer()
{
	stop();
}

void Timer::start()
{
    if (m_thread.joinable()) {
        stop();
    }
    m_isRunning.store(true, std::memory_order_release);
    m_thread = std::thread(&Timer::run, this);
}

void Timer::update(const uint8_t cycles)
{
    m_clock.fetch_add(cycles, std::memory_order_relaxed);
}

void Timer::stop()
{
    m_isRunning.store(false, std::memory_order_release);
    if (m_thread.joinable()) {
        m_thread.join();
        reset();
    }
}

void Timer::run()
{
    static const uint16_t ticks[4] = { 1024, 16, 64, 256 };

    while (m_isRunning.load(std::memory_order_acquire))
    {
        if (m_clock.load(std::memory_order_acquire) == 0) {
            continue;
        }

        uint8_t tac = getTAC();
        if (~tac & 0x04) {
            m_clock.store(0, std::memory_order_relaxed);
            continue;
        }

        uint16_t tick = ticks[tac & 0x03];

         while(m_clock.load(std::memory_order_relaxed) >= tick) {
            m_clock.fetch_sub(tick, std::memory_order_relaxed);

            uint8_t tima = getTIMA();
            if (tima == 0xFF) {
                setTIMA(getTMA()); 
                uint8_t IF = m_memory.read(Tools::IF_ADDR);
                IF |= Tools::TIMER_BIT;
                m_memory.write(Tools::IF_ADDR, IF);
            }
            else {
                setTIMA(tima + 1); 
            }
        }
    }
}

void Timer::reset()
{
    setTIMA(0xFF);
    setTMA(0x00);
    setTAC(0x01);
    m_clock.store(0, std::memory_order_relaxed);
}

uint8_t Timer::getTIMA()
{
    return m_memory.read(Tools::TIMA_ADDR);
}

void Timer::setTIMA(uint8_t value)
{
    m_memory.write(Tools::TIMA_ADDR, value);
}

uint8_t Timer::getTMA()
{
    return m_memory.read(Tools::TMA_ADDR);
}

void Timer::setTMA(uint8_t value)
{
    m_memory.write(Tools::TMA_ADDR, value);
}

uint8_t Timer::getTAC()
{
    return m_memory.read(Tools::TAC_ADDR);
}

void Timer::setTAC(uint8_t value)
{
    m_memory.write(Tools::TAC_ADDR, value);
}

}
}