#version 150

in  vec4 color;
out vec4 fColor;

uniform vec4 colorID;

void main()
{
	if (colorID.x >= 0.0 && colorID.y >= 0.0 && colorID.z >= 0.0)
		fColor = colorID;
	else
		fColor = color;
}
