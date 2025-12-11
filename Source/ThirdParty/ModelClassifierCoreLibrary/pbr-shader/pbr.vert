#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aTangent; // xyz = tangent, w = handedness
layout(location = 3) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
    vec3 ViewPos; // for convenience (copied from uniform in app)
} vs_out;

uniform vec3 uCameraPos; // optional: for passing to vertex->fragment

void main()
{
    vec3 posWorld = vec3(model * vec4(aPos, 1.0));
    vec3 N = normalize(mat3(transpose(inverse(model))) * aNormal);
    vec3 T = normalize(mat3(model) * aTangent.xyz);
    // Use handedness to compute bitangent sign
    vec3 B = cross(N, T) * aTangent.w;

    vs_out.TBN = mat3(T, B, N);
    vs_out.FragPos = posWorld;
    vs_out.TexCoords = aTexCoords;
    vs_out.ViewPos = uCameraPos;

    gl_Position = projection * view * vec4(posWorld, 1.0);
}
