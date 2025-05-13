#pragma once 

// std
#include <iostream>

namespace GBCPUX {
namespace Hardware {
namespace Tools {

constexpr uint16_t JOYPAD_ADDR = 0xFF00; // Joypad Register
constexpr uint16_t SB_ADDR = 0xFF01; // Serial transfer data
constexpr uint16_t SC_ADDR = 0xFF02; // SIO control
constexpr uint16_t DIV_ADDR = 0xFF04; // Divider Register
constexpr uint16_t IF_ADDR = 0xFF0F;  // Interrupt Flag Register
constexpr uint16_t IE_ADDR = 0xFFFF;  // Interrupt Enable Register

constexpr uint16_t TIMA_ADDR = 0xFF05; // Timer counter
constexpr uint16_t TMA_ADDR = 0xFF06; // Timer modulo
constexpr uint16_t TAC_ADDR = 0xFF07; // Timer control

constexpr uint8_t V_BLANK_BIT = 0x01;
constexpr uint8_t LCD_STAT_BIT = 0x02;
constexpr uint8_t TIMER_BIT = 0x04;
constexpr uint8_t SERIAL_BIT = 0x08;
constexpr uint8_t JOYPAD_BIT = 0x10;

constexpr uint8_t V_BLANK_INT = 0x40;
constexpr uint8_t LCD_STAT_INT = 0x48;
constexpr uint8_t TIMER_INT = 0x50;
constexpr uint8_t SERIAL_INT = 0x58;
constexpr uint8_t JOYPAD_INT = 0x60;

constexpr uint16_t LCDC_ADDR = 0xFF40;
constexpr uint16_t STAT_ADDR = 0xFF41;
constexpr uint16_t SCY_ADDR = 0xFF42;
constexpr uint16_t SCX_ADDR = 0xFF43;
constexpr uint16_t LY_ADDR = 0xFF44;
constexpr uint16_t LYC_ADDR = 0xFF45;
constexpr uint16_t DMA_ADDR = 0xFF46;
constexpr uint16_t BGP_ADDR = 0xFF47;
constexpr uint16_t OBP0_ADDR = 0xFF48;
constexpr uint16_t OBP1_ADDR = 0xFF49;
constexpr uint16_t WY_ADDR = 0xFF4A;
constexpr uint16_t WX_ADDR = 0xFF4B;

constexpr uint8_t LCDC_ENABLE = 0x80;
constexpr uint8_t LCDC_SELECT_WINDOW = 0x40;
constexpr uint8_t LCDC_STAT_WINDOW = 0x20;
constexpr uint8_t LCDC_SELECT_TITLE = 0x10;
constexpr uint8_t LCDC_SELECT_BACKGROND = 0x08;
constexpr uint8_t LCDC_SIZE_OBJECT = 0x04;
constexpr uint8_t LCDC_STAT_OBJECT = 0x02;
constexpr uint8_t LCDC_STAT_BACKGROUND = 0x01;

constexpr uint8_t STAT_INTERRUPT_LYC = 0x40;
constexpr uint8_t STAT_INTERRUPT_OAM = 0x20;
constexpr uint8_t STAT_INTERRUPT_VBLANK = 0x10;
constexpr uint8_t STAT_INTERRUPT_HBLANK = 0x08;
constexpr uint8_t STAT_COINCIDENCE_FLAG = 0x04;
constexpr uint8_t STAT_MODE_PPU = 0x03;

constexpr uint8_t PALETTE_COLOR_00 = 0x03;
constexpr uint8_t PALETTE_COLOR_01 = 0x0C;
constexpr uint8_t PALETTE_COLOR_10 = 0x30;
constexpr uint8_t PALETTE_COLOR_11 = 0xC0;

constexpr uint8_t MASK_BIT_0 = 0x01;
constexpr uint8_t MASK_BIT_7 = 0x80;
constexpr uint8_t SHIFT_BIT_0 = 1;
constexpr uint8_t SHIFT_BIT_4 = 4;
constexpr uint8_t SHIFT_BIT_7 = 7;

constexpr uint8_t MASK_4BITS = 0x0F;
constexpr uint8_t MASK_8BITS = 0xFF;
constexpr uint16_t MASK_12BITS = 0x0FFF;
constexpr uint16_t MASK_16BITS = 0xFFFF;

inline uint8_t lsb_8(const uint16_t value)
{
    return value & MASK_8BITS;
}

inline uint8_t msb_8(const uint16_t value)
{
    return value >> 8 & MASK_8BITS;
}

inline uint16_t unsigned_16(const uint8_t& msb, const uint8_t& lsb)
{
    return ((uint16_t)msb << 8) | lsb;
}

inline int8_t signed_8(const uint8_t& value) 
{
    return +static_cast<int8_t>(value);
}

struct CPURegisters {
    uint8_t A = 0, F = 0, B = 0, C = 0, D = 0, E = 0, H = 0, L = 0;
    uint16_t SP = 0, PC = 0;
    bool IME = false, NIME = false, HALT = false, HALT_BUG = false;

    void setZero(bool value) 
    {
        if (value)
            F |= 0x80;
        else
            F &= ~0x80;
    }

    bool getZero() const { return (F & 0x80) != 0; }

    void setSubtract(bool value) 
    {
        if (value)
            F |= 0x40;
        else
            F &= ~0x40;
    }

    bool getSubtract() const { return (F & 0x40) != 0; }

    void setHalfCarry(bool value) 
    {
        if (value)
            F |= 0x20;
        else
            F &= ~0x20;
    }

    bool getHalfCarry() const  {return (F & 0x20) != 0; }

    void setCarry(bool value) 
    {
        if (value) {
            F |= 0x10;
        }
        else {
            F &= ~0x10;
        }
    }

    bool getCarry() const { return (F & 0x10) != 0; }
};

}
}
}