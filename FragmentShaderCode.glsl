#version 430 core

in vec2 UV;
in vec3 FragPos;
in vec3 Normal;
out vec4 theColor;


uniform sampler2D objTexture;
uniform sampler2D objNormal;
uniform vec3 viewPos;
uniform bool normalMapping_flag;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
Material material;

struct DirLight {
    vec3 direction;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 intensity;
};
uniform DirLight dirLight;

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform PointLight pointLights[2];

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    material.diffuse = texture(objTexture, UV).rgb;
    material.ambient = texture(objTexture, UV).rgb;
    material.specular = vec3(0.3f, 0.3f, 0.3f);
    material.shininess = 16.0f;
    
    vec3 norm = normalize(Normal);
    if(normalMapping_flag) {
        norm = texture(objNormal, UV).rgb;
        norm = normalize(norm * 2.0 - 1.0);
    }
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    
    for(int i = 0; i < 1; i++) {
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }
    
    /*vec3 color = texture(objTexture, UV).rgb;
    theColor = vec4(color, 1.0f);*/
    
    theColor = vec4(result, 1.0f);
    
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // Combine results
    vec3 ambient = light.ambient * material.ambient;
    vec3 diffuse = light.intensity * light.diffuse * diff * material.diffuse;
    vec3 specular = light.intensity * light.specular * spec * material.specular;
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // Combine results
    vec3 ambient = light.ambient * material.ambient;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}
