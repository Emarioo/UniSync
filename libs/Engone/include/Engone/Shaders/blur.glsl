R"(
#shader vertex
#version 330 core

layout(location = 0) in vec4 vPos;

out vec2 fUV;

void main()
{
	//vec4 color = blur(vPos.zw);

	fUV=vPos.zw;

	//gl_Position = vec4((vPos.x*uSize.x+uPos.x)/uWindow.x*2-1, 1-(vPos.y*uSize.y+uPos.y)/uWindow.y*2, 0, 1);
	//gl_Position = vec4(vPos.x, vPos.y, 0, 1);
	//gl_Position = vec4(2*vPos.x-1, 2*vPos.y-1, color.w, 1);
	gl_Position = vec4(2*vPos.x-1, 2*vPos.y-1, 0, 1);
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 oColor;

//out float gl_FragDepth;

in vec2 fUV;

uniform sampler2D uTexture;
uniform vec2 uInvTextureSize;

vec4 blur(vec2 uv){
	const int kernelSize=3;
	float kernel[kernelSize*kernelSize] = float[](
	1,1,1,
	1,1,1,
	1,1,1
	);
	vec4 sum = vec4(0,0,0,0);
	float minDepth = 9999;
	float div=0;
	for(int i=0;i<kernelSize*kernelSize;i++){
		vec2 coord = vec2(0,0);
		float ox = i%kernelSize-1;
		float oy = i/kernelSize-1;

		coord.x=uv.x+ox*uInvTextureSize.x;
		coord.y=uv.y+oy*uInvTextureSize.y;

		vec4 tx = texture(uTexture,coord);
		if(tx.w<minDepth){
			minDepth = tx.w;
		}

		sum += kernel[i]*tx;
		div+=kernel[i];
	}
	sum/=div;
	//sum.w = minDepth;
	
	return sum;
}

void main()
{
	//vec4 color = blur(fUV);
	vec4 color = texture(uTexture,fUV);

	oColor = vec4(color.xyz,1);
	//oColor = vec4(color.w,color.w,color.w,color.w);
	//gl_FragDepth = 1-color.y;
};
)"