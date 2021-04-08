#define NOMINMAX

#include <inttypes.h>
#include "Defines.h"
#include "PPU.h"
#include "CPU.h"
#include "NES6502.h"

#include "gui.h"


int main(int, char**)
{
    std::string file_name = "Pac-Man(J).nes";
    NES nes;
    if (!nes.LoadRom(file_name)) return 0;

    BasicInitGui(&nes); 

    return 0;
}
