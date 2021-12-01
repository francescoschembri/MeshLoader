

#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 normal;
    vec2 texCoords;
} gs_in[];

out vec2 TexCoords;
out vec3 Norm;

void adjust_vertex(vec4 position, vec3 norm, vec2 texCoords)
{    
    TexCoords = texCoords; 
    Norm = norm;
    gl_Position = position; 
    EmitVertex();
}

vec3 GetNormal()
{
    vec3 a = vec3(gl_in[1].gl_Position) - vec3(gl_in[0].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[0].gl_Position);
    return normalize(cross(b, a));
}

void main() {    
    vec3 normal = GetNormal();
    //vec3 normal = normalize(gs_in[0].normal + gs_in[1].normal + gs_in[2].normal);

    adjust_vertex(gl_in[0].gl_Position, normal, gs_in[0].texCoords);
    adjust_vertex(gl_in[1].gl_Position, normal, gs_in[1].texCoords);
    adjust_vertex(gl_in[2].gl_Position, normal, gs_in[2].texCoords);
    EndPrimitive();
}

