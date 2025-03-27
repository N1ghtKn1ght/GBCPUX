#pragma once 

#include <iostream>
#include <map>
#include <vector>

#include "tools/commontoolkit.h"

namespace GBCPUX {
namespace Hardware {
class Memory;

constexpr uint32_t OAM_SEARCH_DURATION = 80;
constexpr uint32_t PIXEL_TRANSFER_DURATION = 172;
constexpr uint32_t HBLANK_DURATION = 204;
constexpr uint32_t VBLANK_DURATION = 4560;
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
    PIXEL_TRANSFER = 3
};

class PPU;
struct PPUMethod
{
    uint32_t duration = 0;
    void (PPU::* func)() = nullptr;

    PPUMethod(uint32_t duration, void (PPU::* func)())
        : duration(duration), func(func) { }

    PPUMethod() {}
    PPUMethod(PPUMethod&) = delete;
};

struct Sptire 
{
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t tile = 0;
    uint8_t flag = 0;
};

struct LCDC
{
public:
    LCDC(uint8_t value)
        : value(value) {}

    /// <summary>
    /// Background and window enable. This bit determines whether the background and window layers should be drawn or not.
    /// </summary>
    bool isBackgroundEnabled() const { return value & Tools::LCDC_STAT_BACKGROUND; }

    /// <summary>
    /// Object enable. This bit determines whether sprites should be drawn or not.
    /// </summary>
    bool isSpriteEnabled() const { return value & Tools::LCDC_STAT_OBJECT; }

    /// <summary>
    /// Object size. This bit determines whether sprites consist of one (8x8) or two (8x16) tiles.
    /// </summary>
    bool getSpriteSize() const { return value & Tools::LCDC_SIZE_OBJECT; }

    /// <summary>
    /// Background tile map select. This bit determines which map should be used for the background layer.
    /// </summary>
    bool getBackgroundTileMap() const { return value & Tools::LCDC_SELECT_BACKGROND; }

    /// <summary>
    /// Tile set select. This bit determines which tile set should be used by the tile maps.
    /// </summary>
    bool getTileSetSelect() const { return value & Tools::LCDC_SELECT_TITLE; }

    /// <summary>
    /// Window enable. This bit determines whether the window layer should be drawn or not.
    /// </summary>
    bool isWindowEnabled() const { return value & Tools::LCDC_STAT_WINDOW; }

    /// <summary>
    /// Window tile map select. This bit determines which map should be used for the window layer.
    /// </summary>
    bool getWindowTileMap() const { return value & Tools::LCDC_SELECT_WINDOW; }

    /// <summary>
    /// LCD enable. If this bit is 0, the display is off.
    /// </summary>
    bool isLCDEEnabled() const { return value & Tools::LCDC_STAT; }


private:
    const uint8_t value;
};

class LCDSCR {

public:
	LCDSCR(uint8_t value)
		: value(value) {
	}
    
	/// <summary>
	/// LYC interrupt enable. Enables interrupts when the current scanline equals the value of the LYC register.
	/// </summary>
	bool getLYCInterrupt() const { return value & Tools::STAT_INTERRUPT_LYC; }
	/// <summary>
	/// OAM search interrupt enable. Enables interrupts when the PPU enters OAM search.
	/// </summary>
	bool getOAMInterrupt() const { return value & Tools::STAT_INTERRUPT_OAM; }
	/// <summary>
	/// V-blank interrupt enable. Enables interrupts when the PPU enters a V-blank.
	/// </summary>
	bool getVBlankInterrupt() const { return value & Tools::STAT_INTERRUPT_VBLANK; }
	/// <summary>
	///  H-blank interrupt enable. Enables interrupts when the PPU enters an H - blank.
	/// </summary>
	bool getHBlankInterrupt() const { return value & Tools::STAT_INTERRUPT_HBLANK; }
	/// <summary>
	/// LYC=LY flag. Is set to 1 if the current scanline equals the value of the LYC register.
	/// </summary>
	bool getCoincidenceFlag() const { return value & Tools::STAT_COINCIDENCE_FLAG; }
	/// <summary>
	/// Mode flag. This bit determines the current mode of the PPU.
	/// </summary>
	bool getModeFlag() const { return value & Tools::STAT_MODE_PPU; }

private:
	const uint8_t value;
};

class PPU {
public:
    PPU(Memory& memory);
    ~PPU() = default;

    void update(const uint8_t cycles);

private:
    Memory& m_memory;

    PPUMode m_mode = PPUMode::NONE;
    std::map<PPUMode, PPUMethod> m_modes;
    uint32_t m_clock = 0;
    std::vector<Sptire> m_sprites;

private: 
    void reset();

    void OAMSearch();
    void pixelTransfer();
    void hblank();
    void vblank();

    uint8_t getLCDC() const;
    void setLCDC(uint8_t value);
    uint8_t getLCDSCR() const;
    void setLCDSCR(uint8_t value);
    uint8_t getSCY() const;
    void setSCY(uint8_t value);
    uint8_t getSCX() const;
    void setSCX(uint8_t value);
    uint8_t getLY() const;
    void setLY(uint8_t value);
    uint8_t getLYC() const;
    void setLYC(uint8_t value);
    uint8_t getWY() const;
    void setWY(uint8_t value);
    uint8_t getWX() const;
    void setWX(uint8_t value);
    uint8_t getBGP() const;
    void setBGP(uint8_t value);
    uint8_t getOBP0() const;
    void setOBP0(uint8_t value);
    uint8_t getOBP1() const;
    void setOBP1(uint8_t value);
    uint8_t getDMA() const;
    void setDMA(uint8_t value);
};
}
}
