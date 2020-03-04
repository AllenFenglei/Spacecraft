#version 430  // GLSL version your computer supports
in layout(location=0) vec3 pos;
in layout(location=1) vec2 texUV;
in layout(location=2) vec3 normal;

uniform mat4 transform;
uniform mat4 view;
uniform float scale;
uniform int skybox;

out vec3 uv;
out vec3 normalWorld;
out vec3 posWorld;

void main()
{
	vec4 v = vec4(scale * pos, 1.0);
	vec4 pos_temp = transform * v;
	gl_Position = view * pos_temp;
	vec4 normal_temp = transform * vec4(normal, 0.0);
	normalWorld = normal_temp.xyz;
	posWorld = pos_temp.xyz;
	if (skybox == 1)
	{
		uv = pos;
	} else 
	{
		uv = vec3(texUV, 0.0);
	}
}