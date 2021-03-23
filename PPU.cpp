#pragma once

#include "PPU.h"
#include "NES6502.h"

bool PPU::Load(uint8_t* pattern, size_t size) {
	paletteTable.resize(0xFF, 0);
	VRAM.resize(2048, 0);
	CHRROM.resize(size);
	memcpy(CHRROM.data(), pattern, size);
	return true;
}

void PPU::Step() {
	clockCycle++;
	if (clockCycle > 341) {
		clockCycle = 0;
		horiLines++;
		if (horiLines > 262)
			horiLines = -1;
	}
	if (horiLines == 241 && clockCycle == 1) {
		Write(0x2002, PPUSTATUS | FLAGS::B7);
		if (PPUCTRL & FLAGS::B7) nes->SendNMI();
	}
	if (horiLines == 261 && clockCycle == 1) Write(0x2002, PPUSTATUS & ~FLAGS::B7);
}

void PPU::Reset() {
	VRAM.clear();
	VRAMaddr = 0x0;
	clockCycle = 0;
	horiLines = 0;
}

void PPU::ConnectToNes(NES* nes) {
	this->nes = nes;
}

uint8_t& PPU::Read(uint16_t addr) {
	switch (addr) {
	case 0x2000:
		return PPUCTRL;
	case 0x2001:
		return PPUMASK;
	case 0x2002: {
		uint8_t cpy = PPUSTATUS;
		Write(0x2002, PPUSTATUS & ~FLAGS::B7);
		addrLatch = false;
		return cpy;
	}
	case 0x2003:
		return OAMADDR;
	case 0x2004:
		return OAMDATA;
	case 0x2005:
		return PPUSCROLL;
	case 0x2006:
		return PPUADDR;
	case 0x2007:
		VRAMaddr += (PPUCTRL & FLAGS::B2 ? 32 : 1);
		return PPUDATA;
	case 0x4014:
		return OAMDMA;
	}
}

void PPU::Write(uint16_t addr, uint8_t value) {
	switch (addr) {
	case 0x2000:
		PPUCTRL = value;
		break;
	case 0x2001:
		PPUMASK = value;
		break;
	case 0x2002:
		PPUSTATUS = value;
		break;
	case 0x2003:
		OAMADDR = value;
		break;
	case 0x2004:
		OAMDATA = value;
		break;
	case 0x2005:
		PPUSCROLL = value;
		break;
	case 0x2006:
		if (addrLatch) {
			VRAMaddr = value;
			VRAMaddr |= uint16_t(PPUADDR) << 8;
		}
		PPUADDR = value;
		break;
	case 0x2007:
		PPUDATA = value;
		VRAMaddr += (PPUCTRL & FLAGS::B2 ? 32 : 1);
		break;
	case 0x4014:
		OAMDMA = value;
		break;
	}
}