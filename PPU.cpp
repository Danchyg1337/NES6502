#pragma once

#include "PPU.h"
#include "NES6502.h"

bool PPU::Load(uint8_t* pattern, size_t size) {
	OAM.resize(0x0100, 0);
	paletteRAM.resize(0x20, 0);
	VRAM.resize(0x0800, 0);
	patternTable.resize(0x2000, 0);
	ATtoRender.resize(0x80, 0);
	CHRROM.resize(size);
	memcpy(CHRROM.data(), pattern, size);
	memcpy(patternTable.data(), pattern, 0x2000);
	return true;
}

void PPU::Step() {

	if (currentPixelX >= 8) {
		currentPixelX = 0;
		currentTileX++;
	}

	if (currentPixelY >= 8) {
		currentPixelY = 0;
		currentTileY++;
	}

	frameIsReady = false;
	if (horiLines == -1 && clockCycle == 1) {
		PPUSTATUS &= ~FLAGS::B7;
		PPUSTATUS &= ~FLAGS::B6;
	}
	if (horiLines == 0 && clockCycle == 0) clockCycle = 1;
	if (clockCycle <= 256 && horiLines < 240 && horiLines != -1) {

		uint16_t currentTileY = horiLines / 8;
		uint16_t currentTileX = clockCycle / 8;
		uint16_t currentPixelY = horiLines % 8;
		uint16_t currentPixelX = clockCycle % 8;
		uint16_t scrTileX = scrollX / 8;
		uint16_t scrTileY = scrollY / 8;
		uint16_t scrPixelX = scrollX % 8;
		uint16_t scrPixelY = scrollY % 8;
		int32_t pxlPos = horiLines * 768 + clockCycle * 3 - scrPixelX * 3 - scrPixelY * 768;
		if(pxlPos < 0) pxlPos = 0;

		int16_t finalX = scrTileX + currentTileX;
		int16_t finalY = scrTileY + currentTileY;
	
		switch (mirroringMode)
		{
		case MIRRORING::HORIZONTAL:
			if (finalX >= 32) {
				finalX -= 32;
			}
			if (finalY >= 30) {
				finalY -= 30;
				if (nametableBank == 0)
					finalX += 0x0400;
				else
					finalX -= 0x0400;
			}
			finalX += nametableBank * 0x0200;
			break;
		case MIRRORING::VERTICAL:
			if (finalY >= 30) {
				finalY -= 30;
			}
			if (finalX >= 32) {
				finalX -= 32;
				if (nametableBank == 0)
					finalX += 0x0400;
				else
					finalX -= 0x0400;
			}
			finalX += nametableBank * 0x0400;
			break;
		}

		uint16_t finalPos = finalY * 32 + finalX;
		uint16_t chrPos = VRAM[finalPos];
		if (BanktoRenderBG) chrPos += 256;
		chrPos *= 16;

		uint8_t LSB = patternTable[chrPos + currentPixelY];
		uint8_t MSB = patternTable[chrPos + currentPixelY + 8];


		uint8_t l = LSB >> (7 - currentPixelX) & 1;
		uint8_t h = MSB >> (7 - currentPixelX) & 1;
		uint8_t colorN = ((h << 1) | l) & 0x3;


		uint8_t tile4Colors = ATtoRender[currentTileY / 4 * 8 + currentTileX / 4];

		bool right = currentTileX / 2 % 2;
		bool bottom = currentTileY / 2 % 2;

		uint8_t tileColor = (tile4Colors >> (bottom * 4) >> (right * 2)) & 0x3;

		Palette pal;

		switch (tileColor) {
		case 0:
			pal = bgPalettes.palette0;
			break;
		case 1:
			pal = bgPalettes.palette1;
			break;
		case 2:
			pal = bgPalettes.palette2;
			break;
		case 3:
			pal = bgPalettes.palette3;
			break;
		}


		switch (colorN)
		{
		case 1:
			pixls[pxlPos] =		pal.c1.r;
			pixls[pxlPos + 1] = pal.c1.g;
			pixls[pxlPos + 2] = pal.c1.b;
			break;
		case 2:
			pixls[pxlPos] =		pal.c2.r;
			pixls[pxlPos + 1] = pal.c2.g;
			pixls[pxlPos + 2] = pal.c2.b;
			break;
		case 3:
			pixls[pxlPos] =		pal.c3.r;
			pixls[pxlPos + 1] = pal.c3.g;
			pixls[pxlPos + 2] = pal.c3.b;
			break;
		default:
			pixls[pxlPos] =		bgColor.r;
			pixls[pxlPos + 1] = bgColor.g;
			pixls[pxlPos + 2] = bgColor.b;
			break;
		}


		uint16_t y = OAM[0];
		uint16_t x = OAM[3];

		if ((PPUMASK & FLAGS::B3) && (PPUMASK & FLAGS::B4) && !(PPUSTATUS & FLAGS::B6) && (horiLines >= y - 1 && horiLines <= y + 7) && (clockCycle >= x && clockCycle <= x + 8)) {
			if (pixls[pxlPos] == bgColor.r && pixls[pxlPos + 1] == bgColor.g && pixls[pxlPos + 2] == bgColor.b) {
				PPUSTATUS |= FLAGS::B6;
				//printf("X: %i, Y: %i, T: %i\n", clockCycle, horiLines, OAM[1]);
			}
			//printf("IN\n");
		}
	}
	if (clockCycle > 340) {
		clockCycle = 0;
		currentTileX = 0;
		horiLines++;
		currentPixelY++;
		if (horiLines > 260) {
			horiLines = -1;
			VRAMtoRender = std::vector<uint8_t>(VRAM.begin(), VRAM.begin() + 0x03C0);
			VRAMtoRender.insert(VRAMtoRender.end(), VRAM.begin() + 0x0400, VRAM.begin() + 0x0400 + 0x03C0);
			ATtoRender = std::vector<uint8_t>(VRAM.begin() + 0x03C0, VRAM.begin() + 0x03C0 + 64);
			ATtoRender.insert(ATtoRender.end(), VRAM.begin() + 0x0400 + 0x03C0, VRAM.begin() + 0x0400 +0x03C0 + 64);
			mode8x16	   = PPUCTRL & FLAGS::B5;
			BanktoRenderBG = PPUCTRL & FLAGS::B4;
			BanktoRenderFG = PPUCTRL & FLAGS::B3;
			bgColor = paletteTable[0];
			GetPalette();
			frameIsReady = true;
			
			currentPixelX = currentPixelY = currentTileX = currentTileY = 0;
		}
	}
	if (horiLines == 241 && clockCycle == 1) {
		PPUSTATUS |= FLAGS::B7;
		if (PPUCTRL & FLAGS::B7) nes->SendNMI();
	}

	currentPixelX++;

	clockCycle++;
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
		data = PPUSTATUS;
		PPUSTATUS &= ~FLAGS::B7;
		addrLatch = false;
		return data;
	}
	case 0x2004:
		return OAMDATA;
	case 0x2007: {
		data = fetched;
		fetched = ReadData(VRAMaddr);
		if (VRAMaddr >= 0x3F00) data = fetched;
		VRAMaddr += (PPUCTRL & FLAGS::B2 ? 32 : 1);
		return data;
	}
	}
}

uint8_t PPU::ReadData(uint16_t addr) {
	if (addr < 0x2000) {
		return patternTable[addr];
	}
	if (addr >= 0x2000 && addr < 0x3000) {
		uint16_t mirrAddr = addr - 0x2000;
		if (mirroringMode == MIRRORING::HORIZONTAL) {
			if		(addr >= 0x2400 && addr < 0x2C00) mirrAddr -= 0x0400;
			else if (addr >= 0x2C00) mirrAddr -= 0x0800;
		}
		if (mirroringMode == MIRRORING::VERTICAL) {
			if (addr >= 0x2800) mirrAddr -= 0x0800;
		}
		return VRAM[mirrAddr];
	}
	if (addr >= 0x3F00 && addr <= 0x3FFF) {
		if (addr % 4 == 0)
			return paletteRAM[0];
		return paletteRAM[(addr - 0x3F00) % 0x20];
	}
}

void PPU::WriteData(uint16_t addr, uint8_t value) {
	if (addr < 0x2000) {
		patternTable[addr] = value;
	}
	if (addr >= 0x2000 && addr < 0x3000) {
		uint16_t mirrAddr = addr - 0x2000;
		if (mirroringMode == MIRRORING::HORIZONTAL) {
			if		(addr >= 0x2400 && addr < 0x2C00) mirrAddr -= 0x0400;
			else if (addr >= 0x2C00) mirrAddr -= 0x0800;
		}
		if (mirroringMode == MIRRORING::VERTICAL) {
			if (addr >= 0x2800) mirrAddr -= 0x0800;
		}
		VRAM[mirrAddr] = value;
	}	
	if (addr >= 0x3F00 && addr <= 0x3FFF) {
		if (addr % 4 == 0)
			paletteRAM[0] = value;
		else
			paletteRAM[(addr - 0x3F00) % 0x20] = value;
	}
}

void PPU::Write(uint16_t addr, uint8_t value) {
	switch (addr) {
	case 0x2000:
		PPUCTRL = value;
		nametableBank = PPUCTRL & 0x03;
		//printf("BANK %i\n", nametableBank);
		baseBankAddr = nametableBank * 0x0400;
		break;
	case 0x2001:
		PPUMASK = value;
		break;
	case 0x2002:
		break;
	case 0x2003:

		break;
	case 0x2004:

		break;
	case 0x2005:
		if (addrLatch) {
			scrollY = value;
			addrLatch = false;
		}
		else {
			scrollX = value;
			addrLatch = true;
		}
		//printf("X %i, Y%i\n", scrollX, scrollY);
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
		OAM = nes->DMAOAM;
		break;
	}
}

void PPU::GetPalette() {
	bgColor = paletteTable[ReadData(0x3F00) % 64];
	bgPalettes.palette0 = { paletteTable[ReadData(0x3F00 + 1) % 64],  paletteTable[ReadData(0x3F00 + 2) % 64],  paletteTable[ReadData(0x3F00 + 3) % 64] };
	bgPalettes.palette1 = { paletteTable[ReadData(0x3F00 + 5) % 64],  paletteTable[ReadData(0x3F00 + 6) % 64],  paletteTable[ReadData(0x3F00 + 7) % 64] };
	bgPalettes.palette2 = { paletteTable[ReadData(0x3F00 + 9) % 64],  paletteTable[ReadData(0x3F00 + 10) % 64], paletteTable[ReadData(0x3F00 + 11) % 64] };
	bgPalettes.palette3 = { paletteTable[ReadData(0x3F00 + 13) % 64], paletteTable[ReadData(0x3F00 + 14) % 64], paletteTable[ReadData(0x3F00 + 15) % 64] };

	fgPalettes.palette0 = { paletteTable[ReadData(0x3F10 + 1) ],  paletteTable[ReadData(0x3F10 + 2) ],  paletteTable[ReadData(0x3F10 + 3) ] };
	fgPalettes.palette1 = { paletteTable[ReadData(0x3F10 + 5) ],  paletteTable[ReadData(0x3F10 + 6) ],  paletteTable[ReadData(0x3F10 + 7) ] };
	fgPalettes.palette2 = { paletteTable[ReadData(0x3F10 + 9) ],  paletteTable[ReadData(0x3F10 + 10)], paletteTable[ReadData(0x3F10 + 11) ] };
	fgPalettes.palette3 = { paletteTable[ReadData(0x3F10 + 13) ], paletteTable[ReadData(0x3F10 + 14) ], paletteTable[ReadData(0x3F10 + 15) ] };
}