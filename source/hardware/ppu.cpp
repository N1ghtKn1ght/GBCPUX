#include "ppu.h"

#include "memory.h"

namespace GBCPUX {
namespace Hardware {
PPU::PPU(Memory& memory) :
	m_memory(memory)
{
    reset();
}

void PPU::update(const uint8_t cycles)
{
	m_clock += cycles;
	while (m_clock != 0) {
        const PPUMethod& mode = m_modes[m_mode];
        if (m_clock < mode.duration) {
            break;
        }
        (this->*mode.func)();
        m_clock -= mode.duration;
        m_mode = PPUMode((m_mode + 1) % m_modes.size());
	}
}

void PPU::reset()
{
    m_clock = 0;
    m_modes.clear();
	m_mode = PPUMode::OAM_SEARCH;
    m_modes[PPUMode::OAM_SEARCH] = PPUMethod(OAM_SEARCH_DURATION, &PPU::OAMSearch);
    m_modes[PPUMode::PIXEL_TRANSFER] = PPUMethod(PIXEL_TRANSFER_DURATION, &PPU::pixelTransfer);
    m_modes[PPUMode::HBLANK] = PPUMethod(HBLANK_DURATION, &PPU::hblank);
    m_modes[PPUMode::VBLANK] = PPUMethod(VBLANK_DURATION, &PPU::vblank);

    setLCDC(0x91);
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

void PPU::OAMSearch()
{
    m_sprites.clear();
    for (uint16_t i = 0; i < TOTAL_SPRITES; ++i) {
        uint16_t addr = i * BYTES_PER_SPRITE + OAM_MAP;
        Sptire sprite;
        sprite.x = m_memory.read(addr);
        sprite.y = m_memory.read(++addr);
        sprite.tile = m_memory.read(++addr);
        sprite.flag = m_memory.read(++addr);
		m_sprites.push_back(sprite);
    }
}

void PPU::pixelTransfer()
{
    uint8_t ly = getLY();
    LCDC lcdc = getLCDC();
    for (uint8_t x = 0; x < SCREEN_WIDTH; x++) {
        
    }
}

void PPU::hblank()
{

}

void PPU::vblank()
{

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
    m_memory.write(Tools::LY_ADDR, value);
}

uint8_t PPU::getLYC() const 
{
    return m_memory.read(Tools::LYC_ADDR);
}

void PPU::setLYC(uint8_t value) 
{
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