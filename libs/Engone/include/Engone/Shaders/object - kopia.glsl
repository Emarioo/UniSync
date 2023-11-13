R"(
#shader vertex
#version 330 core

layout(location = 0) in vec3 vPos;
//layout(location = 1) in vec3 vNormal;
//layout(location = 2) in vec3 vTexture;// first two is uv second is material index

out vec3 fPos;
flat out vec3 fNormal;
out vec2 fUV;
flat out int fMat;
out vec4 fPosLightSpace;

uniform mat4 uProj;
uniform mat4 uTransform;
uniform mat4 uLightSpaceMatrix;

void main()
{
	//fPos = vec3(uTransform * vec4(vPos,1));
	//fNormal = fPos-vec3(uTransform * vec4(vPos-vNormal, 1));
	//fUV = vTexture.xy;
	//fMat = int(vTexture.z);
	//fPosLightSpace = uLightSpaceMatrix * vec4(fPos, 1);

	//fNormal = mat3(transpose(inverse(uTransform)))*vNormal; // Do this on the cpu and pass into the shader via uniform
	
	gl_Position = uProj * uTransform * vec4(vPos,1);
};

#shader fragment
#version 330 core

struct Material {
	sampler2D diffuse_map;
	vec3 diffuse_color;
	vec3 specular;
	float shininess;
};
struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
struct PointLight {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};
struct SpotLight {
	vec3 position;
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float cutOff;
	float outerCutOff;
};

out vec4 oColor;

const int N_POINTLIGHTS = 4;
const int N_SPOTLIGHTS = 2;
const int N_MATERIALS = 4;

uniform PointLight uPointLights[N_POINTLIGHTS];
uniform SpotLight uSpotLights[N_SPOTLIGHTS];
uniform DirLight uDirLight;
uniform ivec3 uLightCount;
uniform vec3 uCamera;
uniform Material uMaterials[N_MATERIALS];
uniform sampler2D shadow_map;

in vec3 fPos;
flat in vec3 fNormal;
in vec2 fUV;
flat in int fMat;
in vec4 fPosLightSpace;

float ShadowBias(float shadow, float bias) {
	return shadow - bias > 0 ? 1 : 0;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow) {
	vec3 lightDir = normalize(light.position - fragPos);

	vec3 ambient = light.ambient;

	float diff = max(dot(normal, lightDir), 0);
	vec3 diffuse = light.diffuse * diff;

	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0), 64*uMaterials[fMat].shininess);
	vec3 specular = light.specular * (spec * uMaterials[fMat].specular);
	
	float distance = length(light.position - fragPos);
	float attenuation = 1.0f / (light.constant +
		light.linear * distance +
		light.quadratic * (distance * distance));
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	
	//float bias = max(0.05 * (1. - dot(normal, lightDir)), 0.005);
	return (ambient + (1 - shadow) * (diffuse + specular));
}
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow) {
	vec3 lightDir = normalize(-light.direction);

	vec3 ambient = light.ambient; // *color;

	float diff = max(dot(normal, lightDir), 0);
	vec3 diffuse = light.diffuse * diff;// *color;

	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0), 64* uMaterials[fMat].shininess);
	vec3 specular = light.specular * (spec * uMaterials[fMat].specular);

	return (ambient + (1-shadow)*(diffuse + specular));
}
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir,float shadow) {
	vec3 lightDir = normalize(light.position - fragPos);

	float theta = dot(lightDir, normalize(-light.direction));

	vec3 ambient = light.ambient;// *color;
	if (theta > light.outerCutOff) {

		float diff = max(dot(normal, lightDir), 0);
		vec3 diffuse = light.diffuse * diff;// * color

		vec3 halfwayDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(normal, halfwayDir), 0), 64* uMaterials[fMat].shininess);
		vec3 specular = light.specular * (spec * uMaterials[fMat].specular);

		float epsilon = light.cutOff - light.outerCutOff;
		float intensity = clamp((theta - light.outerCutOff) / epsilon, 0, 1);

		diffuse *= intensity;
		specular *= intensity;
		return (ambient + (1 - shadow) * (diffuse + specular));
	} else {
		return ambient;
	}
}
float ShadowCalculation(vec4 fPosLightSpace) {
	// For different projection matrices
	vec3 projCoords = fPosLightSpace.xyz / fPosLightSpace.w;

	projCoords = projCoords * 0.5 + 0.5;
	if (projCoords.z>1) {
		return 0;
	}
	float bias = 0.005;
	float shadow = 0;
	vec2 texelSize = 1.f / textureSize(shadow_map, 0);
	for (int x = -1; x < 2;++x) {
		for (int y = -1; y < 2;++y) {
			float closestDepth = texture(shadow_map, projCoords.xy + vec2(x,y)*texelSize).r;
			shadow += projCoords.z - bias > closestDepth ? 1 : 0;
		}
	}
	
	return shadow/9;
	/*
	float closestDepth = texture(shadow_map, projCoords.xy).r;
	shadow = projCoords.z - bias > closestDepth ? 1 : 0;
	return shadow;
	*/
}

void main()
{
	//vec3 normal = texture(uMaterials.normal_map, fUV).rgb*fNormal;
	//normal = normalize((normal * 2 - 1));
	// vec3 normal = normalize(fNormal);

	// vec3 viewDir = normalize(uCamera - fPos);
	
	// float shadow = ShadowCalculation(fPosLightSpace);
	/*
	vec3 result = vec3(0);
	if(uLightCount.x==1)
		result += CalcDirLight(uDirLight, normal, viewDir,shadow);
	for (int i = 0; i < uLightCount.y;i++) {
		result += CalcPointLight(uPointLights[i], normal, fPos, viewDir, 0);
	}
	for (int i = 0; i < uLightCount.z; i++) {
		result += CalcSpotLight(uSpotLights[i], normal, fPos, viewDir, 0);
	}
	*/
	//texture(uMaterials[fMat].diffuse_map, fUV).rgb * 
	// result *= texture(uMaterials[fMat].diffuse_map, fUV).rgb * uMaterials[fMat].diffuse_color;
	oColor = vec4(1,1,1,1);
};
)"