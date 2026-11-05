
#version 330 core

out vec4 color;
in vec4 p;

void main() {
    color = vec4(p.x, p.y, 1.0f, 0.0f);
}
