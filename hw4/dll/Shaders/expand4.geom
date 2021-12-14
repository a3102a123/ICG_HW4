#version 430 core
layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 12) out;

in VS_OUT {
    vec3 normal;
    //the position in model space , "gl_in.gl_Position" is the position passed from vertex shader, so it's in screen space beacuse it have alread multipled with M V P matrix.
    vec3 position;
    vec2 uv;
} gs_in[];

uniform vec4 back_line;
uniform vec4 front_line;
uniform vec4 base_line_normal;
// default using fragment to expand a skin to cover the model
uniform bool is_frag = true;

out vec4 color;
out vec2 uv;
// the alpha value of texture
out float alpha;

bool CheckVertexPosition(){
    vec3 center;
    for(int i = 0 ; i < gl_in.length() ; i++){
        center += gs_in[i].position;
    }
    center /= gl_in.length();
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

void Fragment2Triangle(float range)
{
    color = vec4(1.0,1.0,1.0,1.0);
    vec4 center_normal;
    for(int i = 0 ; i < gl_in.length() ; i++){
        center_normal += vec4(gs_in[i].normal, 0.0f);
    }
    center_normal /= gl_in.length();
    if(CheckVertexPosition()){
        for(int i = 0 ; i < gl_in.length() ; i++){
            if(i == 3){
                center_normal = vec4(0.0,0.0,0.0,0.0);
                center_normal += vec4(gs_in[0].normal, 0.0f);
                center_normal += vec4(gs_in[2].normal, 0.0f);
                center_normal += vec4(gs_in[3].normal, 0.0f);
                center_normal /= 3;
                EndPrimitive();
                //color = vec4(0.0,1.0,1.0,1.0);
                uv = gs_in[0].uv;
                gl_Position = gl_in[0].gl_Position + center_normal;
                EmitVertex();
                uv = gs_in[2].uv;
                gl_Position = gl_in[2].gl_Position + center_normal;
                EmitVertex();
                uv = gs_in[3].uv;
                gl_Position = gl_in[3].gl_Position + center_normal;
                EmitVertex();
            }
            else{
                uv = gs_in[i].uv;
                gl_Position = gl_in[i].gl_Position + center_normal;
                EmitVertex();
            }
        }
        EndPrimitive();
    }
}

void erect(float len)
{
    color = vec4(1.0,1.0,1.0,1.0);
    vec4 center , center_normal;
    for(int i = 0 ; i < gl_in.length() ; i++){
        center += gl_in[i].gl_Position;
        center_normal += vec4(gs_in[i].normal, 0.0f);
    }
    center /= gl_in.length();
    center_normal /= gl_in.length();
    if(CheckVertexPosition()){
        color = vec4(1.0,1.0,1.0,1.0);
        for(int i = 0 ; i < gl_in.length() ; i++){
            uv = gs_in[i].uv;
            gl_Position = gl_in[i].gl_Position;
            EmitVertex();
            gl_Position = gl_in[(i+1) % gl_in.length()].gl_Position;
            EmitVertex();
            gl_Position = center + center_normal * len;
            EmitVertex();
            EndPrimitive();
        }
    }
}

void main()
{
    if(is_frag){
        alpha = 0;
        Fragment2Triangle(0);
    }
    else{
        alpha = 0.8;
        erect(0.5);
    }
}
