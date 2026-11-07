
#version 330 core

out vec4 color;
in vec4 p;

void main() {
    color = vec4(abs(p.x/5), abs(p.y/5), abs(p.z/5), 0.0f);
}
