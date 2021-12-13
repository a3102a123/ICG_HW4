#version 430 core
layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 12) out;

in VS_OUT {
    vec3 normal;
    vec3 position;
    vec2 uv;
} gs_in[];

uniform vec4 back_line;
uniform vec4 front_line;
uniform vec4 base_line_normal;

out vec4 color;
out vec2 uv;

bool CheckVertexPosition(){
    vec3 center;
    for(int i = 0 ; i < 4 ; i++){
        center += gs_in[i].position;
    }
    center /= 4;
    vec3 back_point = back_line.xyz;
    vec3 front_point = front_line.xyz;
    vec3 normal = base_line_normal.xyz;
    float back = dot(back_point,normal);
    float front = dot(front_point,normal);
    float pos = dot(center,normal);
    if(pos >= back && pos <= front)
        return true;
    else
        return false;
}

void Fragment2Triangle(int index,float range)
{
    color = vec4(1.0,1.0,1.0,1.0);
    //
    if(index > 2){
        gl_Position = gl_in[0].gl_Position + vec4(gs_in[index - 2].normal, 0.0f) * range;
        EmitVertex();
        gl_Position = gl_in[2].gl_Position + vec4(gs_in[index - 1].normal, 0.0f) * range;
        EmitVertex();
    }
    gl_Position = gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0f) * range;
    EmitVertex();
}

void bump(float len)
{
    color = vec4(1.0,1.0,1.0,1.0);
    vec4 center , center_normal;
    for(int i = 0 ; i < 4 ; i++){
        center += gl_in[i].gl_Position;
        center_normal += vec4(gs_in[i].normal, 0.0f);
    }
    center /= 4;
    center_normal /= 4;
    if(CheckVertexPosition()){
        color = vec4(1.0,1.0,1.0,1.0);
        for(int i = 0 ; i < 4 ; i++){
            gl_Position = gl_in[i].gl_Position;
            EmitVertex();
            gl_Position = gl_in[(i+1)%4].gl_Position;
            EmitVertex();
            gl_Position = center + center_normal * len;
            EmitVertex();
        }
    }
}

void main()
{
    uv = gs_in[0].uv;
    /*for(int i=0;i<gl_in.length();i++){
        Fragment2Triangle(i,0);
    }*/
    bump(0.5);
    EndPrimitive();
}
