#version 330 core

in vec4 ourColor;
in vec2 TexCoord;

out vec4 color;

uniform sampler2D ourTexture;

void main()
{
    vec2 size = textureSize(ourTexture, 0);
    int SIZE = 21;
    int DELTA = SIZE / 2;
    vec2 neighbor;
    color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < SIZE; ++i)
    {
        for (int j = 0; j < SIZE; ++j)
        {
            neighbor.x = (TexCoord.x * size.x + i - DELTA) / size.x;
            neighbor.y = (TexCoord.y * size.y + j - DELTA) / size.y;
            color += texture(ourTexture, neighbor);
        }
    }
    color = color / (SIZE * SIZE); 
}