# CHIP8
Emulation of a CHIP-8 microprocessor.


## Specification

Below are the main characteristics of CHIP-8

- 16 8-bit Registers (labeled from V0 to VF, can hold values from 0x00 to 0xFF)
- 4KB of Memory (4096 bytes with addresses from 0x000 to 0xFFF)
    - Section 1 (0x000-0x1FF): reserved for CHIP-8 interpreter
    - Section 2 (0x050-0x0A0): storage for 16 built-in chars (0 to F)
    - Section 3 (0x200-0xFFF): ROM's space to use
- 16-bit Index Register (stores address for OPs)
- 16-bit Program Counter (next instruction)
- 16-level Stack (i.e. at most 16 PCs for recursion depth)
- 8-bit Stack Pointer (top of the stack)
- 8-bit Delay Timer (count time at 60Hz when non-zero)
- 8-bit Sound Timer (count time at 60Hz and buzzes when non-zero)
- 16 Input Keys (0 to F either pressed or not pressed)
- 64x32 Monochrome Display Memory (64 pixels wide and 32 pixels high)
    - Pixel ON: 0xFFFFFFFF
    - Pixel OFF: 0x00000000

## Instruction Section



## References
- [Building a CHIP-8 Emulator (C++)](https://austinmorlan.com/posts/chip8_emulator/)


