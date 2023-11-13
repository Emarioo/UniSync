R"(
#shader vertex
#version 430 core

const int ForceTypeNone=0;
const int ForceTypeAttractive=1;
const int ForceTypeFriction=2;
const int ForceTypeField=3;

const int ForceTypeCount=4;

struct FocalPoint {
	int forceType;
	vec3 position;
	vec3 velocity;
	float strength;
	float rangeMin;
	float rangeMax;
	float rangeBlend;
};
struct FocalPlane {
	int forceType;
	vec3 position;
	vec3 velocity;
	vec3 direction;
	float strength;
	float rangeMax;
	float rangeBlend;
};

layout(std430, binding = 0) restrict buffer SSBO
{
	float data[];
} buf;
const int bufStride = 3*2;

out vec4 fColor;

const int maxFocalPoints=5;
const int maxFocalPlanes=5;
uniform FocalPoint uFocalPoints[maxFocalPoints];
uniform FocalPlane uFocalPlanes[maxFocalPlanes];

uniform mat4 uProj;
uniform float uPointSize;
uniform int uClipping;

uniform float delta;

// Function from a video on youtube called newtonian particle system. You won't miss it.
vec3 newtonianColor(vec3 velocity){
	const float red = 0.0045 * dot(velocity, velocity);
    const float green = clamp(0.08 * max(velocity.x, max(velocity.y, velocity.z)), 0.2, 0.5);
    const float blue = 0.7 - red;
	return vec3(red,green,blue);
}
vec3 normalColor(vec3 velocity){
	return velocity/5;
}
vec3 warmth(float value) {
	vec3 warm = vec3(0,0,0);

	warm.x = min(2*(0+abs(value-0)),1);
	warm.y = 1.45*min(1.075-2.45*abs(value-0.5),0.8);
	warm.z = min(6*(0.3-abs(value-0.15)),0.94);

	warm.x = clamp(warm.x,0,1);
	warm.y = clamp(warm.y,0,1);
	warm.z = clamp(warm.z,0,1);

	return warm;
}

// NOTE: any physical formulas won't be 100% accurate because of poor precision in floats, i think? also just general error when computing physics?

void main() {
	// since structs can't have vec3 this is the way I have to do it.
	vec3 pos = vec3(buf.data[bufStride*gl_VertexID+0],buf.data[bufStride*gl_VertexID+1],buf.data[bufStride*gl_VertexID+2]);
	vec3 vel = vec3(buf.data[bufStride*gl_VertexID+3],buf.data[bufStride*gl_VertexID+4],buf.data[bufStride*gl_VertexID+5]);

	//-- Physics

	vec3 acc = vec3(0,0,0);

	vec3 debugColor=vec3(0,0,0);

	float velDist = sqrt(dot(vel,vel));
	vec3 velDir = vel/velDist;
	if(velDist==0){
		velDir=vec3(0,0,0);
	}
	int checkForceType=1;
	for(int i=0;i<maxFocalPoints;i++){
		FocalPoint focal = uFocalPoints[i]; // i is changed below which makes it unusable except for here.
		int tempType = checkForceType;
		if(i==maxFocalPoints-1){
			checkForceType++;
			if(checkForceType!=ForceTypeCount){
				i=-1;
			}
		}
		if(focal.forceType!=tempType){
			continue;
		}
		vec3 toFocal = focal.position-pos;
		float focalDist = sqrt(dot(toFocal,toFocal));
		vec3 focalDir = toFocal/focalDist;

		vec3 netVel = vel-focal.velocity;
		float netVelDist = sqrt(dot(netVel,netVel));
		vec3 netVelDir = netVel/netVelDist;
		if(netVelDist==0){
			netVelDir=vec3(0,0,0);
		}

		if(focalDist==0){
			focalDir=vec3(0,0,0);
		}
		if(focal.forceType==ForceTypeAttractive){
			if(focalDist>focal.rangeMin&&focalDist<focal.rangeMax){
				acc+=focalDir*focal.strength;
			}
		}else if(focal.forceType==ForceTypeField){
			if(focal.rangeMax>=focal.rangeMin){
				if(focalDist>focal.rangeMin&&focalDist<focal.rangeMax){
					// outwards - uses particle's velocity
					float againstField = dot(focalDir,netVelDir);
					if(againstField>0){
						float mul = dot(velDir,focalDir);
						vel -= focalDir*mul*velDist;

						vec3 sideDir = (velDir-focalDir*mul);
						sideDir/=sqrt(dot(sideDir,sideDir));
						acc -= sideDir*focal.strength;

						pos-=focalDir*(focal.rangeMax-focalDist)*focal.strength;
					}
				}
			}else{
				if(focalDist>focal.rangeMax&&focalDist<focal.rangeMin){
					// inwards - uses netVelocity
					float againstField = dot(focalDir,netVelDir);
					if(againstField<0){// since field is inwards againstField would be negative
						float mul = dot(netVelDir,focalDir);
						vel -= focalDir*mul*netVelDist;
			
						vec3 sideDir = (netVelDir-focalDir*mul);
						sideDir/=sqrt(dot(sideDir,sideDir));
						acc -= sideDir*focal.strength;
			
						pos-=focalDir*(focal.rangeMax-focalDist)*focal.strength*0.1;
					}
				}
			}
		}else if(focal.forceType==ForceTypeFriction){
			if(focalDist>focal.rangeMin&&focalDist<focal.rangeMax){
				vec3 sideDir = (velDir-focalDir*dot(velDir,focalDir));
				sideDir/=sqrt(dot(sideDir,sideDir)); // need to normalize it
				acc -= sideDir*focal.strength;
			}
		}
	}

	checkForceType=1;
	for(int i=0;i<maxFocalPlanes;i++){
		FocalPlane focal = uFocalPlanes[i];
		int tempType = checkForceType;
		if(i==maxFocalPlanes-1){
			checkForceType++;
			if(checkForceType!=ForceTypeCount){
				i=-1;
			}
		}
		if(focal.forceType!=tempType){
			continue;
		}

		vec3 netVel = vel-focal.velocity;
		float netVelDist = sqrt(dot(netVel,netVel));
		vec3 netVelDir = netVel/netVelDist;
		if(netVelDist==0){
			netVelDir=vec3(0,0,0);
		}

		vec3 toFocalPos = focal.position-pos;
		vec3 focalPosDir = toFocalPos/sqrt(dot(toFocalPos,toFocalPos)); // direction to focalPosition

		float focalCos = dot(focalPosDir,focal.direction); // negative when plane normal is facing the point
		vec3 focalDir = focal.direction;
		if(focalCos<0){
			focalDir = -focal.direction;
		}
		//acc+=focal.direction;

		// direction on the plan, not the normal
		vec3 planeDir = focalPosDir-focal.direction*focalCos; // direction to collision point
		planeDir = normalize(planeDir);
		// planeDir isn't normalized but it doesn't actually need to be.
		//acc += planeDir;

		float t = 0;
		/*if(planeDir.x-focal.direction.x!=0){ // prevent infinity
			t = (pos.x-focal.position.x)/(planeDir.x-focal.direction.x);
		}*/
		if(planeDir.y-focal.direction.y!=0){ // prevent infinity
			t = (pos.y-focal.position.y)/(planeDir.y-focal.direction.y);
		}
		float focalDist = 0;
		if(t!=0){
			focalDist = sqrt(dot(focalDir*t,focalDir*t));
		}
		if(t>0){
			focalDist=-focalDist;
			//debugColor=vec3(1,0,0);
		}

		if(focal.forceType==ForceTypeAttractive){
			if(focalDist<0&&focalDist<focal.rangeMax){
				acc+=focalDir*focal.strength;
			}
		}else if(focal.forceType==ForceTypeField){
			// ISSUE: focalDist stands for current position. But it doesn't account for if velocity will change position to inside the field.
			if(focalDist>0&&focalDist<focal.rangeMax){
				//debugColor=vec3(1,0,0);
				float againstField = dot(focal.direction,netVelDir);
				if(againstField<0){
					float mul = dot(netVelDir,focal.direction);
					vel -= focal.direction*mul*netVelDist;
						
					//vec3 sideDir = (velDir-focal.direction*mul);
					//sideDir/=sqrt(dot(sideDir,sideDir));
					//acc -= sideDir*focal.strength;
						
					pos+=focal.direction*(focal.rangeMax-focalDist)*focal.strength;
				}
			}
		}else if(focal.forceType==ForceTypeFriction){
			if(focalDist>0&&focalDist<focal.rangeMax){
				vec3 sideDir = (velDir-focalDir*dot(velDir,focalDir));
				sideDir/=sqrt(dot(sideDir,sideDir)); // need to normalize it
				acc -= sideDir*focal.strength;
			}
		}
	}

	//-- Euler integration
	//pos += vel*delta + 0.5f*acc*delta*delta;
	//vel += acc*delta;

	//-- Formulas v=at, s=vt
	vel += acc*delta;
	pos += vel*delta;

	//-- Setting new data
	buf.data[bufStride*gl_VertexID+0]=pos.x;
	buf.data[bufStride*gl_VertexID+1]=pos.y;
	buf.data[bufStride*gl_VertexID+2]=pos.z;
	buf.data[bufStride*gl_VertexID+3]=vel.x;
	buf.data[bufStride*gl_VertexID+4]=vel.y;
	buf.data[bufStride*gl_VertexID+5]=vel.z;

	//-- Coloring
	//float warmthValue = clamp(velSqr/60,0,1);
	//fColor = vec4(warmth(warmthValue),1);

	//fColor = vec4(normalColor(vel),1);

	fColor = vec4(newtonianColor(vel),1);
	//if(velDist<0.01)
	//	fColor.w=0;

	if(!(debugColor.x==0&&debugColor.y==0&&debugColor.z==0)){
		fColor = vec4(debugColor,1);
	}

	gl_Position = uProj * vec4(pos,1);
	
	float size = 1/gl_Position.z;

	//int clipped = 0;
	//int clipLevels=100;
	//float clipChance = 1-2*size;
	
	//if(gl_VertexID%clipLevels<clipChance*clipLevels&&1==uClipping){
	//	clipped=1;
	//}

	gl_PointSize = min(size,10);
	//if(clipped==1){
	//	gl_ClipDistance[0]=-1;
	//}else{
	//	gl_ClipDistance[0]=1;
	//}
};

#shader fragment
#version 430 core

layout(location = 0) out vec4 oColor;

in vec4 fColor;

void main()
{
	//oColor = vec4(1,1,1,1);
	oColor = fColor;
};
)"