extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

// "but holo, this is C++, why are you using the C API?", you might ask.
// well, my dear reader, it is simple: C APIs are easier to comprehend.
//
// Lua doesn't really do C++-style magic anyhow.
#include <bgfx/c99/bgfx.h>
#include <bgfx/c99/bgfxplatform.h>
}

#include <bx/fpumath.h>

#include <cstring>
#include <map>

namespace {
	void perspective(
		float *_result,
		float aspect,
		float fovy     = 60.0f,
		float zNear    = 0.1f,
		bool  infinite = true,
		float zFar     = 1000.0f
	) {
		memset(_result, 0, sizeof(float)*16);

		float range  = tan(fovy / 2.0f);

		bool ogl_ndc = true;
		// for OpenGL
		if (ogl_ndc) {
			_result[11] = -1.0f;
		}
		else {
			_result[11] = 1.0f;
		}

		if (infinite) {
			range *= zNear;

			const float ep     = 0.0f;
			const float left   = -range * aspect;
			const float right  =  range * aspect;
			const float bottom = -range;
			const float top    =  range;

			_result[0] = (2.0f * zNear) / (right - left);
			_result[5] = (2.0f * zNear) / (top - bottom);
			_result[9] = ep - 1.0f;
			_result[14] = (ep - 2.0f) * zNear;
		}
		else {
			_result[0] = 1.0f / (aspect * range);
			_result[5] = 1.0f / (range);
			_result[9] = -(zFar + zNear) / (zFar - zNear);
			_result[14] = -(2.0f * zFar * zNear) / (zFar - zNear);
		}
	}
}

#ifndef luaL_typerror
LUALIB_API int luaL_typerror (lua_State *L, int narg, const char *tname) {
	const char *msg = lua_pushfstring(L,
		"%s expected, got %s", tname, luaL_typename(L, narg)
	);
	return luaL_argerror(L, narg, msg);
}
#endif

static bgfx_program_handle_t *to_program_ud(lua_State *L, int index) {
	bgfx_program_handle_t *ud = (bgfx_program_handle_t*)lua_touserdata(L, index);
	if (ud == NULL) luaL_typerror(L, index, "bgfx_program");
	return ud;
}

static bgfx_texture_handle_t *to_texture_ud(lua_State *L, int index) {
	bgfx_texture_handle_t *ud = (bgfx_texture_handle_t*)lua_touserdata(L, index);
	if (ud == NULL) luaL_typerror(L, index, "bgfx_texture");
	return ud;
}

static bgfx_vertex_decl_t *to_vertex_format_ud(lua_State *L, int index) {
	bgfx_vertex_decl_t *ud = (bgfx_vertex_decl_t*)lua_touserdata(L, index);
	if (ud == NULL) luaL_typerror(L, index, "bgfx_vertex_format");
	return ud;
}

static bgfx_vertex_buffer_handle_t *to_vertex_buffer_ud(lua_State *L, int index) {
	bgfx_vertex_buffer_handle_t *ud = (bgfx_vertex_buffer_handle_t*)lua_touserdata(L, index);
	if (ud == NULL) luaL_typerror(L, index, "bgfx_vertex_buffer");
	return ud;
}

static bgfx_index_buffer_handle_t *to_index_buffer_ud(lua_State *L, int index) {
	bgfx_index_buffer_handle_t *ud = (bgfx_index_buffer_handle_t*)lua_touserdata(L, index);
	if (ud == NULL) luaL_typerror(L, index, "bgfx_index_buffer");
	return ud;
}

static const luaL_Reg program_fn[] = {
	{ "__gc",  [](lua_State *L) {
		printf("gc shader program\n");
		bgfx_program_handle_t *ud = to_program_ud(L, 1);
		bgfx_destroy_program(*ud);
		return 0;
	} },
	{ "__tostring", [](lua_State *L) {
		char buff[32];
		sprintf(buff, "%p", to_program_ud(L, 1));
		lua_pushfstring(L, "bgfx_program (%s)", buff);
		return 1;
	} },
	{ NULL, NULL }
};

static const luaL_Reg texture_fn[] = {
	{ "__gc",  [](lua_State *L) {
		printf("gc texture\n");
		bgfx_texture_handle_t *ud = to_texture_ud(L, 1);
		bgfx_destroy_texture(*ud);
		return 0;
	} },
	{ "__tostring", [](lua_State *L) {
		char buff[32];
		sprintf(buff, "%p", to_texture_ud(L, 1));
		lua_pushfstring(L, "bgfx_texture (%s)", buff);
		return 1;
	} },
	{ NULL, NULL }
};

static const luaL_Reg vertex_format_fn[] = {
	{ "__tostring", [](lua_State *L) {
		char buff[32];
		sprintf(buff, "%p", to_vertex_format_ud(L, 1));
		lua_pushfstring(L, "bgfx_vertex_format (%s)", buff);
		return 1;
	} },
	{ NULL, NULL }
};

static const luaL_Reg vertex_buffer_fn[] = {
	{ "__gc",  [](lua_State *L) {
		printf("gc vb\n");
		bgfx_vertex_buffer_handle_t *ud = to_vertex_buffer_ud(L, 1);
		bgfx_destroy_vertex_buffer(*ud);
		return 0;
	} },
	{ "__tostring", [](lua_State *L) {
		char buff[32];
		sprintf(buff, "%p", to_vertex_buffer_ud(L, 1));
		lua_pushfstring(L, "bgfx_vertex_buffer (%s)", buff);
		return 1;
	} },
	{ NULL, NULL }
};

static const luaL_Reg index_buffer_fn[] = {
	{ "__gc",  [](lua_State *L) {
		printf("gc ib\n");
		bgfx_index_buffer_handle_t *ud = to_index_buffer_ud(L, 1);
		bgfx_destroy_index_buffer(*ud);
		return 0;
	} },
	{ "__tostring", [](lua_State *L) {
		char buff[32];
		sprintf(buff, "%p", to_index_buffer_ud(L, 1));
		lua_pushfstring(L, "bgfx_index_buffer (%s)", buff);
		return 1;
	} },
	{ NULL, NULL }
};

struct fuck_off_cpp {
	bool operator() (char const *a, char const *b) {
		return std::strcmp(a, b) < 0;
	}
};

// incomplete: doesn't include the macro function stuff
static std::map<const char*, uint32_t, fuck_off_cpp> buffer_lookup = {
	{ "none", BGFX_BUFFER_NONE },

	{ "compute_format_8x1",  BGFX_BUFFER_COMPUTE_FORMAT_8x1 },
	{ "compute_format_8x2",  BGFX_BUFFER_COMPUTE_FORMAT_8x2 },
	{ "compute_format_8x4",  BGFX_BUFFER_COMPUTE_FORMAT_8x4 },
	{ "compute_format_16x1", BGFX_BUFFER_COMPUTE_FORMAT_16x1 },
	{ "compute_format_16x2", BGFX_BUFFER_COMPUTE_FORMAT_16x2 },
	{ "compute_format_16x4", BGFX_BUFFER_COMPUTE_FORMAT_16x4 },
	{ "compute_format_32x1", BGFX_BUFFER_COMPUTE_FORMAT_32x1 },
	{ "compute_format_32x2", BGFX_BUFFER_COMPUTE_FORMAT_32x2 },
	{ "compute_format_32x4", BGFX_BUFFER_COMPUTE_FORMAT_32x4 },

	{ "compute_type_uint",  BGFX_BUFFER_COMPUTE_TYPE_UINT },
	{ "compute_type_int",   BGFX_BUFFER_COMPUTE_TYPE_INT },
	{ "compute_type_float", BGFX_BUFFER_COMPUTE_TYPE_FLOAT },

	{ "compute_read",  BGFX_BUFFER_COMPUTE_READ },
	{ "compute_write", BGFX_BUFFER_COMPUTE_WRITE },

	{ "draw_indirect", BGFX_BUFFER_DRAW_INDIRECT },
	{ "allow_resize",  BGFX_BUFFER_ALLOW_RESIZE },
	{ "index32",       BGFX_BUFFER_INDEX32 }//,

	//{ "compute_read_write", BGFX_BUFFER_COMPUTE_READ_WRITE }
};

static std::map<const char*, uint32_t, fuck_off_cpp> caps_lookup = {
	{ "texture_compare_lequal", BGFX_CAPS_TEXTURE_COMPARE_LEQUAL },
	{ "texture_compare_all",    BGFX_CAPS_TEXTURE_COMPARE_ALL },

	{ "vertex_attrib_half",   BGFX_CAPS_VERTEX_ATTRIB_HALF },
	{ "vertex_attrib_uint10", BGFX_CAPS_VERTEX_ATTRIB_UINT10 },

	{ "texture_3d",              BGFX_CAPS_TEXTURE_3D },
	{ "instancing",              BGFX_CAPS_INSTANCING },
	{ "renderer_multithreading", BGFX_CAPS_RENDERER_MULTITHREADED },
	{ "fragment_depth",          BGFX_CAPS_FRAGMENT_DEPTH },
	{ "blend_independent",       BGFX_CAPS_BLEND_INDEPENDENT },
	{ "compute",                 BGFX_CAPS_COMPUTE },
	{ "fragment_ordering",       BGFX_CAPS_FRAGMENT_ORDERING },
	{ "swap_chain",              BGFX_CAPS_SWAP_CHAIN },
	{ "hmd",                     BGFX_CAPS_HMD },
	{ "index32",                 BGFX_CAPS_INDEX32 },
	{ "draw_indirect",           BGFX_CAPS_DRAW_INDIRECT },
	{ "hidpi",                   BGFX_CAPS_HIDPI },
	{ "texture_blit",            BGFX_CAPS_TEXTURE_BLIT },
	{ "texture_read_back",       BGFX_CAPS_TEXTURE_READ_BACK },
	{ "occlusion_query",         BGFX_CAPS_OCCLUSION_QUERY },
	{ "alpha_to_converge",       BGFX_CAPS_ALPHA_TO_COVERAGE },
	{ "conservative_raster",     BGFX_CAPS_CONSERVATIVE_RASTER },

	{ "format_texture_none",             BGFX_CAPS_FORMAT_TEXTURE_NONE },
	{ "format_texture_2d",               BGFX_CAPS_FORMAT_TEXTURE_2D },
	{ "format_texture_2d_srgb",          BGFX_CAPS_FORMAT_TEXTURE_2D_SRGB },
	{ "format_texture_2d_emulated",      BGFX_CAPS_FORMAT_TEXTURE_2D_EMULATED },
	{ "format_texture_3d",               BGFX_CAPS_FORMAT_TEXTURE_3D },
	{ "format_texture_3d_srgb",          BGFX_CAPS_FORMAT_TEXTURE_3D_SRGB },
	{ "format_texture_3d_emulated",      BGFX_CAPS_FORMAT_TEXTURE_3D_EMULATED },
	{ "format_texture_cube",             BGFX_CAPS_FORMAT_TEXTURE_CUBE },
	{ "format_texture_cube_srgb",        BGFX_CAPS_FORMAT_TEXTURE_CUBE_SRGB },
	{ "format_texture_cube_emulated",    BGFX_CAPS_FORMAT_TEXTURE_CUBE_EMULATED },
	{ "format_texture_vertex",           BGFX_CAPS_FORMAT_TEXTURE_VERTEX },
	{ "format_texture_image",            BGFX_CAPS_FORMAT_TEXTURE_IMAGE },
	{ "format_texture_framebuffer",      BGFX_CAPS_FORMAT_TEXTURE_FRAMEBUFFER },
	{ "format_texture_framebuffer_msaa", BGFX_CAPS_FORMAT_TEXTURE_FRAMEBUFFER_MSAA },
	{ "format_texture_msaa",             BGFX_CAPS_FORMAT_TEXTURE_MSAA }
};

static std::map<const char*, uint32_t, fuck_off_cpp> clear_lookup = {
	{ "none",    BGFX_CLEAR_NONE },
	{ "color",   BGFX_CLEAR_COLOR },
	{ "depth",   BGFX_CLEAR_DEPTH },
	{ "stencil", BGFX_CLEAR_STENCIL },

	{ "discard_color_0", BGFX_CLEAR_DISCARD_COLOR_0 },
	{ "discard_color_1", BGFX_CLEAR_DISCARD_COLOR_1 },
	{ "discard_color_2", BGFX_CLEAR_DISCARD_COLOR_2 },
	{ "discard_color_3", BGFX_CLEAR_DISCARD_COLOR_3 },
	{ "discard_color_4", BGFX_CLEAR_DISCARD_COLOR_4 },
	{ "discard_color_5", BGFX_CLEAR_DISCARD_COLOR_5 },
	{ "discard_color_6", BGFX_CLEAR_DISCARD_COLOR_6 },
	{ "discard_color_7", BGFX_CLEAR_DISCARD_COLOR_7 },
	{ "discard_depth",   BGFX_CLEAR_DISCARD_DEPTH },
	{ "discard_stencil", BGFX_CLEAR_DISCARD_STENCIL }
};

static std::map<const char*, uint32_t, fuck_off_cpp> cube_map_lookup = {
	{ "positive_x", BGFX_CUBE_MAP_POSITIVE_X },
	{ "negative_x", BGFX_CUBE_MAP_NEGATIVE_X },
	{ "positive_y", BGFX_CUBE_MAP_POSITIVE_Y },
	{ "negative_y", BGFX_CUBE_MAP_NEGATIVE_Y },
	{ "positive_z", BGFX_CUBE_MAP_POSITIVE_Z },
	{ "negative_z", BGFX_CUBE_MAP_NEGATIVE_Z }
};

static std::map<const char*, uint32_t, fuck_off_cpp> debug_lookup = {
	{ "none",      BGFX_DEBUG_NONE },
	{ "wireframe", BGFX_DEBUG_WIREFRAME },
	{ "stats",     BGFX_DEBUG_STATS },
	{ "ifh",       BGFX_DEBUG_IFH },
	{ "text",      BGFX_DEBUG_TEXT }
};

static std::map<const char*, uint32_t, fuck_off_cpp> hmd_lookup = {
	{ "none",              BGFX_HMD_NONE },
	{ "device_resolution", BGFX_HMD_DEVICE_RESOLUTION },
	{ "rendering",         BGFX_HMD_RENDERING }
};

static std::map<const char*, uint32_t, fuck_off_cpp> reset_lookup = {
	{ "none",               BGFX_RESET_NONE },
	{ "fullscreen",         BGFX_RESET_FULLSCREEN },
	{ "msaa_x2",            BGFX_RESET_MSAA_X2 },
	{ "msaa_x4",            BGFX_RESET_MSAA_X4 },
	{ "msaa_x8",            BGFX_RESET_MSAA_X8 },
	{ "msaa_x16",           BGFX_RESET_MSAA_X16 },
	{ "vsync",              BGFX_RESET_VSYNC },
	{ "max_anisotropy",     BGFX_RESET_MAXANISOTROPY },
	{ "capture",            BGFX_RESET_CAPTURE },
	{ "hmd",                BGFX_RESET_HMD },
	{ "hmd_debug",          BGFX_RESET_HMD_DEBUG },
	{ "hmd_recenter",       BGFX_RESET_HMD_RECENTER },
	{ "flush_after_render", BGFX_RESET_FLUSH_AFTER_RENDER },
	{ "flip_after_render",  BGFX_RESET_FLIP_AFTER_RENDER },
	{ "srgb_backbuffer",    BGFX_RESET_SRGB_BACKBUFFER },
	{ "hidpi",              BGFX_RESET_HIDPI },
	{ "depth_clamp",        BGFX_RESET_DEPTH_CLAMP },
	{ "suspend",            BGFX_RESET_SUSPEND },
};

// incomplete: doesn't include the macro function stuff
static std::map<const char*, uint32_t, fuck_off_cpp> state_lookup = {
	{ "none",    BGFX_STATE_NONE },
	{ "default", BGFX_STATE_DEFAULT },

	{ "rgb_write",   BGFX_STATE_RGB_WRITE },
	{ "alpha_write", BGFX_STATE_ALPHA_WRITE },
	{ "depth_write", BGFX_STATE_DEPTH_WRITE },

	{ "depth_test_less",     BGFX_STATE_DEPTH_TEST_LESS },
	{ "depth_test_lequal",   BGFX_STATE_DEPTH_TEST_LEQUAL },
	{ "depth_test_equal",    BGFX_STATE_DEPTH_TEST_EQUAL },
	{ "depth_test_gequal",   BGFX_STATE_DEPTH_TEST_GEQUAL },
	{ "depth_test_greater",  BGFX_STATE_DEPTH_TEST_GREATER },
	{ "depth_test_notequal", BGFX_STATE_DEPTH_TEST_NOTEQUAL },
	{ "depth_test_never",    BGFX_STATE_DEPTH_TEST_NEVER },
	{ "depth_test_always",   BGFX_STATE_DEPTH_TEST_ALWAYS },

	{ "blend_zero",          BGFX_STATE_BLEND_ZERO },
	{ "blend_one",           BGFX_STATE_BLEND_ONE },
	{ "blend_src_color",     BGFX_STATE_BLEND_SRC_COLOR },
	{ "blend_inv_src_color", BGFX_STATE_BLEND_INV_SRC_COLOR },
	{ "blend_src_alpha",     BGFX_STATE_BLEND_SRC_ALPHA },
	{ "blend_inv_src_alpha", BGFX_STATE_BLEND_INV_SRC_ALPHA },
	{ "blend_dst_alpha",     BGFX_STATE_BLEND_DST_ALPHA },
	{ "blend_inv_dst_alpha", BGFX_STATE_BLEND_INV_DST_ALPHA },
	{ "blend_dst_color",     BGFX_STATE_BLEND_DST_COLOR },
	{ "blend_inv_dst_color", BGFX_STATE_BLEND_INV_DST_COLOR },
	{ "blend_src_alpha_sat", BGFX_STATE_BLEND_SRC_ALPHA_SAT },
	// { "blend_factor",        BGFX_STATE_BLEND_FACTOR },
	// { "blend_inv_factor",    BGFX_STATE_BLEND_INV_FACTOR },

	{ "alpha_to_coverage", BGFX_STATE_BLEND_ALPHA_TO_COVERAGE },

	{ "cull_cw",  BGFX_STATE_CULL_CW },
	{ "cull_ccw", BGFX_STATE_CULL_CCW },

	{ "pt_tristrip",  BGFX_STATE_PT_TRISTRIP },
	{ "pt_lines",     BGFX_STATE_PT_LINES },
	{ "pt_linestrip", BGFX_STATE_PT_LINESTRIP },
	{ "pt_points",    BGFX_STATE_PT_POINTS },

	{ "msaa",                BGFX_STATE_MSAA },
	{ "line_aa",             BGFX_STATE_LINEAA },
	{ "conservative_raster", BGFX_STATE_CONSERVATIVE_RASTER },

	{ "blend_add",         BGFX_STATE_BLEND_ADD },
	{ "blend_alpha",       BGFX_STATE_BLEND_ALPHA },
	{ "blend_darken",      BGFX_STATE_BLEND_DARKEN },
	{ "blend_lighten",     BGFX_STATE_BLEND_LIGHTEN },
	{ "blend_multiply",    BGFX_STATE_BLEND_MULTIPLY },
	{ "blend_normal",      BGFX_STATE_BLEND_NORMAL },
	{ "blend_screen",      BGFX_STATE_BLEND_SCREEN },
	{ "blend_linear_burn", BGFX_STATE_BLEND_LINEAR_BURN }
};

// incomplete: doesn't include the macro function stuff
static std::map<const char*, uint32_t, fuck_off_cpp> stencil_lookup = {
	{ "test_less",     BGFX_STENCIL_TEST_LESS },
	{ "test_lequal",   BGFX_STENCIL_TEST_LEQUAL },
	{ "test_equal",    BGFX_STENCIL_TEST_EQUAL },
	{ "test_gequal",   BGFX_STENCIL_TEST_GEQUAL },
	{ "test_greater",  BGFX_STENCIL_TEST_GREATER },
	{ "test_notequal", BGFX_STENCIL_TEST_NOTEQUAL },
	{ "test_never",    BGFX_STENCIL_TEST_NEVER },
	{ "test_always",   BGFX_STENCIL_TEST_ALWAYS },

	{ "op_fail_s_zero",    BGFX_STENCIL_OP_FAIL_S_ZERO },
	{ "op_fail_s_keep",    BGFX_STENCIL_OP_FAIL_S_KEEP },
	{ "op_fail_s_replace", BGFX_STENCIL_OP_FAIL_S_REPLACE },
	{ "op_fail_s_incr",    BGFX_STENCIL_OP_FAIL_S_INCR },
	{ "op_fail_s_incrsat", BGFX_STENCIL_OP_FAIL_S_INCRSAT },
	{ "op_fail_s_decr",    BGFX_STENCIL_OP_FAIL_S_DECR },
	{ "op_fail_s_decrsat", BGFX_STENCIL_OP_FAIL_S_DECRSAT },
	{ "op_fail_s_invert",  BGFX_STENCIL_OP_FAIL_S_INVERT },

	{ "op_fail_z_zero",    BGFX_STENCIL_OP_FAIL_Z_ZERO },
	{ "op_fail_z_keep",    BGFX_STENCIL_OP_FAIL_Z_KEEP },
	{ "op_fail_z_replace", BGFX_STENCIL_OP_FAIL_Z_REPLACE },
	{ "op_fail_z_incr",    BGFX_STENCIL_OP_FAIL_Z_INCR },
	{ "op_fail_z_incrsat", BGFX_STENCIL_OP_FAIL_Z_INCRSAT },
	{ "op_fail_z_decr",    BGFX_STENCIL_OP_FAIL_Z_DECR },
	{ "op_fail_z_decrsat", BGFX_STENCIL_OP_FAIL_Z_DECRSAT },
	{ "op_fail_z_invert",  BGFX_STENCIL_OP_FAIL_Z_INVERT },

	{ "op_pass_z_zero",    BGFX_STENCIL_OP_PASS_Z_ZERO },
	{ "op_pass_z_keep",    BGFX_STENCIL_OP_PASS_Z_KEEP },
	{ "op_pass_z_replace", BGFX_STENCIL_OP_PASS_Z_REPLACE },
	{ "op_pass_z_incr",    BGFX_STENCIL_OP_PASS_Z_INCR },
	{ "op_pass_z_incrsat", BGFX_STENCIL_OP_PASS_Z_INCRSAT },
	{ "op_pass_z_decr",    BGFX_STENCIL_OP_PASS_Z_DECR },
	{ "op_pass_z_decrsat", BGFX_STENCIL_OP_PASS_Z_DECRSAT },
	{ "op_pass_z_invert",  BGFX_STENCIL_OP_PASS_Z_INVERT },

	{ "none",    BGFX_STENCIL_NONE },
	{ "default", BGFX_STENCIL_DEFAULT }//,

	//{ "func_ref",   BGFX_STENCIL_FUNC_REF },
	//{ "func_rmask", BGFX_STENCIL_FUNC_RMASK }
};

static std::map<const char*, uint32_t, fuck_off_cpp> submit_eye_lookup = {
	{ "left",  BGFX_SUBMIT_EYE_LEFT },
	{ "right", BGFX_SUBMIT_EYE_RIGHT },
	{ "first", BGFX_SUBMIT_EYE_FIRST }
};

// incomplete: doesn't include the macro function stuff
static std::map<const char*, uint32_t, fuck_off_cpp> texture_lookup = {
	{ "none", BGFX_TEXTURE_NONE },

	{ "u_mirror", BGFX_TEXTURE_U_MIRROR },
	{ "u_clamp",  BGFX_TEXTURE_U_CLAMP },
	{ "u_border", BGFX_TEXTURE_U_BORDER },
	{ "v_mirror", BGFX_TEXTURE_V_MIRROR },
	{ "v_clamp",  BGFX_TEXTURE_V_CLAMP },
	{ "v_border", BGFX_TEXTURE_V_BORDER },
	{ "w_mirror", BGFX_TEXTURE_W_MIRROR },
	{ "w_clamp",  BGFX_TEXTURE_W_CLAMP },
	{ "w_border", BGFX_TEXTURE_W_BORDER },

	{ "min_point",       BGFX_TEXTURE_MIN_POINT },
	{ "min_anisotropic", BGFX_TEXTURE_MIN_ANISOTROPIC },
	{ "mag_point",       BGFX_TEXTURE_MAG_POINT },
	{ "mag_anisotropic", BGFX_TEXTURE_MAG_ANISOTROPIC },
	{ "mip_point",       BGFX_TEXTURE_MIP_POINT },

	{ "msaa_sample",   BGFX_TEXTURE_MSAA_SAMPLE },
	{ "rt",            BGFX_TEXTURE_RT },
	{ "rt_msaa_x2",    BGFX_TEXTURE_RT_MSAA_X2 },
	{ "rt_msaa_x4",    BGFX_TEXTURE_RT_MSAA_X4 },
	{ "rt_msaa_x8",    BGFX_TEXTURE_RT_MSAA_X8 },
	{ "rt_msaa_x16",   BGFX_TEXTURE_RT_MSAA_X16 },
	{ "rt_write_only", BGFX_TEXTURE_RT_WRITE_ONLY },

	{ "compare_less",     BGFX_TEXTURE_COMPARE_LESS },
	{ "compare_lequal",   BGFX_TEXTURE_COMPARE_LEQUAL },
	{ "compare_equal",    BGFX_TEXTURE_COMPARE_EQUAL },
	{ "compare_gequal",   BGFX_TEXTURE_COMPARE_GEQUAL },
	{ "compare_greater",  BGFX_TEXTURE_COMPARE_GREATER },
	{ "compare_notequal", BGFX_TEXTURE_COMPARE_NOTEQUAL },
	{ "compare_never",    BGFX_TEXTURE_COMPARE_NEVER },
	{ "compare_always",   BGFX_TEXTURE_COMPARE_ALWAYS },
	{ "compute_write",    BGFX_TEXTURE_COMPUTE_WRITE },

	{ "srgb",         BGFX_TEXTURE_SRGB },
	{ "blit_dst",     BGFX_TEXTURE_BLIT_DST },
	{ "read_back",    BGFX_TEXTURE_READ_BACK }//,
	//{ "border_color", BGFX_TEXTURE_BORDER_COLOR }
};

static std::map<const char*, uint32_t, fuck_off_cpp> view_lookup = {
	{ "none",   BGFX_VIEW_NONE },
	{ "stereo", BGFX_VIEW_STEREO }
};

const char *renderer2str(bgfx_renderer_type_t t) {
	switch(t) {
		case BGFX_RENDERER_TYPE_DIRECT3D9:  return "d3d9";
		case BGFX_RENDERER_TYPE_DIRECT3D11: return "d3d11";
		case BGFX_RENDERER_TYPE_DIRECT3D12: return "d3d12";
		case BGFX_RENDERER_TYPE_METAL:      return "metal";
		case BGFX_RENDERER_TYPE_OPENGLES:   return "opengles";
		case BGFX_RENDERER_TYPE_OPENGL:     return "opengl";
		case BGFX_RENDERER_TYPE_VULKAN:     return "vulkan";
		case BGFX_RENDERER_TYPE_NULL:       return "null";
		default: break;
	}
	return "invalid";
}

static void stack_dump(lua_State *L) {
	int i = lua_gettop(L);
	int m = i;
	printf("---------------- Stack Dump ----------------\n");
	while(i) {
		int t = lua_type(L, i);
		int n = -(m - i + 1);
		switch (t) {
			case LUA_TSTRING:
				printf("%d (%d):`%s'\n", i, n, lua_tostring(L, i));
				break;
			case LUA_TBOOLEAN:
				printf("%d (%d): %s\n", i, n, lua_toboolean(L, i) ? "true" : "false");
				break;
			case LUA_TNUMBER:
				printf("%d (%d): %g\n", i, n, lua_tonumber(L, i));
				break;
			default:
				printf("%d (%d): %s\n", i, n, lua_typename(L, t));
				break;
		}
		i--;
	}
	printf("--------------- Stack Dump Finished ---------------\n");
}

template <typename F>
static void table_scan(lua_State *L, int index, F fn) {
	lua_pushvalue(L, index);
	lua_pushnil(L);

	while (lua_next(L, -2)) {
		lua_pushvalue(L, -2);
		const char *key = lua_tostring(L, -1);
		const char *value = lua_tostring(L, -2);

		// printf("%s: %s\n", key, value);

		fn(key, value);

		lua_pop(L, 2);
	}

	lua_pop(L, 1);
}

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

static SDL_Window *_window = NULL;

static const luaL_Reg m[] = {
	// bgfx.init()
	// TODO: actually take some args for this
	{ "init", [](lua_State *L) {
		bool use_sdl = false;
		if (lua_isboolean(L, 1)) {
			use_sdl = lua_toboolean(L, 1) ? true : false;
		}
		if (use_sdl) {
			SDL_InitSubSystem(SDL_INIT_VIDEO);
			_window = SDL_CreateWindow("bgfx!",
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				1280, 720, 0
				| SDL_WINDOW_ALLOW_HIGHDPI
				| SDL_WINDOW_OPENGL
			);
			bgfx_platform_data_t data;
			SDL_SysWMinfo wmi;
			SDL_VERSION(&wmi.version);
			if (!SDL_GetWindowWMInfo(_window, &wmi)) {
				return 0;
			}
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
			data.ndt          = wmi.info.x11.display;
			data.nwh          = (void*)(uintptr_t)wmi.info.x11.window;
#elif BX_PLATFORM_OSX
			data.ndt          = NULL;
			data.nwh          = wmi.info.cocoa.window;
#elif BX_PLATFORM_WINDOWS
			data.ndt          = NULL;
			data.nwh          = wmi.info.win.window;
#elif BX_PLATFORM_STEAMLINK
			data.ndt          = wmi.info.vivante.display;
			data.nwh          = wmi.info.vivante.window;
#endif // BX_PLATFORM_
			data.context      = NULL;
			data.backBuffer   = NULL;
			data.backBufferDS = NULL;

			bgfx_set_platform_data(&data);
		}
		if (!bgfx_init(BGFX_RENDERER_TYPE_OPENGL, BGFX_PCI_ID_NONE, 0, NULL, NULL)) {
			printf(":(\n");
			return 0;
		}
		return 0;
	} },

	// bgfx.shutdown()
	{ "shutdown", [](lua_State *) {
		bgfx_shutdown();
		if (_window) {
			SDL_DestroyWindow(_window);
			SDL_QuitSubSystem(SDL_INIT_VIDEO);
		}
		return 0;
	} },

	// bgfx.debug_text_print(1, 1, 0x7f, string.format("hello %s!", "world"))
	{ "debug_text_print", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n == 4);
		(void)n;
		uint16_t x = (uint16_t)lua_tonumber(L, 1);
		uint16_t y = (uint16_t)lua_tonumber(L, 2);
		uint8_t attr = (uint8_t)lua_tonumber(L, 3);
		const char *str = lua_tostring(L, 4);
		// printf("%dx%d 0x%2x %s\n", x, y, attr, str);
		bgfx_dbg_text_printf(x, y, attr, "%s", str);
		return 0;
	} },

	// bgfx.debug_text_clear()
	{ "debug_text_clear", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n >= 0 || n <= 2);
		uint8_t attr = n >= 1 ? (uint8_t)lua_tonumber(L, 1) : 0;
		bool small   = n >= 2 ? (bool)lua_toboolean(L, 2) : false;
		bgfx_dbg_text_clear(attr, small);
		return 0;
	} },

	// bgfx.set_debug {
	// 	"wireframe",
	// 	"stats",
	// 	"ifh",
	// 	"text"
	// }
	{ "set_debug", [](lua_State *L) {
		uint32_t debug = 0;

		table_scan(L, 1, [&](const char *, const char *v) {
			auto val = debug_lookup.find(v);
			if (val != debug_lookup.end()) {
				debug |= val->second;
			}
		});

		bgfx_set_debug(debug);

		return 0;
	} },

	// bgfx.reset (1280, 720, {
	// 	"vsync",
	// 	"depth_clamp",
	// 	"srgb_backbuffer",
	// 	"flip_after_render",
	// 	"flush_after_render"
	// })
	{ "reset", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n == 3);
		(void)n;
		int w = (int)lua_tonumber(L, 1);
		int h = (int)lua_tonumber(L, 2);
		uint32_t reset = 0;

		table_scan(L, 3, [&](const char *, const char *v) {
			auto val = reset_lookup.find(v);
			if (val != reset_lookup.end()) {
				reset |= val->second;
			}
		});

		bgfx_reset(w, h, reset);
		return 0;
	} },

	// bgfx.reset_view(0)
	{ "reset_view", [](lua_State *L){
		bgfx_reset_view((uint8_t)luaL_checkinteger(L, 1));
		return 0;
	} },

	// bgfx.touch(0)
	{ "touch", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n == 1);
		(void)n;
		uint8_t id = (uint8_t)lua_tonumber(L, 1);
		unsigned int ret = bgfx_touch(id);
		lua_pushnumber(L, double(ret));
		return 1;
	} },

	// bgfx.frame()
	{ "frame", [](lua_State *L) {
		int r = bgfx_frame();
		lua_pushnumber(L, (double)r);
		return 1;
	} },

	// bgfx.set_state {
	// 	"msaa",
	// 	"alpha_write",
	// 	"rgb_write",
	// 	"cull_ccw",
	// 	"depth_write",
	// 	"depth_test_lequal"
	// }
	{ "set_state", [](lua_State *L) {
		uint64_t flags = 0;
		uint32_t rgba = 0;

		table_scan(L, 1, [&](const char *, const char *v) {
			auto val = state_lookup.find(v);
			if (val != state_lookup.end()) {
				flags |= val->second;
			}
		});

		bgfx_set_state((uint32_t)flags, rgba);

		return 0;
	} },

	{ "set_view_sequential", [](lua_State *L){
		bool enabled = true;
		if (lua_isboolean(L, 2)) {
			enabled = lua_toboolean(L, 2) ? true : false;
		}
		bgfx_set_view_seq((uint8_t)luaL_checkinteger(L, 1), enabled);
		return 0;
	} },

	// bgfx.set_view_rect(0, 0, 0, 1280, 720)
	// bgfx.set_view_rect(0) -- auto set to full extents
	{ "set_view_rect", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n >= 1);
		uint8_t id = (uint8_t)lua_tonumber(L, 1);
		uint16_t x = 0;
		uint16_t y = 0;
		if (n >= 3) {
			x = (uint16_t)lua_tonumber(L, 2);
			y = (uint16_t)lua_tonumber(L, 3);
		}
		if (n <= 3) {
			bgfx_set_view_rect_auto(id, x, y, BGFX_BACKBUFFER_RATIO_EQUAL);
			return 0;
		}
		if (n == 5) {
			uint16_t w = (uint16_t)lua_tonumber(L, 4);
			uint16_t h = (uint16_t)lua_tonumber(L, 5);
			bgfx_set_view_rect(id, x, y, w, h);
		} else {
			lua_assert(false);
		}
		return 0;
	} },

	// bgfx.set_view_scissor(0, 400, 300, 800, 600)
	{ "set_view_scissor", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n == 5);
		(void)n;
		uint8_t id = (uint8_t)lua_tonumber(L, 1);
		uint16_t x = (uint16_t)lua_tonumber(L, 2);
		uint16_t y = (uint16_t)lua_tonumber(L, 3);
		uint16_t w = (uint16_t)lua_tonumber(L, 4);
		uint16_t h = (uint16_t)lua_tonumber(L, 5);
		bgfx_set_view_scissor(id, x, y, w, h);
		return 0;
	} },

	{ "set_view_clear", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n == 5);
		(void)n;
		uint8_t id = (uint8_t)lua_tonumber(L, 1);

		uint16_t flags = 0;
		table_scan(L, 2, [&](const char *, const char *v) {
			auto val = clear_lookup.find(v);
			if (val != clear_lookup.end()) {
				flags |= val->second;
			}
		});

		uint32_t rgba = (uint32_t)lua_tonumber(L, 3);
		float depth = (float)lua_tonumber(L, 4);
		uint8_t stencil = (uint8_t)lua_tonumber(L, 5);
		bgfx_set_view_clear(id, flags, rgba, depth, stencil);
		return 0;
	} },

	// bgfx.set_view_name("steve")
	{ "set_view_name", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n == 2);
		(void)n;
		uint8_t id = (uint8_t)lua_tonumber(L, 1);
		const char *name = lua_tostring(L, 2);
		bgfx_set_view_name(id, name);
		return 0;
	} },

	// bgfx.set_marker("miku")
	{ "set_marker", [](lua_State *L) {
		bgfx_set_marker(luaL_checkstring(L, 1));
		return 0;
	}},

	// bgfx.submit(0, program)
	// bgfx.submit(0, program, 0, false)
	{ "submit", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n == 4);
		uint8_t id = (uint8_t)luaL_checkinteger(L, 1);

		bgfx_program_handle_t *program = to_program_ud(L, 2);
		int32_t depth = 0;
		bool preserve_state = false;
		if (n >= 3) {
			depth = (int32_t)lua_tonumber(L, 3);
		}
		if (n >= 4) {
			preserve_state = lua_toboolean(L, 4) ? true : false;
		}
		uint32_t r = bgfx_submit(id, *program, depth, preserve_state);
		lua_pushnumber(L, r);
		return 1;
	} },

	{ "dispatch", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n == 4);

		uint8_t id = (uint8_t)lua_tonumber(L, 1);
		bgfx_program_handle_t *program = to_program_ud(L, 2);

		uint16_t nx = luaL_checkinteger(L, 3);
		uint16_t ny = luaL_checkinteger(L, 4);
		uint16_t nz = luaL_checkinteger(L, 5);

		// TODO: stereo support
		uint32_t r = bgfx_dispatch(id, *program, nx, ny, nz, 0);

		lua_pushnumber(L, r);

		return 1;
	}},

	// TODO: Stereo views
	{ "set_view_transform", [](lua_State *L) {
		float view[16], proj[16];
		memset(view, 0, 16*sizeof(float));
		memset(proj, 0, 16*sizeof(float));

		auto load_matrix = [](lua_State *L, int index, float *mtx) {
			lua_pushvalue(L, index);
			int num = lua_objlen(L, -1);

			// we only accept 4x4 matrices
			if (num % 16 != 0) {
				printf("Invalid table length %d, must be divisible by 16.\n", num);
				return;
			}

			for (int i=1; i<=16; i++) {
				lua_rawgeti(L, -1, i);
				if (lua_isnil(L,-1)) {
					lua_pop(L, 1);
					break;
				}
				mtx[i-1] = (float)luaL_checknumber(L, -1);
				lua_pop(L, 1);
			}

			// printf(
			// 	"%+2.2f %+2.2f %+2.2f %+2.2f\n"
			// 	"%+2.2f %+2.2f %+2.2f %+2.2f\n"
			// 	"%+2.2f %+2.2f %+2.2f %+2.2f\n"
			// 	"%+2.2f %+2.2f %+2.2f %+2.2f\n\n",
			// 	mtx[0], mtx[4], mtx[8],  mtx[12],
			// 	mtx[1], mtx[5], mtx[9],  mtx[13],
			// 	mtx[2], mtx[6], mtx[10], mtx[14],
			// 	mtx[3], mtx[7], mtx[11], mtx[15]
			// );

			lua_pop(L, 1);
		};

		uint8_t id = (uint8_t)luaL_checkinteger(L, 1);

		// float x90 = bx::toRad(-90.0f);
		// float fov = bx::toRad(60.0f);
		// float aspect = 1280.f/720.f;
		//
		// float tmp[16], rot[16];
		// bx::mtxTranslate(tmp, 0.0f, 2.0f, -1.6f);
		// bx::mtxRotateX(rot, -x90);
		// bx::mtxMul(view, tmp, rot);// rot, tmp);
		//
		// ::perspective(proj, aspect, fov);//, 0.01f, true, 1000.0f);
		// // bgfx::setViewTransform(0, view, tmp);

		// bx::mtxTranslate(view, float _tx, float _ty, float _tz)
		load_matrix(L, 2, view);
		load_matrix(L, 3, proj);

		bgfx_set_view_transform(id, view, proj);
		// bgfx_set_view_transform_stereo(id, view, proj_l, flags, proj_r);

		return 0;
	} },

	// bgfx.set_transform(mtx)
	{ "set_transform", [](lua_State *L) {
		// TODO: accept tables of matrices.
		int num = lua_objlen(L, 1);

		// we only accept 4x4 matrices
		if (num % 16 != 0) {
			printf("Invalid table length %d, must be divisible by 16.\n", num);
			return 0;
		}

		float *mtx = new float[num];
		for (int i=1; ; i++) {
			lua_rawgeti(L, -1, i);
			if (lua_isnil(L,-1)) {
				lua_pop(L, 1);
				break;
			}
			mtx[i-1] = (uint16_t)luaL_checkinteger(L, -1);
			lua_pop(L, 1);
		}

		bgfx_set_transform(mtx, num / 16);

		delete[] mtx;

		return 0;
	} },

	// bgfx.new_texture(data, width, height)
	{ "new_texture", [](lua_State *L) {
		if (!lua_isstring(L, 1)) {
			lua_pushboolean(L, 0);
			return 1;
		}

		size_t size = 0;
		const char *data = lua_tolstring(L, 1, &size);

		bgfx_texture_info_t info;
		info.format       = BGFX_TEXTURE_FORMAT_RGBA8;
		info.storageSize  = size;
		info.width        = luaL_checkinteger(L, 2);
		info.height       = luaL_checkinteger(L, 3);
		info.depth        = 0;
		info.numMips      = 0;
		info.bitsPerPixel = 32;
		info.cubeMap      = false;

		bgfx_texture_handle_t *ud = (bgfx_texture_handle_t*)lua_newuserdata(L, sizeof(bgfx_texture_handle_t));
		*ud = bgfx_create_texture(bgfx_copy(data, size), 0, 0, &info);

		luaL_getmetatable(L, "bgfx_texture");
		lua_setmetatable(L, -2);

		return 1;
	} },

	{ "new_program", [](lua_State *L) {
		bool compute = false;

		if (!lua_isstring(L, 1)) {
			lua_pushboolean(L, 0);
			return 1;
		}

		if (lua_isboolean(L, 2)) {
			compute = true;
		}

		if (!compute && !lua_isstring(L, 2)) {
			lua_pushboolean(L, 0);
			return 1;
		}

		size_t size = 0;
		const char *data = lua_tolstring(L, 1, &size);
		bgfx_shader_handle_t vsh = bgfx_create_shader(bgfx_copy(data, size));
		bgfx_shader_handle_t fsh;

		if (!compute) {
			data = lua_tolstring(L, 2, &size);
			fsh = bgfx_create_shader(bgfx_copy(data, size));
		}

		bgfx_program_handle_t *ud = (bgfx_program_handle_t*)lua_newuserdata(L, sizeof(bgfx_program_handle_t));

		if (compute) {
			*ud = bgfx_create_compute_program(vsh, true);
		}
		else {
			*ud = bgfx_create_program(vsh, fsh, true);
		}

		luaL_getmetatable(L, "bgfx_program");
		lua_setmetatable(L, -2);

		return 1;
	} },

	{ "new_vertex_format", [](lua_State *L) {
		bgfx_vertex_decl_t *decl = (bgfx_vertex_decl_t*)lua_newuserdata(L, sizeof(bgfx_vertex_decl_t));
		bgfx_renderer_type_t renderer = bgfx_get_renderer_type();
		bgfx_vertex_decl_begin(decl, renderer);

		luaL_getmetatable(L, "bgfx_vertex_format");
		lua_setmetatable(L, -2);

		static std::map<const char*, bgfx_attrib_t, fuck_off_cpp> format_lookup = {
			{ "position",  BGFX_ATTRIB_POSITION },
			{ "normal",    BGFX_ATTRIB_NORMAL },
			{ "tangent",   BGFX_ATTRIB_TANGENT },
			{ "bitangent", BGFX_ATTRIB_BITANGENT },
			{ "color0",    BGFX_ATTRIB_COLOR0 },
			{ "color1",    BGFX_ATTRIB_COLOR1 },
			{ "indices",   BGFX_ATTRIB_INDICES },
			{ "weight",    BGFX_ATTRIB_WEIGHT },
			{ "texcoord0", BGFX_ATTRIB_TEXCOORD0 },
			{ "texcoord1", BGFX_ATTRIB_TEXCOORD1 },
			{ "texcoord2", BGFX_ATTRIB_TEXCOORD2 },
			{ "texcoord3", BGFX_ATTRIB_TEXCOORD3 },
			{ "texcoord4", BGFX_ATTRIB_TEXCOORD4 },
			{ "texcoord5", BGFX_ATTRIB_TEXCOORD5 },
			{ "texcoord6", BGFX_ATTRIB_TEXCOORD6 },
			{ "texcoord7", BGFX_ATTRIB_TEXCOORD7 }
		};

		static std::map<const char*, bgfx_attrib_type_t, fuck_off_cpp> type_lookup = {
			{ "byte",   BGFX_ATTRIB_TYPE_UINT8 },
			{ "short",  BGFX_ATTRIB_TYPE_INT16 },
			{ "float",  BGFX_ATTRIB_TYPE_FLOAT }
			// { "half",   BGFX_ATTRIB_TYPE_HALF }
			// { "uint10", BGFX_ATTRIB_TYPE_UINT10 }
		};

		table_scan(L, -2, [&](const char *, const char *) {
			bgfx_attrib_t attrib    = BGFX_ATTRIB_POSITION;
			bgfx_attrib_type_t type = BGFX_ATTRIB_TYPE_FLOAT;
			uint8_t size            = 1;
			bool normalized         = false;
			bool as_int             = false;

			table_scan(L, -2, [&](const char *k, const char *v) {
				std::string key = std::string(k);

				printf("= %s %s\n", k, v);

				if (key == "type") {
					auto val = type_lookup.find(v);
					if (val != type_lookup.end()) {
						type = val->second;
					}
					return;
				} else if (key == "attrib") {
					auto val = format_lookup.find(v);
					if (val != format_lookup.end()) {
						attrib = val->second;
					}
					return;
				} else if (key == "size") {
					size = (uint8_t)atoi(v);
					return;
				} else if (key == "normalized") {
					normalized = true;
					return;
				}
			});

			if (normalized) {
				printf("normalizing\n");
			}
			bgfx_vertex_decl_add(decl, attrib, size, type, normalized, as_int);
			printf("\n");
		});

		bgfx_vertex_decl_end(decl);

		return 1;
	}},

	{ "new_vertex_buffer", [](lua_State *L) {
		bgfx_vertex_buffer_handle_t *ud = (bgfx_vertex_buffer_handle_t*)lua_newuserdata(L, sizeof(bgfx_vertex_buffer_handle_t));

		if (!lua_isstring(L, 1)) {
			lua_pushboolean(L, 0);
			return 1;
		}

		size_t size = 0;
		const char *data = lua_tolstring(L, 1, &size);
		const bgfx_memory_t *mem = bgfx_copy(data, size);

		// this is absolutely going to segfault when gc happens
		// const bgfx_memory_t *mem = bgfx_make_ref(data, size);
		bgfx_vertex_decl_t *decl = to_vertex_format_ud(L, 2);

		*ud = bgfx_create_vertex_buffer(mem, decl, 0);

		luaL_getmetatable(L, "bgfx_vertex_buffer");
		lua_setmetatable(L, -2);

		return 1;
	} },

	{ "new_index_buffer", [](lua_State *L) {
		int vertices = lua_objlen(L, -1);
		const bgfx_memory_t *mem = bgfx_alloc(sizeof(uint16_t) * vertices);
		uint16_t* data = (uint16_t*)mem->data;

		for (int i=1; ; i++) {
			lua_rawgeti(L, -1, i);
			if (lua_isnil(L,-1)) {
				lua_pop(L, 1);
				break;
			}
			data[i-1] = (uint16_t)luaL_checkinteger(L, -1);
			lua_pop(L,1);
		}

		bgfx_index_buffer_handle_t *ud = (bgfx_index_buffer_handle_t*)lua_newuserdata(L, sizeof(bgfx_index_buffer_handle_t));
		*ud = bgfx_create_index_buffer(mem, 0);

		luaL_getmetatable(L, "bgfx_index_buffer");
		lua_setmetatable(L, -2);
		return 1;
	} },

	// bgfx.set_vertex_buffer(vb, 0, 32)
	{ "set_vertex_buffer", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n == 3);
		(void)n;

		bgfx_vertex_buffer_handle_t* handle = to_vertex_buffer_ud(L, 1);
		uint32_t start = 0;
		uint32_t num = UINT32_MAX;
		if (lua_isnumber(L, 2)) {
			start = (uint32_t)lua_tonumber(L, 2);
		}
		if (lua_isnumber(L, 3)) {
			num = (uint32_t)lua_tonumber(L, 3);
		}

		bgfx_set_vertex_buffer(*handle, start, num);
		return 0;
	} },

	// bgfx.set_index_buffer(ib, 0, 60)
	{ "set_index_buffer", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n == 3);
		(void)n;

		bgfx_index_buffer_handle_t* handle = to_index_buffer_ud(L, 1);
		int first = (int)lua_tonumber(L, 2);
		int num = (int)lua_tonumber(L, 3);
		bgfx_set_index_buffer(*handle, first, num);
		return 0;
	} },

	{ "discard", [](lua_State *) {
		bgfx_discard();
		return 0;
	} },

	// local hmd = bgfx.get_hmd()
	{ "get_hmd", [](lua_State *L) {
		const bgfx_hmd_t *hmd = bgfx_get_hmd();

		if (hmd) {
			lua_createtable(L, 0, 4);

			lua_pushstring(L, "width");
			lua_pushnumber(L, hmd->width);
			lua_settable(L, -3);

			lua_pushstring(L, "height");
			lua_pushnumber(L, hmd->height);
			lua_settable(L, -3);

			// TODO: populate eye fields
			lua_pushstring(L, "eye");
			lua_pushnil(L);
			lua_settable(L, -3);
			return 1;
		}

		lua_pushboolean(L, 0);

		return 1;
	} },

	{ "get_renderer_info", [](lua_State *L) {
		bgfx_renderer_type_t t = bgfx_get_renderer_type();

		lua_newtable(L);

		lua_pushstring(L, "name");
		lua_pushstring(L, bgfx_get_renderer_name(t));
		lua_settable(L, -3);

		lua_pushstring(L, "type");
		lua_pushstring(L, renderer2str(t));
		lua_settable(L, -3);

		return 1;
	} },

	{ NULL, NULL }
};

#ifdef _MSC_VER
# define LUA_EXPORT __declspec(dllexport)
#else
# define LUA_EXPORT
#endif

extern "C" LUA_EXPORT int luaopen_bgfx(lua_State*);

LUA_EXPORT
int luaopen_bgfx(lua_State *L) {
	luaL_newmetatable(L, "bgfx_texture");
	luaL_register(L, NULL, texture_fn);
	lua_pushvalue(L, -1);
	lua_setfield(L, -1, "__index");

	luaL_newmetatable(L, "bgfx_program");
	luaL_register(L, NULL, program_fn);
	lua_pushvalue(L, -1);
	lua_setfield(L, -1, "__index");

	luaL_newmetatable(L, "bgfx_vertex_format");
	luaL_register(L, NULL, vertex_format_fn);
	lua_pushvalue(L, -1);
	lua_setfield(L, -1, "__index");

	luaL_newmetatable(L, "bgfx_vertex_buffer");
	luaL_register(L, NULL, vertex_buffer_fn);
	lua_pushvalue(L, -1);
	lua_setfield(L, -1, "__index");

	luaL_newmetatable(L, "bgfx_index_buffer");
	luaL_register(L, NULL, index_buffer_fn);
	lua_pushvalue(L, -1);
	lua_setfield(L, -1, "__index");

	lua_pop(L, lua_gettop(L));

	luaL_register(L, "bgfx", m);

	// shut up gcc when we aren't doing any stack dumps
	(void)stack_dump;

	return 1;
}
