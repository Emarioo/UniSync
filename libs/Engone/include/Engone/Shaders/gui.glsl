R"(
#shader vertex
#version 330 core

layout(location = 0) in vec4 vPos;

uniform vec2 uPos;
uniform vec2 uSize;
uniform vec2 uWindow;

out vec2 fUV;

void main()
{
	fUV=vPos.zw;
	gl_Position = vec4((vPos.x*uSize.x+uPos.x)/uWindow.x*2-1, 1-(vPos.y*uSize.y+uPos.y)/uWindow.y*2, 0, 1);
	//gl_Position = vec4(vPos.x, vPos.y, 0, 1);
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 oColor;

in vec2 fUV;

uniform sampler2D uTextures;
uniform vec4 uColor;
uniform int uColorMode;

void main()
{
	vec4 tx = texture(uTextures,fUV);
	if(uColorMode==0){
		oColor = uColor;
	}else if(uColorMode==1){
		oColor = uColor*tx;
	}else if(uColorMode==2){
		oColor = vec4(
			uColor.r*(uColor.a-tx.a)+tx.r*tx.a,
			uColor.g*(uColor.a-tx.a)+tx.g*tx.a,
			uColor.b*(uColor.a-tx.a)+tx.b*tx.a,
			uColor.a);
	}
};
)"