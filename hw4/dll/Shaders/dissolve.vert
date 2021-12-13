#version 430

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 texcoord;

uniform mat4 M, V,P;
 
uniform vec4 back_line;
uniform vec4 front_line;
uniform vec4 base_line_normal;

out vec2 uv;

bool CheckVertexPosition(){
    vec3 back_point = back_line.xyz;
    vec3 front_point = front_line.xyz;
    vec3 normal = base_line_normal.xyz;
    float back = dot(back_point,normal);
    float front = dot(front_point,normal);
    float pos = dot(in_position,normal);
    if(pos >= back && pos <= front)
        return true;
    else
        return false;
}

void main() {
  if(CheckVertexPosition())
    uv = texcoord;
  else
    uv = vec2(-1,-1);
  gl_Position = P * V * M * vec4(in_position, 1.0);
}
