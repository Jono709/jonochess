#version 330

layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos.z, aPos.x, aPos.y, 1.0);
}