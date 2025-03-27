#include <cstdint>
#include <iostream>

#include "hardware/cpu.h"
#include "hardware/memory.h"
#include "hardware/ppu.h"

int main(int argc, char* argv[]) {
    using namespace GBCPUX::Hardware;

    if (argc <= 1) {
		return 1;
    }
    
    Memory memory;
    PPU ppu(memory);
    CPU cpu(memory, ppu);
	std::string gb = argv[1];
    if (cpu.load(gb)) {
        cpu.start();
    } 
	else {
		std::cout << "Error loading game" << std::endl;
		return 1;
	}

    return 0;
}
