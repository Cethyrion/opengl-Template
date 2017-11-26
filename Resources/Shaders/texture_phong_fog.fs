#version 430

in vec4 outFragmentPosition;
in vec3 outFragmentNormal;
in vec2 outFragmentTexCoord;

layout (binding=0) uniform sampler2D textureSample;
layout (binding=1) uniform sampler2D specularSample;
layout (binding=2) uniform sampler2D normalSample;

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
	vec3 position;
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

void main()
{
	vec3 ambient = material.ambient;

	vec3 positionToLight = normalize(light.position - vec3(outFragmentPosition));

	float diffuseIntensity = max(dot(positionToLight, outFragmentNormal), 0.0);

	vec3 diffuse = light.diffuse * material.diffuse * diffuseIntensity;

	vec3 specular = vec3(0.0);

	vec3 positionToView = normalize(-outFragmentPosition.xyz); 
	vec3 reflectLightVector = reflect(-positionToLight, outFragmentNormal);
	float specularIntensity = max(dot(reflectLightVector, positionToView), 0.0);

	specularIntensity = pow(specularIntensity, material.shininess);
	specular = light.specular * material.specular * specularIntensity;
	

	vec3 textureColor = vec3(texture(textureSample, outFragmentTexCoord));

	vec3 specularMapColor = vec3(texture(specularSample, outFragmentTexCoord));

	float specularMapValue = (specularMapColor.x + specularMapColor.y + specularMapColor.z) / 3;


	
	float distance = abs(outFragmentPosition.z);

	float fogIntensity = clamp((distance - fog.distanceMin) / (fog.distanceMax - fog.distanceMin), 0.0, 1.0);

	vec3 outVertexColor = textureColor * (ambient + diffuse) + (specular * specularMapValue);

	vec3 color = mix(outVertexColor, fog.color, fogIntensity);
	outFragmentColor = vec4(color, 1.0);
}