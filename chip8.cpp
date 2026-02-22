#include <chrono>  // measure time
#include <cstring> // memset
#include <fstream> // IO
#include <iterator>

#include "chip8.hpp"

// Bytes for the fontset (each byte represents a row on display)
const uint8_t fontset[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
// Start address for the fontset (section 2 in memory)
const uint32_t FONSET_START_ADDRESS = 0x50;

// Constructor: initialize the VM
Chip8::Chip8()
    : randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
  // Initialize PC to section 3 (start for ROM)
  pc = START_ADDRESS;

  // Load fontset in memory
  for (uint32_t i = 0; i < FONTSET_SIZE; i++) {
    memory[FONSET_START_ADDRESS + i] = fontset[i];
  }

  // Initialize RNG
  randBytes = std::uniform_int_distribution<uint8_t>(0, 255U);
}

// Load ROM into VM
void Chip8::LoadROM(char const *filename) {
  // Open file in binary mode and move fp to the end
  std::ifstream file(filename, std::ios::binary | std::ios::ate);

  if (file.is_open()) {
    // Get file size and allocate buffer
    std::streampos size = file.tellg();
    char *buffer = new char[size];

    // Move fp to the beginning of file
    file.seekg(0, std::ios::beg);
    file.read(buffer, size);
    file.close();

    // Load ROM into memory, starting at 0x200
    for (long i = 0; i < size; i++) {
      memory[START_ADDRESS + i] = buffer[i];
    }

    // Free the buffer
    delete[] buffer;
  }
}

// Emulation of the instructions
// Source: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#0.0

// '00E0': CLS
// Clear the display
void Chip8::OP_00E0() {
  // reset the video buffer
  std::fill(std::begin(video), std::end(video), 0);
}

// '00EE': RET
// Return from subroutine
void Chip8::OP_00EE() {
  sp--;
  pc = stack[sp];
}

// '1nnn': JP addr
// Jump to location nnn
// We use the mask 0x0FFFu to retrieve the lowest 12 bits of opcode.
void Chip8::OP_1nnn() {
  uint16_t addr = opcode & 0x0FFFu;

  pc = addr;
}

// '2nnn': CALL addr
// Call subroutine at nnn (lowest 12 bits of the instruction)
// We use the mask 0x0FFFu to retrieve the lowest 12 bits of opcode.
// Remember that PC is incremented by 2. Therefore, PC points to the next
// instruction to execute after the call.
void Chip8::OP_2nnn() {
  uint16_t addr = opcode & 0x0FFFu;

  // remember where to return
  stack[sp] = pc;
  sp++;
  // jump to the caller subroutine
  pc = addr;
}

// '3xkk': SE Vx, byte
// Skip next instruction if Vx == kk, where Vx is a register
// kk corresponds to the lowest 8 bits of opcode.
// x corresponds to the lower 4 bits of the high byte of the opcode.
// Conditional jump based on test
void Chip8::OP_3xkk() {
  // take the lower 4 bits of the high byte
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  // take the first
  uint8_t byte = opcode & 0x00FFu;

  // conditional jump if Vx == k
  if (registers[Vx] == byte)
    pc += 2;
}

// '4xkk': SNE Vx, byte
// Skip next instruction if Vx != kk, where Vx is a register
// kk corresponds to the lowest 8 bits of opcode.
// x corresponds to the lower 4 bits of the high byte of the opcode.
// Conditional jump based on test
void Chip8::OP_4xkk() {
  // take the lower 4 bits of the high byte
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  // take the first
  uint8_t byte = opcode & 0x00FFu;

  // conditional jump if Vx == k
  if (registers[Vx] != byte)
    pc += 2;
}

// '5xy0': SE Vx, Vy
// Skip next instruction if Vx == Vy
// x corresponds to the lower 4 bits of the high byte of the opcode.
// y corresponds to the upper 4 bits of the low byte of the opcode.
void Chip8::OP_5xy0() {
  // take the lower 4 bits of the high byte
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  // take the upper 4 bits of the low byte
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  // conditional jump if Vx == Vy
  if (registers[Vx] == registers[Vy])
    pc += 2;
}

// '6xkk': LD Vx, byte
// Set Vx = kk
// x corresponds to the lower 4 bits of the high byte of the opcode.
// kk corresponds to the lowest 8 bits of the opcode.
void Chip8::OP_6xkk() {
  // take the lowest 8 bits
  uint8_t byte = (opcode & 0x00FFu);
  // take the lower 4 bits of the high byte
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  // load value to register
  registers[Vx] = byte;
}

// '7xkk': ADD Vx, byte
void Chip8::OP_7xkk() {
  // take the lowest 8 bits
  uint8_t byte = (opcode & 0x00FFu);
  // take the lower 4 bits of the high byte
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  // add value to register
  registers[Vx] += byte;
}

// '8xy0': LD Vx, Vy
// Set Vx = Vy
void Chip8::OP_8xy0() {
  // take the lower 4 bits of the high byte
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  // take the upper 4 bits of the low byte
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  registers[Vx] = registers[Vy];
}

// '8xy1': OR Vx, Vy
// Set Vx = Vx OR Vy
void Chip8::OP_8xy1() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  registers[Vx] |= registers[Vy];
}

// '8xy2': AND Vx, Vy
// Set Vx = Vx AND Vy
void Chip8::OP_8xy2() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  registers[Vx] &= registers[Vy];
}

// '8xy3': XOR Vx, Vy
// Set Vx = Vx XOR Vy
void Chip8::OP_8xy3() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  registers[Vx] ^= registers[Vy];
}

// '8xy4': ADD Vx, Vy
void Chip8::OP_8xy4() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  uint16_t sum = registers[Vx] + registers[Vy];

  // Set VF to carry (if overflow 8 bits)
  registers[0xF] = (sum > 0xFFu);

  // put the lowest 8 bits in Vx
  registers[Vx] = (sum & 0xFFu);
}

// '8xy5': SUB Vx, Vy
void Chip8::OP_8xy5() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  // Set VF to carry (if overflow 8 bits)
  registers[0xF] = (registers[Vx] > registers[Vy]);

  // put the lowest 8 bits in Vx
  registers[Vx] -= registers[Vy];
}

// '8xy6': Set Vx = Vx SHR 1
void Chip8::OP_8xy6() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  // Save LSB in VF
  registers[0xF] = (registers[Vx] & 0x0001u);

  // Shift right by one
  registers[Vx] >>= 1;
}

// '8xy7': SUBN Vx, Vy
void Chip8::OP_8xy7() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  // Set VF to carry (if overflow 8 bits)
  registers[0xF] = (registers[Vy] > registers[Vx]);

  // put the lowest 8 bits in Vx
  registers[Vx] = registers[Vy] - registers[Vx];
}

// '8xyE': SHL Vx
void Chip8::OP_8xyE() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  // Save MSB to VF
  registers[0xF] = (registers[Vx] & 0x0080u);

  // Shift left by one
  registers[Vx] <<= 1;
}

// '9xy0': SNE Vx, Vy
// Conditional jump: skip next instruction if Vx != Vy
void Chip8::OP_9xy0() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  // conditional jump
  if (registers[Vx] != registers[Vy]) {
    pc += 2;
  }
}

// 'Annn': LD I, addr
// Set I = nnn
void Chip8::OP_Annn() {
  uint16_t nnn = (opcode & 0x0FFFu);

  index = nnn;
}

// 'Bnnn': JP V0, addr
// Jump to location nnn + V0
void Chip8::OP_Bnnn() {
  uint16_t nnn = (opcode & 0x0FFFu);

  pc = nnn + registers[0];
}

// 'Cxkk': RND Vx, byte
// Set Vx = random byte AND kk
void Chip8::OP_Cxkk() {
  uint8_t kk = (opcode & 0x000Fu);
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  registers[Vx] = randByte(randGen) & kk;
}

// 'Dxyn': DRW Vx, Vy, nibble
// Display n-byte sprite starting at memory location I at (Vx, Vy), set VF =
// collision
// A sprite is 8-pixel wide
// Thus, there are 8 columns (width) to have 64 pixels width
void Chip8::OP_Dxyn() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;
  uint8_t height = (opcode & 0x000Fu);

  // Wrap if going beyond display boundaries
  uint8_t xPos = registers[Vx] % DISPLAY_WIDTH;
  uint8_t yPos = registers[Vy] % DISPLAY_HEIGHT;

  registers[0xF] = 0;

  for (uint8_t row = 0; row < height; row++) {
    uint8_t spriteByte = memory[index + row];
    for (uint8_t col = 0; col < 8; col++) {
      uint8_t spritePixel = (spriteByte & (0x80u >> col));
      uint32_t *videoPixel = &video[(yPos + row) * DISPLAY_WIDTH + xPos + col];
      if (spritePixel) {
        // collision
        if (*videoPixel == 0xFFFFFFFF) {
          registers[0xF] = 1;
        }
        // videoPixel XOR spritePixel
        *videoPixel ^= 0xFFFFFFFF;
      }
    }
  }
}
