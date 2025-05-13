// std
#include <chrono>
#include <fstream>

// project
#include "cpu.h"
#include "tools/commontoolkit.h"

namespace GBCPUX {
namespace Hardware {

CPU::CPU(Memory& memory) 
    : m_memory(memory)
{
    init();
}

CPU::~CPU() 
{
    stop();
}

void CPU::start()
{
    if (m_thread.joinable()) {
        stop();
    }

    m_isRunning.store(true, std::memory_order_release);
    m_thread = std::thread(&CPU::run, this);
}

void CPU::stop() 
{
    m_isRunning.store(false, std::memory_order_release);
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void CPU::update(const uint8_t cycles)
{
    m_clock.fetch_add(cycles, std::memory_order_release);
}

void CPU::run()
{
    while (m_isRunning.load(std::memory_order_acquire))
    {
        int cycles = step() * 4;

        waitHandle();

        if (m_regs.NIME) {
            m_regs.IME = true;
            m_regs.NIME = false;
        }
    }
}

uint8_t CPU::fetch()
{
    if (m_regs.HALT_BUG) {
        m_regs.HALT_BUG = false;
        return m_memory.read(m_regs.PC);
    }

	uint8_t opcode = m_memory.read(m_regs.PC++);
	return opcode;
}

void CPU::init()
{
    m_regs.A = 0x01;
    m_regs.F = 0xB0;
    m_regs.B = 0x00;
    m_regs.C = 0x13;
    m_regs.D = 0x00;
    m_regs.E = 0xD8;
    m_regs.H = 0x01;
    m_regs.L = 0x4D;

    m_regs.SP = 0xFFFE;
    m_regs.PC = 0x0100;

    for (uint8_t i = 0; i < 0xFF; ++i) {
        opcodes[i] = &CPU::NOP;
        prefixs[i] = &CPU::NOP;
    }

    opcodes[0x00] = &CPU::NOP;	    opcodes[0x01] = &CPU::LD_BC_nn;	opcodes[0x02] = &CPU::LD_RBC_A;	    opcodes[0x03] = &CPU::INC_BC;	    opcodes[0x04] = &CPU::INC_B;	    opcodes[0x05] = &CPU::DEC_B;	opcodes[0x06] = &CPU::LD_B_n;	    opcodes[0x07] = &CPU::RLCA;	    opcodes[0x08] = &CPU::LD_nn_SP;	    opcodes[0x09] = &CPU::ADD_HL_BC;	opcodes[0x0A] = &CPU::LD_A_RBC;	    opcodes[0x0B] = &CPU::DEC_BC;	    opcodes[0x0C] = &CPU::INC_C;	    opcodes[0x0D] = &CPU::DEC_C;	    opcodes[0x0E] = &CPU::LD_C_n;	    opcodes[0x0F] = &CPU::RRCA;
    opcodes[0x10] = &CPU::STOP;	    opcodes[0x11] = &CPU::LD_DE_nn;	opcodes[0x12] = &CPU::LD_RDE_A;	    opcodes[0x13] = &CPU::INC_DE;	    opcodes[0x14] = &CPU::INC_D;	    opcodes[0x15] = &CPU::DEC_D;	opcodes[0x16] = &CPU::LD_D_n;	    opcodes[0x17] = &CPU::RLA;	    opcodes[0x18] = &CPU::JR_e;	        opcodes[0x19] = &CPU::ADD_HL_DE;	opcodes[0x1A] = &CPU::LD_A_RDE;	    opcodes[0x1B] = &CPU::DEC_DE;   	opcodes[0x1C] = &CPU::INC_E;	    opcodes[0x1D] = &CPU::DEC_E;	    opcodes[0x1E] = &CPU::LD_E_n;	    opcodes[0x1F] = &CPU::RRA;
    opcodes[0x20] = &CPU::JR_NZ_e;	opcodes[0x21] = &CPU::LD_HL_nn;	opcodes[0x22] = &CPU::LD_RHL_INC_A;	opcodes[0x23] = &CPU::INC_HL;	    opcodes[0x24] = &CPU::INC_H;	    opcodes[0x25] = &CPU::DEC_H;	opcodes[0x26] = &CPU::LD_H_n;	    opcodes[0x27] = &CPU::DAA;	    opcodes[0x28] = &CPU::JR_Z_e;	    opcodes[0x29] = &CPU::ADD_HL_HL;	opcodes[0x2A] = &CPU::LD_A_RHL_INC;	opcodes[0x2B] = &CPU::DEC_HL;   	opcodes[0x2C] = &CPU::INC_L;	    opcodes[0x2D] = &CPU::DEC_L;	    opcodes[0x2E] = &CPU::LD_L_n;	    opcodes[0x2F] = &CPU::CPL;
    opcodes[0x30] = &CPU::JR_NC_e;	opcodes[0x31] = &CPU::LD_SP_nn;	opcodes[0x32] = &CPU::LD_RHL_DEC_A;	opcodes[0x33] = &CPU::INC_SP;	    opcodes[0x34] = &CPU::INC_RHL;	    opcodes[0x35] = &CPU::DEC_RHL;	opcodes[0x36] = &CPU::LD_RHL;	    opcodes[0x37] = &CPU::SCF;	    opcodes[0x38] = &CPU::JR_C_e;	    opcodes[0x39] = &CPU::ADD_HL_SP;	opcodes[0x3A] = &CPU::LD_A_RHL_DEC;	opcodes[0x3B] = &CPU::DEC_SP;	    opcodes[0x3C] = &CPU::INC_A;	    opcodes[0x3D] = &CPU::DEC_A;	    opcodes[0x3E] = &CPU::LD_A_n;	    opcodes[0x3F] = &CPU::CCF;
    opcodes[0x40] = &CPU::LD_B_B;	opcodes[0x41] = &CPU::LD_B_C;	opcodes[0x42] = &CPU::LD_B_D;	    opcodes[0x43] = &CPU::LD_B_E;	    opcodes[0x44] = &CPU::LD_B_H;	    opcodes[0x45] = &CPU::LD_B_L;	opcodes[0x46] = &CPU::LD_B_RHL;	    opcodes[0x47] = &CPU::LD_B_A;	opcodes[0x48] = &CPU::LD_C_B;	    opcodes[0x49] = &CPU::LD_C_C;	    opcodes[0x4A] = &CPU::LD_C_D;	    opcodes[0x4B] = &CPU::LD_C_E;	    opcodes[0x4C] = &CPU::LD_C_H;	    opcodes[0x4D] = &CPU::LD_C_L;	    opcodes[0x4E] = &CPU::LD_C_RHL;	    opcodes[0x4F] = &CPU::LD_C_A;
    opcodes[0x50] = &CPU::LD_D_B;	opcodes[0x51] = &CPU::LD_D_C;	opcodes[0x52] = &CPU::LD_D_D;	    opcodes[0x53] = &CPU::LD_D_E;	    opcodes[0x54] = &CPU::LD_D_H;	    opcodes[0x55] = &CPU::LD_D_L;	opcodes[0x56] = &CPU::LD_D_RHL;	    opcodes[0x57] = &CPU::LD_D_A;	opcodes[0x58] = &CPU::LD_E_B;	    opcodes[0x59] = &CPU::LD_E_C;	    opcodes[0x5A] = &CPU::LD_E_D;	    opcodes[0x5B] = &CPU::LD_E_E;   	opcodes[0x5C] = &CPU::LD_E_H;	    opcodes[0x5D] = &CPU::LD_E_L;	    opcodes[0x5E] = &CPU::LD_E_RHL;	    opcodes[0x5F] = &CPU::LD_E_A;
    opcodes[0x60] = &CPU::LD_H_B;	opcodes[0x61] = &CPU::LD_H_C;	opcodes[0x62] = &CPU::LD_H_D;	    opcodes[0x63] = &CPU::LD_H_E;	    opcodes[0x64] = &CPU::LD_H_H;	    opcodes[0x65] = &CPU::LD_H_L;	opcodes[0x66] = &CPU::LD_H_RHL;	    opcodes[0x67] = &CPU::LD_H_A;	opcodes[0x68] = &CPU::LD_L_B;	    opcodes[0x69] = &CPU::LD_L_C;	    opcodes[0x6A] = &CPU::LD_L_D;	    opcodes[0x6B] = &CPU::LD_L_E;	    opcodes[0x6C] = &CPU::LD_L_H;	    opcodes[0x6D] = &CPU::LD_L_L;	    opcodes[0x6E] = &CPU::LD_L_RHL;	    opcodes[0x6F] = &CPU::LD_L_A;
    opcodes[0x70] = &CPU::LD_RHL_B;	opcodes[0x71] = &CPU::LD_RHL_C;	opcodes[0x72] = &CPU::LD_RHL_D;	    opcodes[0x73] = &CPU::LD_RHL_E; 	opcodes[0x74] = &CPU::LD_RHL_H;	    opcodes[0x75] = &CPU::LD_RHL_L;	opcodes[0x76] = &CPU::HALT;	        opcodes[0x77] = &CPU::LD_RHL_A;	opcodes[0x78] = &CPU::LD_A_B;	    opcodes[0x79] = &CPU::LD_A_C;	    opcodes[0x7A] = &CPU::LD_A_D;	    opcodes[0x7B] = &CPU::LD_A_E;	    opcodes[0x7C] = &CPU::LD_A_H;	    opcodes[0x7D] = &CPU::LD_A_L;	    opcodes[0x7E] = &CPU::LD_A_RHL;	    opcodes[0x7F] = &CPU::LD_A_A;
    opcodes[0x80] = &CPU::ADD_B;	opcodes[0x81] = &CPU::ADD_C;	opcodes[0x82] = &CPU::ADD_D;	    opcodes[0x83] = &CPU::ADD_E;	    opcodes[0x84] = &CPU::ADD_H;	    opcodes[0x85] = &CPU::ADD_L;	opcodes[0x86] = &CPU::ADD_RHL;	    opcodes[0x87] = &CPU::ADD_A;	opcodes[0x88] = &CPU::ADC_B;	    opcodes[0x89] = &CPU::ADC_C;	    opcodes[0x8A] = &CPU::ADC_D;	    opcodes[0x8B] = &CPU::ADC_E;	    opcodes[0x8C] = &CPU::ADC_H;	    opcodes[0x8D] = &CPU::ADC_L;	    opcodes[0x8E] = &CPU::ADC_RHL;	    opcodes[0x8F] = &CPU::ADC_A;
    opcodes[0x90] = &CPU::SUB_B;	opcodes[0x91] = &CPU::SUB_C;	opcodes[0x92] = &CPU::SUB_D;	    opcodes[0x93] = &CPU::SUB_E;	    opcodes[0x94] = &CPU::SUB_H;	    opcodes[0x95] = &CPU::SUB_L;	opcodes[0x96] = &CPU::SUB_RHL;	    opcodes[0x97] = &CPU::SUB_A;	opcodes[0x98] = &CPU::SBC_B;	    opcodes[0x99] = &CPU::SBC_C;	    opcodes[0x9A] = &CPU::SBC_D;	    opcodes[0x9B] = &CPU::SBC_E;	    opcodes[0x9C] = &CPU::SBC_H;	    opcodes[0x9D] = &CPU::SBC_L;	    opcodes[0x9E] = &CPU::SBC_RHL;	    opcodes[0x9F] = &CPU::SBC_A;
    opcodes[0xA0] = &CPU::AND_B;	opcodes[0xA1] = &CPU::AND_C;	opcodes[0xA2] = &CPU::AND_D;	    opcodes[0xA3] = &CPU::AND_E;	    opcodes[0xA4] = &CPU::AND_H;	    opcodes[0xA5] = &CPU::AND_L;	opcodes[0xA6] = &CPU::AND_RHL;	    opcodes[0xA7] = &CPU::AND_A;	opcodes[0xA8] = &CPU::XOR_B;	    opcodes[0xA9] = &CPU::XOR_C;	    opcodes[0xAA] = &CPU::XOR_D;	    opcodes[0xAB] = &CPU::XOR_E;	    opcodes[0xAC] = &CPU::XOR_H;	    opcodes[0xAD] = &CPU::XOR_L;	    opcodes[0xAE] = &CPU::XOR_RHL;	    opcodes[0xAF] = &CPU::XOR_A;
    opcodes[0xB0] = &CPU::OR_B;	    opcodes[0xB1] = &CPU::OR_C;	    opcodes[0xB2] = &CPU::OR_D;	        opcodes[0xB3] = &CPU::OR_E;	        opcodes[0xB4] = &CPU::OR_H;	        opcodes[0xB5] = &CPU::OR_L;	    opcodes[0xB6] = &CPU::OR_RHL;	    opcodes[0xB7] = &CPU::OR_A;	    opcodes[0xB8] = &CPU::CP_B;	        opcodes[0xB9] = &CPU::CP_C;	        opcodes[0xBA] = &CPU::CP_D;	        opcodes[0xBB] = &CPU::CP_E;	        opcodes[0xBC] = &CPU::CP_H; 	    opcodes[0xBD] = &CPU::CP_L; 	    opcodes[0xBE] = &CPU::CP_RHL;	    opcodes[0xBF] = &CPU::CP_A;
    opcodes[0xC0] = &CPU::RET_NZ;	opcodes[0xC1] = &CPU::POP_BC;	opcodes[0xC2] = &CPU::JP_NZ_nn;	    opcodes[0xC3] = &CPU::JP_nn;	    opcodes[0xC4] = &CPU::CALL_NZ_nn;	opcodes[0xC5] = &CPU::PUSH_BC;	opcodes[0xC6] = &CPU::ADD_A_n;	    opcodes[0xC7] = &CPU::RST_00H;	opcodes[0xC8] = &CPU::RET_Z;	    opcodes[0xC9] = &CPU::RET;	        opcodes[0xCA] = &CPU::JP_Z_nn;	    opcodes[0xCB] = &CPU::PREFIX_CB;	opcodes[0xCC] = &CPU::CALL_Z_nn;	opcodes[0xCD] = &CPU::CALL_nn;	    opcodes[0xCE] = &CPU::ADC_A_n;	    opcodes[0xCF] = &CPU::RST_08H;
    opcodes[0xD0] = &CPU::RET_NC;	opcodes[0xD1] = &CPU::POP_DE;	opcodes[0xD2] = &CPU::JP_NC_nn;	    opcodes[0xD3] = &CPU::UNDEFINED_D3;	opcodes[0xD4] = &CPU::CALL_NC_nn;	opcodes[0xD5] = &CPU::PUSH_DE;	opcodes[0xD6] = &CPU::SUB_n;	    opcodes[0xD7] = &CPU::RST_10H;	opcodes[0xD8] = &CPU::RET_C;	    opcodes[0xD9] = &CPU::RETI;	        opcodes[0xDA] = &CPU::JP_C_nn;	    opcodes[0xDB] = &CPU::UNDEFINED_DB;	opcodes[0xDC] = &CPU::CALL_C_nn;	opcodes[0xDD] = &CPU::UNDEFINED_DD;	opcodes[0xDE] = &CPU::SBC_A_n;	    opcodes[0xDF] = &CPU::RST_18H;
    opcodes[0xE0] = &CPU::LDH_Rn_A;	opcodes[0xE1] = &CPU::POP_HL;	opcodes[0xE2] = &CPU::LDH_RC_A;	    opcodes[0xE3] = &CPU::UNDEFINED_E3;	opcodes[0xE4] = &CPU::UNDEFINED_E4;	opcodes[0xE5] = &CPU::PUSH_HL;	opcodes[0xE6] = &CPU::AND_n;	    opcodes[0xE7] = &CPU::RST_20H;	opcodes[0xE8] = &CPU::ADD_SP_e;	    opcodes[0xE9] = &CPU::JP_HL;	    opcodes[0xEA] = &CPU::LD_Rnn_A;	    opcodes[0xEB] = &CPU::UNDEFINED_EB;	opcodes[0xEC] = &CPU::UNDEFINED_EC;	opcodes[0xED] = &CPU::UNDEFINED_ED;	opcodes[0xEE] = &CPU::XOR_n;	    opcodes[0xEF] = &CPU::RST_28H;
    opcodes[0xF0] = &CPU::LDH_A_Rn;	opcodes[0xF1] = &CPU::POP_AF;	opcodes[0xF2] = &CPU::LDH_A_RC;	    opcodes[0xF3] = &CPU::DI;	        opcodes[0xF4] = &CPU::UNDEFINED_F4;	opcodes[0xF5] = &CPU::PUSH_AF;	opcodes[0xF6] = &CPU::OR_n;	        opcodes[0xF7] = &CPU::RST_30H;	opcodes[0xF8] = &CPU::LD_HL_SP_e;	opcodes[0xF9] = &CPU::LD_SP_HL;	    opcodes[0xFA] = &CPU::LD_A_Rnn;	    opcodes[0xFB] = &CPU::EI;	        opcodes[0xFC] = &CPU::UNDEFINED_FC;	opcodes[0xFD] = &CPU::UNDEFINED_FD;	opcodes[0xFE] = &CPU::CP_n;	        opcodes[0xFF] = &CPU::RST_38H;

    prefixs[0x00] = &CPU::RLC_B;	prefixs[0x01] = &CPU::RLC_C;	prefixs[0x02] = &CPU::RLC_D;	    prefixs[0x03] = &CPU::RLC_E;	    prefixs[0x04] = &CPU::RLC_H;	    prefixs[0x05] = &CPU::RLC_L;	prefixs[0x06] = &CPU::RLC_RHL;	    prefixs[0x07] = &CPU::RLC_A;	prefixs[0x08] = &CPU::RRC_B;	    prefixs[0x09] = &CPU::RRC_C;	    prefixs[0x0A] = &CPU::RRC_D;	    prefixs[0x0B] = &CPU::RRC_E;	    prefixs[0x0C] = &CPU::RRC_H;	    prefixs[0x0D] = &CPU::RRC_L;	    prefixs[0x0E] = &CPU::RRC_RHL;	    prefixs[0x0F] = &CPU::RRC_A;
    prefixs[0x10] = &CPU::RL_B;	    prefixs[0x11] = &CPU::RL_C;	    prefixs[0x12] = &CPU::RL_D;	        prefixs[0x13] = &CPU::RL_E;	        prefixs[0x14] = &CPU::RL_H;	        prefixs[0x15] = &CPU::RL_L;	    prefixs[0x16] = &CPU::RL_RHL;	    prefixs[0x17] = &CPU::RL_A;	    prefixs[0x18] = &CPU::RR_B;	        prefixs[0x19] = &CPU::RR_C;	        prefixs[0x1A] = &CPU::RR_D;	        prefixs[0x1B] = &CPU::RR_E;	        prefixs[0x1C] = &CPU::RR_H; 	    prefixs[0x1D] = &CPU::RR_L;	        prefixs[0x1E] = &CPU::RR_RHL;	    prefixs[0x1F] = &CPU::RR_A;
    prefixs[0x20] = &CPU::SLA_B;	prefixs[0x21] = &CPU::SLA_C;	prefixs[0x22] = &CPU::SLA_D;	    prefixs[0x23] = &CPU::SLA_E;	    prefixs[0x24] = &CPU::SLA_H;	    prefixs[0x25] = &CPU::SLA_L;	prefixs[0x26] = &CPU::SLA_RHL;	    prefixs[0x27] = &CPU::SLA_A;	prefixs[0x28] = &CPU::SRA_B;	    prefixs[0x29] = &CPU::SRA_C;	    prefixs[0x2A] = &CPU::SRA_D;	    prefixs[0x2B] = &CPU::SRA_E;	    prefixs[0x2C] = &CPU::SRA_H;	    prefixs[0x2D] = &CPU::SRA_L;	    prefixs[0x2E] = &CPU::SRA_RHL;	    prefixs[0x2F] = &CPU::SRA_A;
    prefixs[0x30] = &CPU::SWAP_B;	prefixs[0x31] = &CPU::SWAP_C;	prefixs[0x32] = &CPU::SWAP_D;	    prefixs[0x33] = &CPU::SWAP_E;	    prefixs[0x34] = &CPU::SWAP_H;	    prefixs[0x35] = &CPU::SWAP_L;	prefixs[0x36] = &CPU::SWAP_RHL;	    prefixs[0x37] = &CPU::SWAP_A;	prefixs[0x38] = &CPU::SRL_B;	    prefixs[0x39] = &CPU::SRL_C;	    prefixs[0x3A] = &CPU::SRL_D;	    prefixs[0x3B] = &CPU::SRL_E;	    prefixs[0x3C] = &CPU::SRL_H;	    prefixs[0x3D] = &CPU::SRL_L;	    prefixs[0x3E] = &CPU::SRL_RHL;	    prefixs[0x3F] = &CPU::SRL_A;
    prefixs[0x40] = &CPU::BIT_0_B;	prefixs[0x41] = &CPU::BIT_0_C;	prefixs[0x42] = &CPU::BIT_0_D;	    prefixs[0x43] = &CPU::BIT_0_E;	    prefixs[0x44] = &CPU::BIT_0_H;	    prefixs[0x45] = &CPU::BIT_0_L;	prefixs[0x46] = &CPU::BIT_0_RHL;	prefixs[0x47] = &CPU::BIT_0_A;	prefixs[0x48] = &CPU::BIT_1_B;	    prefixs[0x49] = &CPU::BIT_1_C;	    prefixs[0x4A] = &CPU::BIT_1_D;	    prefixs[0x4B] = &CPU::BIT_1_E;	    prefixs[0x4C] = &CPU::BIT_1_H;	    prefixs[0x4D] = &CPU::BIT_1_L;	    prefixs[0x4E] = &CPU::BIT_1_RHL;	prefixs[0x4F] = &CPU::BIT_1_A;
    prefixs[0x50] = &CPU::BIT_2_B;	prefixs[0x51] = &CPU::BIT_2_C;	prefixs[0x52] = &CPU::BIT_2_D;	    prefixs[0x53] = &CPU::BIT_2_E;	    prefixs[0x54] = &CPU::BIT_2_H;	    prefixs[0x55] = &CPU::BIT_2_L;	prefixs[0x56] = &CPU::BIT_2_RHL;	prefixs[0x57] = &CPU::BIT_2_A;	prefixs[0x58] = &CPU::BIT_3_B;	    prefixs[0x59] = &CPU::BIT_3_C;	    prefixs[0x5A] = &CPU::BIT_3_D;	    prefixs[0x5B] = &CPU::BIT_3_E;	    prefixs[0x5C] = &CPU::BIT_3_H;	    prefixs[0x5D] = &CPU::BIT_3_L;	    prefixs[0x5E] = &CPU::BIT_3_RHL;	prefixs[0x5F] = &CPU::BIT_3_A;
    prefixs[0x60] = &CPU::BIT_4_B;	prefixs[0x61] = &CPU::BIT_4_C;	prefixs[0x62] = &CPU::BIT_4_D;	    prefixs[0x63] = &CPU::BIT_4_E;	    prefixs[0x64] = &CPU::BIT_4_H;	    prefixs[0x65] = &CPU::BIT_4_L;	prefixs[0x66] = &CPU::BIT_4_RHL;	prefixs[0x67] = &CPU::BIT_4_A;	prefixs[0x68] = &CPU::BIT_5_B;	    prefixs[0x69] = &CPU::BIT_5_C;	    prefixs[0x6A] = &CPU::BIT_5_D;	    prefixs[0x6B] = &CPU::BIT_5_E;	    prefixs[0x6C] = &CPU::BIT_5_H;	    prefixs[0x6D] = &CPU::BIT_5_L;	    prefixs[0x6E] = &CPU::BIT_5_RHL;	prefixs[0x6F] = &CPU::BIT_5_A;
    prefixs[0x70] = &CPU::BIT_6_B;	prefixs[0x71] = &CPU::BIT_6_C;	prefixs[0x72] = &CPU::BIT_6_D;	    prefixs[0x73] = &CPU::BIT_6_E;	    prefixs[0x74] = &CPU::BIT_6_H;	    prefixs[0x75] = &CPU::BIT_6_L;	prefixs[0x76] = &CPU::BIT_6_RHL;	prefixs[0x77] = &CPU::BIT_6_A;	prefixs[0x78] = &CPU::BIT_7_B;	    prefixs[0x79] = &CPU::BIT_7_C;	    prefixs[0x7A] = &CPU::BIT_7_D;	    prefixs[0x7B] = &CPU::BIT_7_E;	    prefixs[0x7C] = &CPU::BIT_7_H;	    prefixs[0x7D] = &CPU::BIT_7_L;	    prefixs[0x7E] = &CPU::BIT_7_RHL;	prefixs[0x7F] = &CPU::BIT_7_A;
    prefixs[0x80] = &CPU::RES_0_B;	prefixs[0x81] = &CPU::RES_0_C;	prefixs[0x82] = &CPU::RES_0_D;	    prefixs[0x83] = &CPU::RES_0_E;	    prefixs[0x84] = &CPU::RES_0_H;	    prefixs[0x85] = &CPU::RES_0_L;	prefixs[0x86] = &CPU::RES_0_RHL;	prefixs[0x87] = &CPU::RES_0_A;	prefixs[0x88] = &CPU::RES_1_B;	    prefixs[0x89] = &CPU::RES_1_C;	    prefixs[0x8A] = &CPU::RES_1_D;	    prefixs[0x8B] = &CPU::RES_1_E;	    prefixs[0x8C] = &CPU::RES_1_H;	    prefixs[0x8D] = &CPU::RES_1_L;	    prefixs[0x8E] = &CPU::RES_1_RHL;	prefixs[0x8F] = &CPU::RES_1_A;
    prefixs[0x90] = &CPU::RES_2_B;	prefixs[0x91] = &CPU::RES_2_C;	prefixs[0x92] = &CPU::RES_2_D;	    prefixs[0x93] = &CPU::RES_2_E;	    prefixs[0x94] = &CPU::RES_2_H;	    prefixs[0x95] = &CPU::RES_2_L;	prefixs[0x96] = &CPU::RES_2_RHL;	prefixs[0x97] = &CPU::RES_2_A;	prefixs[0x98] = &CPU::RES_3_B;	    prefixs[0x99] = &CPU::RES_3_C;	    prefixs[0x9A] = &CPU::RES_3_D;	    prefixs[0x9B] = &CPU::RES_3_E;	    prefixs[0x9C] = &CPU::RES_3_H;	    prefixs[0x9D] = &CPU::RES_3_L;	    prefixs[0x9E] = &CPU::RES_3_RHL;	prefixs[0x9F] = &CPU::RES_3_A;
    prefixs[0xA0] = &CPU::RES_4_B;	prefixs[0xA1] = &CPU::RES_4_C;	prefixs[0xA2] = &CPU::RES_4_D;	    prefixs[0xA3] = &CPU::RES_4_E;	    prefixs[0xA4] = &CPU::RES_4_H;	    prefixs[0xA5] = &CPU::RES_4_L;	prefixs[0xA6] = &CPU::RES_4_RHL;	prefixs[0xA7] = &CPU::RES_4_A;	prefixs[0xA8] = &CPU::RES_5_B;	    prefixs[0xA9] = &CPU::RES_5_C;	    prefixs[0xAA] = &CPU::RES_5_D;	    prefixs[0xAB] = &CPU::RES_5_E;	    prefixs[0xAC] = &CPU::RES_5_H;	    prefixs[0xAD] = &CPU::RES_5_L;	    prefixs[0xAE] = &CPU::RES_5_RHL;	prefixs[0xAF] = &CPU::RES_5_A;
    prefixs[0xB0] = &CPU::RES_6_B;	prefixs[0xB1] = &CPU::RES_6_C;	prefixs[0xB2] = &CPU::RES_6_D;	    prefixs[0xB3] = &CPU::RES_6_E;	    prefixs[0xB4] = &CPU::RES_6_H;	    prefixs[0xB5] = &CPU::RES_6_L;	prefixs[0xB6] = &CPU::RES_6_RHL;	prefixs[0xB7] = &CPU::RES_6_A;	prefixs[0xB8] = &CPU::RES_7_B;	    prefixs[0xB9] = &CPU::RES_7_C;	    prefixs[0xBA] = &CPU::RES_7_D;	    prefixs[0xBB] = &CPU::RES_7_E;	    prefixs[0xBC] = &CPU::RES_7_H;	    prefixs[0xBD] = &CPU::RES_7_L;	    prefixs[0xBE] = &CPU::RES_7_RHL;	prefixs[0xBF] = &CPU::RES_7_A;
    prefixs[0xC0] = &CPU::SET_0_B;	prefixs[0xC1] = &CPU::SET_0_C;	prefixs[0xC2] = &CPU::SET_0_D;	    prefixs[0xC3] = &CPU::SET_0_E;	    prefixs[0xC4] = &CPU::SET_0_H;	    prefixs[0xC5] = &CPU::SET_0_L;	prefixs[0xC6] = &CPU::SET_0_RHL;	prefixs[0xC7] = &CPU::SET_0_A;	prefixs[0xC8] = &CPU::SET_1_B;	    prefixs[0xC9] = &CPU::SET_1_C;	    prefixs[0xCA] = &CPU::SET_1_D;	    prefixs[0xCB] = &CPU::SET_1_E;	    prefixs[0xCC] = &CPU::SET_1_H;	    prefixs[0xCD] = &CPU::SET_1_L;	    prefixs[0xCE] = &CPU::SET_1_RHL;	prefixs[0xCF] = &CPU::SET_1_A;
    prefixs[0xD0] = &CPU::SET_2_B;	prefixs[0xD1] = &CPU::SET_2_C;	prefixs[0xD2] = &CPU::SET_2_D;	    prefixs[0xD3] = &CPU::SET_2_E;	    prefixs[0xD4] = &CPU::SET_2_H;	    prefixs[0xD5] = &CPU::SET_2_L;	prefixs[0xD6] = &CPU::SET_2_RHL;	prefixs[0xD7] = &CPU::SET_2_A;	prefixs[0xD8] = &CPU::SET_3_B;	    prefixs[0xD9] = &CPU::SET_3_C;	    prefixs[0xDA] = &CPU::SET_3_D;	    prefixs[0xDB] = &CPU::SET_3_E;	    prefixs[0xDC] = &CPU::SET_3_H;	    prefixs[0xDD] = &CPU::SET_3_L;	    prefixs[0xDE] = &CPU::SET_3_RHL;	prefixs[0xDF] = &CPU::SET_3_A;
    prefixs[0xE0] = &CPU::SET_4_B;	prefixs[0xE1] = &CPU::SET_4_C;	prefixs[0xE2] = &CPU::SET_4_D;	    prefixs[0xE3] = &CPU::SET_4_E;	    prefixs[0xE4] = &CPU::SET_4_H;	    prefixs[0xE5] = &CPU::SET_4_L;	prefixs[0xE6] = &CPU::SET_4_RHL;	prefixs[0xE7] = &CPU::SET_4_A;	prefixs[0xE8] = &CPU::SET_5_B;	    prefixs[0xE9] = &CPU::SET_5_C;	    prefixs[0xEA] = &CPU::SET_5_D;	    prefixs[0xEB] = &CPU::SET_5_E;	    prefixs[0xEC] = &CPU::SET_5_H;	    prefixs[0xED] = &CPU::SET_5_L;	    prefixs[0xEE] = &CPU::SET_5_RHL;	prefixs[0xEF] = &CPU::SET_5_A;
    prefixs[0xF0] = &CPU::SET_6_B;	prefixs[0xF1] = &CPU::SET_6_C;	prefixs[0xF2] = &CPU::SET_6_D;	    prefixs[0xF3] = &CPU::SET_6_E;	    prefixs[0xF4] = &CPU::SET_6_H;	    prefixs[0xF5] = &CPU::SET_6_L;	prefixs[0xF6] = &CPU::SET_6_RHL;	prefixs[0xF7] = &CPU::SET_6_A;	prefixs[0xF8] = &CPU::SET_7_B;	    prefixs[0xF9] = &CPU::SET_7_C;	    prefixs[0xFA] = &CPU::SET_7_D;	    prefixs[0xFB] = &CPU::SET_7_E;	    prefixs[0xFC] = &CPU::SET_7_H;	    prefixs[0xFD] = &CPU::SET_7_L;	    prefixs[0xFE] = &CPU::SET_7_RHL;	prefixs[0xFF] = &CPU::SET_7_A;
}

uint8_t CPU::decute(uint8_t opcode)
{
    uint8_t(CPU:: * func)() = opcodes[static_cast<int32_t>(opcode)];
    uint8_t machineCycles = (this->*func)();
    return machineCycles;
}

uint8_t CPU::step()
{
    if (m_regs.HALT)
    {
        uint8_t IF = m_memory.read(Tools::IF_ADDR);
        uint8_t IE = m_memory.read(Tools::IE_ADDR);
        if (!m_regs.IME && (IE & IF) != 0) {
            m_regs.HALT_BUG = true;
            m_regs.HALT = false;
        }
        else if ((IE & IF) == 0) {
            m_regs.HALT = true;
            return NOP();
        }
    }

    uint8_t opecode = fetch();
    return decute(opecode);
}

void CPU::waitHandle()
{
    if (!m_regs.IME) {
        return;
    }

    uint8_t IF = m_memory.read(Tools::IF_ADDR);
    uint8_t IE = m_memory.read(Tools::IE_ADDR);

    uint8_t pend = IF & IE;
    if (pend == 0) {
        return;
    }

    m_regs.IME = false;

    using namespace Tools;
    uint8_t interrupt = 0;
    uint16_t PC = 0;
    if (pend & V_BLANK_BIT) {
        interrupt = V_BLANK_BIT;
        PC = V_BLANK_INT;
    }
    else if (pend & LCD_STAT_BIT) {
        interrupt = LCD_STAT_BIT;
        PC = LCD_STAT_INT;
    }
    else if (pend & TIMER_BIT) {
        interrupt = TIMER_BIT;
        PC = TIMER_INT;
    }
    else if (pend & JOYPAD_BIT) {
        interrupt = JOYPAD_BIT;
        PC = JOYPAD_INT;
    }
    else {
        std::string error = "Missed or incorrect interrupt: " + std::to_string(pend);
        throw std::runtime_error(error);
    }

    m_memory.write(Tools::IF_ADDR, IF & ~interrupt);
    m_regs.SP -= 2;
    m_memory.write(m_regs.SP, lsb_8(m_regs.PC));
    m_memory.write(m_regs.SP + 1, msb_8(m_regs.PC));

    m_regs.PC = PC;
}

uint8_t CPU::NOP()
{
    return 1;
}

uint8_t CPU::LD_BC_nn()
{
    uint8_t low = fetch();
    uint8_t high = fetch();
    m_regs.B = high;
    m_regs.C = low;
    return 3;
}

uint8_t CPU::LD_RBC_A()
{
    uint16_t addr = Tools::unsigned_16(m_regs.B, m_regs.C);
    m_memory.write(addr, m_regs.A);
    return 2;
}

uint8_t CPU::INC_BC()
{
    uint16_t value = Tools::unsigned_16(m_regs.B, m_regs.C);
    value += 1;
    m_regs.B = Tools::msb_8(value);
    m_regs.C = Tools::lsb_8(value);
    return 2;
}

uint8_t CPU::INC_B()
{
    uint8_t value = m_regs.B;
    uint8_t result = value + 1;
    m_regs.B = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((value & Tools::MASK_4BITS) + 1) > Tools::MASK_4BITS);
    return 1;
}

uint8_t CPU::DEC_B()
{
    uint8_t value = m_regs.B;
    uint8_t result = value - 1;
    m_regs.B = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry((value & 0x0F) == 0);
    return 1;
}

uint8_t CPU::LD_B_n()
{
    uint8_t value = fetch();
    m_regs.B = value;
    return 2;
}

uint8_t CPU::RLCA()
{
    uint8_t bit = (m_regs.A & Tools::MASK_BIT_7) >> Tools::SHIFT_BIT_7;
    m_regs.A = (m_regs.A << 1) | bit;

    m_regs.setZero(false);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(bit);
    return 1;
}

uint8_t CPU::LD_nn_SP()
{
    uint8_t low = fetch();
    uint8_t high = fetch();
    uint16_t addr = Tools::unsigned_16(high, low);
    m_memory.write(addr, Tools::lsb_8(m_regs.SP));
    addr += 1;
    m_memory.write(addr, Tools::msb_8(m_regs.SP));
    return 5;
}

uint8_t CPU::ADD_HL_BC()
{
    uint16_t RR = Tools::unsigned_16(m_regs.B, m_regs.C);
    uint16_t HL = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint32_t result = static_cast<uint32_t>(HL) + static_cast<uint32_t>(RR);

    m_regs.H = Tools::msb_8(result);
    m_regs.L = Tools::lsb_8(result);

    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((HL & Tools::MASK_12BITS) + (RR & Tools::MASK_12BITS)) > Tools::MASK_12BITS);
    m_regs.setCarry(result > Tools::MASK_16BITS);

    return 2;
}

uint8_t CPU::LD_A_RBC()
{
    uint16_t addr = Tools::unsigned_16(m_regs.B, m_regs.C);
    m_regs.A = m_memory.read(addr);
    return 2;
}

uint8_t CPU::DEC_BC()
{
    uint16_t value = Tools::unsigned_16(m_regs.B, m_regs.C);
    value -= 1;
    m_regs.B = Tools::msb_8(value);
    m_regs.C = Tools::lsb_8(value);
    return 2;
}

uint8_t CPU::STOP()
{
    // TODO
    return 1;
}

uint8_t CPU::LD_DE_nn()
{
    uint8_t low = fetch();
    uint8_t high = fetch();
    m_regs.D = high;
    m_regs.E = low;
    return 3;
}

uint8_t CPU::LD_RDE_A()
{
    uint16_t addr = Tools::unsigned_16(m_regs.D, m_regs.E);
    m_memory.write(addr, m_regs.A);
    return 2;
}

uint8_t CPU::INC_DE()
{
    uint16_t value = Tools::unsigned_16(m_regs.D, m_regs.E);
    value += 1;
    m_regs.D = Tools::msb_8(value);
    m_regs.E = Tools::lsb_8(value);
    return 2;
}

uint8_t CPU::INC_D()
{
    uint8_t value = m_regs.D;
    uint8_t result = value + 1;
    m_regs.D = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((value & Tools::MASK_4BITS) + 1) > Tools::MASK_4BITS);
    return 1;
}

uint8_t CPU::DEC_D()
{
    uint8_t value = m_regs.D;
    uint8_t result = value - 1;
    m_regs.D = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry((value & 0x0F) == 0);
    return 1;
}

uint8_t CPU::LD_D_n()
{
    uint8_t value = fetch();
    m_regs.D = value;
    return 2;
}

uint8_t CPU::RLA()
{
    uint8_t bit = (m_regs.A & Tools::MASK_BIT_7) >> Tools::SHIFT_BIT_7;
    m_regs.A = (m_regs.A << 1) | static_cast<uint8_t>(m_regs.getCarry());

    m_regs.setZero(false);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(bit);
    return 1;
}

uint8_t CPU::JR_NZ_e()
{
    int8_t e = Tools::signed_8(fetch());
    if (m_regs.getZero() == false) {
        m_regs.PC += e;
        return 3;
    }
    return 2;
}

uint8_t CPU::LD_HL_nn()
{
    uint8_t low = fetch();
    uint8_t high = fetch();
    m_regs.H = high;
    m_regs.L = low;
    return 3;
}

uint8_t CPU::LD_RHL_INC_A()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    m_memory.write(addr, m_regs.A);
    addr += 1;
    m_regs.H = Tools::msb_8(addr);
    m_regs.L = Tools::lsb_8(addr);
    return 2;
}

uint8_t CPU::INC_HL()
{
    uint16_t value = Tools::unsigned_16(m_regs.H, m_regs.L);
    value += 1;
    m_regs.H = Tools::msb_8(value);
    m_regs.L = Tools::lsb_8(value);
    return 2;
}

uint8_t CPU::INC_H()
{
    uint8_t value = m_regs.H;
    uint8_t result = value + 1;
    m_regs.H = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((value & Tools::MASK_4BITS) + 1) > Tools::MASK_4BITS);
    return 1;
}

uint8_t CPU::DEC_H()
{
    uint8_t value = m_regs.H;
    uint8_t result = value - 1;
    m_regs.H = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry((value & 0x0F) == 0);
    return 1;
}

uint8_t CPU::LD_H_n()
{
    uint8_t value = fetch();
    m_regs.H = value;
    return 2;
}

uint8_t CPU::DAA()
{
    if (!m_regs.getSubtract()) {
        if (m_regs.A > 0x99 || m_regs.getCarry()) {
            m_regs.A += 0x60;
            m_regs.setCarry(true);
        }

        if ((m_regs.A & 0x0F) > 0x9 || m_regs.getHalfCarry()) {
            m_regs.A += 0x06;
        }
    }
    else
    {
        if (m_regs.getHalfCarry())
        {
            m_regs.A -= 0x06;
        }

        if (m_regs.getCarry())
        {
            m_regs.A -= 0x60;
        }
    }

    m_regs.setZero(m_regs.A == 0);
    m_regs.setHalfCarry(false);

    return 4;
}

uint8_t CPU::JR_NC_e()
{
    int8_t e = Tools::signed_8(fetch());
    if (!m_regs.getCarry()) {
        m_regs.PC += e;
        return 3;
    }
    return 2;
}

uint8_t CPU::LD_SP_nn()
{
    uint8_t low = fetch();
    uint8_t high = fetch();
    m_regs.SP = Tools::unsigned_16(high, low);
    return 3;
}

uint8_t CPU::LD_RHL_DEC_A()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    m_memory.write(addr, m_regs.A);
    addr -= 1;
    m_regs.H = Tools::msb_8(addr);
    m_regs.L = Tools::lsb_8(addr);
    return 2;
}

uint8_t CPU::INC_SP()
{
    m_regs.SP += 1;
    return 2;
}

uint8_t CPU::INC_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    uint8_t result = value + 1;
    m_memory.write(addr, result);
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((value & Tools::MASK_4BITS) + 1) > Tools::MASK_4BITS);
    return 3;
}

uint8_t CPU::DEC_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    uint8_t result = value - 1;
    m_memory.write(addr, result);
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry((value & 0x0F) == 0);
    return 3;
}

uint8_t CPU::LD_RHL()
{
    uint8_t value = fetch();
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    m_memory.write(addr, value);
    return 3;
}

uint8_t CPU::SCF()
{
    m_regs.setCarry(true);

    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);

    return 1;
}

uint8_t CPU::JR_e()
{
    int8_t e = Tools::signed_8(fetch());
    m_regs.PC += e;
    return 3;
}

uint8_t CPU::ADD_HL_DE()
{
    uint16_t RR = Tools::unsigned_16(m_regs.D, m_regs.E);
    uint16_t HL = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint32_t result = static_cast<uint32_t>(HL) + static_cast<uint32_t>(RR);

    m_regs.H = Tools::msb_8(result);
    m_regs.L = Tools::lsb_8(result);

    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((HL & Tools::MASK_12BITS) + (RR & Tools::MASK_12BITS)) > Tools::MASK_12BITS);
    m_regs.setCarry(result > Tools::MASK_16BITS);

    return 2;
}

uint8_t CPU::LD_A_RDE()
{
    uint16_t addr = Tools::unsigned_16(m_regs.D, m_regs.E);
    m_regs.A = m_memory.read(addr);
    return 2;
}

uint8_t CPU::DEC_DE()
{
    uint16_t value = Tools::unsigned_16(m_regs.D, m_regs.E);
    value -= 1;
    m_regs.D = Tools::msb_8(value);
    m_regs.E = Tools::lsb_8(value);
    return 2;
}

uint8_t CPU::JR_Z_e()
{
    int8_t e = Tools::signed_8(fetch());
    if (m_regs.getZero()) {
        m_regs.PC += e;
        return 3;
    }
    return 2;
}

uint8_t CPU::ADD_HL_HL()
{
    uint16_t RR = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint16_t HL = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint32_t result = static_cast<uint32_t>(HL) + static_cast<uint32_t>(RR);

    m_regs.H = Tools::msb_8(result);
    m_regs.L = Tools::lsb_8(result);

    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((HL & Tools::MASK_12BITS) + (RR & Tools::MASK_12BITS)) > Tools::MASK_12BITS);
    m_regs.setCarry(result > Tools::MASK_16BITS);

    return 2;
}

uint8_t CPU::LD_A_RHL_INC()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    m_regs.A = m_memory.read(addr);
    addr += 1;
    m_regs.H = Tools::msb_8(addr);
    m_regs.L = Tools::lsb_8(addr);
    return 2;
}

uint8_t CPU::DEC_HL()
{
    uint16_t value = Tools::unsigned_16(m_regs.H, m_regs.L);
    value -= 1;
    m_regs.H = Tools::msb_8(value);
    m_regs.L = Tools::lsb_8(value);
    return 2;
}

uint8_t CPU::JR_C_e()
{
    int8_t e = Tools::signed_8(fetch());
    if (m_regs.getCarry()) {
        m_regs.PC += e;
        return 3;
    }
    return 2;
}

uint8_t CPU::ADD_HL_SP()
{
    uint16_t RR = m_regs.SP;
    uint16_t HL = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint32_t result = static_cast<uint32_t>(HL) + static_cast<uint32_t>(RR);

    m_regs.H = Tools::msb_8(result);
    m_regs.L = Tools::lsb_8(result);

    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((HL & Tools::MASK_12BITS) + (RR & Tools::MASK_12BITS)) > Tools::MASK_12BITS);
    m_regs.setCarry(result > Tools::MASK_16BITS);

    return 2;
}

uint8_t CPU::LD_A_RHL_DEC()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    m_regs.A = m_memory.read(addr);
    addr -= 1;
    m_regs.H = Tools::msb_8(addr);
    m_regs.L = Tools::lsb_8(addr);
    return 2;
}

uint8_t CPU::DEC_SP()
{
    uint16_t value = m_regs.SP;
    value -= 1;
    m_regs.SP = value;
    return 2;
}

uint8_t CPU::INC_C()
{
    uint8_t value = m_regs.C;
    uint8_t result = value + 1;
    m_regs.C = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((value & Tools::MASK_4BITS) + 1) > Tools::MASK_4BITS);
    return 1;
}

uint8_t CPU::DEC_C()
{
    uint8_t value = m_regs.C;
    uint8_t result = value - 1;
    m_regs.C = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry((value & 0x0F) == 0);
    return 1;
}

uint8_t CPU::LD_C_n()
{
    uint8_t value = fetch();
    m_regs.C = value;
    return 2;
}

uint8_t CPU::RRCA()
{
    uint8_t bit = m_regs.A & 0x01;
    m_regs.A = (m_regs.A >> Tools::MASK_BIT_0) | (bit << Tools::SHIFT_BIT_7);

    m_regs.setZero(false);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(bit);
    return 1;
}

uint8_t CPU::INC_E()
{
    uint8_t value = m_regs.E;
    uint8_t result = value + 1;
    m_regs.E = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((value & Tools::MASK_4BITS) + 1) > Tools::MASK_4BITS);
    return 1;
}

uint8_t CPU::DEC_E()
{
    uint8_t value = m_regs.E;
    uint8_t result = value - 1;
    m_regs.E = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry((value & 0x0F) == 0);
    return 1;
}

uint8_t CPU::LD_E_n()
{
    uint8_t value = fetch();
    m_regs.E = value;
    return 2;
}

uint8_t CPU::RRA()
{
    uint8_t bit = m_regs.A & 0x01;
    m_regs.A = (m_regs.A >> Tools::MASK_BIT_0) | (m_regs.getCarry() << Tools::SHIFT_BIT_7);

    m_regs.setZero(false);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(bit);
    return 1;
}

uint8_t CPU::INC_L()
{
    uint8_t value = m_regs.L;
    uint8_t result = value + 1;
    m_regs.L = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((value & Tools::MASK_4BITS) + 1) > Tools::MASK_4BITS);
    return 1;
}

uint8_t CPU::DEC_L()
{
    uint8_t value = m_regs.L;
    uint8_t result = value - 1;
    m_regs.L = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry((value & 0x0F) == 0);
    return 1;
}

uint8_t CPU::LD_L_n()
{
    uint8_t value = fetch();
    m_regs.L = value;
    return 2;
}

uint8_t CPU::CPL()
{
    m_regs.A = ~m_regs.A;
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(true);
    return 1;
}

uint8_t CPU::INC_A()
{
    uint8_t value = m_regs.A;
    uint8_t result = value + 1;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((value & Tools::MASK_4BITS) + 1) > Tools::MASK_4BITS);
    return 1;
}

uint8_t CPU::DEC_A()
{
    uint8_t value = m_regs.A;
    uint8_t result = value - 1;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry((value & 0x0F) == 0);
    return 1;
}

uint8_t CPU::LD_A_n()
{
    uint8_t value = fetch();
    m_regs.A = value;
    return 2;
}

uint8_t CPU::CCF()
{
    bool carry = m_regs.getCarry();
    m_regs.setHalfCarry(false);
    m_regs.setSubtract(false);
    m_regs.setCarry(!carry);
    return 1;
}

uint8_t CPU::LD_B_B()
{
    m_regs.B = m_regs.B;
    return 1;
}

uint8_t CPU::LD_B_C()
{
    m_regs.B = m_regs.C;
    return 1;
}

uint8_t CPU::LD_B_D()
{
    m_regs.B = m_regs.D;
    return 1;
}

uint8_t CPU::LD_B_E()
{
    m_regs.B = m_regs.E;
    return 1;
}

uint8_t CPU::LD_B_H()
{
    m_regs.B = m_regs.H;
    return 1;
}

uint8_t CPU::LD_B_L()
{
    m_regs.B = m_regs.L;
    return 1;
}

uint8_t CPU::LD_B_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    m_regs.B = m_memory.read(addr);
    return 2;
}

uint8_t CPU::LD_B_A()
{
    m_regs.B = m_regs.A;
    return 1;
}

uint8_t CPU::LD_C_B()
{
    m_regs.C = m_regs.B;
    return 1;
}

uint8_t CPU::LD_C_C()
{
    m_regs.C = m_regs.C;
    return 1;
}

uint8_t CPU::LD_C_D()
{
    m_regs.C = m_regs.D;
    return 1;
}

uint8_t CPU::LD_C_E()
{
    m_regs.C = m_regs.E;
    return 1;
}

uint8_t CPU::LD_C_H()
{
    m_regs.C = m_regs.H;
    return 1;
}

uint8_t CPU::LD_C_L()
{
    m_regs.C = m_regs.L;
    return 1;
}

uint8_t CPU::LD_C_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    m_regs.C = m_memory.read(addr);
    return 2;
}

uint8_t CPU::LD_C_A()
{
    m_regs.C = m_regs.A;
    return 1;
}

uint8_t CPU::LD_D_B()
{
    m_regs.D = m_regs.B;
    return 1;
}

uint8_t CPU::LD_D_C()
{
    m_regs.D = m_regs.C;
    return 1;
}

uint8_t CPU::LD_D_D()
{
    m_regs.D = m_regs.D;
    return 1;
}

uint8_t CPU::LD_D_E()
{
    m_regs.D = m_regs.E;
    return 1;
}

uint8_t CPU::LD_D_H()
{
    m_regs.D = m_regs.H;
    return 1;
}

uint8_t CPU::LD_D_L()
{
    m_regs.D = m_regs.L;
    return 1;
}

uint8_t CPU::LD_D_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    m_regs.D = m_memory.read(addr);
    return 2;
}

uint8_t CPU::LD_D_A()
{
    m_regs.D = m_regs.A;
    return 1;
}

uint8_t CPU::LD_E_B()
{
    m_regs.E = m_regs.B;
    return 1;
}

uint8_t CPU::LD_E_C()
{
    m_regs.E = m_regs.C;
    return 1;
}

uint8_t CPU::LD_E_D()
{
    m_regs.E = m_regs.D;
    return 1;
}

uint8_t CPU::LD_E_E()
{
    m_regs.E = m_regs.E;
    return 1;
}

uint8_t CPU::LD_E_H()
{
    m_regs.E = m_regs.H;
    return 1;
}

uint8_t CPU::LD_E_L()
{
    m_regs.E = m_regs.L;
    return 1;
}

uint8_t CPU::LD_E_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    m_regs.E = m_memory.read(addr);
    return 2;
}

uint8_t CPU::LD_E_A()
{
    m_regs.E = m_regs.A;
    return 1;
}

uint8_t CPU::LD_H_B()
{
    m_regs.H = m_regs.B;
    return 1;
}

uint8_t CPU::LD_H_C()
{
    m_regs.H = m_regs.C;
    return 1;
}

uint8_t CPU::LD_H_D()
{
    m_regs.H = m_regs.D;
    return 1;
}

uint8_t CPU::LD_H_E()
{
    m_regs.H = m_regs.E;
    return 1;
}

uint8_t CPU::LD_H_H()
{
    m_regs.H = m_regs.H;
    return 1;
}

uint8_t CPU::LD_H_L()
{
    m_regs.H = m_regs.L;
    return 1;
}

uint8_t CPU::LD_H_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    m_regs.H = m_memory.read(addr);
    return 2;
}

uint8_t CPU::LD_H_A()
{
    m_regs.H = m_regs.A;
    return 1;
}

uint8_t CPU::LD_L_B()
{
    m_regs.L = m_regs.B;
    return 1;
}

uint8_t CPU::LD_L_C()
{
    m_regs.L = m_regs.C;
    return 1;
}

uint8_t CPU::LD_L_D()
{
    m_regs.L = m_regs.D;
    return 1;
}

uint8_t CPU::LD_L_E()
{
    m_regs.L = m_regs.E;
    return 1;
}

uint8_t CPU::LD_L_H()
{
    m_regs.L = m_regs.H;
    return 1;
}

uint8_t CPU::LD_L_L()
{
    m_regs.L = m_regs.L;
    return 1;
}

uint8_t CPU::LD_L_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    m_regs.L = m_memory.read(addr);
    return 2;
}

uint8_t CPU::LD_L_A()
{
    m_regs.L = m_regs.A;
    return 1;
}

uint8_t CPU::LD_RHL_B()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    m_memory.write(addr, m_regs.B);
    return 2;
}

uint8_t CPU::LD_RHL_C()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    m_memory.write(addr, m_regs.C);
    return 2;
}

uint8_t CPU::LD_RHL_D()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    m_memory.write(addr, m_regs.D);
    return 2;
}

uint8_t CPU::LD_RHL_E()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    m_memory.write(addr, m_regs.E);
    return 2;
}

uint8_t CPU::LD_RHL_H()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    m_memory.write(addr, m_regs.H);
    return 2;
}

uint8_t CPU::LD_RHL_L()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    m_memory.write(addr, m_regs.L);
    return 2;
}

uint8_t CPU::HALT()
{ 
    uint8_t IF = m_memory.read(Tools::IF_ADDR);
    uint8_t IE = m_memory.read(Tools::IE_ADDR);
    m_regs.HALT = true;
    return 1;
}

uint8_t CPU::LD_RHL_A()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    m_memory.write(addr, m_regs.A);
    return 2;
}

uint8_t CPU::LD_A_B()
{
    m_regs.A = m_regs.B;
    return 1;
}

uint8_t CPU::LD_A_C()
{
    m_regs.A = m_regs.C;
    return 1;
}

uint8_t CPU::LD_A_D()
{
    m_regs.A = m_regs.D;
    return 1;
}

uint8_t CPU::LD_A_E()
{
    m_regs.A = m_regs.E;
    return 1;
}

uint8_t CPU::LD_A_H()
{
    m_regs.A = m_regs.H;
    return 1;
}

uint8_t CPU::LD_A_L()
{
    m_regs.A = m_regs.L;
    return 1;
}

uint8_t CPU::LD_A_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    m_regs.A = m_memory.read(addr);
    return 2;
}

uint8_t CPU::LD_A_A()
{
    m_regs.A = m_regs.A;
    return 1;
}

uint8_t CPU::ADD_B()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.B;
    uint8_t result = A + R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) + (R & Tools::MASK_4BITS)) > Tools::MASK_4BITS);
    m_regs.setCarry(((A & Tools::MASK_8BITS) + (R & Tools::MASK_8BITS)) > Tools::MASK_8BITS);
    
    return 1;
}
uint8_t CPU::ADD_C()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.C;
    uint8_t result = A + R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) + (R & Tools::MASK_4BITS)) > Tools::MASK_4BITS);
    m_regs.setCarry(((A & Tools::MASK_8BITS) + (R & Tools::MASK_8BITS)) > Tools::MASK_8BITS);

    return 1;
}

uint8_t CPU::ADD_D()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.D;
    uint8_t result = A + R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) + (R & Tools::MASK_4BITS)) > Tools::MASK_4BITS);
    m_regs.setCarry(((A & Tools::MASK_8BITS) + (R & Tools::MASK_8BITS)) > Tools::MASK_8BITS);

    return 1;
}

uint8_t CPU::ADD_E()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.E;
    uint8_t result = A + R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) + (R & Tools::MASK_4BITS)) > Tools::MASK_4BITS);
    m_regs.setCarry(((A & Tools::MASK_8BITS) + (R & Tools::MASK_8BITS)) > Tools::MASK_8BITS);

    return 1;
}

uint8_t CPU::ADD_H()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.H;
    uint8_t result = A + R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) + (R & Tools::MASK_4BITS)) > Tools::MASK_4BITS);
    m_regs.setCarry(((A & Tools::MASK_8BITS) + (R & Tools::MASK_8BITS)) > Tools::MASK_8BITS);

    return 1;
}

uint8_t CPU::ADD_L()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.L;
    uint8_t result = A + R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) + (R & Tools::MASK_4BITS)) > Tools::MASK_4BITS);
    m_regs.setCarry(((A & Tools::MASK_8BITS) + (R & Tools::MASK_8BITS)) > Tools::MASK_8BITS);

    return 1;
}

uint8_t CPU::ADD_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t A = m_regs.A;
    uint8_t data = m_memory.read(addr);
    uint8_t result = A + data;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) + (data & Tools::MASK_4BITS)) > Tools::MASK_4BITS);
    m_regs.setCarry(((A & Tools::MASK_8BITS) + (data & Tools::MASK_8BITS)) > Tools::MASK_8BITS);
    return 2;
}

uint8_t CPU::ADD_A()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.A;
    uint8_t result = A + R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) + (R & Tools::MASK_4BITS)) > Tools::MASK_4BITS);
    m_regs.setCarry(((A & Tools::MASK_8BITS) + (R & Tools::MASK_8BITS)) > Tools::MASK_8BITS);

    return 1;
}

uint8_t CPU::ADC_B()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.B;
    uint8_t C = static_cast<uint8_t>(m_regs.getCarry());
    uint8_t result = A + R + C;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) + (R & Tools::MASK_4BITS) + C) > Tools::MASK_4BITS);
    m_regs.setCarry(((A & Tools::MASK_8BITS) + (R & Tools::MASK_8BITS) + C) > Tools::MASK_8BITS);

    return 1;
}

uint8_t CPU::ADC_C()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.C;
    uint8_t C = static_cast<uint8_t>(m_regs.getCarry());
    uint8_t result = A + R + C;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) + (R & Tools::MASK_4BITS) + C) > Tools::MASK_4BITS);
    m_regs.setCarry(((A & Tools::MASK_8BITS) + (R & Tools::MASK_8BITS) + C) > Tools::MASK_8BITS);

    return 1;
}

uint8_t CPU::ADC_D()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.D;
    uint8_t C = static_cast<uint8_t>(m_regs.getCarry());
    uint8_t result = A + R + C;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) + (R & Tools::MASK_4BITS) + C) > Tools::MASK_4BITS);
    m_regs.setCarry(((A & Tools::MASK_8BITS) + (R & Tools::MASK_8BITS) + C) > Tools::MASK_8BITS);

    return 1;
}

uint8_t CPU::ADC_E()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.E;
    uint8_t C = static_cast<uint8_t>(m_regs.getCarry());
    uint8_t result = A + R + C;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) + (R & Tools::MASK_4BITS) + C) > Tools::MASK_4BITS);
    m_regs.setCarry(((A & Tools::MASK_8BITS) + (R & Tools::MASK_8BITS) + C) > Tools::MASK_8BITS);

    return 1;
}

uint8_t CPU::ADC_H()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.H;
    uint8_t C = static_cast<uint8_t>(m_regs.getCarry());
    uint8_t result = A + R + C;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) + (R & Tools::MASK_4BITS) + C) > Tools::MASK_4BITS);
    m_regs.setCarry(((A & Tools::MASK_8BITS) + (R & Tools::MASK_8BITS) + C) > Tools::MASK_8BITS);

    return 1;
}

uint8_t CPU::ADC_L()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.L;
    uint8_t C = static_cast<uint8_t>(m_regs.getCarry());
    uint8_t result = A + R + C;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) + (R & Tools::MASK_4BITS) + C) > Tools::MASK_4BITS);
    m_regs.setCarry(((A & Tools::MASK_8BITS) + (R & Tools::MASK_8BITS) + C) > Tools::MASK_8BITS);

    return 1;
}

uint8_t CPU::ADC_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t A = m_regs.A;
    uint8_t data = m_memory.read(addr);
    uint8_t C = static_cast<uint8_t>(m_regs.getCarry());
    uint8_t result = A + data + C;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) + (data & Tools::MASK_4BITS) + C) > Tools::MASK_4BITS);
    m_regs.setCarry(((A & Tools::MASK_8BITS) + (data & Tools::MASK_8BITS) + C) > Tools::MASK_8BITS);
    
    return 2;
}

uint8_t CPU::ADC_A()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.A;
    uint8_t C = static_cast<uint8_t>(m_regs.getCarry());
    uint8_t result = A + R + C;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) + (R & Tools::MASK_4BITS) + C) > Tools::MASK_4BITS);
    m_regs.setCarry(((A & Tools::MASK_8BITS) + (R & Tools::MASK_8BITS) + C) > Tools::MASK_8BITS);

    return 1;
}

uint8_t CPU::SUB_B()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.B;
    uint8_t result = A - R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) < (R & Tools::MASK_4BITS))); 
    m_regs.setCarry(A < R);

    return 1;
}

uint8_t CPU::SUB_C()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.C;
    uint8_t result = A - R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) < (R & Tools::MASK_4BITS)));
    m_regs.setCarry(A < R);

    return 1;
}

uint8_t CPU::SUB_D()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.D;
    uint8_t result = A - R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) < (R & Tools::MASK_4BITS)));
    m_regs.setCarry(A < R);

    return 1;
}

uint8_t CPU::SUB_E()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.E;
    uint8_t result = A - R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) < (R & Tools::MASK_4BITS)));
    m_regs.setCarry(A < R);

    return 1;
}

uint8_t CPU::SUB_H()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.H;
    uint8_t result = A - R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) < (R & Tools::MASK_4BITS)));
    m_regs.setCarry(A < R);

    return 1;
}

uint8_t CPU::SUB_L()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.L;
    uint8_t result = A - R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) < (R & Tools::MASK_4BITS)));
    m_regs.setCarry(A < R);

    return 1;
}

uint8_t CPU::SUB_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t A = m_regs.A;
    uint8_t data = m_memory.read(addr);
    uint8_t result = A - data;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) < (data & Tools::MASK_4BITS)));
    m_regs.setCarry(A < data);

    return 2;
}

uint8_t CPU::SUB_A()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.A;
    uint8_t result = A - R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) < (R & Tools::MASK_4BITS)));
    m_regs.setCarry(A < R);

    return 1;
}

uint8_t CPU::SBC_B()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.B;
    uint8_t C = m_regs.getCarry();
    uint8_t result = A - R - C;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) - (R & Tools::MASK_4BITS) - C) < 0);
    m_regs.setCarry(A < (R + C));

    return 1;
}

uint8_t CPU::SBC_C()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.C;
    uint8_t C = m_regs.getCarry();
    uint8_t result = A - R - C;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) - (R & Tools::MASK_4BITS) - C) < 0);
    m_regs.setCarry(A < (R + C));

    return 1;
}

uint8_t CPU::SBC_D()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.D;
    uint8_t C = m_regs.getCarry();
    uint8_t result = A - R - C;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) - (R & Tools::MASK_4BITS) - C) < 0);
    m_regs.setCarry(A < (R + C));

    return 1;
}

uint8_t CPU::SBC_E()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.E;
    uint8_t C = m_regs.getCarry();
    uint8_t result = A - R - C;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) - (R & Tools::MASK_4BITS) - C) < 0);
    m_regs.setCarry(A < (R + C));

    return 1;
}

uint8_t CPU::SBC_H()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.H;
    uint8_t C = m_regs.getCarry();
    uint8_t result = A - R - C;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) - (R & Tools::MASK_4BITS) - C) < 0);
    m_regs.setCarry(A < (R + C));

    return 1;
}

uint8_t CPU::SBC_L()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.L;
    uint8_t C = m_regs.getCarry();
    uint8_t result = A - R - C;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) - (R & Tools::MASK_4BITS) - C) < 0);
    m_regs.setCarry(A < (R + C));

    return 1;
}

uint8_t CPU::SBC_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t A = m_regs.A;
    uint8_t data = m_memory.read(addr);
    uint8_t C = m_regs.getCarry();
    uint8_t result = A - data - C;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) - (data & Tools::MASK_4BITS) - C) < 0);
    m_regs.setCarry(A < (data + C));

    return 1;
}

uint8_t CPU::SBC_A()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.A;
    uint8_t C = m_regs.getCarry();
    uint8_t result = A - R - C;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) - (R & Tools::MASK_4BITS) - C) < 0);
    m_regs.setCarry(A < (R + C));

    return 1;
}

uint8_t CPU::AND_B()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.B;
    uint8_t result = A & R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(true);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::AND_C()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.C;
    uint8_t result = A & R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(true);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::AND_D()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.D;
    uint8_t result = A & R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(true);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::AND_E()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.E;
    uint8_t result = A & R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(true);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::AND_H()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.H;
    uint8_t result = A & R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(true);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::AND_L()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.L;
    uint8_t result = A & R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(true);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::AND_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t A = m_regs.A;
    uint8_t data = m_memory.read(addr);
    uint8_t result = A & data;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(true);
    m_regs.setCarry(false);

    return 2;
}

uint8_t CPU::AND_A()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.A;
    uint8_t result = A & R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(true);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::XOR_B()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.B;
    uint8_t result = A ^ R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::XOR_C()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.C;
    uint8_t result = A ^ R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::XOR_D()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.D;
    uint8_t result = A ^ R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::XOR_E()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.E;
    uint8_t result = A ^ R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::XOR_H()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.H;
    uint8_t result = A ^ R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::XOR_L()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.L;
    uint8_t result = A ^ R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::XOR_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t A = m_regs.A;
    uint8_t data = m_memory.read(addr);
    uint8_t result = A ^ data;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(false);

    return 2;
}

uint8_t CPU::XOR_A()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.A;
    uint8_t result = A ^ R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::OR_B()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.B;
    uint8_t result = A | R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::OR_C()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.C;
    uint8_t result = A | R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::OR_D()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.D;
    uint8_t result = A | R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::OR_E()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.E;
    uint8_t result = A | R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::OR_H()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.H;
    uint8_t result = A | R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::OR_L()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.L;
    uint8_t result = A | R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::OR_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t A = m_regs.A;
    uint8_t data = m_memory.read(addr);
    uint8_t result = A | data;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(false);

    return 2;
}

uint8_t CPU::OR_A()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.A;
    uint8_t result = A | R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(false);

    return 1;
}

uint8_t CPU::CP_B()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.B;
    uint8_t result = A - R;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) < (R & Tools::MASK_4BITS)));
    m_regs.setCarry(A < R);

    return 1;
}

uint8_t CPU::CP_C()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.C;
    uint8_t result = A - R;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) < (R & Tools::MASK_4BITS)));
    m_regs.setCarry(A < R);

    return 1;
}

uint8_t CPU::CP_D()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.D;
    uint8_t result = A - R;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) < (R & Tools::MASK_4BITS)));
    m_regs.setCarry(A < R);

    return 1;
}

uint8_t CPU::CP_E()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.E;
    uint8_t result = A - R;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) < (R & Tools::MASK_4BITS)));
    m_regs.setCarry(A < R);

    return 1;
}

uint8_t CPU::CP_H()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.H;
    uint8_t result = A - R;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) < (R & Tools::MASK_4BITS)));
    m_regs.setCarry(A < R);

    return 1;
}

uint8_t CPU::CP_L()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.L;
    uint8_t result = A - R;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) < (R & Tools::MASK_4BITS)));
    m_regs.setCarry(A < R);

    return 1;
}

uint8_t CPU::CP_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t A = m_regs.A;
    uint8_t data = m_memory.read(addr);
    uint8_t result = A - data;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) < (data & Tools::MASK_4BITS)));
    m_regs.setCarry(A < data);

    return 1;
}

uint8_t CPU::CP_A()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.A;
    uint8_t result = A - R;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) < (R & Tools::MASK_4BITS)));
    m_regs.setCarry(A < R);

    return 1;
}

uint8_t CPU::RET_NZ()
{
    if (!m_regs.getZero()) {
        uint8_t low = m_memory.read(m_regs.SP);
        m_regs.SP += 1;
        uint8_t high = m_memory.read(m_regs.SP);
        m_regs.SP += 1;
        m_regs.PC = Tools::unsigned_16(high, low);
        return 5;
    }
    return 2;
}

uint8_t CPU::POP_BC()
{
    uint8_t low = m_memory.read(m_regs.SP);
    m_regs.SP += 1;
    uint8_t high = m_memory.read(m_regs.SP);
    m_regs.SP += 1;
    m_regs.B = high;
    m_regs.C = low;
    return 3;
}

uint8_t CPU::JP_NZ_nn()
{
    uint8_t low = fetch();
    uint8_t high = fetch();
    uint16_t value = Tools::unsigned_16(high, low);
    
    if (!m_regs.getZero()) {
        m_regs.PC = value;
        return 4;
    }

    return 3;
}

uint8_t CPU::JP_nn()
{
    uint8_t low = fetch();
    uint8_t high = fetch();
    m_regs.PC = Tools::unsigned_16(high, low);

    return 4;
}

uint8_t CPU::CALL_NZ_nn()
{
    uint8_t low = fetch();
    uint8_t high = fetch();
    uint16_t value = Tools::unsigned_16(high, low);
    if (!m_regs.getZero()) {
        m_regs.SP -= 1;
        m_memory.write(m_regs.SP, Tools::msb_8(m_regs.PC));
        m_regs.SP -= 1;
        m_memory.write(m_regs.SP, Tools::lsb_8(m_regs.PC));
        m_regs.PC = value;
        return 6;
    }
    return 3;
}

uint8_t CPU::PUSH_BC()
{
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, m_regs.B);
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, m_regs.C);
    return 4;
}

uint8_t CPU::ADD_A_n()
{
    uint8_t A = m_regs.A;
    uint8_t value = fetch();
    uint8_t result = A + value;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) + (value & Tools::MASK_4BITS)) > Tools::MASK_4BITS);
    m_regs.setCarry(((A & Tools::MASK_8BITS) + (value & Tools::MASK_8BITS)) > Tools::MASK_8BITS);

    return 2;
}

uint8_t CPU::RST_00H()
{
    uint8_t value = 0x00;
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, Tools::msb_8(m_regs.PC));
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, Tools::lsb_8(m_regs.PC));
    m_regs.PC = Tools::unsigned_16(0x00, value);
    return 4;
}

uint8_t CPU::RET_Z()
{
    if (m_regs.getZero()) {
        uint8_t low = m_memory.read(m_regs.SP);
        m_regs.SP += 1;
        uint8_t high = m_memory.read(m_regs.SP);
        m_regs.SP += 1;
        m_regs.PC = Tools::unsigned_16(high, low);
        return 5;
    }
    return 2;
}

uint8_t CPU::RET()
{
    uint8_t low = m_memory.read(m_regs.SP);
    m_regs.SP += 1;
    uint8_t high = m_memory.read(m_regs.SP);
    m_regs.SP += 1;
    m_regs.PC = Tools::unsigned_16(high, low);
    return 4;
}

uint8_t CPU::JP_Z_nn()
{
    uint8_t low = fetch();
    uint8_t high = fetch();
    uint16_t value = Tools::unsigned_16(high, low);

    if (m_regs.getZero()) {
        m_regs.PC = value;
        return 4;
    }

    return 3;
}

uint8_t CPU::PREFIX_CB()
{
    uint8_t opcode = fetch();
    uint8_t(CPU::*func)() = prefixs[static_cast<int32_t>(opcode)];
    return (this->*func)();
}

uint8_t CPU::CALL_Z_nn()
{
    uint8_t low = fetch();
    uint8_t high = fetch();
    uint16_t value = Tools::unsigned_16(high, low);
    if (m_regs.getZero()) {
        m_regs.SP -= 1;
        m_memory.write(m_regs.SP, Tools::msb_8(m_regs.PC));
        m_regs.SP -= 1;
        m_memory.write(m_regs.SP, Tools::lsb_8(m_regs.PC));
        m_regs.PC = value;
        return 6;
    }
    return 3;
}

uint8_t CPU::CALL_nn()
{
    uint8_t low = fetch();
    uint8_t high = fetch();
    uint16_t value = Tools::unsigned_16(high, low);
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, Tools::msb_8(m_regs.PC));
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, Tools::lsb_8(m_regs.PC));
    m_regs.PC = value;
    return 6;
}

uint8_t CPU::ADC_A_n()
{
    uint8_t A = m_regs.A;
    uint8_t R = fetch();
    uint8_t C = static_cast<uint8_t>(m_regs.getCarry());
    uint8_t result = A + R + C;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) + (R & Tools::MASK_4BITS) + C) > Tools::MASK_4BITS);
    m_regs.setCarry(((A & Tools::MASK_8BITS) + (R & Tools::MASK_8BITS) + C) > Tools::MASK_8BITS);

    return 2;
}

uint8_t CPU::RST_08H()
{
    uint8_t value = 0x08;
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, Tools::msb_8(m_regs.PC));
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, Tools::lsb_8(m_regs.PC));
    m_regs.PC = Tools::unsigned_16(0x00, value);
    return 4;
}

uint8_t CPU::RET_NC()
{
    if (!m_regs.getCarry()) {
        uint8_t low = m_memory.read(m_regs.SP);
        m_regs.SP += 1;
        uint8_t high = m_memory.read(m_regs.SP);
        m_regs.SP += 1;
        m_regs.PC = Tools::unsigned_16(high, low);
        return 5;
    }

    return 2;
}

uint8_t CPU::POP_DE()
{
    uint8_t low = m_memory.read(m_regs.SP);
    m_regs.SP += 1;
    uint8_t high = m_memory.read(m_regs.SP);
    m_regs.SP += 1;
    m_regs.D = high;
    m_regs.E = low;
    return 3;
}

uint8_t CPU::JP_NC_nn()
{
    uint8_t low = fetch();
    uint8_t high = fetch();
    uint16_t value = Tools::unsigned_16(high, low);

    if (!m_regs.getCarry()) {
        m_regs.PC = value;
        return 4;
    }

    return 3;
}

uint8_t CPU::UNDEFINED_D3()
{
    throw std::exception("UNDEFINED D3");
}

uint8_t CPU::CALL_NC_nn()
{
    uint8_t low = fetch();
    uint8_t high = fetch();
    uint16_t value = Tools::unsigned_16(high, low);
    if (!m_regs.getCarry()) {
        m_regs.SP -= 1;
        m_memory.write(m_regs.SP, Tools::msb_8(m_regs.PC));
        m_regs.SP -= 1;
        m_memory.write(m_regs.SP, Tools::lsb_8(m_regs.PC));
        m_regs.PC = value;
        return 6;
    }
    return 3;
}

uint8_t CPU::PUSH_DE()
{
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, m_regs.D);
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, m_regs.E);
    return 4;
}

uint8_t CPU::SUB_n()
{
    uint8_t A = m_regs.A;
    uint8_t value = fetch();
    uint8_t result = A - value;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) < (value & Tools::MASK_4BITS)));
    m_regs.setCarry(A < value);

    return 2;
}

uint8_t CPU::RST_10H()
{
    uint8_t value = 0x10;
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, Tools::msb_8(m_regs.PC));
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, Tools::lsb_8(m_regs.PC));
    m_regs.PC = Tools::unsigned_16(0x00, value);
    return 4;
}

uint8_t CPU::RET_C()
{
    if (m_regs.getCarry()) {
        uint8_t low = m_memory.read(m_regs.SP);
        m_regs.SP += 1;
        uint8_t high = m_memory.read(m_regs.SP);
        m_regs.SP += 1;
        m_regs.PC = Tools::unsigned_16(high, low);
        return 5;
    }
    return 2;
}

uint8_t CPU::RETI()
{
    uint8_t low = m_memory.read(m_regs.SP);
    m_regs.SP += 1;
    uint8_t high = m_memory.read(m_regs.SP);
    m_regs.SP += 1;

    m_regs.PC = Tools::unsigned_16(high, low);
    m_regs.IME = true;

    return 4;
}

uint8_t CPU::JP_C_nn()
{
    uint8_t low = fetch();
    uint8_t high = fetch();
    uint16_t value = Tools::unsigned_16(high, low);

    if (m_regs.getCarry()) {
        m_regs.PC = value;
        return 4;
    }

    return 3;
}

uint8_t CPU::UNDEFINED_DB()
{
    throw std::exception("UNDEFINED DB");
}

uint8_t CPU::CALL_C_nn()
{
    uint8_t low = fetch();
    uint8_t high = fetch();
    uint16_t value = Tools::unsigned_16(high, low);
    if (m_regs.getCarry()) {
        m_regs.SP -= 1;
        m_memory.write(m_regs.SP, Tools::msb_8(m_regs.PC));
        m_regs.SP -= 1;
        m_memory.write(m_regs.SP, Tools::lsb_8(m_regs.PC));
        m_regs.PC = value;
        return 6;
    }
    return 3;
}

uint8_t CPU::UNDEFINED_DD()
{
    throw std::exception("UNDEFINED DD");
}

uint8_t CPU::SBC_A_n()
{
    uint8_t A = m_regs.A;
    uint8_t R = m_regs.A;
    uint8_t C = m_regs.getCarry();
    uint8_t result = A - R - C;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) - (R & Tools::MASK_4BITS) - C) < 0);
    m_regs.setCarry(A < (R + C));

    return 2;
}

uint8_t CPU::RST_18H()
{
    uint8_t value = 0x18;
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, Tools::msb_8(m_regs.PC));
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, Tools::lsb_8(m_regs.PC));
    m_regs.PC = Tools::unsigned_16(0x00, value);
    return 4;
}

uint8_t CPU::LDH_Rn_A()
{
    uint8_t value = fetch();
    uint16_t addr = Tools::unsigned_16(Tools::MASK_8BITS, value);
    m_memory.write(addr, m_regs.A);
    return 3;
}

uint8_t CPU::POP_HL()
{
    uint8_t low = m_memory.read(m_regs.SP);
    m_regs.SP += 1;
    uint8_t high = m_memory.read(m_regs.SP);
    m_regs.SP += 1;
    m_regs.H = high;
    m_regs.L = low;
    return 3;
}

uint8_t CPU::LDH_RC_A()
{
    uint16_t addr = Tools::unsigned_16(Tools::MASK_8BITS, m_regs.C);
    m_memory.write(addr, m_regs.A);
    return 2;
}

uint8_t CPU::UNDEFINED_E3()
{
    throw std::exception("UNDEFINED E3");
}

uint8_t CPU::UNDEFINED_E4()
{
    throw std::exception("UNDEFINED E4");
}

uint8_t CPU::PUSH_HL()
{
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, m_regs.H);
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, m_regs.L);
    return 4;
}

uint8_t CPU::AND_n()
{
    uint8_t A = m_regs.A;
    uint8_t value = fetch();
    uint8_t result = A & value;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(true);
    m_regs.setCarry(false);

    return 2;
}

uint8_t CPU::RST_20H()
{
    uint8_t value = 0x20;
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, Tools::msb_8(m_regs.PC));
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, Tools::lsb_8(m_regs.PC));
    m_regs.PC = Tools::unsigned_16(0x00, value);
    return 4;
}

uint8_t CPU::ADD_SP_e()
{
    uint16_t SP = m_regs.SP;
    int8_t value = Tools::signed_8(fetch());
    uint16_t result = SP + value;
    m_regs.SP = result;
    m_regs.setZero(false);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(((SP ^ value ^ result) & 0x10) != 0);
    m_regs.setCarry(((SP ^ value ^ result) & 0x100) != 0);
    return 4;
}

uint8_t CPU::JP_HL()
{
    m_regs.PC = Tools::unsigned_16(m_regs.H, m_regs.L);
    return 1;
}

uint8_t CPU::LD_Rnn_A()
{
    uint8_t low = fetch();
    uint8_t high = fetch();
    uint16_t addr = Tools::unsigned_16(high, low);
    m_memory.write(addr, m_regs.A);

    return 4;
}

uint8_t CPU::UNDEFINED_EB()
{
    throw std::exception("UNDEFINED EB");
}

uint8_t CPU::UNDEFINED_EC()
{
    throw std::exception("UNDEFINED EC");
}

uint8_t CPU::UNDEFINED_ED()
{
    throw std::exception("UNDEFINED ED");
}

uint8_t CPU::XOR_n()
{
    uint8_t A = m_regs.A;
    uint8_t R = fetch();
    uint8_t result = A ^ R;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(false);

    return 2;
}

uint8_t CPU::RST_28H()
{
    uint8_t value = 0x28;
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, Tools::msb_8(m_regs.PC));
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, Tools::lsb_8(m_regs.PC));
    m_regs.PC = Tools::unsigned_16(0x00, value);
    return 4;
}

uint8_t CPU::LDH_A_Rn()
{
    static bool a = false;
    uint8_t low = fetch();
    uint16_t addr = Tools::unsigned_16(Tools::MASK_8BITS, low);
    m_regs.A = m_memory.read(addr);
    return 3;
}

uint8_t CPU::POP_AF()
{
    uint8_t low = m_memory.read(m_regs.SP);
    m_regs.SP += 1;
    uint8_t high = m_memory.read(m_regs.SP);
    m_regs.SP += 1;
    m_regs.A = high;
    m_regs.F = low & 0xF0;
    return 3;
}

uint8_t CPU::LDH_A_RC()
{
    uint16_t addr = Tools::unsigned_16(Tools::MASK_8BITS, m_regs.C);
    m_regs.A = m_memory.read(addr);
    return 2;
}

uint8_t CPU::DI()
{
    m_regs.IME = false;
    return 1;
}

uint8_t CPU::UNDEFINED_F4()
{
    throw std::exception("UNDEFINED F4");
}

uint8_t CPU::PUSH_AF()
{
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, m_regs.A);
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, m_regs.F & 0xF0);
    return 4;
}

uint8_t CPU::OR_n()
{
    uint8_t A = m_regs.A;
    uint8_t value = fetch();
    uint8_t result = A - value;
    m_regs.A = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(false);

    return 2;
}

uint8_t CPU::RST_30H()
{
    uint8_t value = 0x30;
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, Tools::msb_8(m_regs.PC));
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, Tools::lsb_8(m_regs.PC));
    m_regs.PC = Tools::unsigned_16(0x00, value);
    return 4;
}

uint8_t CPU::LD_HL_SP_e()
{
    int8_t value = Tools::signed_8(fetch());
    uint16_t result = m_regs.SP + value;
    m_regs.H = Tools::msb_8(result);
    m_regs.L = Tools::lsb_8(result);
    m_regs.setZero(false);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry((m_regs.SP & Tools::MASK_4BITS) + (value & Tools::MASK_4BITS) > Tools::MASK_4BITS);
    m_regs.setCarry((m_regs.SP & Tools::MASK_8BITS) + (value & Tools::MASK_8BITS) > Tools::MASK_8BITS);
    return 3;
}

uint8_t CPU::LD_SP_HL()
{
    m_regs.SP = Tools::unsigned_16(m_regs.H, m_regs.L);
    return 2;
}

uint8_t CPU::LD_A_Rnn()
{
    uint8_t low = fetch();
    uint8_t high = fetch();
    uint16_t addr = Tools::unsigned_16(high, low);
    m_regs.A = m_memory.read(addr);

    return 4;
}

uint8_t CPU::EI()
{
    m_regs.NIME = true;
    return 1;
}

uint8_t CPU::UNDEFINED_FC()
{
    throw std::exception("UNDEFINED FC");
}

uint8_t CPU::UNDEFINED_FD()
{
    throw std::exception("UNDEFINED FD");
}

uint8_t CPU::CP_n()
{
    uint8_t A = m_regs.A;
    uint8_t R = fetch();
    uint8_t result = A - R;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(true);
    m_regs.setHalfCarry(((A & Tools::MASK_4BITS) < (R & Tools::MASK_4BITS)));
    m_regs.setCarry(A < R);
    return 2;
}

uint8_t CPU::RST_38H()
{
    uint8_t value = 0x38;
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, Tools::msb_8(m_regs.PC));
    m_regs.SP -= 1;
    m_memory.write(m_regs.SP, Tools::lsb_8(m_regs.PC));
    m_regs.PC = Tools::unsigned_16(0x00, value);
    return 4;
}


uint8_t CPU::RLC_R(uint8_t& R)
{
    uint8_t bit = (R & Tools::MASK_BIT_7) >> Tools::SHIFT_BIT_7;
    uint8_t result = (R << Tools::SHIFT_BIT_0) | bit;
    R = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(bit);
    return 2;
}

uint8_t CPU::RRC_R(uint8_t& R)
{
    uint8_t bit = R & Tools::MASK_BIT_0;
    uint8_t result = (R >> Tools::SHIFT_BIT_0) | (bit << Tools::SHIFT_BIT_7);
    R = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(bit);
    return 2;
}

uint8_t CPU::RL_R(uint8_t& R)
{
    uint8_t bit = (R & Tools::MASK_BIT_7) >> Tools::SHIFT_BIT_7;
    uint8_t result = (R << Tools::SHIFT_BIT_0) | static_cast<uint8_t>(m_regs.getCarry());
    R = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(bit);
    return 2;
}

uint8_t CPU::RR_R(uint8_t& R)
{
    uint8_t bit = (R & Tools::MASK_BIT_0);
    uint8_t C = static_cast<uint8_t>(m_regs.getCarry());
    uint8_t result = (R >> Tools::SHIFT_BIT_0) | (C << Tools::SHIFT_BIT_7);
    R = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(bit);
    return 2;
}

uint8_t CPU::SLA_R(uint8_t& R)
{
    uint8_t bit = (R & Tools::MASK_BIT_7) >> Tools::SHIFT_BIT_7;
    uint8_t result = R << Tools::SHIFT_BIT_0;
    R = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(bit);
    return 2;
}

uint8_t CPU::SRA_R(uint8_t& R)
{
    uint8_t bit = (R & Tools::MASK_BIT_0);
    uint8_t result = (R >> Tools::SHIFT_BIT_0) | (R & Tools::MASK_BIT_7);
    R = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(bit);
    return 2;
}

uint8_t CPU::SWAP_R(uint8_t& R)
{
    uint8_t result = (R >> Tools::SHIFT_BIT_4) | (R << Tools::SHIFT_BIT_4);
    R = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(false);
    return 2;
}

uint8_t CPU::SRL_R(uint8_t& R)
{
    uint8_t bit = (R & Tools::MASK_BIT_0);
    uint8_t result = (R >> Tools::SHIFT_BIT_0);
    R = result;
    m_regs.setZero(result == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(false);
    m_regs.setCarry(bit);
    return 2;
}

uint8_t CPU::BIT_R(const uint8_t shift, const uint8_t R)
{
    uint8_t bit = (R & (Tools::MASK_BIT_0 << shift)) >> shift;
    m_regs.setZero(bit == 0);
    m_regs.setSubtract(false);
    m_regs.setHalfCarry(true);
    return 2;
}

uint8_t CPU::RES_R(const uint8_t shift, uint8_t& R)
{
    R &= ~(Tools::MASK_BIT_0 << shift);
    return 2;
}

uint8_t CPU::SET_R(const uint8_t shift, uint8_t& R)
{
    R |= (Tools::MASK_BIT_0 << shift);
    return 2;
}

uint8_t CPU::RLC_B()
{
    return RLC_R(m_regs.B);
}

uint8_t CPU::RLC_C()
{
    return RLC_R(m_regs.C);
}

uint8_t CPU::RLC_D()
{
    return RLC_R(m_regs.D);
}

uint8_t CPU::RLC_E()
{
    return RLC_R(m_regs.E);
}

uint8_t CPU::RLC_H()
{
    return RLC_R(m_regs.H);
}

uint8_t CPU::RLC_L()
{
    return RLC_R(m_regs.L);
}

uint8_t CPU::RLC_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    RLC_R(value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::RLC_A()
{
    return RLC_R(m_regs.A);
}

uint8_t CPU::RRC_B()
{
    return RRC_R(m_regs.B);
}

uint8_t CPU::RRC_C()
{
    return RRC_R(m_regs.C);
}

uint8_t CPU::RRC_D()
{
    return RRC_R(m_regs.D);
}

uint8_t CPU::RRC_E()
{
    return RRC_R(m_regs.E);
}

uint8_t CPU::RRC_H()
{
    return RRC_R(m_regs.H);
}

uint8_t CPU::RRC_L()
{
    return RRC_R(m_regs.L);
}

uint8_t CPU::RRC_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    RRC_R(value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::RRC_A()
{
    return RRC_R(m_regs.A);
}

uint8_t CPU::RL_B()
{
    return RL_R(m_regs.B);
}

uint8_t CPU::RL_C()
{
    return RL_R(m_regs.C);
}

uint8_t CPU::RL_D()
{
    return RL_R(m_regs.D);
}

uint8_t CPU::RL_E()
{
    return RL_R(m_regs.E);
}

uint8_t CPU::RL_H()
{
    return RL_R(m_regs.H);
}

uint8_t CPU::RL_L()
{
    return RL_R(m_regs.L);
}

uint8_t CPU::RL_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    RL_R(value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::RL_A()
{
    return RL_R(m_regs.A);
}

uint8_t CPU::RR_B()
{
    return RR_R(m_regs.B);
}

uint8_t CPU::RR_C()
{
    return RR_R(m_regs.C);
}

uint8_t CPU::RR_D()
{
    return RR_R(m_regs.D);
}

uint8_t CPU::RR_E()
{
    return RR_R(m_regs.E);
}

uint8_t CPU::RR_H()
{
    return RR_R(m_regs.H);
}

uint8_t CPU::RR_L()
{
    return RR_R(m_regs.L);
}

uint8_t CPU::RR_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    RR_R(value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::RR_A()
{
    return RR_R(m_regs.A);
}

uint8_t CPU::SLA_B()
{
    return SLA_R(m_regs.B);
}

uint8_t CPU::SLA_C()
{
    return SLA_R(m_regs.C);
}

uint8_t CPU::SLA_D()
{
    return SLA_R(m_regs.D);
}

uint8_t CPU::SLA_E()
{
    return SLA_R(m_regs.E);
}

uint8_t CPU::SLA_H()
{
    return SLA_R(m_regs.H);
}

uint8_t CPU::SLA_L()
{
    return SLA_R(m_regs.L);
}

uint8_t CPU::SLA_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    SLA_R(value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::SLA_A()
{
    return SLA_R(m_regs.A);
}

uint8_t CPU::SRA_B()
{
    return SRA_R(m_regs.B);
}

uint8_t CPU::SRA_C()
{
    return SRA_R(m_regs.C);
}

uint8_t CPU::SRA_D()
{
    return SRA_R(m_regs.D);
}

uint8_t CPU::SRA_E()
{
    return SRA_R(m_regs.E);
}

uint8_t CPU::SRA_H()
{
    return SRA_R(m_regs.H);
}

uint8_t CPU::SRA_L()
{
    return SRA_R(m_regs.L);
}

uint8_t CPU::SRA_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    SRA_R(value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::SRA_A()
{
    return SRA_R(m_regs.A);
}

uint8_t CPU::SWAP_B()
{
    return SWAP_R(m_regs.B);
}

uint8_t CPU::SWAP_C()
{
    return SWAP_R(m_regs.C);
}

uint8_t CPU::SWAP_D()
{
    return SWAP_R(m_regs.D);
}

uint8_t CPU::SWAP_E()
{
    return SWAP_R(m_regs.E);
}

uint8_t CPU::SWAP_H()
{
    return SWAP_R(m_regs.H);
}

uint8_t CPU::SWAP_L()
{
    return SWAP_R(m_regs.L);
}

uint8_t CPU::SWAP_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    SWAP_R(value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::SWAP_A()
{
    return SWAP_R(m_regs.A);
}

uint8_t CPU::SRL_B()
{
    return SRL_R(m_regs.B);
}

uint8_t CPU::SRL_C()
{
    return SRL_R(m_regs.C);
}

uint8_t CPU::SRL_D()
{
    return SRL_R(m_regs.D);
}

uint8_t CPU::SRL_E()
{
    return SRL_R(m_regs.E);
}

uint8_t CPU::SRL_H()
{
    return SRL_R(m_regs.H);
}

uint8_t CPU::SRL_L()
{
    return SRL_R(m_regs.L);
}

uint8_t CPU::SRL_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    SRL_R(value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::SRL_A()
{
    return SRL_R(m_regs.A);
}

uint8_t CPU::BIT_0_B()
{
    return BIT_R(0, m_regs.B);
}

uint8_t CPU::BIT_0_C()
{
    return BIT_R(0, m_regs.C);
}

uint8_t CPU::BIT_0_D()
{
    return BIT_R(0, m_regs.D);
}

uint8_t CPU::BIT_0_E()
{
    return BIT_R(0, m_regs.E);
}

uint8_t CPU::BIT_0_H()
{
    return BIT_R(0, m_regs.H);
}

uint8_t CPU::BIT_0_L()
{
    return BIT_R(0, m_regs.L);
}

uint8_t CPU::BIT_0_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    BIT_R(0, value);
    return 3;
}

uint8_t CPU::BIT_0_A()
{
    return BIT_R(0, m_regs.A);
}

uint8_t CPU::BIT_1_B()
{
    return BIT_R(1, m_regs.B);
}

uint8_t CPU::BIT_1_C()
{
    return BIT_R(1, m_regs.C);
}

uint8_t CPU::BIT_1_D()
{
    return BIT_R(1, m_regs.D);
}

uint8_t CPU::BIT_1_E()
{
    return BIT_R(1, m_regs.E);
}

uint8_t CPU::BIT_1_H()
{
    return BIT_R(1, m_regs.H);
}

uint8_t CPU::BIT_1_L()
{
    return BIT_R(1, m_regs.L);
}

uint8_t CPU::BIT_1_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    BIT_R(1, value);
    return 3;
}

uint8_t CPU::BIT_1_A()
{
    return BIT_R(1, m_regs.A);
}

uint8_t CPU::BIT_2_B()
{
    return BIT_R(2, m_regs.B);
}

uint8_t CPU::BIT_2_C()
{
    return BIT_R(2, m_regs.C);
}

uint8_t CPU::BIT_2_D()
{
    return BIT_R(2, m_regs.D);
}

uint8_t CPU::BIT_2_E()
{
    return BIT_R(2, m_regs.E);
}

uint8_t CPU::BIT_2_H()
{
    return BIT_R(2, m_regs.H);
}

uint8_t CPU::BIT_2_L()
{
    return BIT_R(2, m_regs.L);
}

uint8_t CPU::BIT_2_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    BIT_R(2, value);
    return 3;
}

uint8_t CPU::BIT_2_A()
{
    return BIT_R(2, m_regs.A);
}

uint8_t CPU::BIT_3_B()
{
    return BIT_R(3, m_regs.B);
}

uint8_t CPU::BIT_3_C()
{
    return BIT_R(3, m_regs.C);
}

uint8_t CPU::BIT_3_D()
{
    return BIT_R(3, m_regs.D);
}

uint8_t CPU::BIT_3_E()
{
    return BIT_R(3, m_regs.E);
}

uint8_t CPU::BIT_3_H()
{
    return BIT_R(3, m_regs.H);
}

uint8_t CPU::BIT_3_L()
{
    return BIT_R(3, m_regs.L);
}

uint8_t CPU::BIT_3_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    BIT_R(3, value);
    return 3;
}

uint8_t CPU::BIT_3_A()
{
    return BIT_R(3, m_regs.A);
}

uint8_t CPU::BIT_4_B()
{
    return BIT_R(4, m_regs.B);
}

uint8_t CPU::BIT_4_C()
{
    return BIT_R(4, m_regs.C);
}

uint8_t CPU::BIT_4_D()
{
    return BIT_R(4, m_regs.D);
}

uint8_t CPU::BIT_4_E()
{
    return BIT_R(4, m_regs.E);
}

uint8_t CPU::BIT_4_H()
{
    return BIT_R(4, m_regs.H);
}

uint8_t CPU::BIT_4_L()
{
    return BIT_R(4, m_regs.L);
}

uint8_t CPU::BIT_4_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    BIT_R(4, value);
    return 3;
}

uint8_t CPU::BIT_4_A()
{
    return BIT_R(4, m_regs.A);
}

uint8_t CPU::BIT_5_B()
{
    return BIT_R(5, m_regs.B);
}

uint8_t CPU::BIT_5_C()
{
    return BIT_R(5, m_regs.C);
}

uint8_t CPU::BIT_5_D()
{
    return BIT_R(5, m_regs.D);
}

uint8_t CPU::BIT_5_E()
{
    return BIT_R(5, m_regs.E);
}

uint8_t CPU::BIT_5_H()
{
    return BIT_R(5, m_regs.H);
}

uint8_t CPU::BIT_5_L()
{
    return BIT_R(5, m_regs.L);
}

uint8_t CPU::BIT_5_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    BIT_R(5, value);
    return 3;
}

uint8_t CPU::BIT_5_A()
{
    return BIT_R(5, m_regs.A);
}


uint8_t CPU::BIT_6_B()
{
    return BIT_R(6, m_regs.B);
}

uint8_t CPU::BIT_6_C()
{
    return BIT_R(6, m_regs.C);
}

uint8_t CPU::BIT_6_D()
{
    return BIT_R(6, m_regs.D);
}

uint8_t CPU::BIT_6_E()
{
    return BIT_R(6, m_regs.E);
}

uint8_t CPU::BIT_6_H()
{
    return BIT_R(6, m_regs.H);
}

uint8_t CPU::BIT_6_L()
{
    return BIT_R(6, m_regs.L);
}

uint8_t CPU::BIT_6_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    BIT_R(6, value);
    return 3;
}

uint8_t CPU::BIT_6_A()
{
    return BIT_R(6, m_regs.A);
}

uint8_t CPU::BIT_7_B()
{
    return BIT_R(7, m_regs.B);
}

uint8_t CPU::BIT_7_C()
{
    return BIT_R(7, m_regs.C);
}

uint8_t CPU::BIT_7_D()
{
    return BIT_R(7, m_regs.D);
}

uint8_t CPU::BIT_7_E()
{
    return BIT_R(7, m_regs.E);
}

uint8_t CPU::BIT_7_H()
{
    return BIT_R(7, m_regs.H);
}

uint8_t CPU::BIT_7_L()
{
    return BIT_R(7, m_regs.L);
}

uint8_t CPU::BIT_7_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    BIT_R(7, value);
    return 3;
}

uint8_t CPU::BIT_7_A()
{
    return BIT_R(7, m_regs.A);
}


uint8_t CPU::RES_0_B()
{
    return RES_R(0, m_regs.B);
}

uint8_t CPU::RES_0_C()
{
    return RES_R(0, m_regs.C);
}

uint8_t CPU::RES_0_D()
{
    return RES_R(0, m_regs.D);
}

uint8_t CPU::RES_0_E()
{
    return RES_R(0, m_regs.E);
}

uint8_t CPU::RES_0_H()
{
    return RES_R(0, m_regs.H);
}

uint8_t CPU::RES_0_L()
{
    return RES_R(0, m_regs.L);
}

uint8_t CPU::RES_0_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    RES_R(0, value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::RES_0_A()
{
    return RES_R(0, m_regs.A);
}

uint8_t CPU::RES_1_B()
{
    return RES_R(1, m_regs.B);
}

uint8_t CPU::RES_1_C()
{
    return RES_R(1, m_regs.C);
}

uint8_t CPU::RES_1_D()
{
    return RES_R(1, m_regs.D);
}

uint8_t CPU::RES_1_E()
{
    return RES_R(1, m_regs.E);
}

uint8_t CPU::RES_1_H()
{
    return RES_R(1, m_regs.H);
}

uint8_t CPU::RES_1_L()
{
    return RES_R(1, m_regs.L);
}

uint8_t CPU::RES_1_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    RES_R(1, value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::RES_1_A()
{
    return RES_R(1, m_regs.A);
}

uint8_t CPU::RES_2_B()
{
    return RES_R(2, m_regs.B);
}

uint8_t CPU::RES_2_C()
{
    return RES_R(2, m_regs.C);
}

uint8_t CPU::RES_2_D()
{
    return RES_R(2, m_regs.D);
}

uint8_t CPU::RES_2_E()
{
    return RES_R(2, m_regs.E);
}

uint8_t CPU::RES_2_H()
{
    return RES_R(2, m_regs.H);
}

uint8_t CPU::RES_2_L()
{
    return RES_R(2, m_regs.L);
}

uint8_t CPU::RES_2_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    RES_R(2, value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::RES_2_A()
{
    return RES_R(2, m_regs.A);
}

uint8_t CPU::RES_3_B()
{
    return RES_R(3, m_regs.B);
}

uint8_t CPU::RES_3_C()
{
    return RES_R(3, m_regs.C);
}

uint8_t CPU::RES_3_D()
{
    return RES_R(3, m_regs.D);
}

uint8_t CPU::RES_3_E()
{
    return RES_R(3, m_regs.E);
}

uint8_t CPU::RES_3_H()
{
    return RES_R(3, m_regs.H);
}

uint8_t CPU::RES_3_L()
{
    return RES_R(3, m_regs.L);
}

uint8_t CPU::RES_3_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    RES_R(3, value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::RES_3_A()
{
    return RES_R(3, m_regs.A);
}

uint8_t CPU::RES_4_B()
{
    return RES_R(4, m_regs.B);
}

uint8_t CPU::RES_4_C()
{
    return RES_R(4, m_regs.C);
}

uint8_t CPU::RES_4_D()
{
    return RES_R(4, m_regs.D);
}

uint8_t CPU::RES_4_E()
{
    return RES_R(4, m_regs.E);
}

uint8_t CPU::RES_4_H()
{
    return RES_R(4, m_regs.H);
}

uint8_t CPU::RES_4_L()
{
    return RES_R(4, m_regs.L);
}

uint8_t CPU::RES_4_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    RES_R(4, value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::RES_4_A()
{
    return RES_R(4, m_regs.A);
}

uint8_t CPU::RES_5_B()
{
    return RES_R(5, m_regs.B);
}

uint8_t CPU::RES_5_C()
{
    return RES_R(5, m_regs.C);
}

uint8_t CPU::RES_5_D()
{
    return RES_R(5, m_regs.D);
}

uint8_t CPU::RES_5_E()
{
    return RES_R(5, m_regs.E);
}

uint8_t CPU::RES_5_H()
{
    return RES_R(5, m_regs.H);
}

uint8_t CPU::RES_5_L()
{
    return RES_R(5, m_regs.L);
}

uint8_t CPU::RES_5_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    RES_R(5, value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::RES_5_A()
{
    return RES_R(5, m_regs.A);
}


uint8_t CPU::RES_6_B()
{
    return RES_R(6, m_regs.B);
}

uint8_t CPU::RES_6_C()
{
    return RES_R(6, m_regs.C);
}

uint8_t CPU::RES_6_D()
{
    return RES_R(6, m_regs.D);
}

uint8_t CPU::RES_6_E()
{
    return RES_R(6, m_regs.E);
}

uint8_t CPU::RES_6_H()
{
    return RES_R(6, m_regs.H);
}

uint8_t CPU::RES_6_L()
{
    return RES_R(6, m_regs.L);
}

uint8_t CPU::RES_6_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    RES_R(6, value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::RES_6_A()
{
    return RES_R(6, m_regs.A);
}

uint8_t CPU::RES_7_B()
{
    return RES_R(7, m_regs.B);
}

uint8_t CPU::RES_7_C()
{
    return RES_R(7, m_regs.C);
}

uint8_t CPU::RES_7_D()
{
    return RES_R(7, m_regs.D);
}

uint8_t CPU::RES_7_E()
{
    return RES_R(7, m_regs.E);
}

uint8_t CPU::RES_7_H()
{
    return RES_R(7, m_regs.H);
}

uint8_t CPU::RES_7_L()
{
    return RES_R(7, m_regs.L);
}

uint8_t CPU::RES_7_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    RES_R(7, value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::RES_7_A()
{
    return RES_R(7, m_regs.A);
}

uint8_t CPU::SET_0_B()
{
    return SET_R(0, m_regs.B);
}

uint8_t CPU::SET_0_C()
{
    return SET_R(0, m_regs.C);
}

uint8_t CPU::SET_0_D()
{
    return SET_R(0, m_regs.D);
}

uint8_t CPU::SET_0_E()
{
    return SET_R(0, m_regs.E);
}

uint8_t CPU::SET_0_H()
{
    return SET_R(0, m_regs.H);
}

uint8_t CPU::SET_0_L()
{
    return SET_R(0, m_regs.L);
}

uint8_t CPU::SET_0_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    SET_R(0, value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::SET_0_A()
{
    return SET_R(0, m_regs.A);
}

uint8_t CPU::SET_1_B()
{
    return SET_R(1, m_regs.B);
}

uint8_t CPU::SET_1_C()
{
    return SET_R(1, m_regs.C);
}

uint8_t CPU::SET_1_D()
{
    return SET_R(1, m_regs.D);
}

uint8_t CPU::SET_1_E()
{
    return SET_R(1, m_regs.E);
}

uint8_t CPU::SET_1_H()
{
    return SET_R(1, m_regs.H);
}

uint8_t CPU::SET_1_L()
{
    return SET_R(1, m_regs.L);
}

uint8_t CPU::SET_1_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    SET_R(1, value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::SET_1_A()
{
    return SET_R(1, m_regs.A);
}

uint8_t CPU::SET_2_B()
{
    return SET_R(2, m_regs.B);
}

uint8_t CPU::SET_2_C()
{
    return SET_R(2, m_regs.C);
}

uint8_t CPU::SET_2_D()
{
    return SET_R(2, m_regs.D);
}

uint8_t CPU::SET_2_E()
{
    return SET_R(2, m_regs.E);
}

uint8_t CPU::SET_2_H()
{
    return SET_R(2, m_regs.H);
}

uint8_t CPU::SET_2_L()
{
    return SET_R(2, m_regs.L);
}

uint8_t CPU::SET_2_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    SET_R(2, value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::SET_2_A()
{
    return SET_R(2, m_regs.A);
}

uint8_t CPU::SET_3_B()
{
    return SET_R(3, m_regs.B);
}

uint8_t CPU::SET_3_C()
{
    return SET_R(3, m_regs.C);
}

uint8_t CPU::SET_3_D()
{
    return SET_R(3, m_regs.D);
}

uint8_t CPU::SET_3_E()
{
    return SET_R(3, m_regs.E);
}

uint8_t CPU::SET_3_H()
{
    return SET_R(3, m_regs.H);
}

uint8_t CPU::SET_3_L()
{
    return SET_R(3, m_regs.L);
}

uint8_t CPU::SET_3_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    SET_R(3, value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::SET_3_A()
{
    return SET_R(3, m_regs.A);
}

uint8_t CPU::SET_4_B()
{
    return SET_R(4, m_regs.B);
}

uint8_t CPU::SET_4_C()
{
    return SET_R(4, m_regs.C);
}

uint8_t CPU::SET_4_D()
{
    return SET_R(4, m_regs.D);
}

uint8_t CPU::SET_4_E()
{
    return SET_R(4, m_regs.E);
}

uint8_t CPU::SET_4_H()
{
    return SET_R(4, m_regs.H);
}

uint8_t CPU::SET_4_L()
{
    return SET_R(4, m_regs.L);
}

uint8_t CPU::SET_4_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    SET_R(4, value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::SET_4_A()
{
    return SET_R(4, m_regs.A);
}

uint8_t CPU::SET_5_B()
{
    return SET_R(5, m_regs.B);
}

uint8_t CPU::SET_5_C()
{
    return SET_R(5, m_regs.C);
}

uint8_t CPU::SET_5_D()
{
    return SET_R(5, m_regs.D);
}

uint8_t CPU::SET_5_E()
{
    return SET_R(5, m_regs.E);
}

uint8_t CPU::SET_5_H()
{
    return SET_R(5, m_regs.H);
}

uint8_t CPU::SET_5_L()
{
    return SET_R(5, m_regs.L);
}

uint8_t CPU::SET_5_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    SET_R(5, value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::SET_5_A()
{
    return SET_R(5, m_regs.A);
}


uint8_t CPU::SET_6_B()
{
    return SET_R(6, m_regs.B);
}

uint8_t CPU::SET_6_C()
{
    return SET_R(6, m_regs.C);
}

uint8_t CPU::SET_6_D()
{
    return SET_R(6, m_regs.D);
}

uint8_t CPU::SET_6_E()
{
    return SET_R(6, m_regs.E);
}

uint8_t CPU::SET_6_H()
{
    return SET_R(6, m_regs.H);
}

uint8_t CPU::SET_6_L()
{
    return SET_R(6, m_regs.L);
}

uint8_t CPU::SET_6_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    SET_R(6, value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::SET_6_A()
{
    return SET_R(6, m_regs.A);
}

uint8_t CPU::SET_7_B()
{
    return SET_R(7, m_regs.B);
}

uint8_t CPU::SET_7_C()
{
    return SET_R(7, m_regs.C);
}

uint8_t CPU::SET_7_D()
{
    return SET_R(7, m_regs.D);
}

uint8_t CPU::SET_7_E()
{
    return SET_R(7, m_regs.E);
}

uint8_t CPU::SET_7_H()
{
    return SET_R(7, m_regs.H);
}

uint8_t CPU::SET_7_L()
{
    return SET_R(7, m_regs.L);
}

uint8_t CPU::SET_7_RHL()
{
    uint16_t addr = Tools::unsigned_16(m_regs.H, m_regs.L);
    uint8_t value = m_memory.read(addr);
    SET_R(7, value);
    m_memory.write(addr, value);
    return 4;
}

uint8_t CPU::SET_7_A()
{
    return SET_R(7, m_regs.A);
}

}
}