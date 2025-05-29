#version 430

flat in int v_faceID;

layout(location = 0) out int vFragColor;

void main()
{
    vFragColor = v_faceID;
}