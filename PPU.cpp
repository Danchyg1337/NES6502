#pragma once

#include "PPU.h"
#include "NES6502.h"

bool PPU::Load(uint8_t* pattern, size_t size) {
	paletteRAM.resize(0x20, 0);
	VRAM.resize(0x0800, 0);
	toRender.resize(0x0800, 0);
	ATtoRender.resize(0x40, 0);
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
			ATtoRender = std::vector<uint8_t>(toRender.begin() + 0x03C0, toRender.begin() + 0x03C0 + 64);
			BanktoRender = PPUCTRL & FLAGS::B4;
			GetPalette();
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
	if (addr >= 0x3F00 && addr <= 0x3FFF) {
		return paletteRAM[(addr - 0x3F00) % 0x20];
	}
}

void PPU::WriteData(uint16_t addr, uint8_t value) {
	if (addr >= 0x2000 && addr < 0x2800) {
		VRAM[addr - 0x2000] = value;
	}	
	if (addr >= 0x3F00 && addr <= 0x3FFF) {
		paletteRAM[(addr - 0x3F00) % 0x20] = value;
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

void PPU::GetPalette() {
	bgColor = paletteTable[ReadData(0x3F00) % 64];
	palettes.bgPalettte0 = { paletteTable[ReadData(0x3F00 + 1) % 64],  paletteTable[ReadData(0x3F00 + 2) % 64],  paletteTable[ReadData(0x3F00 + 3) % 64] };
	palettes.bgPalettte1 = { paletteTable[ReadData(0x3F00 + 5) % 64],  paletteTable[ReadData(0x3F00 + 6) % 64],  paletteTable[ReadData(0x3F00 + 7) % 64] };
	palettes.bgPalettte2 = { paletteTable[ReadData(0x3F00 + 9) % 64],  paletteTable[ReadData(0x3F00 + 10) % 64], paletteTable[ReadData(0x3F00 + 11) % 64] };
	palettes.bgPalettte3 = { paletteTable[ReadData(0x3F00 + 13) % 64], paletteTable[ReadData(0x3F00 + 14) % 64], paletteTable[ReadData(0x3F00 + 15) % 64] };
}