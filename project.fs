#version 330 core

in vec3 normal;
in vec3 position;
uniform vec4 colour;
uniform vec3 Eye;
uniform vec3 light;
uniform vec4 material;

void main() {
	vec4 white = vec4(1.0, 1.0, 1.0, 1.0);
	float diffuse;
	vec3 L = normalize(light);
	vec3 N;
	vec3 R = normalize(reflect(-L,normal));
	float specular;
	N = normalize(normal);
	diffuse = dot(N,L);
	if(diffuse < 0.0) {
		diffuse = 0.0;
		specular = 0.0;
	} else {
		specular = pow(max(0.0, dot(N,R)),material.w);
	}
	
	gl_FragColor = min(material.x*colour + material.y*diffuse*colour + 
		material.z*white*specular, vec4(1.0));
	gl_FragColor.a = colour.a;
}