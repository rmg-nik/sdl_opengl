#version 330 core
//VERTEX SHADER 

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 color;

out vec4 ourColor;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f);
    //gl_Position = vec4(position.x, position.y, position.z, 1.0f); 
    TexCoord = vec2(texCoord.s, 1.0f - texCoord.t);
    ourColor = vec4(color, 1.0f);
}