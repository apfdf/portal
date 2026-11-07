
/*

SDL3 är window och textures och bla bla bla
glad är opengl funktioner
glm är linjär aLgebra

*/

#include "glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL3/SDL.h>
#include <bits/stdc++.h>
using namespace std;

#define PI 3.14159265359
#define FUZZ 0.1f

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
    float base_acc;
    float base_vel;
    float fall_vel;
    float fov;
    float r;
    bool floor_contact;
    bool wall_contact;

};

struct Portal {

    glm::mat3 orientation;

};

// 2d polygoner som spelaren kan kollidera med.
// tar in koordinater för hörnen, texture koordinater. (ha stor texturemap?)
// dela in i trianglar för att göra kollisionsdetektion enklare (och till rendering)
struct PhysTriangle {

    glm::vec3 a;
    glm::vec3 b;
    glm::vec3 c;

    glm::vec2 ta;
    glm::vec2 tb;
    glm::vec2 tc;

    glm::vec3 normal;
    bool is_floor;
    float area;

    PhysTriangle(
        glm::vec3 a, 
        glm::vec3 b, 
        glm::vec3 c,
        glm::vec2 ta = glm::vec2(0.0f),
        glm::vec2 tb = glm::vec2(0.0f),
        glm::vec2 tc = glm::vec2(0.0f)
    ) : a(a), b(b), c(c), ta(ta), tb(tb), tc(tc) {

        normal = glm::normalize(glm::cross(b-a, c-a));
        glm::vec3 up_vec = {0.0f, 1.0f, 0.0f};
        // is_floor <=> angle between floor_vec and normal
        is_floor = abs(glm::dot(normal, up_vec)) >= cos(PI/4);
        if (!is_floor) {
            cout << "test" << endl;
        }
        area = glm::length(glm::cross(b-a, c-a))/2;

    }

};

struct PhysObject {

    vector<PhysTriangle> triangles;

};

// projection of v on u
glm::vec3 proj(glm::vec3 u, glm::vec3 v) {
    if (glm::length(u) == 0.0f || glm::length(v) == 0.0f)
        return glm::vec3(0.0f);
    return (glm::dot(u, v)/(glm::length(u)*glm::length(v)))*u;
}

pair<bool, glm::vec3> collide_dif(PhysTriangle& tri, glm::vec3 p, float r) {

    glm::vec3 x = p + tri.normal*r;
    if ((glm::dot(tri.normal, p-tri.a) < 0.0f) == (glm::dot(tri.normal, x-tri.a) < 0.0f))
        x = p - tri.normal*r;
    if ((glm::dot(tri.normal, p-tri.a) < 0.0f) == (glm::dot(tri.normal, x-tri.a) < 0.0f))
        return {false, glm::vec3(0.0f)};
    
    // vektorn som sfären justeras längs
    glm::vec3 v(0.0f);
    if (tri.is_floor) {
        v = {0.0f, 1.0f, 0.0f};
    } else {
        // v = normal of triangle projected to xz-plane
        v = tri.normal - proj({0.0f, 1.0f, 0.0f}, tri.normal);
    }

    glm::vec3 intersect = p + (glm::dot(tri.a-p, tri.normal)/glm::dot(v, tri.normal))*v;
    float s = 0.0f;
    s += glm::length(glm::cross(intersect-tri.a, intersect-tri.b))/2;
    s += glm::length(glm::cross(intersect-tri.b, intersect-tri.c))/2;
    s += glm::length(glm::cross(intersect-tri.c, intersect-tri.a))/2;

    if (abs(s-tri.area) < FUZZ)
        return {true, intersect-x};
    else
        return {false, glm::vec3(0.0f)};

}

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

    glad_glViewport(0, 0, ww, wh);
    glad_glClearColor(0.0f, 0.5f, 1.0f, 0.0f);

    vector<PhysTriangle> triangles;
    triangles.push_back(PhysTriangle({0.0f, 0.0f, -1.0f}, {1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, -1.0f}));
    triangles.push_back(PhysTriangle({0.0f, 0.0f, -1.0f}, {1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, -1.0f}));
    triangles.push_back(PhysTriangle({4.0f, -1.0f, 4.0f}, {4.0f, -1.0f, -4.0f}, {-4.0f, -1.0f, -4.0f}));
    triangles.push_back(PhysTriangle({4.0f, -1.0f, 4.0f}, {-4.0f, -1.0f, 4.0f}, {-4.0f, -1.0f, -4.0f}));

    vector<float> vertices;
    for (PhysTriangle& tri : triangles) {
        vertices.push_back(tri.a.x);
        vertices.push_back(tri.a.y);
        vertices.push_back(tri.a.z);
        vertices.push_back(tri.b.x);
        vertices.push_back(tri.b.y);
        vertices.push_back(tri.b.z);
        vertices.push_back(tri.c.x);
        vertices.push_back(tri.c.y);
        vertices.push_back(tri.c.z);
    }

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
    SDL_SetWindowRelativeMouseMode(window, true);

    Player plr;
    plr.p = glm::vec3(0.0f);
    plr.v = glm::vec3(0.0f);
    plr.yaw = 0.0f;
    plr.pitch = 0.0f;
    plr.base_acc = 100.0f;
    plr.base_vel = 2.0f;
    plr.fall_vel = 2.0f;
    plr.fov = PI/2;
    plr.r = 0.5f;
    plr.floor_contact = false;
    plr.wall_contact = false;

    float surface_f = 10.0f;
    float air_f = 0.2f;
    float g = 9.82f;

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);

    bool quit = false;
    SDL_Event ev;
    while (!quit) {
        
        time = SDL_GetTicks();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(default_program.id);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindVertexArray(0);
        
        SDL_GL_SwapWindow(window);

        float dmx = 0.0f;
        float dmy = 0.0f;
        glm::vec3 move(0.0f);
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT)
                quit = true;
            if (ev.type == SDL_EVENT_KEY_DOWN) {
                if (ev.key.key == SDLK_SPACE && plr.floor_contact) {
                    plr.v.y = 2*g;
                }
                key_pressed[ev.key.key] = true;
            } else if (ev.type == SDL_EVENT_KEY_UP) {
                key_pressed[ev.key.key] = false;
            } else if (ev.type == SDL_EVENT_MOUSE_MOTION) {
                dmx = ev.motion.xrel;
                dmy = ev.motion.yrel;
            }
        }

        if (key_pressed[SDLK_W])
            move.z -= plr.base_acc;
        if (key_pressed[SDLK_S])
            move.z += plr.base_acc;
        if (key_pressed[SDLK_A])
            move.x -= plr.base_acc;
        if (key_pressed[SDLK_D])
            move.x += plr.base_acc;
        if (key_pressed[SDLK_SPACE] && plr.floor_contact)
            plr.v.y = 2*g;
        if (key_pressed[SDLK_ESCAPE])
            quit = true;
            
        float yawdif = (dmx/wh)*PI;
        float pitchdif = (dmy/wh)*PI;
        plr.yaw -= yawdif;
        plr.pitch -= pitchdif;
        
        glm::mat4 difrot = glm::rotate(glm::mat4(1.0f), yawdif, {0.0f, 1.0f, 0.0f});
        glm::mat4 totrot = glm::rotate(glm::mat4(1.0f), plr.yaw, {0.0f, 1.0f, 0.0f});
        plr.v = difrot * glm::vec4(plr.v, 0.0f);
        plr.v += totrot * glm::vec4(move, 0.0f) * dt;
        plr.v.y -= g*dt;

        // friction
        float xz_f = (plr.floor_contact) ? surface_f : air_f;
        glm::vec3 xz_v = {plr.v.x, 0.0f, plr.v.z};
        if (xz_v != glm::vec3(0.0f)) {
            glm::vec3 friction = glm::normalize(xz_v)*xz_f*dt;
            if (glm::length(friction) < glm::length(xz_v))
                plr.v -= friction;
            else
                plr.v -= xz_v;
            xz_v = {plr.v.x, 0.0f, plr.v.z};
            if (glm::length(xz_v) > plr.base_vel) {
                xz_v = glm::normalize(xz_v)*plr.base_vel;
                plr.v.x = xz_v.x;
                plr.v.z = xz_v.z;
            }
        }
        glm::vec3 y_v = {0.0f, plr.v.y, 0.0f};
        if (y_v != glm::vec3(0.0f)) {
            if (abs(plr.v.y) > plr.fall_vel) {
                plr.v.y = ((plr.v.y)/abs(plr.v.y))*plr.fall_vel;
            }
        }

        plr.v.y = max(plr.v.y, -2.0f);
        plr.v.y = min(plr.v.y, 2.0f);

        plr.p += plr.v*dt;

        // collision
        plr.floor_contact = false;
        plr.wall_contact = false;
        if (plr.v != glm::vec3(0.0f)) {
            for (PhysTriangle& tri : triangles) {
                auto [collides, adjust] = collide_dif(tri, plr.p, plr.r);
                if (collides) {
                    if (tri.is_floor) {
                        plr.floor_contact = true;
                        plr.v.y = 0.0f;
                    } else {
                        plr.wall_contact = true;
                        plr.v -= proj(adjust, plr.v);
                    }
                    plr.p += adjust;
                }
            }
        }

        glm::mat4 view_mat(1.0f);
        view_mat = glm::rotate(view_mat, -plr.pitch, {1.0f, 0.0f, 0.0f});
        view_mat = glm::rotate(view_mat, -plr.yaw, {0.0f, 1.0f, 0.0f});
        view_mat = glm::translate(view_mat, -plr.p);
        glm::mat4 project_mat = glm::mat4(1.0f);
        project_mat = glm::perspective(plr.fov/2, (float)ww/wh, 0.01f, 100.0f);

        glUniformMatrix4fv(default_program.uloc["view_mat"], 1, 0, glm::value_ptr(view_mat));
        glUniformMatrix4fv(default_program.uloc["project_mat"], 1, 0, glm::value_ptr(project_mat));

        dt = (float)(SDL_GetTicks()-time)/1000;

    }

}
