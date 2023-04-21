// FRAGMENT SHADER
#version 330

in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;

out vec4 outColor;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

// View Matrix
uniform mat4 matrixView;

// Texture
uniform sampler2D texture0;

// Shadowing
in vec4 shadowCoord;
uniform sampler2DShadow shadowMap;

struct SPOT
{
int on;
vec3 position;
vec3 diffuse;
vec3 specular;
vec3 direction;
float cutoff;
float auttenuation;
mat4 matrix;
};

struct POINT
{
int on;
vec3 position;
vec3 diffuse;
vec3 specular;
};
uniform SPOT lightSpot;

vec4 SpotLight(SPOT light)
{
	// Calculate Directional Light
	vec4 color = vec4(0, 0, 0, 0);

	// diffuse light
	vec3 L = normalize(light.matrix * vec4(light.position, 1) - position).xyz;
	float NdotL = dot(normal, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;

	// specular light
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal);
	float RdotV = dot(R, V);
	if (NdotL > 0 && RdotV > 0)
		color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);

	// spot light
	vec3 D = normalize(mat3(matrixView) * light.direction);
	float spotFactor = -dot(L, D);
	float angle = acos(spotFactor);
	if (angle <= (radians(light.cutoff))){
		spotFactor = pow(spotFactor, light.auttenuation);
	}	
	else{
		spotFactor = 0.0;
	}
	return spotFactor * color;
}

vec4 PointLight(POINT light)
{
	// Calculate Directional Light
	vec4 color = vec4(0, 0, 0, 0);

	// diffuse light
	vec3 L = normalize(matrixView * vec4(light.position, 1) - position).xyz;
	float NdotL = dot(normal, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;

	// specular light
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal);
	float RdotV = dot(R, V);
	if (NdotL > 0 && RdotV > 0)
		color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);

	
	return  color;
}

void main(void)
{
	outColor = color;
	// Calculation of the shadow
	float shadow = 1.0;

	if (lightSpot.on == 1){
		outColor += SpotLight(lightSpot);
		if (shadowCoord.w > 0) // if shadowCoord.w < 0 fragment is out of the Light POV
			shadow = 0.5 + 0.5 * textureProj(shadowMap, shadowCoord - 0.001);
	}
	
	outColor *= texture(texture0, texCoord0);

	outColor *= shadow;
}