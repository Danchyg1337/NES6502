#pragma once


class PPU {
public:
	uint16_t clockCycle = 0;

	std::vector<uint8_t> CHRROM;
	std::vector<uint8_t> VRAM;
	PPU() {
		
	}

    bool Load(uint8_t* pattern, size_t size) {
		VRAM.resize(2048, 0);
		CHRROM.resize(size);
		memcpy(CHRROM.data(), pattern, size);
		return true;
    }
};