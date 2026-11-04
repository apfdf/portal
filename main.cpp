
/*

SDL3 är window och textures och bla bla bla
glad är opengl funktioner
glm är typ linjär aLgebra

*/

#include "glad/glad.h"
#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include <bits/stdc++.h>
using namespace std;

int main() {

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Could not init video", SDL_GetError());
    }
    SDL_GL_LoadLibrary(NULL);

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    SDL_Window* window;
    SDL_GLContext glcontext;

    int ww = 800;
    int wh = 800;
    window = SDL_CreateWindow("portal", ww, wh, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    glcontext = SDL_GL_CreateContext(window);
    gladLoadGL();

    SDL_GL_SetSwapInterval(1);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glad_glViewport(0, 0, ww, wh);
    glad_glClearColor(0.0f, 0.5f, 1.0f, 0.0f);

    bool quit = false;
    SDL_Event ev;
    while (!quit) {
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(window);
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT)
                quit = true;
        }
    }

}
