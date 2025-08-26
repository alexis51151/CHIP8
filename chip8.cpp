#include <cstdint>
#include <fstream>
#include <iosfwd>

// Useful constants for CHIP8
#define N_REG 16
#define MEM_B 4096
#define N_KEYS 16
#define VIDEO_WIDTH 64
#define VIDEO_HEIGHT 32
#define START_ADDRESS 0x200

class Chip8 {
  public:
    // 16 8-bit registers
    uint8_t registers[N_REG]{};
    // 4KB of memory
    uint8_t memory[MEM_B]{};
    // 16-bit index register (used for OPs)
    uint16_t index{};
    // 16-bit program counter
    uint16_t pc{};
    // 8-bit time clock
    uint8_t delayTimer{};
    // 8-bit sound clock
    uint16_t soundTimer{};
    // 16 input keys 
    uint16_t keypad[N_KEYS]{};
    // display screen
    uint8_t video[VIDEO_WIDTH * VIDEO_HEIGHT]{};
    // current OP code
    uint16_t opcode;

  public:
    Chip8();
    void LoadROM(char const* filename);
};

// Constructor: initialize the VM
Chip8::Chip8() {
  // Initialize PC to section 3 (start for ROM)
  pc = START_ADDRESS;
}

// Load ROM into VM
void Chip8::LoadROM(char const* filename) {
  // Open file in binary mode and move fp to the end
  std::ifstream file(filename, std::ios::binary | std::ios::ate);

  if (file.is_open()) {
    // Get file size and allocate buffer
    std::streampos size=  file.tellg();
    char* buffer = new char[size];

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

