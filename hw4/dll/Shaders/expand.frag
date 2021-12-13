#version 430

uniform sampler2D texture;

in vec2 uv;
in vec4 color;
out vec4 fragcolor;

void main()
{
	fragcolor = texture2D(texture, uv) * 0.8 + color * 0.2;
} 