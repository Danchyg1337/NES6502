#pragma once


class PPU {
	uint8_t* memory = nullptr;
	PPU() {
		memory = new uint8_t[0x10000];
		std::vector<uint8_t> CHRROM;
	}
};