#version 430

uniform sampler2D texture;

in vec2 uv;
in vec4 color;
in float alpha;
out vec4 fragcolor;

void main()
{
	fragcolor = texture2D(texture, uv) * alpha + color * (1 - alpha);
} 