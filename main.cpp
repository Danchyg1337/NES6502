#define NOMINMAX

#include <inttypes.h>
#include "Defines.h"
#include "PPU.h"
#include "CPU.h"
#include "NES6502.h"
#include "Mapper.h"

#include "gui.h"


int main(int, char**)
{
    std::string game_name = "Milon's Secret Castle.nes";
    std::string file_name = "Games/" + game_name;
    NES nes;
    if (!nes.LoadRom(file_name)) return 0;

    BasicInitGui(&nes, game_name);

    return 0;
}
