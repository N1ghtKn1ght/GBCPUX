#pragma once 

// std
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

// project
#include "tools/commontoolkit.h"

namespace GBCPUX {
namespace Hardware {
class Memory;

class Timer {
public:
	Timer(Memory& memory);
	~Timer();

	void start();
	void update(const uint8_t cycles);
	void stop();

private:
	Memory& m_memory;
	std::thread m_thread;
	std::atomic<uint32_t> m_clock = 0;
	std::atomic<bool> m_isRunning;

private:
	void run();
	void reset();

	/// <summary>
	/// Gets the value of the Timer Counter (TIMA).
	/// </summary>
	uint8_t getTIMA();

	/// <summary>
	/// Sets the value of the Timer Counter (TIMA).
	/// </summary>
	void setTIMA(uint8_t value);

	/// <summary>
	/// Gets the value of the Timer Modulo (TMA).
	/// </summary>
	uint8_t getTMA();

	/// <summary>
	/// Sets the value of the Timer Modulo (TMA).
	/// </summary>
	void setTMA(uint8_t value);

	/// <summary>
	/// Gets the value of the Timer Control (TAC).
	/// </summary>
	uint8_t getTAC();

	/// <summary>
	/// Sets the value of the Timer Control (TAC).
	/// </summary>
	void setTAC(uint8_t value);
};
}
}