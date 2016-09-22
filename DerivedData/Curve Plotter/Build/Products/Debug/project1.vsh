#version 410 core

in vec2 position;
uniform vec4 scaleTrans;

void main()
{
	float ldsX = scaleTrans[0]*position.x + scaleTrans[1];
	float ldsY = scaleTrans[2]*position.y + scaleTrans[3];
	gl_Position = vec4(ldsX, ldsY, 0, 1);
}

