#pragma once
#include "Mapper.h"

void Mapper::Load(uint8_t* prg, size_t size) {
	PRGROM.resize(size);
	memcpy(PRGROM.data(), prg, size);
}

void Mapper::Write(uint16_t addr, uint8_t value) {
	//
}

uint8_t& Mapper::Read(uint16_t addr) {
	if (addr >= 0x8000 && addr <= 0xFFFF) {
		return PRGROM[(addr - 0x8000) % PRGROM.size()];
	}
}

void UXROM::Write(uint16_t addr, uint8_t value) {
	if (addr >= 0x8000 && addr <= 0xFFFF) {
		selectedBank = value & 0xF;
	}
}

uint8_t& UXROM::Read(uint16_t addr) {
	if (addr >= 0x8000 && addr < 0xC000) {
		return PRGROM[selectedBank * 0x4000 + (addr - 0x8000)];
	}
	else if (addr >= 0xC000 && addr <= 0xFFFF) {
		return PRGROM[PRGROM.size() - 0x4000 + (addr - 0xC000)];
	}
}