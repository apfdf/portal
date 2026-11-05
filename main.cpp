
/*

SDL3 är window och textures och bla bla bla
glad är opengl funktioner
glm är typ linjär aLgebra

*/

#include "glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL3/SDL.h>
#include <bits/stdc++.h>
using namespace std;

#define PI 3.14159265359

GLint compileShader(const char* filename, GLenum type) {

    FILE* file = fopen(filename, "rb");

    if (file == NULL) {
        std::cerr << "Cannot open shader " << filename << std::endl;
        abort();
    }

    fseek(file, 0, SEEK_END);
    const int size = ftell(file);
    rewind(file);

    const GLchar* source = new GLchar[size+1];
    fread(const_cast<char*>(source), sizeof(char), size, file);
    const_cast<char&>(source[size]) = '\0';

    const GLint shader = glCreateShader(type);

    if (not shader) {
        std::cerr << "Cannot create a shader of type " << shader << std::endl;
        abort();
    }

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    {
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (not compiled) {
            std::cerr << "Cannot compile shader " << filename << std::endl;
            abort();
        }
    }

    return shader;

}

GLint compileShaderProgram(const char* vertexShaderFilename, const char* fragmentShaderFilename) {

    const GLint program = glCreateProgram();

    if (not program) {
        std::cerr << "Cannot create a shader program" << std::endl;
        abort();
    }

    glAttachShader(program, compileShader(vertexShaderFilename, GL_VERTEX_SHADER));
    glAttachShader(program, compileShader(fragmentShaderFilename, GL_FRAGMENT_SHADER));

    glLinkProgram(program);

    {
        GLint linked;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (not linked) {
            std::cerr << "Cannot link shader program with shaders " << vertexShaderFilename << " and " << fragmentShaderFilename << std::endl;
            abort();
        }
    }

    return program;

}

struct ShaderProgram {

    int id;
    unordered_map<const char*, unsigned> uloc;

    void add_uloc(const char* name) {
        uloc[name] = glGetUniformLocation(id, name);
        if (uloc[name] == -1) {
            cerr << "error finding location for uniform: " << name << endl;
        }
    }

};

struct Player {

    glm::vec3 p;
    glm::vec3 v;
    float yaw;
    float pitch;
    float base_vel;
    float fov;

};

struct Portal {

    glm::mat3 orientation;

};

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

    vector<float> vertices = {
        0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };

    // shader program setup
    ShaderProgram default_program;
    default_program.id = compileShaderProgram("shaders/default.vert", "shaders/default.frag");
    glUseProgram(default_program.id);

    vector<const char*> uniform_names = {
        "view_mat",
        "project_mat"
    };
    for (const char* uname : uniform_names)
        default_program.add_uloc(uname);

    GLuint VAO, VBO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

    // game stuff
    Uint64 time = SDL_GetTicks();
    float dt = 0.0f;
    unordered_map<SDL_Keycode, bool> key_pressed;

    Player plr;
    plr.p = glm::vec3(0.0f);
    plr.yaw = 0.0f;
    plr.pitch = 0.0f;
    plr.base_vel = 1.0f;
    plr.fov = PI/2;

    bool quit = false;
    SDL_Event ev;
    while (!quit) {
        
        time = SDL_GetTicks();

        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(default_program.id);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindVertexArray(0);
        
        SDL_GL_SwapWindow(window);

        glm::vec3 move(0.0f);
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT)
                quit = true;
            if (ev.type == SDL_EVENT_KEY_DOWN) {
                key_pressed[ev.key.key] = true;
            } else if (ev.type == SDL_EVENT_KEY_UP) {
                key_pressed[ev.key.key] = false;
            }
        }

        if (key_pressed[SDLK_W])
            move.z -= plr.base_vel*dt;
        if (key_pressed[SDLK_S])
            move.z += plr.base_vel*dt;
        if (key_pressed[SDLK_A])
            move.x -= plr.base_vel*dt;
        if (key_pressed[SDLK_D])
            move.x += plr.base_vel*dt;
        if (key_pressed[SDLK_SPACE])
            move.y += plr.base_vel*dt;
        if (key_pressed[SDLK_LSHIFT])
            move.y -= plr.base_vel*dt;
        if (key_pressed[SDLK_ESCAPE])
            quit = true;
        
        glm::mat4 plr_rotate(1.0f);
        plr_rotate = glm::rotate(plr_rotate, plr.yaw, {0.0f, 1.0f, 0.0f});
        plr_rotate = glm::rotate(plr_rotate, plr.pitch, {1.0f, 0.0f, 0.0f});
        plr.v = plr_rotate * glm::vec4(move, 0.0f);
        plr.p += plr.v;

        cout << plr.p.x << " " << plr.p.y << " " << plr.p.z << endl;
        
        glm::mat4 view_mat = plr_rotate;
        view_mat = glm::translate(view_mat, -plr.p);
        glm::mat4 project_mat = glm::mat4(1.0f);
        project_mat = glm::perspective(plr.fov/2, (float)ww/wh, 0.01f, 100.0f);

        glUniformMatrix4fv(default_program.uloc["view_mat"], 1, 0, glm::value_ptr(view_mat));
        glUniformMatrix4fv(default_program.uloc["project_mat"], 1, 0, glm::value_ptr(project_mat));

        dt = (float)(SDL_GetTicks()-time)/1000;

    }

}
