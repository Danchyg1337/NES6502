#pragma once
#include <vector>
class Mapper {
protected:
	std::vector<uint8_t> PRGROM;
public:
	std::vector<uint8_t> CHRROM;

	uint16_t PRGBank = 0;
	uint16_t CHRBank = 0;

	void LoadPRG(uint8_t* prg, size_t size);
	void LoadCHR(uint8_t* chr, size_t size);

	virtual void Write(uint16_t addr, uint8_t value);
	virtual uint8_t& CPURead(uint16_t addr);
	virtual uint8_t& PPURead(uint16_t addr);
	virtual void PPUWrite(uint16_t addr, uint8_t value);
};

class UXROM : public Mapper {
	
	void Write(uint16_t addr, uint8_t value) override;
	uint8_t& CPURead(uint16_t addr) override;
};

class CNROM : public Mapper {

	void Write(uint16_t addr, uint8_t value) override;
};