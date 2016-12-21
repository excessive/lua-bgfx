$input v_color, v_normal

const vec3 light = vec3(0.25, -0.25, 1.0);

void main()
{
	vec3 n = normalize(v_normal);
	vec3 l = normalize(light);
	float shade = max(dot(n, l), 0.0);

	gl_FragColor = vec4(vec3(shade), 1.0) * v_color;
}
