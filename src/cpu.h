#ifndef NESEMU_CPU_H
#define NESEMU_CPU_H

#include <cstdint>
#include <string>

#include "ram.h"
#include "rom.h"

uint16_t fix_endian(uint8_t* bin);

class CPU {
private:
    uint8_t a;      // Accumulator
    uint8_t x;      // Register X
    uint8_t y;      // Register Y
    uint16_t pc;    // Program Counter
    uint8_t sp;     // Stack Pointer

    /**
     * 7	N	Negative	Compare: Set if the register's value is less than the input value
     *                      Otherwise: Set if the result was negative, i.e. bit 7 of the result was set
     * 6	V	Overflow	Arithmetic: Set if a signed overflow occurred during addition or subtraction, i.e. the sign
     *                          of the result differs from the sign of both the input and the accumulator
     *                      BIT: Set to bit 6 of the input
     * 5	-	(Unused)	Always set
     * 4	B   Break	    Set if an interrupt request has been triggered by a BRK instruction
     * 3	D	Decimal	    Decimal mode: mathematical instructions will treat the inputs and outputs as decimal numbers.
     *                      E.g. $09 + $01 = $10
     * 2    I	Interrupt Disable	Disables interrupts while set
     * 1	Z	Zero        Compare: Set if the register's value is equal to the input value
     *                      BIT: Set if the result of logically ANDing the accumulator with the input results in 0
     *                      Otherwise: Set if result was zero
     * 0	C	Carry	    Carry/Borrow flag used in math and rotate operations
     *                      Arithmetic: Set if an unsigned overflow occurred during addition or subtraction, i.e. the
     *                          result is less than the initial value
     *                      Compare: Set if register's value is greater than or equal to the input value Shifting: Set
     *                          to the value of the eliminated bit of the input, i.e. bit 7 when shifting left, or bit 0 when shifting right
     */
    union status {
        uint8_t sr;
        struct flag {
            uint8_t c:1;   // Carry (low order bit)
            uint8_t z:1;   // Zero
            uint8_t i:1;   // Interrupt
            uint8_t d:1;   // Decimal
            uint8_t b:1;   // Break
            uint8_t u:1;   // Unused
            uint8_t v:1;   // Overflow
            uint8_t n:1;   // Negative (high order bit)
        } flag;
    } status;

    RAM& ram;
    ROM& rom;
    /** TODO: Move when I figure out where these should actually go */
    uint8_t* ppu_reg;
    uint8_t* apu_io_reg;
    uint8_t* apu_io_test;
    uint8_t* cart_space;

    typedef struct inst_info {
        char inst_name[4];
        int inst_size;
    } InstInfo;

    typedef struct cpu_state {
    	uint8_t a;
    	uint8_t x;
    	uint8_t y;
    	uint16_t pc;
    	uint8_t sp;
    	uint8_t sr;
    } CPUState;

	void log(InstInfo info, CPUState state);
    uint8_t* access_mem(uint16_t addr);
    uint16_t load_address(uint16_t addr);
    InstInfo exec_inst(uint8_t* inst);
    CPUState save_cpu_state();

    /** CPU INSTRUCTIONS */
    InstInfo adc(uint8_t* inst);
    InstInfo and_(uint8_t* inst);
    InstInfo asl(uint8_t* inst);
    InstInfo bcc(uint8_t* inst);
    InstInfo bcs(uint8_t* inst);
    InstInfo beq(uint8_t* inst);
    InstInfo bit(uint8_t* inst);
    InstInfo bmi(uint8_t* inst);
    InstInfo bne(uint8_t* inst);
    InstInfo bpl(uint8_t* inst);
    InstInfo brk(uint8_t* inst);
    InstInfo bvc(uint8_t* inst);
    InstInfo bvs(uint8_t* inst);
    InstInfo clc(uint8_t* inst);
    InstInfo cld(uint8_t* inst);
    InstInfo cli(uint8_t* inst);
    InstInfo clv(uint8_t* inst);
    InstInfo cmp(uint8_t* inst);
    InstInfo cpx(uint8_t* inst);
    InstInfo cpy(uint8_t* inst);
    InstInfo dec(uint8_t* inst);
    InstInfo dex(uint8_t* inst);
    InstInfo dey(uint8_t* inst);
    InstInfo eor(uint8_t* inst);
    InstInfo inc(uint8_t* inst);
    InstInfo inx(uint8_t* inst);
    InstInfo iny(uint8_t* inst);
    InstInfo jmp(uint8_t* inst);
    InstInfo jsr(uint8_t* inst);
    InstInfo lda(uint8_t* inst);
    InstInfo ldx(uint8_t* inst);
    InstInfo ldy(uint8_t* inst);
    InstInfo lsr(uint8_t* inst);
    InstInfo nop(uint8_t* inst);
    InstInfo ora(uint8_t* inst);
    InstInfo pha(uint8_t* inst);
    InstInfo php(uint8_t* inst);
    InstInfo pla(uint8_t* inst);
    InstInfo plp(uint8_t* inst);
    InstInfo rol(uint8_t* inst);
    InstInfo ror(uint8_t* inst);
    InstInfo rti(uint8_t* inst);
    InstInfo rts(uint8_t* inst);
    InstInfo sbc(uint8_t* inst);
    InstInfo sec(uint8_t* inst);
    InstInfo sed(uint8_t* inst);
    InstInfo sei(uint8_t* inst);
    InstInfo sta(uint8_t* inst);
    InstInfo stx(uint8_t* inst);
    InstInfo sty(uint8_t* inst);
    InstInfo tax(uint8_t* inst);
    InstInfo tay(uint8_t* inst);
    InstInfo tsx(uint8_t* inst);
    InstInfo txa(uint8_t* inst);
    InstInfo txs(uint8_t* inst);
    InstInfo tya(uint8_t* inst);

    InstInfo ill_nop(uint8_t* inst);


public:
    CPU(RAM& ram, ROM& rom);

	void run();
};


#endif //NESEMU_CPU_H
