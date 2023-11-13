R"(
#shader vertex
#version 330 core

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec2 vUV;
layout(location = 2) in vec4 vColor;
layout(location = 3) in float vTexture;

uniform vec2 uWindow;

out vec2 fUV;
out vec4 fColor;
flat out int fTexture;

void main()
{	
	fUV=vUV;
	fColor=vColor;
	fTexture=int(vTexture);

	gl_Position = vec4((vPos.x)/uWindow.x*2-1, 1-(vPos.y)/uWindow.y*2, 0, 1);
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 oColor;

in vec2 fUV;
in vec4 fColor;
flat in int fTexture;

uniform sampler2D uSampler[8];

void main()
{
	if (fTexture==-1){
		oColor = fColor;
	} else {
		oColor = texture(uSampler[fTexture], fUV);
	}
};
)"