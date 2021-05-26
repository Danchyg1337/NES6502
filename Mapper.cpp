#pragma once
#include "Mapper.h"

void Mapper::LoadPRG(uint8_t* prg, size_t size) {
	PRGROM.resize(size);
	memcpy(PRGROM.data(), prg, size);
}

void Mapper::LoadCHR(uint8_t* chr, size_t size) {
	CHRROM.resize(size);
	memcpy(CHRROM.data(), chr, size);
}

void Mapper::Write(uint16_t addr, uint8_t value) {
	//
}

uint8_t& Mapper::CPURead(uint16_t addr) {
	if (addr >= 0x8000 && addr <= 0xFFFF) {
		return PRGROM[(addr - 0x8000) % PRGROM.size()];
	}
}

uint8_t& Mapper::PPURead(uint16_t addr) {
	if (addr < 0x2000) {
		uint32_t finalAddr = CHRBank * 0x2000 + addr;
		if (CHRROM.size() <= finalAddr) return  PRGROM[0];
		return CHRROM[finalAddr];
	}
}

void Mapper::PPUWrite(uint16_t addr, uint8_t value) {
	if (addr < 0x2000) {
		uint32_t finalAddr = CHRBank * 0x2000 + addr;
		if (finalAddr >= CHRROM.size()) {
			CHRROM.resize(CHRROM.size() + 0x2000);
			PPUWrite(addr, value);
			return;
		}
		CHRROM[CHRBank * 0x2000 + addr] = value;
	}
}

void UXROM::Write(uint16_t addr, uint8_t value) {
	if (addr >= 0x8000 && addr <= 0xFFFF) {
		PRGBank = value & 0xF;
	}
}

uint8_t& UXROM::CPURead(uint16_t addr) {
	if (addr >= 0x8000 && addr < 0xC000) {
		return PRGROM[PRGBank * 0x4000 + (addr - 0x8000)];
	}
	else if (addr >= 0xC000 && addr <= 0xFFFF) {
		return PRGROM[PRGROM.size() - 0x4000 + (addr - 0xC000)];
	}
}

void CNROM::Write(uint16_t addr, uint8_t value) {
	if (addr >= 0x8000 && addr <= 0xFFFF) {
		CHRBank = value & 0x3;
	}
}