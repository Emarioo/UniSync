R"(
#shader vertex
#version 330 core

layout(location = 0) in vec3 vPos;

uniform mat4 uProj;

void main()
{
	gl_Position = uProj * vec4(vPos, 1);
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 oColor;

void main()
{
	oColor = vec4(1,1,1,1);
};
)"