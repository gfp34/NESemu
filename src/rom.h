#ifndef NESEMU_ROM_H
#define NESEMU_ROM_H

#include <cstdint>

class ROM {
private:
    uint8_t* header;
//    uint8_t* trainer;
    uint8_t** prg_rom_pages;
    uint8_t* chr_rom;

public:
    ROM(const char* filename);

    uint8_t* get_prg_rom_lo(uint16_t addr);
    uint8_t* get_prg_rom_hi(uint16_t addr);
    uint8_t* get_chr_rom(uint16_t addr);
};


#endif //NESEMU_ROM_H
