#version 430

struct MaterialInfo{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
};

uniform sampler2D Texture;
uniform MaterialInfo Material;
out vec4 vFragColor;

//lighting color
vec4 ambientColor  = vec4(0.1,0.1,0.1,1);
vec4 diffuseColor  = vec4(0.8,0.8,0.8,1);   
vec4 specularColor = vec4(1,1,1,1);

in vec3 vVaryingNormal;
in vec3 vVaryingLightDir;
in vec2 vTexCoord;

float Shininess = 128.0;//for material specular

void main(void)
{ 
    vec4 texColor = texture(Texture, vTexCoord);

    float diff = max(0.0, dot(normalize(vVaryingNormal), normalize(vVaryingLightDir)));
    vec4 diffuse = diff * texColor * vec4(Material.Kd, 1);

    vec4 ambient = ambientColor * vec4(Material.Ka, 1);

    vec3 vReflection = normalize(reflect(-normalize(vVaryingLightDir), normalize(vVaryingNormal)));

    float spec = 0.0;
    if (diff > 0.0) {
        spec = pow(max(0.0, dot(normalize(vVaryingNormal), vReflection)), Shininess);
    }
    vec4 specular = spec * specularColor * vec4(Material.Ks, 1);

    vFragColor = diffuse + ambient + specular;
    //vFragColor = vec4(1.0, 0.5, 0.6, 1.0);
}
	
    