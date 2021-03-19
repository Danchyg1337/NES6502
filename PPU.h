#pragma once


class PPU {
	uint8_t* memory = nullptr;
	PPU() {
		memory = new uint8_t[0xFFFF];
		std::vector<uint8_t> CHRROM;
	}
};