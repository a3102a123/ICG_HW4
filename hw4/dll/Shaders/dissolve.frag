#version 430

uniform sampler2D texture;

in vec2 uv;
in vec4 color;
out vec4 fragcolor;

void main()
{
	if(uv == vec2(-1,-1))
		discard;
	else
		fragcolor = texture2D(texture, uv);
} 