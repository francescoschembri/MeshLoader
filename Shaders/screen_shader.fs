#version 330 core

out vec4 FragColor;
flat in vec2 TexCoords;

uniform sampler2D screen_texture;

void main()
{    
    FragColor = texture(screen_texture, TexCoords);
}

