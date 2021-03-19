#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

#include <GLFW/glfw3.h>
#include <stdio.h>
#include "Defines.h"
#include "NES6502.h"
#include "Shader.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback (int error, const char* description)
{
    fprintf (stderr, "Glfw Error %d: %s\n", error, description);
}

GLuint CHRdump(CPU* CPU6502, Shader& fillTexture, uint8_t bank = 0) {
    if (!CPU6502 || CPU6502->CHRsize == 0) return -1;

    uint16_t width = 8 * 22;
    uint16_t height = 8 * 24;

    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    GLuint squareVAO, squareVBO;
    glGenVertexArrays(1, &squareVAO);
    glGenBuffers(1, &squareVBO);
    glBindVertexArray(squareVAO);
    glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(primitive::square), &primitive::square, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    
    GLenum drawbuffer[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawbuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return -1;

    uint32_t* data = new uint32_t[256];
    uint16_t offset = 0x2000 * bank;
    float palette[9] = {1, 0, 0,
                        0, 1, 0,
                        0, 0, 1};

    memcpy(data, &CPU6502->CHRROM[offset], 1024);



    fillTexture.Use();
    glUniform1i(fillTexture.SetUniform("textureWidth"), width);
    glUniform1i(fillTexture.SetUniform("textureHeight"), height);
    glUniform1i(fillTexture.SetUniform("dataSize"), 256);
    glUniformMatrix3fv(fillTexture.SetUniform("palette"), 1, GL_FALSE, palette);
    glUniform1uiv(fillTexture.SetUniform("data"), 256, data);
    glViewport(0, 0, width, height);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return texture;
}

std::deque<std::string> instructions_dump(CPU *CPU6502, uint16_t &currentLine) {
    std::deque<std::string> queque_;

    uint16_t local_pc = CPU6502->startAddr;
    uint8_t opcode = CPU6502->Read(local_pc);

    int current = 0, aboveLocalPC = 0;
    while ((aboveLocalPC < 5 || queque_.size () < 10) && opcode != 0x00) { //and !(SR & FLAGS::B))) Break opcode

        opcode = CPU6502->Read(local_pc);
        if (CPU6502->instructions.find (opcode) == CPU6502->instructions.end ()) {
            std::string line (15, ' ');
            sprintf_s (const_cast<char*>(line.data ()), line.size (), "%04X %s -> %02X", local_pc, "???", opcode);
            queque_.push_back (line);
            aboveLocalPC++;
            if (queque_.size () > 10) {
                queque_.pop_front ();
                current--;
            }
            continue;
        }
        const auto& instruction = CPU6502->instructions[opcode];

        std::string line (9, ' ');
        sprintf_s (const_cast<char*>(line.data ()), line.size (), "%04X %s", local_pc, instruction.name.data ());
        line[8] = ' ';

        for (uint8_t i = 1; i < instruction.bytes; i++) {
            std::string buff (3, ' ');
            sprintf_s (const_cast<char*>(buff.data ()), buff.size (), "%02X", CPU6502->Read(local_pc + i));
            line += buff.substr(0, 2) + ' ';
        }

        queque_.push_back (line);

        if (local_pc == CPU6502->PC) current = queque_.size () - 1;
        if (local_pc > CPU6502->PC) aboveLocalPC++;
        local_pc += instruction.bytes;


        if (queque_.size () > 10) {
            queque_.pop_front ();
            current--;
        }
    }
    currentLine = current;
    return queque_;
}

int BasicInitGui (NES *nes_cpu) {
    
    // Setup window
    glfwSetErrorCallback (glfw_error_callback);
    if (!glfwInit ())
        return 1;

    // Decide GL+GLSL versions
#ifdef __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    IMGUI_CHECKVERSION ();
    ImGui::CreateContext ();
    // Create window with graphics context
    GLFWwindow * window = glfwCreateWindow (1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent (window);
    glfwSwapInterval (1); // Enable vsync

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL (window, true);

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit () != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit () != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL () == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
    bool err = gladLoadGL (glfwGetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize ();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize ([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress (name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf (stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    ImGuiIO& io = ImGui::GetIO (); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.WantCaptureMouse = 1;
    //ImGui::StyleColorsDark ();
    ImGui::StyleColorsClassic ();
    ImGui_ImplOpenGL3_Init (glsl_version);


    bool show_instruction_window = false;
    bool show_zpage = false;

    ImVec4 clear_color = ImVec4 (0.45f, 0.55f, 0.60f, 1.00f);

    Shader fillTexture("Shaders/chrdump.glsl");
    GLuint tex = CHRdump(&nes_cpu->CPU6502, fillTexture);

    while (!glfwWindowShouldClose (window))
    {   
        glfwPollEvents ();
        ImGui_ImplOpenGL3_NewFrame ();
        ImGui_ImplGlfw_NewFrame ();
        ImGui::NewFrame ();


        {
            ImGui::Begin ("NES");

            ImGui::Text ("Accumulator %02X", nes_cpu->CPU6502.A);
            ImGui::Text ("X Register %02X", nes_cpu->CPU6502.X);
            ImGui::Text ("Y Register %02X", nes_cpu->CPU6502.Y);
            ImGui::Text ("Stack Pointer %02X", nes_cpu->CPU6502.SP);
            ImGui::Text ("Program Counter %04X", nes_cpu->CPU6502.PC);
            ImGui::Text ("Status Register %c %c %c %c %c %c %c %c", BYTE_TO_BINARY (nes_cpu->CPU6502.SR));

            /*
            printf ("Accumulator %02X\n", nes_cpu->CPU6502->A);
            printf ("X Registe %02X\n", nes_cpu->CPU6502->X);
            printf ("Y Register %02X\n", nes_cpu->CPU6502->Y);
            printf ("Stack Pointer %02X\n", nes_cpu->CPU6502->SP);
            printf ("Program Counter %04X\n", nes_cpu->CPU6502->PC);
            printf ("Status Register %c%c%c%c%c%c%c%c\n", BYTE_TO_BINARY (nes_cpu->CPU6502->SP));
            */
            //ImGui::SliderFloat ("float", &f, 0.0f, 1.0f);           
            //ImGui::ColorEdit3 ("clear color", (float*)&clear_color); 

            if (ImGui::Button(nes_cpu->CPU6502.running ? "STOP" : "RUN")) {
                nes_cpu->CPU6502.running = !nes_cpu->CPU6502.running;
                //nes_cpu->CPU6502->Run();
            }
            ImGui::SameLine();
            if (ImGui::Button("STEP") || nes_cpu->CPU6502.running)
                nes_cpu->CPU6502.Step();
            ImGui::SameLine();
            if (ImGui::Button("RESET")) 
                nes_cpu->CPU6502.Reset();
            

            ImGui::Checkbox(("SHOW INSTRUCTION LIST"), &show_instruction_window);
            ImGui::Checkbox(("SHOW Z PAGE"), &show_zpage);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO ().Framerate, ImGui::GetIO ().Framerate);

            ImGui::End();
        }

        if (show_instruction_window)
        {
            ImGui::Begin ("Instructions", &show_instruction_window);

            uint16_t current = 0;
            auto queque_ = instructions_dump(&nes_cpu->CPU6502, current);
            for (int i = 0; i < queque_.size (); i++) {
                if (current == i) 
                    ImGui::TextColored({1, 0, 1, 1}, "%s", queque_[i].c_str());
                else 
                    ImGui::Text ("%s", queque_[i].c_str());
            }
            ImGui::End ();
        }

        if (show_zpage)
        {
            ImGui::Begin ("Z Page & Stack", &show_zpage);
            for (uint16_t row = 0x00; row < 256; row += 8) {
                std::string line (4, ' ');
                sprintf_s(const_cast<char*>(line.data()), line.size(), "$%02X", row);
                line[3] = ' ';
                for (uint16_t column = row; column < row + 8; column++) {
                    std::string val (3, ' ');
                    sprintf_s(const_cast<char*>(val.data()), val.size(), "%02X", nes_cpu->CPU6502.Read(column));
                    line += val.substr(0, 2) + " ";
                }
                ImGui::Text(line.c_str());
            }
            ImGui::Text("Stack");

            for (uint16_t row = nes_cpu->CPU6502.stackBottom + 0xFF; row > nes_cpu->CPU6502.stackBottom + 0xFF * 0.75; row -= 2) {
                std::string line(6, ' ');
                sprintf_s (const_cast<char*>(line.data ()), line.size (), "$%04X", row);
                line[5] = ' ';
                for (uint16_t column = row; column > row - 2; column--) {
                    std::string val (4, ' ');
                    sprintf_s (const_cast<char*>(val.data ()), val.size (), "%02X", nes_cpu->CPU6502.Read(column));
                    line += val.substr(0, 2) + " ";
                }
                ImGui::Text (line.c_str());
            }

            ImGui::End();
        }
        
        if(tex != -1) ImGui::Image((void*)(intptr_t)tex, { 22 * 8, 24 * 8 });

        ImGui::Render ();
        int display_w, display_h;
        glfwGetFramebufferSize (window, &display_w, &display_h);
        glViewport (0, 0, display_w, display_h);
        glClearColor (clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear (GL_COLOR_BUFFER_BIT);
        


        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown ();
    ImGui_ImplGlfw_Shutdown ();
    ImGui::DestroyContext ();

    glfwDestroyWindow (window);
    glfwTerminate ();

}
