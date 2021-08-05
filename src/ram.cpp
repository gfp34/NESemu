#include "ram.h"

#define RAM_SIZE 2048

RAM::RAM() {
    memory = new uint8_t[RAM_SIZE];
}

RAM::~RAM() {
    delete[] memory;
}

uint8_t* RAM::get_ram(uint16_t addr) {
    return &memory[addr];
}