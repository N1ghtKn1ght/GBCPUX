#include "memory.h"

namespace GBCPUX {
namespace Hardware {

bool Memory::load(const std::string& buffer)
{
	if (buffer.size() < 0x4000) {
		return false;
	}
	m_type = buffer[0x147];
	uint8_t ROMSize = buffer[0x148];
	uint8_t banks;
	if (ROMSize < 0x07) {
		banks = 0x01 << (ROMSize + 0x01);
	}
	else if (ROMSize == 0x52) {
		banks = 72;
	}
	else if (ROMSize == 0x53) {
		banks = 80;
	}
	else if (ROMSize == 0x54) {
		banks = 96;
	}
	else {
		return false;
	}

	std::copy(buffer.begin(), buffer.begin() + 0x4000, ROM.begin());
	size_t size = 0x4000 * (banks - 1);
	EROM.resize(size);
	std::copy(buffer.begin() + 0x4000, buffer.end(), EROM.begin());

	return true;
}

uint8_t Memory::read(uint16_t address) const
{
	uint8_t value = 0x0;
	if (ROM_MAP <= address && address <= 0x3FFF) {
		value = ROM[address];
	}
	else if (EROM_MAP <= address && address <= 0x7FFF) {
		uint8_t bank = std::max((uint8_t)0x01, m_currentBankROM);
		uint32_t addr = (address - EROM_MAP) * bank;
		value = EROM[addr];
	}
	else if (VRAM_MAP <= address && address <= 0x9FFF) {
		value = VRAM[address - VRAM_MAP];
	}
	else if (ERAM_MAP <= address && address <= 0xBFFF) {
		value = ERAM[address - ERAM_MAP];
	}
	else if (RAM_MAP <= address && address <= 0xDFFF) {
		value = RAM[address - RAM_MAP];
	}
	else if (OAM_MAP <= address && address <= 0xFE9F) {
		value = OAM[address - OAM_MAP];
	}
	else if (IO_MAP <= address && address <= 0xFF7F) {
		value = IO[address - IO_MAP];
	}
	else if (HRAM_MAP <= address && address <= 0xFFFF) {
		value = HRAM[address - HRAM_MAP];
	}
	else {
		std::cout << "read missing address " << std::hex << address << "\n";
	}

	return value;
}

void Memory::write(uint16_t address, uint8_t value)
{
	if (0x2000 <= address && address <= 0x3FFF) {
		m_currentBankROM = value & 0x7F;
	}
	else if (0x8000 <= address && address <= 0x9FFF) {
		VRAM[address - 0x8000] = value;
	}
	else if (0xA000 <= address && address <= 0xBFFF) {
		ERAM[address - 0xA000] = value;
	}
	else if (0xC000 <= address && address <= 0xDFFF) {
		RAM[address - 0xC000] = value;
	}
	else if (0xFE00 <= address && address <= 0xFE9F) {
		OAM[address - 0xFE00] = value;
	}
	else if (0xFF00 <= address && address <= 0xFF7F) {
		IO[address - 0xFF00] = value;
	}
	else if (0xFF80 <= address && address <= 0xFFFF) {
		HRAM[address - 0xFF80] = value;
	}
	else {
		std::cerr << "write missing address " 
			<< std::hex << address << " value: " << std::hex << (int)value << "\n";
	}
}
}
}