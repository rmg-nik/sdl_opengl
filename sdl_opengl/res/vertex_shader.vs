#version 330 core
//VERTEX SHADER 

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texCoord;

out vec4 ourColor;
out vec2 TexCoord;

uniform mat4 transform;

void main()
{
	gl_Position = transform * vec4(position, 1.0f);
    //gl_Position = vec4(position.x, position.y, position.z, 1.0f); 
    TexCoord = vec2(texCoord.s, 1.0f - texCoord.t);
    ourColor = vec4(color, 1.0f);
}