#version 410 core

uniform int colorMode;

out vec4 fragmentColor;

void main()
{
	switch(colorMode)
	{
		case -1:
			fragmentColor = vec4(1.0, 1.0, 0.0, 1.0);
			break;
			
		case 1:
			fragmentColor = vec4(0.0, 1.0, 0.0, 1.0);
			break;
			
		case 2:
			fragmentColor = vec4(0.0, 0.0, 1.0, 1.0);
			break;
			
		case 3:
			fragmentColor = vec4(0.0, 1.0, 1.0, 1.0);
			break;
			
		default:
			fragmentColor = vec4(1.0, 0.0, 0.0, 1.0);
			break;
	}
}

