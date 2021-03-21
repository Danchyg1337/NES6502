#define NOMINMAX
#include "gui.h"
#include "string"

int main(int, char**)
{
    std::string file_name = "Pac-Man(J).nes";
    NES nes;
    if (!nes.LoadRom(file_name)) return 0;

    std::thread run(&NES::Run, &nes);

    //run.detach();

    BasicInitGui(&nes);

    return 0;
}
