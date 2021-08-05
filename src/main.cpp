#include <iostream>

#include "cpu.h"
#include "ram.h"

using namespace std; 

int main() {
    ROM rom("nestest.nes");
    RAM ram;
    CPU cpu(ram, rom);

    cpu.run();

    return 0;
}
