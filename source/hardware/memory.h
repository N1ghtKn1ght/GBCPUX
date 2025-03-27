#pragma once

#include <iostream>
#include <array>
#include <vector>

namespace GBCPUX {
namespace Hardware {

constexpr uint16_t ROM_MAP = 0x0000;
constexpr uint16_t EROM_MAP = 0x4000;
constexpr uint16_t VRAM_MAP = 0x8000;
constexpr uint16_t ERAM_MAP = 0xA000;
constexpr uint16_t RAM_MAP = 0xC000;
constexpr uint16_t OAM_MAP = 0xFE00;
constexpr uint16_t IO_MAP = 0xFF00;
constexpr uint16_t HRAM_MAP = 0xFF80;

class Memory
{
public:
    Memory() {};
    ~Memory() {};

    bool load(const std::string& buffer);
    uint8_t read(uint16_t address) const;
    void write(uint16_t address, uint8_t value);

private:
    uint8_t m_currentBankROM = 0x00;
    uint8_t m_type = 0x00;
    std::vector<uint8_t> EROM = {};
    std::array<uint8_t, 0x2000> RAM = { 0x00 };
    std::array<uint8_t, 0x2000> VRAM = { 0x00 };
    std::array<uint8_t, 0x4000> ROM = { 0x00 };
    std::array<uint8_t, 0x4000> ERAM = { 0x00 };
    std::array<uint8_t, 0xA0> OAM = { 0x00 };
    std::array<uint8_t, 0x80> IO = { 0x00 };
    std::array<uint8_t, 0x80> HRAM = { 0x00 };

};

}
}