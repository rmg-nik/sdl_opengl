#version 330 core

in vec3 position;
in vec3 color;
in vec2 texCoord;

out vec4 ourColor;
out vec2 TexCoord;

void main()
{
    gl_Position = vec4(position.x, position.y, position.z, 1.0); 
    ourColor = vec4(color, 1.0f);
    TexCoord = vec2(texCoord.s, 1.0f - texCoord.t);
}