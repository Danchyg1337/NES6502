#pragma once
#include <vector>
class Mapper {
protected:
	std::vector<uint8_t> PRGROM;
	std::vector<uint8_t> CHRROM;
public:

	void Load(uint8_t* prg, size_t size);

	virtual void Write(uint16_t addr, uint8_t value);
	virtual uint8_t& Read(uint16_t addr);
};

class UXROM : public Mapper {
	uint16_t selectedBank = 0;

	void Write(uint16_t addr, uint8_t value) override;
	uint8_t& Read(uint16_t addr) override;
};