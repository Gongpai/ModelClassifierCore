#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
    vec3 ViewPos;
} fs_in;

// --------- Uniform texture inputs -----
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

// --------- Light -------------
uniform vec3  lightColor; 
uniform float lightIntensity;   

uniform vec3 lightDirection = normalize(vec3(-0.3, -1.0, -0.5));

const float PI = 3.14159265359;

// --------- Helper Gamma ----------
vec3 SRGBToLinear(vec3 c) { return pow(c, vec3(2.2)); }
vec3 LinearToSRGB(vec3 c) { return pow(c, vec3(1.0/2.2)); }

void main()
{
    vec3 albedo = SRGBToLinear(texture(albedoMap, fs_in.TexCoords).rgb);
    if (length(albedo) < 0.001) albedo = vec3(1.0, 0.1, 0.1);
    // Normal (TBN to World)
    vec3 n = texture(normalMap, fs_in.TexCoords).rgb;
    n = normalize(n * 2.0 - 1.0);
    vec3 N = normalize(fs_in.TBN * n);

    // metallic, roughness, AO
    float metallic   = texture(metallicMap, fs_in.TexCoords).r;
    float roughness  = texture(roughnessMap, fs_in.TexCoords).r;
    float ao         = texture(aoMap, fs_in.TexCoords).r;

    vec3 skyColor    = lightColor;
    vec3 groundColor = lightColor * 0.75;
    float hemi = N.y * 0.5 + 0.5;
    vec3 hemiAmbient = mix(groundColor, skyColor, hemi);
    float ambientStrength = 0.4; 
    vec3 ambient = hemiAmbient * ambientStrength * lightIntensity;

    // ---- Directional Diffuse (Lit) ----
    float diff = max(dot(N, -lightDirection), 0.0);
    float diffuseStrength = 1.0;
    vec3 diffuse = diff * lightColor * lightIntensity * diffuseStrength;

    // ---- Sum Lighting ----
    vec3 lighting = ambient + diffuse;

    // --- AO/Metallic Logic ---
    // lighting *= (1.0 - metallic);
    lighting *= ao;

    vec3 color = lighting * albedo;

    // --- Tone Mapping + Gamma Correction ---
    color = color / (color + vec3(1.0));   // Simple Reinhard tone mapping
    color = LinearToSRGB(color);

    FragColor = vec4(color, 1.0);
}