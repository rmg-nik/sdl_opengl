#version 330 core

in vec4 ourColor;
in vec2 TexCoord;

out vec4 color;

uniform sampler2D ourTexture;

void main()
{
    color = texture(ourTexture, TexCoord);
    if (color.a < 1.0f)
    {
        vec2 size = textureSize(ourTexture, 0);
        int SIZE = 11;
        vec2 neighbor;        
        
        float R = SIZE * SIZE;
        float D = R;
        float A = 0.0f;        
        for (int i = -SIZE; i <= SIZE; ++i)
        {
            for (int j = -SIZE; j <= SIZE; ++j)
            {
                float d = i * i + j * j;
                if ( d > R)
                    continue;
                neighbor.x = (TexCoord.x * size.x + i) / size.x;
                neighbor.y = (TexCoord.y * size.y + j) / size.y;
                float a = texture(ourTexture, neighbor).a;
                if (a > 0.0f && d < D)
                {                 
                    A = a;
                    D = d;
                }
            }
        }
        if (A > 0.0f)
        {            
            color.r = 1.0f * (1.0f - color.a) + color.r * color.a;
            color.g = 0.0f * (1.0f - color.a) + color.g * color.a;
            color.b = 0.0f * (1.0f - color.a) + color.b * color.a;
            color.a = color.a + (1.0f - D / R) * (1.0f - color.a);
        }    
    }
}