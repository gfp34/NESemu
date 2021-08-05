#include "cpu.h"

#include <iostream>
#include <cstring>

#define PC_INIT_ADDR 0xC000
#define REG_INIT 0x00
#define STACK_INIT 0xFD
#define STATUS_INIT 0x24


uint16_t fix_endian(uint8_t* bin) {
    return (bin[1] << 8) + bin[0];
}

uint8_t sign_bit(uint8_t byte) {
    return (byte >> 7) & 0x01;
}

CPU::CPU(RAM& ram, ROM& rom): ram(ram), rom(rom){
    ppu_reg = new uint8_t[0x0008];
    apu_io_reg = new uint8_t[0x0018];
    apu_io_test = new uint8_t[0x0008];
    cart_space = new uint8_t[0xBFE0];

    pc = PC_INIT_ADDR;
    a = REG_INIT;
    x = REG_INIT;
    y = REG_INIT;
    sp = STACK_INIT;
    status.sr = STATUS_INIT;
}

void CPU::run() {
    InstInfo info;
	do {
		CPUState state = save_cpu_state();
        info = exec_inst(access_mem(pc));
        log(info, state);
    } while(strcmp(info.inst_name, "BAD") != 0);
}

void CPU::log(InstInfo info, CPUState state) {
	printf("%04X  ", state.pc);
	for(int i=0; i<3; i++) {
		if(i<info.inst_size) {
			printf("%02X ", access_mem(state.pc)[i]);
		} else {
			printf("   ");
		}
	}
	printf(" %s  ", info.inst_name);
	printf("A:%02X X:%02X Y:%02X P:%02X SP:%02X",
		   state.a, state.x, state.y, state.sr, state.sp);
	printf("\n");
}

uint8_t* CPU::access_mem(uint16_t addr) {
    if(0x0000 <= addr && addr <= 0x1FFF) {
        addr %= 0x0800;
        return ram.get_ram(addr);
    } else if(0x2000 <= addr && addr <= 0x3FFF) {
        addr %= 0x0008;
        return &ppu_reg[addr];
    } else if(0x4000 <= addr && addr <= 0x4017) {
        return &apu_io_reg[addr - 0x4000];
    } else if(0x4018 <= addr && addr <= 0x401F) {
        return &apu_io_reg[addr - 0x4018];
    } else if(0x8000 <= addr && addr <= 0xBFFF) {
        return rom.get_prg_rom_lo(addr - 0x8000);
    } else if(0xC000 <= addr && addr <= 0xFFFF) {
        return rom.get_prg_rom_hi(addr - 0xC000);
    } else {
            return &cart_space[addr - 0x4020];
    }
}

uint16_t CPU::load_address(uint16_t addr) {
	uint16_t ld_addr = *access_mem((addr & 0xFF00) + ((addr+1) & 0xFF)) << 8;
	ld_addr += *access_mem(addr);
	return ld_addr;
}

CPU::InstInfo CPU::exec_inst(uint8_t* inst) {
    switch(inst[0]) {
        case 0x69: case 0x65: case 0x75: case 0x6D:
        case 0x7D: case 0x79: case 0x61: case 0x71:
			return adc(inst);
        case 0x29: case 0x25: case 0x35: case 0x2D:
        case 0x3D: case 0x39: case 0x21: case 0x31:
			return and_(inst);
        case 0x0A: case 0x06: case 0x16:
        case 0x0E: case 0x1E:
			return asl(inst);
        case 0x90:
			return bcc(inst);
        case 0xB0:
			return bcs(inst);
        case 0xF0:
			return beq(inst);
        case 0x24: case 0x2C:
			return bit(inst);
        case 0x30:
			return bmi(inst);
        case 0xD0:
			return bne(inst);
        case 0x10:
			return bpl(inst);
        case 0x00:
			return brk(inst);
        case 0x50:
			return bvc(inst);
        case 0x70:
			return bvs(inst);
        case 0x18:
			return clc(inst);
        case 0xD8:
			return cld(inst);
        case 0x58:
			return cli(inst);
        case 0xB8:
			return clv(inst);
        case 0xC9: case 0xC5: case 0xD5: case 0xCD:
        case 0xDD: case 0xD9: case 0xC1: case 0xD1:
			return cmp(inst);
        case 0xE0: case 0xE4: case 0xEC:
			return cpx(inst);
        case 0xC0: case 0xC4: case 0xCC:
			return cpy(inst);
        case 0xC6: case 0xD6: case 0xCE: case 0xDE:
			return dec(inst);
        case 0xCA:
			return dex(inst);
        case 0x88:
			return dey(inst);
        case 0x49: case 0x45: case 0x55: case 0x4D:
        case 0x5D: case 0x59: case 0x41: case 0x51:
			return eor(inst);
        case 0xE6: case 0xF6: case 0xEE: case 0xFE:
			return inc(inst);
        case 0xE8:
			return inx(inst);
        case 0xC8:
			return iny(inst);
        case 0x4C: case 0x6C:
			return jmp(inst);
        case 0x20:
			return jsr(inst);
        case 0xA9: case 0xA5: case 0xB5: case 0xAD:
        case 0xBD: case 0xB9: case 0xA1: case 0xB1:
			return lda(inst);
        case 0xA2: case 0xA6: case 0xB6: case 0xAE: case 0xBE:
			return ldx(inst);
        case 0xA0: case 0xA4: case 0xB4: case 0xAC: case 0xBC:
			return ldy(inst);
        case 0x4A: case 0x46: case 0x56: case 0x4E: case 0x5E:
			return lsr(inst);
        case 0xEA:
			return nop(inst);
        case 0x09: case 0x05: case 0x15: case 0x0D:
        case 0x1D: case 0x19: case 0x01: case 0x11:
			return ora(inst);
        case 0x48:
			return pha(inst);
        case 0x08:
			return php(inst);
        case 0x68:
			return pla(inst);
        case 0x28:
			return plp(inst);
        case 0x2A: case 0x26: case 0x36: case 0x2E: case 0x3E:
			return rol(inst);
        case 0x6A: case 0x66: case 0x76: case 0x6E: case 0x7E:
			return ror(inst);
        case 0x40:
			return rti(inst);
        case 0x60:
			return rts(inst);
        case 0xE9: case 0xE5: case 0xF5: case 0xED:
        case 0xFD: case 0xF9: case 0xE1: case 0xF1:
			return sbc(inst);
        case 0x38:
			return sec(inst);
        case 0xF8:
			return sed(inst);
        case 0x78:
			return sei(inst);
        case 0x85: case 0x95: case 0x8D: case 0x9D:
        case 0x99: case 0x81: case 0x91:
			return sta(inst);
        case 0x86: case 0x96: case 0x8E:
			return stx(inst);
        case 0x84: case 0x94: case 0x8C:
			return sty(inst);
        case 0xAA:
			return tax(inst);
        case 0xA8:
			return tay(inst);
        case 0xBA:
			return tsx(inst);
        case 0x8A:
			return txa(inst);
        case 0x9A:
			return txs(inst);
        case 0x98:
			return tya(inst);
		case 0x1A: case 0x3A: case 0x5A: case 0x7A: case 0xDA:
		case 0xFA: case 0x80: case 0x82: case 0x89: case 0xC2:
		case 0xE2: case 0x04: case 0x44: case 0x64: case 0x14:
		case 0x34: case 0x54: case 0x74: case 0xD4: case 0xF4:
		case 0x0C: case 0x1C: case 0x3C: case 0x5C: case 0x7C:
		case 0xDC: case 0xFC:
			return ill_nop(inst);
    }
    InstInfo null_info = {"BAD", 1};
    return null_info;
}

CPU::CPUState CPU::save_cpu_state() {
	return {a, x, y, pc, sp, status.sr};
}

CPU::InstInfo CPU::adc(uint8_t* inst) {
    InstInfo info = {"ADC", 2};
    uint8_t op1 = a;
    uint8_t op2;
    switch(inst[0]) {
        case 0x69: // Immediate
            op2 = inst[1];
            break;
        case 0x65: // Zero Page
            op2 = *access_mem(inst[1]);
            break;
        case 0x75: // Zero Page, X
            op2 = *access_mem((uint8_t)(inst[1] + x));
            break;
        case 0x6D: // Absolute
            op2 = *access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
        case 0x7D: // Absolute, X
            op2 = *access_mem(fix_endian(&inst[1]) + x);
            info.inst_size = 3;
            break;
        case 0x79: // Absolute, Y
            op2 = *access_mem(fix_endian(&inst[1]) + y);
            info.inst_size = 3;
            break;
        case 0x61: // (Indirect, x)
            op2 = *access_mem(load_address((uint8_t)(inst[1] + x)));
            break;
        case 0x71: // (Indirect), Y
            op2 = *access_mem(load_address((uint8_t)inst[1]) + y);
            break;
    }
    uint16_t tmp = op1 + op2 + status.flag.c;
    a = (uint8_t) tmp;
    pc += info.inst_size;
    status.flag.c = tmp >> 8;
    status.flag.z = a == 0;
    status.flag.v = sign_bit(op1) == sign_bit(op2) ? sign_bit(a) != sign_bit(op1) : 0;
    status.flag.n = sign_bit(a);
    return info;
}

CPU::InstInfo CPU::and_(uint8_t* inst) {
    InstInfo info = {"AND", 2};
    uint8_t op1 = a;
    uint8_t op2;
    switch(inst[0]) {
        case 0x29: // Immediate
            op2 = inst[1];
            break;
        case 0x25: // Zero Page
            op2 = *access_mem(inst[1]);
            break;
        case 0x35: // Zero Page, X
            op2 = *access_mem((uint8_t)(inst[1] + x));
            break;
        case 0x2D: // Absolute
            op2 = *access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
        case 0x3D: // Absolute, X
            op2 = *access_mem(fix_endian(&inst[1]) + x);
            info.inst_size = 3;
            break;
        case 0x39: // Absolute, Y
            op2 = *access_mem(fix_endian(&inst[1]) + y);
            info.inst_size = 3;
            break;
        case 0x21: // (Indirect, X)
            op2 = *access_mem(load_address((uint8_t)(inst[1] + x)));
            break;
        case 0x31: // (Indirect), Y
            op2 = *access_mem(load_address((uint8_t)inst[1]) + y);
            break;
    }
    a = op1 & op2;
    pc += info.inst_size;
    status.flag.z = a == 0;
    status.flag.n = sign_bit(a);
    return info;
}

CPU::InstInfo CPU::asl(uint8_t* inst) {
    InstInfo info = {"ASL", 2};
    uint8_t* op;
    switch(inst[0]) {
        case 0x0A: // Accumulator
            op = &a;
            info.inst_size = 1;
            break;
        case 0x06: // Zero Page
            op = access_mem(inst[1]);
            break;
        case 0x16: // Zero Page X
            op = access_mem(inst[1] + x);
            break;
        case 0x0E: // Absolute
            op = access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
        case 0x1E: // Absolute, X
            op = access_mem(fix_endian(&inst[1]) + x);
            info.inst_size = 3;
            break;
    }
    pc += info.inst_size;
    status.flag.c = sign_bit(*op);
    *op = *op << 1;
    status.flag.z = *op == 0;
    status.flag.n = sign_bit(*op);
    return info;
}

CPU::InstInfo CPU::bcc(uint8_t* inst) {
	InstInfo info = {"BCC", 2};
	if(!status.flag.c)
		pc += (int8_t)inst[1];
	pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::bcs(uint8_t* inst) {
	InstInfo info = {"BCS", 2};
	if(status.flag.c)
		pc += (int8_t)inst[1];
	pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::beq(uint8_t* inst) {
	InstInfo info = {"BEQ", 2};
	if(status.flag.z)
		pc += (int8_t)inst[1];
	pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::bit(uint8_t* inst) {
    InstInfo info = {"BIT", 2};
    uint8_t op;
    switch(inst[0]) {
        case 0x24: // Zero Page
            op = *access_mem(inst[1]);
            break;
        case 0x2C: // Absolute
            op = *access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
    }
    pc += info.inst_size;
    status.flag.z = (a & op) == 0;
    status.flag.v = (op >> 6) & 0x01;
    status.flag.n = sign_bit(op);
    return info;
}

CPU::InstInfo CPU::bmi(uint8_t* inst) {
	InstInfo info = {"BMI", 2};
	if(status.flag.n)
		pc += (int8_t)inst[1];
	pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::bne(uint8_t* inst) {
	InstInfo info = {"BNE", 2};
	if(!status.flag.z)
		pc += (int8_t)inst[1];
	pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::bpl(uint8_t* inst) {
	InstInfo info = {"BPL", 2};
	if(!status.flag.n)
		pc += (int8_t)inst[1];
	pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::brk(uint8_t* inst) {
    InstInfo info = {"BRK", 1};
    pc += 1;
    *access_mem(0x0100 + sp) = (pc >> 8) & 0xFF;
    sp--;
    *access_mem(0x0100 + sp) = pc & 0xFF;
    sp--;
    *access_mem(0x0100 + sp) = status.sr | 0x10; // break flag is set to 1
    sp--;
    pc = fix_endian(access_mem(0xFFFE));
    status.flag.b = 1;
    return info;
}

CPU::InstInfo CPU::bvc(uint8_t* inst) {
	InstInfo info = {"BVC", 2};
	if(!status.flag.v)
		pc += (int8_t)inst[1];
	pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::bvs(uint8_t* inst) {
	InstInfo info = {"BVS", 2};
	if(status.flag.v)
		pc += (int8_t)inst[1];
	pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::clc(uint8_t* inst) {
    InstInfo info = {"CLC", 1};
    status.flag.c = 0;
    pc += info.inst_size;
    return info;
}

CPU::InstInfo CPU::cld(uint8_t* inst) {
    InstInfo info = {"CLD", 1};
    status.flag.d = 0;
    pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::cli(uint8_t* inst) {
	InstInfo info = {"CLI", 1};
    status.flag.i = 0;
    pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::clv(uint8_t* inst) {
	InstInfo info = {"CLV", 1};
    status.flag.v = 0;
    pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::cmp(uint8_t* inst) {
	InstInfo info = {"CMP", 2};
    uint8_t op;
    switch(inst[0]) {
        case 0xC9: // Immediate
            op = inst[1];
            break;
        case 0xC5: // Zero Page
            op = *access_mem(inst[1]);
            break;
        case 0xD5: // Zero Page, X
            op = *access_mem((uint8_t)(inst[1] + x));
            break;
        case 0xCD: // Absolute
            op = *access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
        case 0xDD: // Absolute, X
            op = *access_mem(fix_endian(&inst[1]) + x);
            info.inst_size = 3;
            break;
        case 0xD9: // Absolute, Y
            op = *access_mem(fix_endian(&inst[1]) + y);
            info.inst_size = 3;
            break;
        case 0xC1: // (Indirect, X)
            op = *access_mem(load_address((uint8_t)(inst[1] + x)));
            break;
        case 0xD1: // (Indirect), Y
            op = *access_mem(load_address((uint8_t)inst[1]) + y);
            break;
    }
    pc += info.inst_size;
    status.flag.c = a >= op;
    status.flag.z = a == op;
    status.flag.n = sign_bit(a - op);
	return info;
}

CPU::InstInfo CPU::cpx(uint8_t* inst) {
	InstInfo info = {"CPX", 2};
    uint8_t op;
    switch(inst[0]) {
        case 0xE0: // Immediate
            op = inst[1];
            break;
        case 0xE4: // Zero Page
            op = *access_mem(inst[1]);
            break;
        case 0xEC: // Absolute
            op = *access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
    }
    pc += info.inst_size;
    status.flag.c = x >= op;
    status.flag.z = x == op;
    status.flag.n = sign_bit(x - op);
	return info;
}

CPU::InstInfo CPU::cpy(uint8_t* inst) {
	InstInfo info = {"CPY", 2};
    uint8_t op;
    switch(inst[0]) {
        case 0xC0: // Immediate
            op = inst[1];
            break;
        case 0xC4: // Zero Page
            op = *access_mem(inst[1]);
            break;
        case 0xCC: // Absolute
            op = *access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
    }
    pc += info.inst_size;
    status.flag.c = y >= op;
    status.flag.z = y == op;
    status.flag.n = sign_bit(y - op);
	return info;
}

CPU::InstInfo CPU::dec(uint8_t* inst) {
	InstInfo info = {"DEC", 2};
    uint8_t* op;
    switch(inst[0]) {
        case 0xC6: // Zero Page
            op = access_mem(inst[1]);
            break;
        case 0xD6: // Zero Page, X
            op = access_mem((uint8_t)(inst[1] + x));
            break;
        case 0xCE: // Absolute
            op = access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
        case 0xDE: // Absolute, X
            op = access_mem(fix_endian(&inst[1]) + x);
           info.inst_size = 3;
            break;
    }
    (*op)--;
    pc += info.inst_size;
    status.flag.z = *op == 0;
    status.flag.n = sign_bit(*op);
	return info;
}

CPU::InstInfo CPU::dex(uint8_t* inst) {
	InstInfo info = {"DEX", 1};
    x--;
    pc += info.inst_size;
    status.flag.z = x == 0;
    status.flag.n = sign_bit(x);
	return info;
}

CPU::InstInfo CPU::dey(uint8_t* inst) {
	InstInfo info = {"DEY", 1};
    y--;
    pc += info.inst_size;
    status.flag.z = y == 0;
    status.flag.n = sign_bit(y);
	return info;
}

CPU::InstInfo CPU::eor(uint8_t* inst) {
	InstInfo info = {"EOR", 2};
    uint8_t op1 = a;
    uint8_t op2;
    switch(inst[0]) {
        case 0x49: // Immediate
            op2 = inst[1];
            break;
        case 0x45: // Zero Page
            op2 = *access_mem(inst[1]);
            break;
        case 0x55: // Zero Page, X
            op2 = *access_mem((uint8_t)(inst[1] + x));
            break;
        case 0x4D: // Absolute
            op2 = *access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
        case 0x5D: // Absolute, X
            op2 = *access_mem(fix_endian(&inst[1]) + x);
            info.inst_size = 3;
            break;
        case 0x59: // Absolute, Y
            op2 = *access_mem(fix_endian(&inst[1]) + y);
            info.inst_size = 3;
            break;
        case 0x41: // (Indirect, X)
            op2 = *access_mem(load_address((uint8_t)(inst[1] + x)));
            break;
        case 0x51: // (Indirect), Y
            op2 = *access_mem(load_address((uint8_t)inst[1]) + y);
            break;
    }
    a = op1 ^ op2;
    pc += info.inst_size;
    status.flag.z = a == 0;
    status.flag.n = sign_bit(a);
	return info;
}

CPU::InstInfo CPU::inc(uint8_t* inst) {
	InstInfo info = {"INC", 2};
    uint8_t* op;
    switch(inst[0]) {
        case 0xE6: // Zero Page
            op = access_mem(inst[1]);
            break;
        case 0xF6: // Zero Page, X
            op = access_mem((uint8_t)(inst[1] + x));
            break;
        case 0xEE: // Absolute
            op = access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
        case 0xFE: // Absolute, X
            op = access_mem(fix_endian(&inst[1]) + x);
            info.inst_size = 3;
            break;
    }
    (*op)++;
    pc += info.inst_size;
    status.flag.z = *op == 0;
    status.flag.n = sign_bit(*op);
	return info;
}

CPU::InstInfo CPU::inx(uint8_t* inst) {
	InstInfo info = {"INX", 1};
    x++;
    pc += info.inst_size;
    status.flag.z = x == 0;
    status.flag.n = sign_bit(x);
	return info;
}

CPU::InstInfo CPU::iny(uint8_t* inst) {
	InstInfo info = {"INY", 1};
    y++;
    pc += info.inst_size;
    status.flag.z = y == 0;
    status.flag.n = sign_bit(y);
	return info;
}

CPU::InstInfo CPU::jmp(uint8_t* inst) {
    /**
     * An original 6502 has does not correctly fetch the target address if the
     * indirect vector falls on a page boundary (e.g. $xxFF where xx is any value
     * from $00 to $FF). In this case fetches the LSB from $xxFF as expected but
     * takes the MSB from $xx00. This is fixed in some later chips like the 65SC02
     * so for compatibility always ensure the indirect vector is not at the end of
     * the page.
     */
	InstInfo info = {"JMP", 3};
    switch(inst[0]) {
        case 0x4C: // Absolute
            pc = fix_endian(&inst[1]);
            break;
        case 0x6C: // Indirect
        	pc = load_address(fix_endian(&inst[1]));
            break;
    }
	return info;
}

CPU::InstInfo CPU::jsr(uint8_t* inst) {
	InstInfo info = {"JSR", 3};
    pc += 2;
    *access_mem(0x0100 + sp) = (pc >> 8) & 0xFF;
    sp--;
    *access_mem(0x0100 + sp) = pc & 0xFF;
    sp--;
    pc = fix_endian(&inst[1]);
	return info;
}

CPU::InstInfo CPU::lda(uint8_t* inst) {
	InstInfo info = {"LDA", 2};
    uint8_t op;
    switch(inst[0]) {
        case 0xA9: // Immediate
            op = inst[1];
            break;
        case 0xA5: // Zero Page
            op = *access_mem(inst[1]);
            break;
        case 0xB5: // Zero Page, X
            op = *access_mem((uint8_t)(inst[1] + x));
            break;
        case 0xAD: // Absolute
            op = *access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
        case 0xBD: // Absolute, X
            op = *access_mem(fix_endian(&inst[1]) + x);
            info.inst_size = 3;
            break;
        case 0xB9: // Absolute, Y
            op = *access_mem(fix_endian(&inst[1]) + y);
            info.inst_size = 3;
            break;
        case 0xA1: // (Indirect, X)
            op = *access_mem(load_address((uint8_t)(inst[1] + x)));
            break;
        case 0xB1: // (Indirect), Y
            op = *access_mem(load_address((uint8_t)inst[1]) + y);
            break;
    }
    a = op;
    pc += info.inst_size;
    status.flag.z = a == 0;
    status.flag.n = sign_bit(a);
	return info;
}

CPU::InstInfo CPU::ldx(uint8_t* inst) {
	InstInfo info = {"LDX", 2};
    uint8_t op;
    switch(inst[0]) {
        case 0xA2: // Immediate
            op = inst[1];
            break;
        case 0xA6: // Zero Page
            op = *access_mem(inst[1]);
            break;
        case 0xB6: // Zero Page, Y
            op = *access_mem((uint8_t)(inst[1] + y));
            break;
        case 0xAE: // Absolute
            op = *access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
        case 0xBE: // Absolute, Y
            op = *access_mem(fix_endian(&inst[1]) + y);
            info.inst_size = 3;
            break;
    }
    x = op;
    pc += info.inst_size;
    status.flag.z = x == 0;
    status.flag.n = sign_bit(x);
	return info;
}

CPU::InstInfo CPU::ldy(uint8_t* inst) {
	InstInfo info = {"LDY", 2};
    uint8_t op;
    switch(inst[0]) {
        case 0xA0: // Immediate
            op = inst[1];
            break;
        case 0xA4: // Zero Page
            op = *access_mem(inst[1]);
            break;
        case 0xB4: // Zero Page, X
            op = *access_mem((uint8_t)(inst[1] + x));
            break;
        case 0xAC: // Absolute
            op = *access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
        case 0xBC: // Absolute, X
            op = *access_mem(fix_endian(&inst[1]) + x);
            info.inst_size = 3;
            break;
    }
    y = op;
    pc += info.inst_size;
    status.flag.z = y == 0;
    status.flag.n = sign_bit(y);
	return info;
}

CPU::InstInfo CPU::lsr(uint8_t* inst) {
	InstInfo info = {"LSR", 2};
    uint8_t* op;
    switch(inst[0]) {
        case 0x4A: // Accumulator
            op = &a;
            info.inst_size = 1;
            break;
        case 0x46: // Zero Page
            op = access_mem(inst[1]);
            break;
        case 0x56: // Zero Page X
            op = access_mem(inst[1] + x);
            break;
        case 0x4E: // Absolute
            op = access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
        case 0x5E: // Absolute, X
            op = access_mem(fix_endian(&inst[1]) + x);
            info.inst_size = 3;
            break;
    }
    pc += info.inst_size;
    status.flag.c = *op & 0x01;
    *op = (*op >> 1) & 0x7F;
    status.flag.z = *op == 0;
    status.flag.n = sign_bit(*op);
	return info;
}

CPU::InstInfo CPU::nop(uint8_t* inst) {
	InstInfo info = {"NOP", 1};
    pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::ora(uint8_t* inst) {
	InstInfo info = {"ORA", 2};
    uint8_t op1 = a;
    uint8_t op2;
    switch(inst[0]) {
        case 0x09: // Immediate
            op2 = inst[1];
            break;
        case 0x05: // Zero Page
            op2 = *access_mem(inst[1]);
            break;
        case 0x15: // Zero Page, X
            op2 = *access_mem((uint8_t)(inst[1] + x));
            break;
        case 0x0D: // Absolute
            op2 = *access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
        case 0x1D: // Absolute, X
            op2 = *access_mem(fix_endian(&inst[1]) + x);
            info.inst_size = 3;
            break;
        case 0x19: // Absolute, Y
            op2 = *access_mem(fix_endian(&inst[1]) + y);
            info.inst_size = 3;
            break;
        case 0x01: // (Indirect, X)
            op2 = *access_mem(load_address((uint8_t)(inst[1] + x)));
            break;
        case 0x11: // (Indirect), Y
            op2 = *access_mem(load_address((uint8_t)inst[1]) + y);
            break;
    }
    a = op1 | op2;
    pc += info.inst_size;
    status.flag.z = a == 0;
    status.flag.n = sign_bit(a);
	return info;
}

CPU::InstInfo CPU::pha(uint8_t* inst) {
	InstInfo info = {"PHA", 1};
    pc += info.inst_size;
    *access_mem(0x0100 + sp) = a;
    sp--;
	return info;
}

CPU::InstInfo CPU::php(uint8_t* inst) {
	InstInfo info = {"PHP", 1};
    pc += info.inst_size;
    *access_mem(0x0100 + sp) = status.sr | 0x10; // break flag is set to 1
    sp--;
	return info;
}

CPU::InstInfo CPU::pla(uint8_t* inst) {
	InstInfo info = {"PLA", 1};
    pc += info.inst_size;
    sp++;
    a = *access_mem(0x0100 + sp);
    status.flag.z = a == 0;
    status.flag.n = sign_bit(a);
	return info;
}

CPU::InstInfo CPU::plp(uint8_t* inst) {
	InstInfo info = {"PLP", 1};
    pc += info.inst_size;
    sp++;
    status.sr = *access_mem(0x0100 + sp);
    status.flag.b = 0; // Clear break flag
    status.flag.u = 1; // Reset unused flag;
	return info;
}

CPU::InstInfo CPU::rol(uint8_t* inst) {
	InstInfo info = {"ROL", 2};
    uint8_t* op;
    switch(inst[0]) {
        case 0x2A: // Accumulator
            op = &a;
            info.inst_size = 1;
            break;
        case 0x26: // Zero Page
            op = access_mem(inst[1]);
            break;
        case 0x36: // Zero Page X
            op = access_mem(inst[1] + x);
            break;
        case 0x2E: // Absolute
            op = access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
        case 0x3E: // Absolute, X
            op = access_mem(fix_endian(&inst[1]) + x);
            info.inst_size = 3;
            break;
    }
    uint8_t old_bit7 = sign_bit(*op);
    *op = (*op << 1) + status.flag.c;
    pc += info.inst_size;
    status.flag.c = old_bit7;
    status.flag.z = a == 0;
    status.flag.n = sign_bit(*op);
	return info;
}

CPU::InstInfo CPU::ror(uint8_t* inst) {
	InstInfo info = {"ROR", 2};
    uint8_t* op;
    switch(inst[0]) {
        case 0x6A: // Accumulator
            op = &a;
            info.inst_size = 1;
            break;
        case 0x66: // Zero Page
            op = access_mem(inst[1]);
            break;
        case 0x76: // Zero Page X
            op = access_mem(inst[1] + x);
            break;
        case 0x6E: // Absolute
            op = access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
        case 0x7E: // Absolute, X
            op = access_mem(fix_endian(&inst[1]) + x);
            info.inst_size = 3;
            break;
    }
    uint8_t old_bit0 = *op & 0x01;
    *op = ((*op >> 1) & 0x7F) + (uint8_t)(status.flag.c << 7);
    pc += info.inst_size;
    status.flag.c = old_bit0;
    status.flag.z = a == 0;
    status.flag.n = sign_bit(*op);
	return info;
}

CPU::InstInfo CPU::rti(uint8_t* inst) {
	InstInfo info = {"RTI", 1};
    sp++;
    status.sr = *access_mem(0x0100 + sp);
    sp++;
    pc = fix_endian(access_mem(0x0100 + sp));
    sp++;
	status.flag.b = 0; // Clear break flag
	status.flag.u = 1; // Reset unused flag;
	return info;
}

CPU::InstInfo CPU::rts(uint8_t* inst) {
	InstInfo info = {"RTS", 1};
    sp++;
    pc = fix_endian(access_mem(0x0100 + sp));
    sp++;
    pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::sbc(uint8_t* inst) {
	InstInfo info = {"SBC", 2};
    uint8_t op1 = a;
    uint8_t op2;
    switch(inst[0]) {
        case 0xE9: // Immediate
            op2 = inst[1];
            break;
        case 0xE5: // Zero Page
            op2 = *access_mem(inst[1]);
            break;
        case 0xF5: // Zero Page, X
            op2 = *access_mem((uint8_t)(inst[1] + x));
            break;
        case 0xED: // Absolute
            op2 = *access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
        case 0xFD: // Absolute, X
            op2 = *access_mem(fix_endian(&inst[1]) + x);
            info.inst_size = 3;
            break;
        case 0xF9: // Absolute, Y
            op2 = *access_mem(fix_endian(&inst[1]) + y);
            info.inst_size = 3;
            break;
        case 0xE1: // (Indirect, x)
            op2 = *access_mem(load_address((uint8_t)(inst[1] + x)));
            break;
        case 0xF1: // (Indirect), Y
            op2 = *access_mem(load_address((uint8_t)inst[1]) + y);
            break;
    }
    uint16_t tmp = op1 - op2 - (1-status.flag.c);
    a = (uint8_t) tmp;
    pc += info.inst_size;
    status.flag.c = !sign_bit(a);
    status.flag.z = a == 0;
    status.flag.v = sign_bit(op1) != sign_bit(op2) && sign_bit(op1) != sign_bit(a);
    status.flag.n = sign_bit(a);
	return info;
}

CPU::InstInfo CPU::sec(uint8_t* inst) {
	InstInfo info = {"SEC", 1};
    status.flag.c = 1;
    pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::sed(uint8_t* inst) {
	InstInfo info = {"SED", 1};
    status.flag.d = 1;
    pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::sei(uint8_t* inst) {
	InstInfo info = {"SEI", 1};
    status.flag.i = 1;
    pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::sta(uint8_t* inst) {
	InstInfo info = {"STA", 2};
    uint8_t* op;
    switch(inst[0]) {
        case 0x85: // Zero Page
            op = access_mem(inst[1]);
            break;
        case 0x95: // Zero Page, X
            op = access_mem((uint8_t)(inst[1] + x));
            break;
        case 0x8D: // Absolute
            op = access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
        case 0x9D: // Absolute, X
            op = access_mem(fix_endian(&inst[1]) + x);
            info.inst_size = 3;
            break;
        case 0x99: // Absolute, Y
            op = access_mem(fix_endian(&inst[1]) + y);
            info.inst_size = 3;
            break;
        case 0x81: // (Indirect, X)
            op = access_mem(load_address((uint8_t)(inst[1] + x)));
            break;
        case 0x91: // (Indirect), Y
            op = access_mem(load_address((uint8_t)inst[1]) + y);
            break;
    }
    *op = a;
    pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::stx(uint8_t* inst) {
	InstInfo info = {"STX", 2};
    uint8_t* op;
    switch(inst[0]) {
        case 0x86: // Zero Page
            op = access_mem(inst[1]);
            break;
        case 0x96: // Zero Page, Y
            op = access_mem((uint8_t)(inst[1] + y));
            break;
        case 0x8E: // Absolute
            op = access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
    }
    *op = x;
    pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::sty(uint8_t* inst) {
	InstInfo info = {"STY", 2};
    uint8_t* op;
    switch(inst[0]) {
        case 0x84: // Zero Page
            op = access_mem(inst[1]);
            break;
        case 0x94: // Zero Page, X
            op = access_mem((uint8_t)(inst[1] + x));
            break;
        case 0x8C: // Absolute
            op = access_mem(fix_endian(&inst[1]));
            info.inst_size = 3;
            break;
    }
    *op = y;
    pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::tax(uint8_t* inst) {
	InstInfo info = {"TAX", 1};
    x = a;
    pc += info.inst_size;
    status.flag.z = x == 0;
    status.flag.n = sign_bit(x);
	return info;
}

CPU::InstInfo CPU::tay(uint8_t* inst) {
	InstInfo info = {"TAY", 1};
    y = a;
    pc += info.inst_size;
    status.flag.z = y == 0;
    status.flag.n = sign_bit(y);
	return info;
}

CPU::InstInfo CPU::tsx(uint8_t* inst) {
	InstInfo info = {"TSX", 1};
    x = sp;
    pc += info.inst_size;
    status.flag.z = x == 0;
    status.flag.n = sign_bit(x);
	return info;
}

CPU::InstInfo CPU::txa(uint8_t* inst) {
	InstInfo info = {"TXA", 1};
    a = x;
    pc += info.inst_size;
    status.flag.z = a == 0;
    status.flag.n = sign_bit(a);
	return info;
}

CPU::InstInfo CPU::txs(uint8_t* inst) {
	InstInfo info = {"TXS", 1};
    sp = x;
    pc += info.inst_size;
	return info;
}

CPU::InstInfo CPU::tya(uint8_t* inst) {
	InstInfo info = {"TYA", 1};
    a = y;
    pc += info.inst_size;
    status.flag.z = a == 0;
    status.flag.n = sign_bit(a);
	return info;
}

CPU::InstInfo CPU::ill_nop(uint8_t* inst) {
	InstInfo info = {"NOP", 1};
	switch (inst[0]) {
		case 0x1A: case 0x3A: case 0x5A: case 0x7A: case 0xDA: case 0xFA:
			break;
		case 0x80: case 0x82: case 0x89: case 0xC2: case 0xE2:
		case 0x04: case 0x44: case 0x64: case 0x14: case 0x34:
		case 0x54: case 0x74: case 0xD4: case 0xF4:
			info.inst_size = 2;
			break;
		case 0x0C: case 0x1C: case 0x3C: case 0x5C: case 0x7C:
		case 0xDC: case 0xFC:
			info.inst_size = 3;
			break;
	}
	pc += info.inst_size;
	return info;
}
