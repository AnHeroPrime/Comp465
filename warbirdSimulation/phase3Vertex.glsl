/* 
phsae3Vertex.glsl

*/

# version 330 core

in vec4 vPosition;
in vec4 vColor;
in vec3 vNormal;
in vec2 vTexCoord;

out vec2 texCoord;
out vec3 position;
out vec3 normal;
out vec4 color;

uniform mat3 NormalMatrix; // = mat3 (view * model)
uniform mat4 ModelView; // = view * model
uniform mat4 ModelViewProjection;  // = projection * view * model
uniform bool isTexture;

void main() {
	gl_Position = ModelViewProjection * vPosition;
	position = (ModelView * vPosition).xyz;
	normal = normalize(NormalMatrix * vNormal);
	if(isTexture){
		texCoord = vTexCoord;
		color = vec4(0,0,0,0);
	}
	else {
	  texCoord = vec2(0, 0);
      color = vColor;
	}

}