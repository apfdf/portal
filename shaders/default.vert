
#version 330 core

layout (location = 0) in vec4 aPos;

uniform mat4 view_mat;
uniform mat4 project_mat;

out vec4 p;

void main() {

	p = aPos;
    //gl_Position = aPos;

	gl_Position = project_mat * view_mat * vec4(aPos.x, aPos.y, aPos.z, 1.0f);
	
}
