#pragma once

#include "PPU.h"
#include "NES6502.h"

bool PPU::Load() {
	OAM.resize(0x0100, 0);
	paletteRAM.resize(0x20, 0);
	VRAM.resize(0x0800, 0);
	ATtoRender.resize(0x80, 0);
	return true;
}

uint16_t PPU::GetCurrentScreenTileAddr() {
	uint16_t finalX = scrTileX + currentTileX;
	uint16_t finalY = scrTileY + currentTileY;
	uint8_t bank = PPUCTRL & 0x03;
	if (finalX & ~0x1F) {
		bank ^= 1;
	}
	if (finalY > 29) {
		bank ^= 2;
		finalY %= 0x1E;
	}
	return uint16_t(bank << 10) | ((finalY & 0x1F) << 5) | (finalX & 0x1F);
}

uint8_t PPU::GetCurrentTilePalette(uint16_t tilePos) {
	bool right =  (tilePos >> 1) & 1;
	bool bottom = (tilePos >> 6) & 1;

	uint8_t tile4Colors = ReadData(0x23C0 | (tilePos & 0x0C00) | ((tilePos >> 4) & 0x38) | ((tilePos >> 2) & 0x07));
	return (tile4Colors >> (bottom * 4 + right * 2)) & 0x3;
}

void PPU::GetCurrentChrTile() {
	uint16_t tileAddr = GetCurrentScreenTileAddr();
	uint16_t tileNum = ReadData(tileAddr + 0x2000);
	if (PPUCTRL & FLAGS::B4) tileNum += 256;
	tileNum *= 16;
	if (tileNum == currentTile.id) return;
	memcpy(currentTile.LSB, &nes->mapper->PPURead(tileNum), 8);
	memcpy(currentTile.MSB, &nes->mapper->PPURead(tileNum + 8), 8);
	currentTile.id = tileNum;
	currentTile.palette = GetCurrentTilePalette(tileAddr);
}

void PPU::GetCurrentSprite0Tile() {
	uint16_t id = OAM[1];
	if (id == spriteZeroTile.id) return;
	uint16_t tileNum = id;
	bool bank = PPUCTRL & FLAGS::B3;
	if (mode8x16) {
		tileNum &= ~0x1;
		bank = tileNum & 0x1;
	}
	if (bank) tileNum += 256;
	tileNum *= 16;
	memcpy(spriteZeroTile.LSB, &nes->mapper->PPURead(tileNum), 8);
	memcpy(spriteZeroTile.MSB, &nes->mapper->PPURead(tileNum + 8), 8);
	spriteZeroTile.id = id;
}

void PPU::Step() {

	if (currentPixelX >= 8) {
		currentPixelX = 0;
		currentTileX++;
		GetCurrentChrTile();
	}

	if (currentPixelY >= 8) {
		currentPixelY = 0;
		currentTileY++;
		GetCurrentChrTile();
	}

	frameIsReady = false;
	if (horiLines == -1 && clockCycle == 1) {
		std::fill_n(pixls, pixlsSize, 0);
		PPUSTATUS &= ~FLAGS::B7;
		PPUSTATUS &= ~FLAGS::B6;
	}
	if (horiLines == 0 && clockCycle == 0) {
		currentPixelY = currentTileX = currentTileY = 0;
		clockCycle = currentPixelX = 1;
		GetCurrentChrTile();
	}
	if (clockCycle <= 256 && horiLines < 240 && horiLines != -1) {
		int32_t pxlPos = horiLines * 768 + clockCycle * 3 - scrPixelX * 3 - scrPixelY * 768;
		if(pxlPos < 0) pxlPos = 0;

		bool opaque = false;
		if (PPUMASK & FLAGS::B4) {
			uint8_t l = currentTile.LSB[currentPixelY] >> (7 - currentPixelX) & 1;
			uint8_t h = currentTile.MSB[currentPixelY] >> (7 - currentPixelX) & 1;
			uint8_t colorN = ((h << 1) | l) & 0x3;


			if (!colorN) {
				pixls[pxlPos] = bgColor.r;
				pixls[pxlPos + 1] = bgColor.g;
				pixls[pxlPos + 2] = bgColor.b;
			}
			else {
				pixls[pxlPos] = bgPalettes[currentTile.palette].colors[colorN - 1].r;
				pixls[pxlPos + 1] = bgPalettes[currentTile.palette].colors[colorN - 1].g;
				pixls[pxlPos + 2] = bgPalettes[currentTile.palette].colors[colorN - 1].b;
				opaque = true;
			}
		}
		GetCurrentSprite0Tile(); 
		uint8_t y = OAM[0];
		uint8_t x = OAM[3];

		uint8_t yComp = horiLines - y;
		uint8_t xComp = clockCycle - x;

		if ((PPUMASK & FLAGS::B3) && (PPUMASK & FLAGS::B4) && !(PPUSTATUS & FLAGS::B6) && (yComp < 8) && (xComp < 8)) {
			uint8_t l = (spriteZeroTile.LSB[yComp] >> (7 - xComp)) & 1;
			uint8_t h = (spriteZeroTile.MSB[yComp] >> (7 - xComp)) & 1;
			uint8_t colorN = ((h << 1) | l) & 0x3;
			if (colorN != 0) {
				if (opaque) {
					PPUSTATUS |= FLAGS::B6;
				}
				if (showZhit) {
					pixls[pxlPos] = 255;
					pixls[pxlPos + 1] = 0;
					pixls[pxlPos + 2] = 0;
				}
			}
		}
	}
	if (clockCycle > 340) {
		clockCycle = 0;
		currentPixelX = 0;
		currentTileX = 0;
		horiLines++;
		currentPixelY++;
		GetCurrentChrTile();
		if (horiLines > 260) {
			horiLines = -1;
			//VRAMtoRender = std::vector<uint8_t>(VRAM.begin(), VRAM.begin() + 0x03C0);
			//VRAMtoRender.insert(VRAMtoRender.end(), VRAM.begin() + 0x0400, VRAM.begin() + 0x0400 + 0x03C0);
			//ATtoRender = std::vector<uint8_t>(VRAM.begin() + 0x03C0, VRAM.begin() + 0x03C0 + 64);
			//ATtoRender.insert(ATtoRender.end(), VRAM.begin() + 0x0400 + 0x03C0, VRAM.begin() + 0x0400 +0x03C0 + 64);
			mode8x16	   = PPUCTRL & FLAGS::B5;
			BanktoRenderBG = PPUCTRL & FLAGS::B4;
			BanktoRenderFG = PPUCTRL & FLAGS::B3;
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
		return nes->mapper->PPURead(addr);
	}
	if (addr >= 0x2000 && addr < 0x3000) {
		uint16_t mirrAddr = addr & 0x0FFF;
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
		nes->mapper->PPUWrite(addr, value);
	}
	if (addr >= 0x2000 && addr < 0x3000) {
		uint16_t mirrAddr = addr & 0x0FFF;
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
		if (addr % 4 == 0 && addr >= 0x3F10) addr -= 16;
		paletteRAM[(addr - 0x3F00) % 0x20] = value;
	}
}

void PPU::Write(uint16_t addr, uint8_t value) {
	switch (addr) {
	case 0x2000:
		PPUCTRL = value;
		loopy_temp &= 0xC00;
		loopy_temp |= (uint16_t(PPUCTRL) & 0x3) << 10;
		nametableBank = PPUCTRL & 0x03;
		baseBankAddr = nametableBank * 0x0400;
		break;
	case 0x2001:
		PPUMASK = value;
		break;
	case 0x2002:
		break;
	case 0x2003:
		OAMaddr = value;
		break;
	case 0x2004:				//not complete
		OAM[OAMaddr] = value;
		OAMADDR++;
		break;
	case 0x2005:
		if (addrLatch) {
			loopy_temp |= (value & 0xF8) << 2;
			loopy_temp |= (value & 0x7) << 12;
			scrollY = value;
			scrTileY = scrollY / 8;
			scrPixelY = scrollY % 8;
			addrLatch = false;
		}
		else {
			loopy_temp |= value >> 3;
			Xtile = value & 0x7;
			scrollX = value;
			scrTileX = scrollX / 8;
			scrPixelX = scrollX % 8;
			addrLatch = true;
		}
		//printf("X %i, Y%i\n", scrollX, scrollY);
		break;
	case 0x2006:
		if (addrLatch) {
			loopy_temp |= (value & 0x3F) << 8;
			loopy_temp &= ~0xC000;
			PPUCTRL &= ~0x03;
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
	bgColor =		  paletteTable[paletteRAM[0]];
	bgPalettes[0] = { paletteTable[paletteRAM[1]],  paletteTable[paletteRAM[2]],  paletteTable[paletteRAM[3]] };
	bgPalettes[1] = { paletteTable[paletteRAM[5]],  paletteTable[paletteRAM[6]],  paletteTable[paletteRAM[7]] };
	bgPalettes[2] = { paletteTable[paletteRAM[9]],  paletteTable[paletteRAM[10]], paletteTable[paletteRAM[11]] };
	bgPalettes[3] = { paletteTable[paletteRAM[13]], paletteTable[paletteRAM[14]], paletteTable[paletteRAM[15]] };

	fgPalettes[0] = { paletteTable[paletteRAM[17]],  paletteTable[paletteRAM[18]],  paletteTable[paletteRAM[19]] };
	fgPalettes[1] = { paletteTable[paletteRAM[21]],  paletteTable[paletteRAM[22]],  paletteTable[paletteRAM[23]] };
	fgPalettes[2] = { paletteTable[paletteRAM[25]],  paletteTable[paletteRAM[26]],  paletteTable[paletteRAM[27]] };
	fgPalettes[3] = { paletteTable[paletteRAM[29]],  paletteTable[paletteRAM[30]],  paletteTable[paletteRAM[31]] };
}