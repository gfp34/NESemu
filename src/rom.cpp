#include "rom.h"

#include <fstream>

#define HEADER_SIZE 16
#define TRAINER_SIZE 512
#define PRG_ROM_PAGE_SIZE 16384
#define CHR_ROM_PAGE_SIZE 8192
#define PRG_ROM_ADDR 0x8000



ROM::ROM(const char* filename) {
    std::ifstream rom_file(filename, std::ios::binary);

    header = new uint8_t[HEADER_SIZE];
    rom_file.read((char*)header, HEADER_SIZE);

    prg_rom_pages = new uint8_t*[header[4]];
    for(int i=0; i<header[4]; i++) {
        prg_rom_pages[i] = new uint8_t[PRG_ROM_PAGE_SIZE];
        rom_file.read((char*)prg_rom_pages[i], PRG_ROM_PAGE_SIZE);
    }

    int chr_rom_size = header[5] * CHR_ROM_PAGE_SIZE;
    chr_rom = new uint8_t[chr_rom_size];
    rom_file.read((char*)chr_rom, chr_rom_size);

}

uint8_t* ROM::get_prg_rom_lo(uint16_t addr) {
    return &prg_rom_pages[0][addr];
}

uint8_t* ROM::get_prg_rom_hi(uint16_t addr) {
    return &prg_rom_pages[header[4]-1][addr];
}

uint8_t* ROM::get_chr_rom(uint16_t addr) {
    return &chr_rom[addr];
}
