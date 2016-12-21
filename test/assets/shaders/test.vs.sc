$input a_position, a_normal, a_color0
$output v_color, v_normal

#include "bgfx_shader.sh"

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
	v_color = a_color0;
	v_normal = a_normal;
}
