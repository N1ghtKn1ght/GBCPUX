#pragma once 

// std
#include <iostream>
#include <map>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <semaphore>

// project
#include "tools/commontoolkit.h"

namespace GBCPUX {
namespace Hardware {
class Memory;

constexpr uint32_t OAM_SEARCH_DURATION = 80;
constexpr uint32_t PIXEL_TRANSFER_DURATION = 172;
constexpr uint32_t HBLANK_DURATION = 204;
constexpr uint32_t VBLANK_DURATION = 456;
constexpr uint32_t WAIT_DURATION = 244;
constexpr uint16_t SCREEN_WIDTH = 160;
constexpr uint16_t SCREEN_HEIGHT = 144;
constexpr uint8_t TOTAL_SPRITES = 40;
constexpr uint8_t MAX_SPRITES_PER_SCANLINE = 10;
constexpr uint8_t BYTES_PER_SPRITE = 4;

enum PPUMode : int32_t {
    NONE = -1,
    HBLANK = 0,
    VBLANK = 1,
    OAM_SEARCH = 2,
    PIXEL_TRANSFER = 3, 
    LCD_WAIT = 4
};

class PPU;
struct PPUMethod
{
    uint32_t duration = 0;
    PPUMode(PPU::* func)() = nullptr;

    PPUMethod(uint32_t duration, PPUMode(PPU::* func)())
        : duration(duration), func(std::move(func)) { }

    PPUMethod() {}
    PPUMethod(const PPUMethod&) = delete;
};

struct Sprite 
{
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t tile = 0;
    uint8_t flag = 0;
};

struct LCDC
{
public:
    LCDC(uint8_t value) : m_value(value) {}

    /// <summary>
    /// Background and window enable. This bit determines whether the background and window layers should be drawn or not.
    /// </summary>
    bool isBackgroundEnabled() const { return (m_value & Tools::LCDC_STAT_BACKGROUND) != 0; }

    /// <summary>
    /// Object enable. This bit determines whether sprites should be drawn or not.
    /// </summary>
    bool isSpriteEnabled() const { return (m_value & Tools::LCDC_STAT_OBJECT) != 0; }

    /// <summary>
    /// Object size. This bit determines whether sprites consist of one (8x8) or two (8x16) tiles.
    /// </summary>
    bool getSpriteSize() const { return (m_value & Tools::LCDC_SIZE_OBJECT) != 0; }

    /// <summary>
    /// Background tile map select. This bit determines which map should be used for the background layer.
    /// </summary>
    bool getBackgroundTileMap() const { return (m_value & Tools::LCDC_SELECT_BACKGROND) != 0; }

    /// <summary>
    /// Tile set select. This bit determines which tile set should be used by the tile maps.
    /// </summary>
    bool getTileSetSelect() const { return (m_value & Tools::LCDC_SELECT_TITLE) != 0; }

    /// <summary>
    /// Window enable. This bit determines whether the window layer should be drawn or not.
    /// </summary>
    bool isWindowEnabled() const { return (m_value & Tools::LCDC_STAT_WINDOW) != 0; }

    /// <summary>
    /// Window tile map select. This bit determines which map should be used for the window layer.
    /// </summary>
    bool getWindowTileMap() const { return (m_value & Tools::LCDC_SELECT_WINDOW) != 0; }

    /// <summary>
    /// LCD enable. If this bit is 0, the display is off.
    /// </summary>
    bool isEnabled() const { return (m_value & Tools::LCDC_ENABLE) != 0; }

    uint8_t value() const { return m_value; }

private:
    const uint8_t m_value;
};

class LCDSCR {

public:
	LCDSCR(uint8_t value) : m_value(value) {}
    
    /// <summary>
    /// LYC interrupt enable. Enables interrupts when the current scanline equals the value of the LYC register.
    /// </summary>
    bool getLYCInterrupt() const { return (m_value & Tools::STAT_INTERRUPT_LYC) != 0; }

    /// <summary>
    /// OAM search interrupt enable. Enables interrupts when the PPU enters OAM search.
    /// </summary>
    bool getOAMInterrupt() const { return (m_value & Tools::STAT_INTERRUPT_OAM) != 0; }

    /// <summary>
    /// V-blank interrupt enable. Enables interrupts when the PPU enters a V-blank.
    /// </summary>
    bool getVBlankInterrupt() const { return (m_value & Tools::STAT_INTERRUPT_VBLANK) != 0; }

    /// <summary>
    /// H-blank interrupt enable. Enables interrupts when the PPU enters an H-blank.
    /// </summary>
    bool getHBlankInterrupt() const { return (m_value & Tools::STAT_INTERRUPT_HBLANK) != 0; }

    /// <summary>
    /// LYC=LY flag. Is set to 1 if the current scanline equals the value of the LYC register.
    /// </summary>
    bool getCoincidenceFlag() const { return (m_value & Tools::STAT_COINCIDENCE_FLAG) != 0; }

    /// <summary>
    /// Mode flag. This bit determines the current mode of the PPU.
    /// </summary>
    bool getModeFlag() const { return (m_value & Tools::STAT_MODE_PPU) != 0; }


    uint8_t value() const { return m_value; }

private:
	const uint8_t m_value;
};

class PPU {
public:
    PPU(Memory& memory, std::array<uint8_t, 0x5A00>& buffer);
    ~PPU();

    void start();
    void update(const uint8_t cycles);
    void stop();
    void init();

private:
    Memory& m_memory;
    std::array<uint8_t, 0x5A00>& m_buffer;
    PPUMode m_mode = PPUMode::NONE;
    std::map<PPUMode, PPUMethod> m_modes;
    std::vector<Sprite> m_sprites;

    std::atomic<uint32_t> m_clock = 0;
    std::atomic<bool> m_isRunning;
    std::thread m_thread;

private: 
    void run();

    PPUMode OAMSearch();
    PPUMode pixelTransfer();
    PPUMode hblank();
    PPUMode vblank();
    PPUMode wait();
    void renderBackground();
    void renderWindow();
    void renderSprite();

    /// <summary>
    /// Gets the value of the LCD Control Register (LCDC).
    /// </summary>
    uint8_t getLCDC() const;

    /// <summary>
    /// Sets the value of the LCD Control Register (LCDC).
    /// </summary>
    void setLCDC(uint8_t value);

    /// <summary>
    /// Gets the value of the LCD Status Register (STAT).
    /// </summary>
    uint8_t getLCDSCR() const;

    /// <summary>
    /// Sets the value of the LCD Status Register (STAT).
    /// </summary>
    void setLCDSCR(uint8_t value);
    void setSTAT(PPUMode value);

    /// <summary>
    /// Gets the value of the Scroll Y (SCY) register.
    /// </summary>
    uint8_t getSCY() const;

    /// <summary>
    /// Sets the value of the Scroll Y (SCY) register.
    /// </summary>
    void setSCY(uint8_t value);

    /// <summary>
    /// Gets the value of the Scroll X (SCX) register.
    /// </summary>
    uint8_t getSCX() const;

    /// <summary>
    /// Sets the value of the Scroll X (SCX) register.
    /// </summary>
    void setSCX(uint8_t value);

    /// <summary>
    /// Gets the value of the LY (current scanline) register.
    /// </summary>
    uint8_t getLY() const;

    /// <summary>
    /// Sets the value of the LY (current scanline) register.
    /// </summary>
    void setLY(uint8_t value);

    /// <summary>
    /// Gets the value of the LY Compare (LYC) register.
    /// </summary>
    uint8_t getLYC() const;

    /// <summary>
    /// Sets the value of the LY Compare (LYC) register.
    /// </summary>
    void setLYC(uint8_t value);

    /// <summary>
    /// Gets the value of the Window Y (WY) position register.
    /// </summary>
    uint8_t getWY() const;

    /// <summary>
    /// Sets the value of the Window Y (WY) position register.
    /// </summary>
    void setWY(uint8_t value);

    /// <summary>
    /// Gets the value of the Window X (WX) position register.
    /// </summary>
    uint8_t getWX() const;

    /// <summary>
    /// Sets the value of the Window X (WX) position register.
    /// </summary>
    void setWX(uint8_t value);

    /// <summary>
    /// Gets the value of the Background Palette (BGP) register.
    /// </summary>
    uint8_t getBGP() const;

    /// <summary>
    /// Sets the value of the Background Palette (BGP) register.
    /// </summary>
    void setBGP(uint8_t value);

    /// <summary>
    /// Gets the value of the Object Palette 0 (OBP0) register.
    /// </summary>
    uint8_t getOBP0() const;

    /// <summary>
    /// Sets the value of the Object Palette 0 (OBP0) register.
    /// </summary>
    void setOBP0(uint8_t value);

    /// <summary>
    /// Gets the value of the Object Palette 1 (OBP1) register.
    /// </summary>
    uint8_t getOBP1() const;

    /// <summary>
    /// Sets the value of the Object Palette 1 (OBP1) register.
    /// </summary>
    void setOBP1(uint8_t value);

    /// <summary>
    /// Gets the value of the DMA (Direct Memory Access) register.
    /// </summary>
    uint8_t getDMA() const;

    /// <summary>
    /// Sets the value of the DMA (Direct Memory Access) register.
    /// </summary>
    void setDMA(uint8_t value);
};
}
}
