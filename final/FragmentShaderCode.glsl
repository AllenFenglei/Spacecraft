#version 430 //GLSL version your computer supports

in vec3 uv;
in vec3 normalWorld;
in vec3 posWorld;
uniform sampler2D textureSampler;
uniform sampler2D normalSampler;
uniform samplerCube skyboxTexture;
uniform int skybox;
uniform vec3 ambientLight;
uniform vec3 lightPos1;
uniform vec3 lightPos2;
uniform float diffuseFactor;
uniform float specularFactor;
uniform vec3 eyePos;
uniform int green;
uniform int normalMapping;

out vec4 daColor;

void main()
{
	vec3 texColor;
	if (skybox == 1)
	{
		texColor = texture(skyboxTexture, uv).rgb;
	} else if (green == 1)
	{
		texColor = vec3(0.1, 1.0, 0.1);	
	} else
	{
		texColor = texture(textureSampler, uv.xy).rgb;
	}
	
	vec3 normal;
	if (normalMapping == 1)
	{
		normal = texture(normalSampler, uv.xy).rgb;
		normal = normalize(normal*2.0 - 1.0);
		vec3 z = normalWorld;
		vec3 x = normalize(cross(vec3(0.0, 1.0, 0.0), z));
		vec3 y = normalize(cross(z, x));
		mat3 rotation = mat3(x.x, x.y, x.z,
							 y.x, y.y, y.z,
							 z.x, z.y, z.z);
		normal = rotation * normal;
	} else 
	{
		normal = normalWorld;
	}
	vec3 lightVec = normalize(lightPos1 - posWorld);
	float brightness = dot(lightVec, normalize(normal));
	vec3 reflectVec = reflect(-lightVec, normal);
	vec3 eyeVec = normalize(eyePos - posWorld);
	float s = clamp(dot(reflectVec, eyeVec), 0, 1);
	s = pow(s, 50) * specularFactor;
	vec3 diffuseLight = clamp(vec3(brightness, brightness, brightness), 0, 1);
	vec3 color1 = texColor * (ambientLight + diffuseLight * diffuseFactor) + specularFactor * vec3(s, s, s);	
	lightVec = normalize(lightPos2 - posWorld);
	brightness = dot(lightVec, normalize(normal));
	reflectVec = reflect(-lightVec, normal);
	s = clamp(dot(reflectVec, eyeVec), 0, 1);
	s = pow(s, 50) * specularFactor;
	diffuseLight = clamp(vec3(brightness, brightness, brightness), 0, 1);
	vec3 color2 = texColor * (ambientLight + diffuseLight * diffuseFactor) + specularFactor * vec3(s, s, s);	
	vec3 color = color1 + color2;

	daColor = vec4(color, 1.0);
}
