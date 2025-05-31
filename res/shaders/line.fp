#version 430

struct MaterialInfo{
	vec3 Kd;
};

in vec3 v_Color;

uniform MaterialInfo Material;

out vec4 vFragColor;

void main(void)
{
    vFragColor = vec4(v_Color, 1);
}