#define NOMINMAX
#include "gui.h"
#include "string"

int main(int, char**)
{
    std::string file_name = "Super Mario Bros.nes";
    NES nes;
    nes.OnUserCreate(file_name);

    BasicInitGui (&nes);

    return 0;
}
