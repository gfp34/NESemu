#ifndef NESEMU_RAM_H
#define NESEMU_RAM_H

#include <cstdint>

class RAM {
private:
    uint8_t* memory;

public:
    RAM();
    ~RAM();

    uint8_t* get_ram(uint16_t addr);
//    void store(uint16_t addr, uint8_t* buf, size_t size);
};


#endif //NESEMU_RAM_H
