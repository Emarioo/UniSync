R"(
#shader vertex
#version 330 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vColor;

uniform mat4 uProj;

out vec3 fColor;

void main()
{
	fColor=vColor;
	gl_Position = uProj * vec4(vPos, 1);
	
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 oColor;

in vec3 fColor;

void main()
{
	oColor = vec4(fColor,1);
};
)"