#version 330 core
layout (location = 2) in vec3 pPos;

void main()
{
	gl_Position = vec4(pPos, 1.0f);
}