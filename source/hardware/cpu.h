#pragma once 

#include <cstdint>
#include <iostream>
#include <string>
#include <array>

#include "tools/commontoolkit.h"

namespace GBCPUX {
namespace Hardware {

class Memory;
class PPU;

class CPU
{
public:
    CPU(Memory& memory, PPU& ppu);
    ~CPU() = default;
    bool load(std::string path);
    void start();

private:
    Memory& m_memory;
    PPU& m_ppu;
    bool m_IMENext = false;
    Tools::CPURegisters m_regs;
    Tools::CPURegisters m_regs_restore;
    std::array<uint8_t(CPU::*)(), 256> opcodes;
    std::array<uint8_t(CPU::*)(), 256> prefixs;
     
private: 
    void reset();
    uint8_t fetch();
    uint8_t decute(uint8_t opcode);
    uint8_t step();
    void waitHandle();

    //0x00
    uint8_t NOP();
    //0x01
    uint8_t LD_BC_nn();
    //0x02
    uint8_t LD_RBC_A();
    //0x03
    uint8_t INC_BC();
    //0x04
    uint8_t INC_B();
    //0x05
    uint8_t DEC_B();
    //0x06
    uint8_t LD_B_n();
    //0x07
    uint8_t RLCA();
    //0x08
    uint8_t LD_nn_SP();
    //0x09
    uint8_t ADD_HL_BC();
    //0x0A
    uint8_t LD_A_RBC();
    //0x0B
    uint8_t DEC_BC();
    //0x0C
    uint8_t INC_C();
    //0x0D
    uint8_t DEC_C();
    //0x0E
    uint8_t LD_C_n();
    //0x0F
    uint8_t RRCA();
    //0x10
    uint8_t STOP();
    //0x11
    uint8_t LD_DE_nn();
    //0x12
    uint8_t LD_RDE_A();
    //0x13
    uint8_t INC_DE();
    //0x14
    uint8_t INC_D();
    //0x15
    uint8_t DEC_D();
    //0x16
    uint8_t LD_D_n();
    //0x17
    uint8_t RLA();
    //0x18
    uint8_t JR_e();
    //0x19
    uint8_t ADD_HL_DE();
    //0x1A
    uint8_t LD_A_RDE();
    //0x1B
    uint8_t DEC_DE();
    //0x1C
    uint8_t INC_E();
    //0x1D
    uint8_t DEC_E();
    //0x1E
    uint8_t LD_E_n();
    //0x1F
    uint8_t RRA();
    //0x20
    uint8_t JR_NZ_e();
    //0x21
    uint8_t LD_HL_nn();
    //0x22
    uint8_t LD_RHL_INC_A();
    //0x23
    uint8_t INC_HL();
    //0x24
    uint8_t INC_H();
    //0x25
    uint8_t DEC_H();
    //0x26
    uint8_t LD_H_n();
    //0x27
    uint8_t DAA();
    //0x28
    uint8_t JR_Z_e();
    //0x29
    uint8_t ADD_HL_HL();
    //0x2A
    uint8_t LD_A_RHL_INC();
    //0x2B
    uint8_t DEC_HL();
    //0x2C
    uint8_t INC_L();
    //0x2D
    uint8_t DEC_L();
    //0x2E
    uint8_t LD_L_n();
    //0x2F
    uint8_t CPL();
    //0x30
    uint8_t JR_NC_e();
    //0x31
    uint8_t LD_SP_nn();
    //0x32
    uint8_t LD_RHL_DEC_A();
    //0x33
    uint8_t INC_SP();
    //0x34
    uint8_t INC_RHL();
    //0x35
    uint8_t DEC_RHL();
    //0x36
    uint8_t LD_RHL();
    //0x37
    uint8_t SCF();
    //0x38
    uint8_t JR_C_e();
    //0x39
    uint8_t ADD_HL_SP();
    //0x3A
    uint8_t LD_A_RHL_DEC();
    //0x3B
    uint8_t DEC_SP();
    //0x3C
    uint8_t INC_A();
    //0x3D
    uint8_t DEC_A();
    //0x3E
    uint8_t LD_A_n();
    //0x3F
    uint8_t CCF();
    //0x40
    uint8_t LD_B_B();
    //0x41
    uint8_t LD_B_C();
    //0x42
    uint8_t LD_B_D();
    //0x43
    uint8_t LD_B_E();
    //0x44
    uint8_t LD_B_H();
    //0x45
    uint8_t LD_B_L();
    //0x46
    uint8_t LD_B_RHL();
    //0x47
    uint8_t LD_B_A();
    //0x48
    uint8_t LD_C_B();
    //0x49
    uint8_t LD_C_C();
    //0x4A
    uint8_t LD_C_D();
    //0x4B
    uint8_t LD_C_E();
    //0x4C
    uint8_t LD_C_H();
    //0x4D
    uint8_t LD_C_L();
    //0x4E
    uint8_t LD_C_RHL();
    //0x4F
    uint8_t LD_C_A();
    //0x50
    uint8_t LD_D_B();
    //0x51
    uint8_t LD_D_C();
    //0x52
    uint8_t LD_D_D();
    //0x53
    uint8_t LD_D_E();
    //0x54
    uint8_t LD_D_H();
    //0x55
    uint8_t LD_D_L();
    //0x56
    uint8_t LD_D_RHL();
    //0x57
    uint8_t LD_D_A();
    //0x58
    uint8_t LD_E_B();
    //0x59
    uint8_t LD_E_C();
    //0x5A
    uint8_t LD_E_D();
    //0x5B
    uint8_t LD_E_E();
    //0x5C
    uint8_t LD_E_H();
    //0x5D
    uint8_t LD_E_L();
    //0x5E
    uint8_t LD_E_RHL();
    //0x5F
    uint8_t LD_E_A();
    //0x60
    uint8_t LD_H_B();
    //0x61
    uint8_t LD_H_C();
    //0x62
    uint8_t LD_H_D();
    //0x63
    uint8_t LD_H_E();
    //0x64
    uint8_t LD_H_H();
    //0x65
    uint8_t LD_H_L();
    //0x66
    uint8_t LD_H_RHL();
    //0x67
    uint8_t LD_H_A();
    //0x68
    uint8_t LD_L_B();
    //0x69
    uint8_t LD_L_C();
    //0x6A
    uint8_t LD_L_D();
    //0x6B
    uint8_t LD_L_E();
    //0x6C
    uint8_t LD_L_H();
    //0x6D
    uint8_t LD_L_L();
    //0x6E
    uint8_t LD_L_RHL();
    //0x6F
    uint8_t LD_L_A();
    //0x70
    uint8_t LD_RHL_B();
    //0x71
    uint8_t LD_RHL_C();
    //0x72
    uint8_t LD_RHL_D();
    //0x73
    uint8_t LD_RHL_E();
    //0x74
    uint8_t LD_RHL_H();
    //0x75
    uint8_t LD_RHL_L();
    //0x76
    uint8_t HALT();
    //0x77
    uint8_t LD_RHL_A();
    //0x78
    uint8_t LD_A_B();
    //0x79
    uint8_t LD_A_C();
    //0x7A
    uint8_t LD_A_D();
    //0x7B
    uint8_t LD_A_E();
    //0x7C
    uint8_t LD_A_H();
    //0x7D
    uint8_t LD_A_L();
    //0x7E
    uint8_t LD_A_RHL();
    //0x7F
    uint8_t LD_A_A();
    //0x80
    uint8_t ADD_B();
    //0x81
    uint8_t ADD_C();
    //0x82
    uint8_t ADD_D();
    //0x83
    uint8_t ADD_E();
    //0x84
    uint8_t ADD_H();
    //0x85
    uint8_t ADD_L();
    //0x86
    uint8_t ADD_RHL();
    //0x87
    uint8_t ADD_A();
    //0x88
    uint8_t ADC_B();
    //0x89
    uint8_t ADC_C();
    //0x8A
    uint8_t ADC_D();
    //0x8B
    uint8_t ADC_E();
    //0x8C
    uint8_t ADC_H();
    //0x8D
    uint8_t ADC_L();
    //0x8E
    uint8_t ADC_RHL();
    //0x8F
    uint8_t ADC_A();
    //0x90
    uint8_t SUB_B();
    //0x91
    uint8_t SUB_C();
    //0x92
    uint8_t SUB_D();
    //0x93
    uint8_t SUB_E();
    //0x94
    uint8_t SUB_H();
    //0x95
    uint8_t SUB_L();
    //0x96
    uint8_t SUB_RHL();
    //0x97
    uint8_t SUB_A();
    //0x98
    uint8_t SBC_B();
    //0x99
    uint8_t SBC_C();
    //0x9A
    uint8_t SBC_D();
    //0x9B
    uint8_t SBC_E();
    //0x9C
    uint8_t SBC_H();
    //0x9D
    uint8_t SBC_L();
    //0x9E
    uint8_t SBC_RHL();
    //0x9F
    uint8_t SBC_A();
    //0xA0
    uint8_t AND_B();
    //0xA1
    uint8_t AND_C();
    //0xA2
    uint8_t AND_D();
    //0xA3
    uint8_t AND_E();
    //0xA4
    uint8_t AND_H();
    //0xA5
    uint8_t AND_L();
    //0xA6
    uint8_t AND_RHL();
    //0xA7
    uint8_t AND_A();
    //0xA8
    uint8_t XOR_B();
    //0xA9
    uint8_t XOR_C();
    //0xAA
    uint8_t XOR_D();
    //0xAB
    uint8_t XOR_E();
    //0xAC
    uint8_t XOR_H();
    //0xAD
    uint8_t XOR_L();
    //0xAE
    uint8_t XOR_RHL();
    //0xAF
    uint8_t XOR_A();
    //0xB0
    uint8_t OR_B();
    //0xB1
    uint8_t OR_C();
    //0xB2
    uint8_t OR_D();
    //0xB3
    uint8_t OR_E();
    //0xB4
    uint8_t OR_H();
    //0xB5
    uint8_t OR_L();
    //0xB6
    uint8_t OR_RHL();
    //0xB7
    uint8_t OR_A();
    //0xB8
    uint8_t CP_B();
    //0xB9
    uint8_t CP_C();
    //0xBA
    uint8_t CP_D();
    //0xBB
    uint8_t CP_E();
    //0xBC
    uint8_t CP_H();
    //0xBD
    uint8_t CP_L();
    //0xBE
    uint8_t CP_RHL();
    //0xBF
    uint8_t CP_A();
    //0xC0
    uint8_t RET_NZ();
    //0xC1
    uint8_t POP_BC();
    //0xC2
    uint8_t JP_NZ_nn();
    //0xC3
    uint8_t JP_nn();
    //0xC4
    uint8_t CALL_NZ_nn();
    //0xC5
    uint8_t PUSH_BC();
    //0xC6
    uint8_t ADD_A_n();
    //0xC7
    uint8_t RST_00H();
    //0xC8
    uint8_t RET_Z();
    //0xC9
    uint8_t RET();
    //0xCA
    uint8_t JP_Z_nn();
    //0xCB
    uint8_t PREFIX_CB();
    //0xCC
    uint8_t CALL_Z_nn();
    //0xCD
    uint8_t CALL_nn();
    //0xCE
    uint8_t ADC_A_n();
    //0xCF
    uint8_t RST_08H();
    //0xD0
    uint8_t RET_NC();
    //0xD1
    uint8_t POP_DE();
    //0xD2
    uint8_t JP_NC_nn();
    //0xD3
    uint8_t UNDEFINED_D3();
    //0xD4
    uint8_t CALL_NC_nn();
    //0xD5
    uint8_t PUSH_DE();
    //0xD6
    uint8_t SUB_n();
    //0xD7
    uint8_t RST_10H();
    //0xD8
    uint8_t RET_C();
    //0xD9
    uint8_t RETI();
    //0xDA
    uint8_t JP_C_nn();
    //0xDB
    uint8_t UNDEFINED_DB();
    //0xDC
    uint8_t CALL_C_nn();
    //0xDD
    uint8_t UNDEFINED_DD();
    //0xDE
    uint8_t SBC_A_n();
    //0xDF
    uint8_t RST_18H();
    //0xE0
    uint8_t LDH_Rn_A();
    //0xE1
    uint8_t POP_HL();
    //0xE2
    uint8_t LDH_RC_A();
    //0xE3
    uint8_t UNDEFINED_E3();
    //0xE4
    uint8_t UNDEFINED_E4();
    //0xE5
    uint8_t PUSH_HL();
    //0xE6
    uint8_t AND_n();
    //0xE7
    uint8_t RST_20H();
    //0xE8
    uint8_t ADD_SP_e();
    //0xE9
    uint8_t JP_HL();
    //0xEA
    uint8_t LD_Rnn_A();
    //0xEB
    uint8_t UNDEFINED_EB();
    //0xEC
    uint8_t UNDEFINED_EC();
    //0xED
    uint8_t UNDEFINED_ED();
    //0xEE
    uint8_t XOR_n();
    //0xEF
    uint8_t RST_28H();
    //0xF0
    uint8_t LDH_A_Rn();
    //0xF1
    uint8_t POP_AF();
    //0xF2
    uint8_t LDH_A_RC();
    //0xF3
    uint8_t DI();
    //0xF4
    uint8_t UNDEFINED_F4();
    //0xF5
    uint8_t PUSH_AF();
    //0xF6
    uint8_t OR_n();
    //0xF7
    uint8_t RST_30H();
    //0xF8
    uint8_t LD_HL_SP_e();
    //0xF9
    uint8_t LD_SP_HL();
    //0xFA
    uint8_t LD_A_Rnn();
    //0xFB
    uint8_t EI();
    //0xFC
    uint8_t UNDEFINED_FC();
    //0xFD
    uint8_t UNDEFINED_FD();
    //0xFE
    uint8_t CP_n();
    //0xFF
    uint8_t RST_38H();

    // prefix CB 
    uint8_t RLC_R(uint8_t& R);
    uint8_t RRC_R(uint8_t& R);
    uint8_t RL_R(uint8_t& R);
    uint8_t RR_R(uint8_t& R);
    uint8_t SLA_R(uint8_t& R);
    uint8_t SRA_R(uint8_t& R);
    uint8_t SWAP_R(uint8_t& R);
    uint8_t SRL_R(uint8_t& R);
    uint8_t BIT_R(const uint8_t shift, const uint8_t R);
    uint8_t RES_R(const uint8_t shift, uint8_t& R);
    uint8_t SET_R(const uint8_t shift, uint8_t& R);
    //0x00
    uint8_t RLC_B();
    //0x01
    uint8_t RLC_C();
    //0x02
    uint8_t RLC_D();
    //0x03
    uint8_t RLC_E();
    //0x04
    uint8_t RLC_H();
    //0x05
    uint8_t RLC_L();
    //0x06
    uint8_t RLC_RHL();
    //0x07
    uint8_t RLC_A();
    //0x08
    uint8_t RRC_B();
    //0x09
    uint8_t RRC_C();
    //0x0A
    uint8_t RRC_D();
    //0x0B
    uint8_t RRC_E();
    //0x0C
    uint8_t RRC_H();
    //0x0D
    uint8_t RRC_L();
    //0x0E
    uint8_t RRC_RHL();
    //0x0F
    uint8_t RRC_A();
    //0x10
    uint8_t RL_B();
    //0x11
    uint8_t RL_C();
    //0x12
    uint8_t RL_D();
    //0x13
    uint8_t RL_E();
    //0x14
    uint8_t RL_H();
    //0x15
    uint8_t RL_L();
    //0x16
    uint8_t RL_RHL();
    //0x17
    uint8_t RL_A();
    //0x18
    uint8_t RR_B();
    //0x19
    uint8_t RR_C();
    //0x1A
    uint8_t RR_D();
    //0x1B
    uint8_t RR_E();
    //0x1C
    uint8_t RR_H();
    //0x1D
    uint8_t RR_L();
    //0x1E
    uint8_t RR_RHL();
    //0x1F
    uint8_t RR_A();
    //0x20
    uint8_t SLA_B();
    //0x21
    uint8_t SLA_C();
    //0x22
    uint8_t SLA_D();
    //0x23
    uint8_t SLA_E();
    //0x24
    uint8_t SLA_H();
    //0x25
    uint8_t SLA_L();
    //0x26
    uint8_t SLA_RHL();
    //0x27
    uint8_t SLA_A();
    //0x28
    uint8_t SRA_B();
    //0x29
    uint8_t SRA_C();
    //0x2A
    uint8_t SRA_D();
    //0x2B
    uint8_t SRA_E();
    //0x2C
    uint8_t SRA_H();
    //0x2D
    uint8_t SRA_L();
    //0x2E
    uint8_t SRA_RHL();
    //0x2F
    uint8_t SRA_A();
    //0x30
    uint8_t SWAP_B();
    //0x31
    uint8_t SWAP_C();
    //0x32
    uint8_t SWAP_D();
    //0x33
    uint8_t SWAP_E();
    //0x34
    uint8_t SWAP_H();
    //0x35
    uint8_t SWAP_L();
    //0x36
    uint8_t SWAP_RHL();
    //0x37
    uint8_t SWAP_A();
    //0x38
    uint8_t SRL_B();
    //0x39
    uint8_t SRL_C();
    //0x3A
    uint8_t SRL_D();
    //0x3B
    uint8_t SRL_E();
    //0x3C
    uint8_t SRL_H();
    //0x3D
    uint8_t SRL_L();
    //0x3E
    uint8_t SRL_RHL();
    //0x3F
    uint8_t SRL_A();
    //0x40
    uint8_t BIT_0_B();
    //0x41
    uint8_t BIT_0_C();
    //0x42
    uint8_t BIT_0_D();
    //0x43
    uint8_t BIT_0_E();
    //0x44
    uint8_t BIT_0_H();
    //0x45
    uint8_t BIT_0_L();
    //0x46
    uint8_t BIT_0_RHL();
    //0x47
    uint8_t BIT_0_A();
    //0x48
    uint8_t BIT_1_B();
    //0x49
    uint8_t BIT_1_C();
    //0x4A
    uint8_t BIT_1_D();
    //0x4B
    uint8_t BIT_1_E();
    //0x4C
    uint8_t BIT_1_H();
    //0x4D
    uint8_t BIT_1_L();
    //0x4E
    uint8_t BIT_1_RHL();
    //0x4F
    uint8_t BIT_1_A();
    //0x50
    uint8_t BIT_2_B();
    //0x51
    uint8_t BIT_2_C();
    //0x52
    uint8_t BIT_2_D();
    //0x53
    uint8_t BIT_2_E();
    //0x54
    uint8_t BIT_2_H();
    //0x55
    uint8_t BIT_2_L();
    //0x56
    uint8_t BIT_2_RHL();
    //0x57
    uint8_t BIT_2_A();
    //0x58
    uint8_t BIT_3_B();
    //0x59
    uint8_t BIT_3_C();
    //0x5A
    uint8_t BIT_3_D();
    //0x5B
    uint8_t BIT_3_E();
    //0x5C
    uint8_t BIT_3_H();
    //0x5D
    uint8_t BIT_3_L();
    //0x5E
    uint8_t BIT_3_RHL();
    //0x5F
    uint8_t BIT_3_A();
    //0x60
    uint8_t BIT_4_B();
    //0x61
    uint8_t BIT_4_C();
    //0x62
    uint8_t BIT_4_D();
    //0x63
    uint8_t BIT_4_E();
    //0x64
    uint8_t BIT_4_H();
    //0x65
    uint8_t BIT_4_L();
    //0x66
    uint8_t BIT_4_RHL();
    //0x67
    uint8_t BIT_4_A();
    //0x68
    uint8_t BIT_5_B();
    //0x69
    uint8_t BIT_5_C();
    //0x6A
    uint8_t BIT_5_D();
    //0x6B
    uint8_t BIT_5_E();
    //0x6C
    uint8_t BIT_5_H();
    //0x6D
    uint8_t BIT_5_L();
    //0x6E
    uint8_t BIT_5_RHL();
    //0x6F
    uint8_t BIT_5_A();
    //0x70
    uint8_t BIT_6_B();
    //0x71
    uint8_t BIT_6_C();
    //0x72
    uint8_t BIT_6_D();
    //0x73
    uint8_t BIT_6_E();
    //0x74
    uint8_t BIT_6_H();
    //0x75
    uint8_t BIT_6_L();
    //0x76
    uint8_t BIT_6_RHL();
    //0x77
    uint8_t BIT_6_A();
    //0x78
    uint8_t BIT_7_B();
    //0x79
    uint8_t BIT_7_C();
    //0x7A
    uint8_t BIT_7_D();
    //0x7B
    uint8_t BIT_7_E();
    //0x7C
    uint8_t BIT_7_H();
    //0x7D
    uint8_t BIT_7_L();
    //0x7E
    uint8_t BIT_7_RHL();
    //0x7F
    uint8_t BIT_7_A();
    //0x80
    uint8_t RES_0_B();
    //0x81
    uint8_t RES_0_C();
    //0x82
    uint8_t RES_0_D();
    //0x83
    uint8_t RES_0_E();
    //0x84
    uint8_t RES_0_H();
    //0x85
    uint8_t RES_0_L();
    //0x86
    uint8_t RES_0_RHL();
    //0x87
    uint8_t RES_0_A();
    //0x88
    uint8_t RES_1_B();
    //0x89
    uint8_t RES_1_C();
    //0x8A
    uint8_t RES_1_D();
    //0x8B
    uint8_t RES_1_E();
    //0x8C
    uint8_t RES_1_H();
    //0x8D
    uint8_t RES_1_L();
    //0x8E
    uint8_t RES_1_RHL();
    //0x8F
    uint8_t RES_1_A();
    //0x90
    uint8_t RES_2_B();
    //0x91
    uint8_t RES_2_C();
    //0x92
    uint8_t RES_2_D();
    //0x93
    uint8_t RES_2_E();
    //0x94
    uint8_t RES_2_H();
    //0x95
    uint8_t RES_2_L();
    //0x96
    uint8_t RES_2_RHL();
    //0x97
    uint8_t RES_2_A();
    //0x98
    uint8_t RES_3_B();
    //0x99
    uint8_t RES_3_C();
    //0x9A
    uint8_t RES_3_D();
    //0x9B
    uint8_t RES_3_E();
    //0x9C
    uint8_t RES_3_H();
    //0x9D
    uint8_t RES_3_L();
    //0x9E
    uint8_t RES_3_RHL();
    //0x9F
    uint8_t RES_3_A();
    //0xA0
    uint8_t RES_4_B();
    //0xA1
    uint8_t RES_4_C();
    //0xA2
    uint8_t RES_4_D();
    //0xA3
    uint8_t RES_4_E();
    //0xA4
    uint8_t RES_4_H();
    //0xA5
    uint8_t RES_4_L();
    //0xA6
    uint8_t RES_4_RHL();
    //0xA7
    uint8_t RES_4_A();
    //0xA8
    uint8_t RES_5_B();
    //0xA9
    uint8_t RES_5_C();
    //0xAA
    uint8_t RES_5_D();
    //0xAB
    uint8_t RES_5_E();
    //0xAC
    uint8_t RES_5_H();
    //0xAD
    uint8_t RES_5_L();
    //0xAE
    uint8_t RES_5_RHL();
    //0xAF
    uint8_t RES_5_A();
    //0xB0
    uint8_t RES_6_B();
    //0xB1
    uint8_t RES_6_C();
    //0xB2
    uint8_t RES_6_D();
    //0xB3
    uint8_t RES_6_E();
    //0xB4
    uint8_t RES_6_H();
    //0xB5
    uint8_t RES_6_L();
    //0xB6
    uint8_t RES_6_RHL();
    //0xB7
    uint8_t RES_6_A();
    //0xB8
    uint8_t RES_7_B();
    //0xB9
    uint8_t RES_7_C();
    //0xBA
    uint8_t RES_7_D();
    //0xBB
    uint8_t RES_7_E();
    //0xBC
    uint8_t RES_7_H();
    //0xBD
    uint8_t RES_7_L();
    //0xBE
    uint8_t RES_7_RHL();
    //0xBF
    uint8_t RES_7_A();
    //0xC0
    uint8_t SET_0_B();
    //0xC1
    uint8_t SET_0_C();
    //0xC2
    uint8_t SET_0_D();
    //0xC3
    uint8_t SET_0_E();
    //0xC4
    uint8_t SET_0_H();
    //0xC5
    uint8_t SET_0_L();
    //0xC6
    uint8_t SET_0_RHL();
    //0xC7
    uint8_t SET_0_A();
    //0xC8
    uint8_t SET_1_B();
    //0xC9
    uint8_t SET_1_C();
    //0xCA
    uint8_t SET_1_D();
    //0xCB
    uint8_t SET_1_E();
    //0xCC
    uint8_t SET_1_H();
    //0xCD
    uint8_t SET_1_L();
    //0xCE
    uint8_t SET_1_RHL();
    //0xCF
    uint8_t SET_1_A();
    //0xD0
    uint8_t SET_2_B();
    //0xD1
    uint8_t SET_2_C();
    //0xD2
    uint8_t SET_2_D();
    //0xD3
    uint8_t SET_2_E();
    //0xD4
    uint8_t SET_2_H();
    //0xD5
    uint8_t SET_2_L();
    //0xD6
    uint8_t SET_2_RHL();
    //0xD7
    uint8_t SET_2_A();
    //0xD8
    uint8_t SET_3_B();
    //0xD9
    uint8_t SET_3_C();
    //0xDA
    uint8_t SET_3_D();
    //0xDB
    uint8_t SET_3_E();
    //0xDC
    uint8_t SET_3_H();
    //0xDD
    uint8_t SET_3_L();
    //0xDE
    uint8_t SET_3_RHL();
    //0xDF
    uint8_t SET_3_A();
    //0xE0
    uint8_t SET_4_B();
    //0xE1
    uint8_t SET_4_C();
    //0xE2
    uint8_t SET_4_D();
    //0xE3
    uint8_t SET_4_E();
    //0xE4
    uint8_t SET_4_H();
    //0xE5
    uint8_t SET_4_L();
    //0xE6
    uint8_t SET_4_RHL();
    //0xE7
    uint8_t SET_4_A();
    //0xE8
    uint8_t SET_5_B();
    //0xE9
    uint8_t SET_5_C();
    //0xEA
    uint8_t SET_5_D();
    //0xEB
    uint8_t SET_5_E();
    //0xEC
    uint8_t SET_5_H();
    //0xED
    uint8_t SET_5_L();
    //0xEE
    uint8_t SET_5_RHL();
    //0xEF
    uint8_t SET_5_A();
    //0xF0
    uint8_t SET_6_B();
    //0xF1
    uint8_t SET_6_C();
    //0xF2
    uint8_t SET_6_D();
    //0xF3
    uint8_t SET_6_E();
    //0xF4
    uint8_t SET_6_H();
    //0xF5
    uint8_t SET_6_L();
    //0xF6
    uint8_t SET_6_RHL();
    //0xF7
    uint8_t SET_6_A();
    //0xF8
    uint8_t SET_7_B();
    //0xF9
    uint8_t SET_7_C();
    //0xFA
    uint8_t SET_7_D();
    //0xFB
    uint8_t SET_7_E();
    //0xFC
    uint8_t SET_7_H();
    //0xFD
    uint8_t SET_7_L();
    //0xFE
    uint8_t SET_7_RHL();
    //0xFF
    uint8_t SET_7_A();

};
}
}