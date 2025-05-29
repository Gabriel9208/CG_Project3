#version 430

struct MaterialInfo{
	vec3 Kd;
};

uniform MaterialInfo Material;

out vec4 vFragColor;

void main(void)
{ 
    // Dot product gives us diffuse intensity
    float diff = max(0.0, dot(normalize(vVaryingNormal), normalize(vVaryingLightDir)));

    // Multiply intensity by diffuse color, force alpha to 1.0
    vFragColor = diff * diffuseColor * vec4(Material.Kd, 1);

    // Add in ambient light
    vFragColor += ambientColor;

    // Specular Light
    vec3 vReflection = normalize(reflect(-normalize(vVaryingLightDir), normalize(vVaryingNormal)));
    float spec = max(0.0, dot(normalize(vVaryingNormal), vReflection));
    if(diff != 0) {
		spec = pow(spec, Shininess);
		vFragColor += specularColor * vec4(Material.Ka, 1) * spec;
    }
}
	
    