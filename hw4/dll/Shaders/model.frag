#version 430

uniform sampler2D texture;

in vec2 uv;

out vec4 color;

void main()
{
	color = texture2D(texture, uv);
} 
