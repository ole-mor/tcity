#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

struct Material {
    vec4 baseColor;
    float metallic;
    float roughness;
};

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};

uniform Material material;
uniform Light lights[1];
uniform vec3 viewPos;

void main() {
    // Ambient lighting
    vec3 ambient = 0.1 * lights[0].color;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lights[0].position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lights[0].color;

    // Specular lighting
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * lights[0].color;

    vec3 lighting = (ambient + diffuse + specular) * material.baseColor.rgb;
    FragColor = vec4(lighting, material.baseColor.a);
}
