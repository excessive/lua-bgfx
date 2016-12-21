$input v_color, v_uv, v_normal

#include "bgfx_shader.sh"

SAMPLER2D(s_tex, 0);

const vec3 light = vec3(0.25, -0.25, 1.0);

void main()
{
	vec3 n = normalize(v_normal);
	vec3 l = normalize(light);
	float shade = max(dot(n, l), 0.0);

	vec4 out_color = texture2D(s_tex, v_uv) * v_color;
	out_color.rgb *= shade;

	gl_FragColor = out_color;
}
