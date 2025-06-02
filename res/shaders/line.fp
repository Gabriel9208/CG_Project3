#version 430

in vec3 v_Color;

out vec4 vFragColor;

void main(void)
{
    vFragColor = vec4(v_Color, 1);
}