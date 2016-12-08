/* 
phase3Fragment.glsl
*/

# version 330 core

in vec3 position;
in vec3 normal;
in vec4 color;
in vec2 texCoord;

out vec4 fragColor;

uniform vec3 HeadLightPosition;
uniform vec3 HeadLightIntensity;
uniform vec3 PointLightPosition; 
uniform vec3 PointLightIntensity;

uniform bool AmbientOn;
uniform bool HeadLightOn; // toggles set in application
uniform bool PointLightOn; // toggles set in application
uniform bool DebugOn; // glUniform1f(,);
uniform sampler2D Texture;
uniform bool isTexture;

// local "debug color" variables
vec3 ambientColor = vec3(1.0, 0.0, 0.0); // red ambient
vec3 diffuseColor = vec3(0.0, 1.0, 0.0); // green diffuse


vec3 vLight(vec3 LightPosition, vec3 LightIntensity, bool directional) {
	float ambient = 0.2f; // scale directional ambient
	
	if(!AmbientOn){
		float ambient = 0.0f; // turn off ambient
	}
	
	float diffuse = 0.0f; // compute diffuse in all cases
	vec3 n, s; // normal, light source
	if (directional){
		s = normalize(LightPosition);
	}
	else { // point light has no ambient
		s = normalize(LightPosition - position);
		ambient = 0.0f; // no ambient w/ Point light
	}
	n = normalize(normal);
	diffuse = max(0.0,dot(s, n) ); // reflected light
	if (DebugOn){
		return ambient * ambientColor + diffuse * diffuseColor;
	}
	else{
		return ambient * LightIntensity + diffuse * LightIntensity;
	}
}

void main() {
	vec3 tempColor = vec3(color) * 0.5f; // initial value
	
	if (HeadLightOn) {
		tempColor += vLight(HeadLightPosition, HeadLightIntensity, true);
	}
	if (PointLightOn){ 
		tempColor += vLight(PointLightPosition, PointLightIntensity, false);	
	}
	if(isTexture){
		fragColor = texture(Texture, texCoord);
	}else{
		fragColor = vec4(tempColor, 1.0);
	}
	//fragColor = color;
}
