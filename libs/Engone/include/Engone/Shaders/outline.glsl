R"(
#shader vertex
#version 330 core

layout(location = 0) in vec3 vPos;

uniform mat4 uProj;
uniform mat4 uTransform;

void main()
{
	gl_Position = uProj * uTransform * vec4(vPos, 1);
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 oColor;

uniform vec3 uColor;

void main()
{
	oColor = vec4(uColor,1);
	//oColor = vec4(1,1,1,1);
};
)"