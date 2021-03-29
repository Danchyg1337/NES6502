#pragma once
#include <vector>
#include <unordered_map>

class NES;

class PPU {
	NES* nes = nullptr;
	void WriteData(uint16_t addr, uint8_t value);
	uint8_t ReadData(uint16_t addr);
public:
	uint16_t clockCycle = 0;
	int16_t horiLines = 0;

	uint8_t fetched;

	enum MIRRORING : uint8_t{
		HORIZONTAL,
		VERTICAL
	};

	uint8_t mirroringMode = MIRRORING::HORIZONTAL;

	uint16_t VRAMaddr = 0x0;
	bool addrLatch = false;

	bool frameIsReady = false;

	std::vector<uint8_t> OAM;
	std::vector<uint8_t> CHRROM;
	std::vector<uint8_t> paletteRAM;
	std::vector<uint8_t> VRAM;
	std::vector<uint8_t> toRender;			//ready copy of VRAM
	std::vector<uint8_t> ATtoRender;		//ready copy of AT
	bool BanktoRenderBG;						
	bool BanktoRenderFG;						
	bool mode8x16;						
	
	struct RGB {
		float r;
		float g;
		float b;
	};

	struct Palette {
		RGB c1;
		RGB c2;
		RGB c3;
	};

	RGB bgColor;

	struct Palettes {
		Palette palettte0;
		Palette palettte1;
		Palette palettte2;
		Palette palettte3;
	};

	Palettes bgPalettes;
	Palettes fgPalettes;

	std::unordered_map<uint8_t, RGB> paletteTable = {
		{0,  {84, 84, 84}},	   {1, {0, 30, 116}},	  {2, {8, 16, 144}},     {3, {48, 0, 136}},     {4, {68, 0, 100}},     {5, {92, 0, 48}},      {6, {84, 4, 0}},       {7,  {60, 24, 0}},     {8, {32, 42, 0}},      {9, {8, 58, 0}},       {10, {0, 64, 0}},      {11, {0, 60, 0}},      {12, {0, 50, 60}},     {13, {0, 0, 0}},       {14, {0, 0, 0}}, {15, {0, 0, 0}},
		{16, {152, 150, 152}}, {17, {8, 76, 196}},	  {18, {48, 50, 236}},   {19, {92, 30, 228}},   {20, {136, 20, 176}},  {21, {160, 20, 100}},  {22, {152, 34, 32}},   {23, {120, 60, 0}},    {24, {84, 90, 0}},     {25, {40, 114, 0}},    {26, {8, 124, 0}},     {27, {0, 118, 40}},    {28, {0, 102, 120}},   {29, {0, 0, 0}},       {30, {0, 0, 0}}, {31, {0, 0, 0}},
		{32, {236, 238, 236}}, {33, {76, 154, 236}},  {34, {120, 124, 236}}, {35, {176, 98, 236}},  {36, {228, 84, 236}},  {37, {236, 88, 180}},  {38, {236, 106, 100}}, {39, {212, 136, 32}},  {40, {160, 170, 0}},   {41, {116, 196, 0}},   {42, {76, 208, 32}},   {43, {56, 204, 108}},  {44, {56, 180, 204}},  {45, {60, 60, 60}},    {46, {0, 0, 0}}, {47, {0, 0, 0}},
		{48, {236, 238, 236}}, {49, {168, 204, 236}}, {50, {188, 188, 236}}, {51, {212, 178, 236}}, {52, {236, 174, 236}}, {53, {236, 174, 212}}, {54, {236, 180, 176}}, {55, {228, 196, 144}}, {56, {204, 210, 120}}, {57, {180, 222, 120}}, {58, {168, 226, 144}}, {59, {152, 226, 180}}, {60, {160, 214, 228}}, {61, {160, 162, 160}}, {62, {0, 0, 0}}, {63, {0, 0, 0}}
	};

	uint8_t PPUCTRL;	//2000
	uint8_t PPUMASK;	//2001
	uint8_t PPUSTATUS;	//2002
	uint8_t OAMADDR;	//2003
	uint8_t OAMDATA;	//2004
	uint8_t PPUSCROLL;	//2005
	uint8_t PPUADDR;	//2006
	uint8_t PPUDATA;	//2007
	uint8_t OAMDMA;		//4014

	enum FLAGS : uint8_t {
		B0 = 1,
		B1 = 2,
		B2 = 4,
		B3 = 8,
		B4 = 16,
		B5 = 32,
		B6 = 64,
		B7 = 128
	};

	bool Load(uint8_t* pattern, size_t size);
	void Step();
	void Reset();
	void ConnectToNes(NES* nes);
	uint8_t& Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	void GetPalette();
};