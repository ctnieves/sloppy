#include <stdio.h>
#include "gbc_emu.hpp"

#include "imgui.h"
#include <SDL.h>

#define IMGUI_IMPL_OPENGL_LOADER_GLBINDING2 1

int main() {
    /*gui_init();*/
    gbc_emu emu = gbc_emu();
    emu.test();
   return 0;
}
