#pragma once

#include <cstdint> // type defs
#include <random>  // RNG1

// Useful constants for CHIP8
//
// Number of registers
const uint32_t N_REG = 16;
// Memory available (in bytes)
const uint32_t MEM_B = 4096;
// Number of input keys
const uint32_t N_KEYS = 16;
// Width of the display screen (in pixels)
const uint32_t DISPLAY_WIDTH = 64;
// Height of the display screen (in pixels)
const uint32_t DISPLAY_HEIGHT = 32;
// Start address of the available memory for ROMs
const uint32_t START_ADDRESS = 0x200;
// Size in memory of the font (16 character of 5 bytes)
const uint32_t FONTSET_SIZE = 80;

class Chip8 {
public:
  // Constructor
  Chip8();
  // Load a ROM into memory
  void LoadROM(char const *filename);
  // Perform one cycle of the simulation
  void Cycle();

private:
  // 16 8-bit registers (V0 to VF)
  uint8_t registers[N_REG]{};
  // 4KB of memory
  uint8_t memory[MEM_B]{};
  // 16-bit index register (used for OPs)
  uint16_t index{};
  // 16-bit program counter
  uint16_t pc{};
  // stack of depth 16 (to store return address)
  uint16_t stack[16]{};
  // stack pointer
  uint8_t sp{};
  // 8-bit time clock
  uint8_t delayTimer{};
  // 8-bit sound clock
  uint8_t soundTimer{};
  // 16 input keys
  uint8_t keypad[N_KEYS]{};
  // display screen
  uint32_t video[DISPLAY_WIDTH * DISPLAY_HEIGHT]{};
  // current OP code
  uint16_t opcode;
  // RNG
  std::default_random_engine randGen;
  // Randomly generated value
  std::uniform_int_distribution<uint8_t> randByte;

  // clear display
  void OP_00E0();
  // return from subroutine
  void OP_00EE();
  // jump to nnn (lowest 12 bits of the instruction)
  void OP_1nnn();
  // call nnn (lowest 12 bits of the instruction)
  void OP_2nnn();
  // conditional eq skip (lowest byte of the instruction)
  void OP_3xkk();
  // conditional neq skip (lowest byte of the instruction)
  void OP_4xkk();
  // conditional skip with cmp between two registers
  void OP_5xy0();
  // load kk to register Vx
  void OP_6xkk();
  // add kk to register Vx
  void OP_7xkk();
  // load value of register Vy to register Vx
  void OP_8xy0();
  // bitwise or of Vx and Vy
  void OP_8xy1();
  // bitwise and of Vx and Vy
  void OP_8xy2();
  // XOR of Vx and Vy
  void OP_8xy3();
  // ADD of Vx and Vy in Vx with overflow flag
  void OP_8xy4();
  // SUB Vx, Vy
  void OP_8xy5();
  // set Vx = Vx SHR 1
  void OP_8xy6();
  // SUBN Vx, Vy (opposite of SUB)
  void OP_8xy7();
  // SHL Vx
  void OP_8xyE();
  // SNE Vx, Vy (conditional jump)
  void OP_9xy0();
  // LD I, addr (put addr at index)
  void OP_Annn();
  // JP V0, addr (jump to location nnn + V0)
  void OP_Bnnn();
  // RND Vx, byte (set Vx = random byte AND kk)
  void OP_Cxkk();
  // DRW Vx, Vy, nibble (display n-byte sprite)
  void OP_Dxyn();
};
