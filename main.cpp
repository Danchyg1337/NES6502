#define NOMINMAX
#include "gui.h"
#include "string"

int main(int, char**)
{
    std::string file_name = "Super Mario Bros.nes";
    NES nes;
    if (!nes.LoadRom(file_name)) return 0;

    BasicInitGui(&nes);

    return 0;
}
