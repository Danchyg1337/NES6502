#pragma once

#include "PPU.h"
#include "NES6502.h"

bool PPU::Load(uint8_t* pattern, size_t size) {
	paletteTable.resize(0xFF, 0);
	VRAM.resize(2048, 0);
	toRender.resize(2048, 0);
	CHRROM.resize(size);
	memcpy(CHRROM.data(), pattern, size);
	return true;
}

void PPU::Step() {
	if (horiLines == -1 && clockCycle == 1) PPUSTATUS &= ~FLAGS::B7;
	clockCycle++;
	if (clockCycle > 340) {
		clockCycle = 0;
		horiLines++;
		if (horiLines > 260) {
			horiLines = -1;
			toRender = VRAM;
		}
	}
	if (horiLines == 241 && clockCycle == 1) {
		PPUSTATUS |= FLAGS::B7;
		if (PPUCTRL & FLAGS::B7) nes->SendNMI();
	}
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
	case 0x2002: {
		uint8_t cpy = PPUSTATUS;
		PPUSTATUS &= ~FLAGS::B7;
		addrLatch = false;
		return cpy;
	}
	case 0x2004:
		return OAMDATA;
	case 0x2007: {
		uint8_t data = fetched;
		fetched = ReadData(VRAMaddr);
		if (VRAMaddr >= 0x3F00) data = fetched;
		VRAMaddr += (PPUCTRL & FLAGS::B2 ? 32 : 1);
		return data;
	}
	}
}

uint8_t PPU::ReadData(uint16_t addr) {
	if (addr >= 0x2000 && addr < 0x2800) {
		return VRAM[addr - 0x2000];
	}
}

void PPU::WriteData(uint16_t addr, uint8_t value) {
	if (addr >= 0x2000 && addr < 0x2800) {
		VRAM[addr - 0x2000] = value;
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
			addrLatch = false;
		}
		else addrLatch = true;
		PPUADDR = value;
		break;
	case 0x2007:
		PPUDATA = value;
		WriteData(VRAMaddr, PPUDATA);
		VRAMaddr += (PPUCTRL & FLAGS::B2 ? 32 : 1);
		break;
	case 0x4014:
		OAMDMA = value;
		break;
	}
}