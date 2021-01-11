#version 330

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 transform;
uniform vec2 TexCoordShift1;

out vec2 TexCoord;

void main()
{
    gl_Position = transform * vec4(aPos, 1.0);
	TexCoord = aTexCoord + TexCoordShift1;
}