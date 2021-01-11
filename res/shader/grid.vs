#version 330

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 transform1;
uniform vec2 TexCoordShift;

void main()
{
    gl_Position = transform1 * vec4(aPos, 1.0);
	TexCoord = aTexCoord + TexCoordShift;
}

