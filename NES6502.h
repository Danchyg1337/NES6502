
#include <fstream>

#include <deque>
#include <vector>
#include <mutex>

#include "CPU.h"


class NES
{
    
public:
    CPU *CPU6502;
    NES () {
        CPU6502 = new CPU ();  // look
    }
    bool running = false;

public:

    bool OnUserCreate (std::string filename)
    {
        std::ifstream file;
        
        file.open (filename, std::ifstream::binary);
        if (!file.is_open ()) {
            std::cout << "No input file " << filename << std::endl;
            return false;
        }
        std::vector<uint8_t> program (std::istreambuf_iterator<char> (file), {});
        CPU6502->load (program.data (), program.size ());
        return true;
    }

};

