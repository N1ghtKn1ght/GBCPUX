// std
#include <algorithm>
#include <chrono>

// project
#include "memory.h"
#include "ppu.h"

namespace GBCPUX {
namespace Hardware {
    PPU::PPU(Memory& memory, std::array<uint8_t, 0x5A00>& buffer)
        : m_memory(memory), m_buffer(buffer)
{
    init();
}

PPU::~PPU()
{
    stop();
}

void PPU::start()
{
    if (m_thread.joinable()) {
        stop();
    }
    init();
    m_isRunning.store(true, std::memory_order_release);
    m_thread = std::thread(&PPU::run, this);
}

void PPU::update(const uint8_t cycles)
{
    m_clock.fetch_add(cycles, std::memory_order_release);
}

void PPU::stop()
{
    m_isRunning.store(false, std::memory_order_release);
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void PPU::run() 
{
    while (m_isRunning.load(std::memory_order_acquire))
    {
        if (m_clock.load(std::memory_order_acquire) == 0) {
            continue;
        }

        LCDC lcdc = getLCDC();
        if (!lcdc.isEnabled()) {
            m_mode = LCD_WAIT;
            m_clock.store(0);
            continue;
        }
        
        while (m_clock.load(std::memory_order_acquire) >= m_modes[m_mode].duration) {
            const PPUMethod& mode = m_modes[m_mode];
            m_clock.fetch_sub(mode.duration);
            m_mode = (this->*mode.func)();
            setSTAT(m_mode);
        }
    }
}

void PPU::init()
{
    m_clock.store(0, std::memory_order_relaxed);
    m_modes.clear();
	m_mode = PPUMode::OAM_SEARCH;
    m_modes[PPUMode::OAM_SEARCH] = PPUMethod(OAM_SEARCH_DURATION, &PPU::OAMSearch);
    m_modes[PPUMode::PIXEL_TRANSFER] = PPUMethod(PIXEL_TRANSFER_DURATION, &PPU::pixelTransfer);
    m_modes[PPUMode::HBLANK] = PPUMethod(HBLANK_DURATION, &PPU::hblank);
    m_modes[PPUMode::VBLANK] = PPUMethod(VBLANK_DURATION, &PPU::vblank);
    m_modes[PPUMode::LCD_WAIT] = PPUMethod(WAIT_DURATION, &PPU::wait);

    setLCDC(0x91);
    setLCDSCR(0x06);
    setSCY(0x00);
    setSCX(0x00);
    setLY(0x00);
    setLYC(0x00);
    setBGP(0xFC);
    setOBP0(0xFF);
    setOBP1(0xFF);
    setWY(0x00);
    setWX(0x00);
}

PPUMode PPU::OAMSearch()
{
    m_sprites.clear();

    const uint8_t ly = getLY();
    const LCDC lcdc = getLCDC();
    const bool is8x16 = lcdc.getSpriteSize();
    for (uint16_t i = 0; i < TOTAL_SPRITES; ++i) {
        uint16_t addr = i * BYTES_PER_SPRITE + OAM_MAP;
        Sprite sprite;
        sprite.y = m_memory.read(addr);
        sprite.x = m_memory.read(addr + 0x01);
        sprite.tile = m_memory.read(addr + 0x02);
        sprite.flag = m_memory.read(addr + 0x03);
        uint8_t sprite_height = is8x16 ? 16 : 8;
        if (ly + 16 >= sprite.y && ly + 16 < sprite.y + sprite_height) {
            m_sprites.push_back(sprite);
        }
    }

    std::sort(m_sprites.begin(), m_sprites.end(), [](const Sprite& a, const Sprite& b) {
        return a.x < b.x;
    });


    return PPUMode::PIXEL_TRANSFER;
}

PPUMode PPU::pixelTransfer()
{
    const uint8_t ly = getLY();
    const uint8_t wy = getWY();
    const LCDC lcdc = getLCDC();


    if (lcdc.isBackgroundEnabled()) {
        renderBackground();
    }

    if (lcdc.isWindowEnabled() && ly >= wy) {
        renderWindow();
    }

    if (lcdc.isSpriteEnabled()) {
        renderSprite();
    }

    return PPUMode::HBLANK;
}

PPUMode PPU::hblank()
{
    const uint8_t ly = getLY();
    const uint8_t lyc = getLYC();

    setLY(ly + 1);

    if (getLY() == 144) {
        return PPUMode::VBLANK;
    }
    return PPUMode::OAM_SEARCH;
}

PPUMode PPU::vblank()
{  
   const uint8_t ly = getLY();

   if (ly == 144) {
       uint8_t IF = m_memory.read(Tools::IF_ADDR);
       IF |= Tools::V_BLANK_BIT;
       m_memory.write(Tools::IF_ADDR, IF);
   }

   if (ly == 153) {
       setLY(0);
       return PPUMode::OAM_SEARCH;
   }

   setLY(ly + 1);
   return PPUMode::VBLANK;
}

PPUMode PPU::wait()
{
    setLY(0);
    m_clock.store(0, std::memory_order_release);
    return PPUMode::HBLANK;
}

void PPU::renderBackground()
{
    const uint8_t scx = getSCX();
    const uint8_t scy = getSCY();
    const uint8_t ly = getLY();
    const LCDC lcdc = getLCDC();

    const bool tileSetSelect = lcdc.getTileSetSelect();
    const bool tileMapSelect = lcdc.getBackgroundTileMap();

    const uint16_t tileMapBase = tileMapSelect ? 0x9C00 : 0x9800;
    const uint16_t tileSetBase = tileSetSelect ? 0x8000 : 0x8800;

    const uint8_t yOffset = (ly + scy) % 256;
    const uint8_t tileRow = yOffset / 8;

    for (uint8_t x = 0; x < SCREEN_WIDTH; ++x) {
        const uint8_t xOffset = (x + scx) % 256;
        const uint8_t tileCol = xOffset / 8;

        const uint16_t tileIndexAddr = tileMapBase + tileRow * 32 + tileCol;
        const uint8_t tileIndex = m_memory.read(tileIndexAddr);

        const int16_t tileAddr = tileSetBase + (tileSetSelect ? tileIndex * 16 : (int8_t)tileIndex * 16);
        const uint8_t tileLine = yOffset % 8;

        const uint8_t lowByte = m_memory.read(tileAddr + tileLine * 2);
        const uint8_t highByte = m_memory.read(tileAddr + tileLine * 2 + 1);

        const uint8_t bitIndex = 7 - (xOffset % 8);
        const uint8_t colorIndex = ((highByte >> bitIndex) & 0x01) << 1 | ((lowByte >> bitIndex) & 0x01);

        m_buffer[ly * SCREEN_WIDTH + x] = colorIndex;
    }
}

void PPU::renderWindow()  
{  
   const uint8_t wx = getWX() - 7;  
   const uint8_t wy = getWY();  
   const uint8_t ly = getLY();  
   const LCDC lcdc = getLCDC();  

   if (ly < wy || wx >= SCREEN_WIDTH) {  
       return;  
   }  

   const bool tileSetSelect = lcdc.getTileSetSelect();  
   const bool tileMapSelect = lcdc.getWindowTileMap();  

   const uint16_t tileMapBase = tileMapSelect ? 0x9C00 : 0x9800;  
   const uint16_t tileSetBase = tileSetSelect ? 0x8000 : 0x8800;  

   const uint8_t windowY = ly - wy;  
   const uint8_t tileRow = windowY / 8;  

   for (uint8_t x = wx; x < SCREEN_WIDTH; ++x) {  
       const uint8_t windowX = x - wx;  
       const uint8_t tileCol = windowX / 8;  

       const uint16_t tileIndexAddr = tileMapBase + tileRow * 32 + tileCol;  
       const uint8_t tileIndex = m_memory.read(tileIndexAddr);  

       const int16_t tileAddr = tileSetBase + (tileSetSelect ? tileIndex * 16 : (int8_t)tileIndex * 16);  
       const uint8_t tileLine = windowY % 8;  

       const uint8_t lowByte = m_memory.read(tileAddr + tileLine * 2);  
       const uint8_t highByte = m_memory.read(tileAddr + tileLine * 2 + 1);  

       const uint8_t bitIndex = 7 - (windowX % 8);  
       const uint8_t colorIndex = ((highByte >> bitIndex) & 0x01) << 1 | ((lowByte >> bitIndex) & 0x01);  

       m_buffer[ly * SCREEN_WIDTH + x] = colorIndex;  
   }  
}

void PPU::renderSprite()  
{  
   const uint8_t ly = getLY();  
   const LCDC lcdc = getLCDC();  

   if (!lcdc.isSpriteEnabled()) {  
       return;  
   }  

   const bool is8x16 = lcdc.getSpriteSize();  

   for (const Sprite& sprite : m_sprites) {  
       const uint8_t spriteHeight = is8x16 ? 16 : 8;  

       if (ly + 16 < sprite.y || ly + 16 >= sprite.y + spriteHeight) {  
           continue;  
       }  

       const uint8_t tileLine = (ly + 16 - sprite.y) % spriteHeight;  
       const uint16_t tileAddr = 0x8000 + sprite.tile * 16 + (tileLine * 2);  

       const uint8_t lowByte = m_memory.read(tileAddr);  
       const uint8_t highByte = m_memory.read(tileAddr + 1);  

       for (uint8_t x = 0; x < 8; ++x) {  
           const uint8_t bitIndex = 7 - x;  
           const uint8_t colorIndex = ((highByte >> bitIndex) & 0x01) << 1 | ((lowByte >> bitIndex) & 0x01);  

           if (colorIndex == 0) {  
               continue;  
           }  

           const int16_t screenX = sprite.x - 8 + x;  

           if (screenX < 0 || screenX >= SCREEN_WIDTH) {  
               continue;  
           }  

           m_buffer[ly * SCREEN_WIDTH + screenX] = colorIndex;  
       }  
   }  
}

uint8_t PPU::getLCDC() const
{
    return m_memory.read(Tools::LCDC_ADDR);
}

void PPU::setLCDC(uint8_t value)
{
    m_memory.write(Tools::LCDC_ADDR, value);
}

uint8_t PPU::getLCDSCR() const
{
    return m_memory.read(Tools::STAT_ADDR);
}

void PPU::setLCDSCR(uint8_t value)
{
    m_memory.write(Tools::STAT_ADDR, value);
}

void PPU::setSTAT(PPUMode value)
{
    LCDSCR stat = (getLCDSCR() & ~0x03) | static_cast<uint8_t>(value);

    setLCDSCR(stat.value());
    uint8_t IF = m_memory.read(Tools::IF_ADDR);
   
    switch (value)
    {
    case PPUMode::HBLANK:
        if (stat.getHBlankInterrupt()) {
            m_memory.write(Tools::IF_ADDR, IF | Tools::LCD_STAT_BIT);
        }
        break;
    case PPUMode::VBLANK:
        if (stat.getVBlankInterrupt()) {
            m_memory.write(Tools::IF_ADDR, IF | Tools::LCD_STAT_BIT);
        }  
        break;
    case PPUMode::OAM_SEARCH:
        if (stat.getOAMInterrupt()) {
            m_memory.write(Tools::IF_ADDR, IF | Tools::LCD_STAT_BIT);
        }
        break;
    }
}

uint8_t PPU::getSCY() const
{
    return m_memory.read(Tools::SCY_ADDR);
}

void PPU::setSCY(uint8_t value)
{
    m_memory.write(Tools::SCY_ADDR,value);
}

uint8_t PPU::getSCX() const 
{
    return m_memory.read(Tools::SCX_ADDR);
}

void PPU::setSCX(uint8_t value) {

    m_memory.write(Tools::SCX_ADDR, value);
}

uint8_t PPU::getLY() const 
{
    return m_memory.read(Tools::LY_ADDR);
}

void PPU::setLY(uint8_t value) 
{
    uint8_t stat = getLCDSCR();
    if (value == getLYC()) {
        stat |= Tools::STAT_COINCIDENCE_FLAG;
    }
    else {
        stat &= ~Tools::STAT_COINCIDENCE_FLAG;
    }

    setLCDSCR(stat);

    m_memory.write(Tools::LY_ADDR, value);
}

uint8_t PPU::getLYC() const 
{
    return m_memory.read(Tools::LYC_ADDR);
}

void PPU::setLYC(uint8_t value) 
{
    uint8_t stat = getLCDSCR();
    if (getLY() == value) {
        stat |= Tools::STAT_COINCIDENCE_FLAG;
    }
    else {
        stat &= ~Tools::STAT_COINCIDENCE_FLAG;
    }

    setLCDSCR(stat);

    m_memory.write(Tools::LYC_ADDR, value);
}

uint8_t PPU::getWY() const 
{
    return m_memory.read(Tools::WY_ADDR);
}

void PPU::setWY(uint8_t value) 
{
    m_memory.write(Tools::WY_ADDR, value);
}

uint8_t PPU::getWX() const 
{
    return m_memory.read(Tools::WX_ADDR);
}

void PPU::setWX(uint8_t value) 
{
    m_memory.write(Tools::WX_ADDR, value);
}

uint8_t PPU::getBGP() const 
{
    return m_memory.read(Tools::BGP_ADDR);
}

void PPU::setBGP(uint8_t value) 
{
    m_memory.write(Tools::BGP_ADDR, value);
}

uint8_t PPU::getOBP0() const 
{
    return m_memory.read(Tools::OBP0_ADDR);
}

void PPU::setOBP0(uint8_t value) 
{
    m_memory.write(Tools::OBP0_ADDR, value);
}

uint8_t PPU::getOBP1() const 
{
    return m_memory.read(Tools::OBP1_ADDR);
}

void PPU::setOBP1(uint8_t value) {
    m_memory.write(Tools::OBP1_ADDR, value);
}

uint8_t PPU::getDMA() const 
{
    return m_memory.read(Tools::DMA_ADDR);
}

void PPU::setDMA(uint8_t value) 
{
    m_memory.write(Tools::DMA_ADDR, value);
}


}
}