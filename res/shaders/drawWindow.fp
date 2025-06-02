#version 460

in vec2 v_TexCoord; 

uniform sampler2D u_windowTexture;

out vec4 out_FragColor;

void main()
{
	out_FragColor = texture(u_windowTexture, v_TexCoord);
}