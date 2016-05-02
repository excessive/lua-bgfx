$input v_color0, v_normal

const vec3 l = vec3(0.0, 0.0, 1.0);

void main()
{
	vec3 n = normalize(v_normal);
	float intensity = dot(n, l);
	intensity = clamp(intensity, 0.5, 1.0);

	gl_FragColor = v_color0 * vec4(vec3(intensity), 1.0);
}
