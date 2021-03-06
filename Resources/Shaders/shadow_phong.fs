#version 430

in vec4 outFragmentPosition;
in vec3 outFragmentNormal;
in vec2 outFragmentTexCoord;

in vec4 outFragmentShadowCoord;

layout (binding=0) uniform sampler2D textureSample;
layout (binding = 1) uniform sampler2DShadow shadowMap;

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct Light
{
	vec3 diffuse;
	vec3 specular;
	vec4 position;
};

struct Fog
{
	float distanceMin;
	float distanceMax;
	vec3 color;
};

uniform Fog fog;

uniform Material material;
uniform Light light;

layout (location=0) out vec4 outFragmentColor;


float calculateShadow()
{
	float bias = 0.005;
	// shadow = 1.0 (not in shadow)
	// shadow = 0.0 (in shadow)
	float shadow = texture(shadowMap, vec3(outFragmentShadowCoord.xy, (outFragmentShadowCoord.z)/outFragmentShadowCoord.w) - bias);

	return shadow;
}


void phong(in vec4 position, in vec3 normal, out vec3 diffuse, out vec3 specular) 
{

	vec3 positionToLight = vec3(normalize(light.position - position));

	float diffuseIntensity = max(dot(positionToLight, normal), 0.0);

	diffuse = light.diffuse * material.diffuse * diffuseIntensity;


	vec3 positionToView = normalize(-position.xyz); 
	vec3 reflectLightVector = reflect(-positionToLight, normal);
	float specularIntensity = max(dot(reflectLightVector, positionToView), 0.0);

	specularIntensity = pow(specularIntensity, material.shininess);
	specular = light.specular * material.specular * specularIntensity;
}

void main()
{
	vec3 ambient = material.ambient;

	vec3 diffuse;
	vec3 specular;
	phong(outFragmentPosition, outFragmentNormal, diffuse, specular);


	vec3 textureColor = vec3(texture(textureSample, outFragmentTexCoord));
	
	float shadow = calculateShadow();

	vec3 outVertexColor = textureColor * (ambient + (diffuse + specular) * shadow);
	//vec3 outVertexColor = vec3(outFragmentShadowCoord.xy, (outFragmentShadowCoord.z)/outFragmentShadowCoord.w);

	outFragmentColor = vec4(outVertexColor, 1.0);
}