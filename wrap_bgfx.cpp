extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

// "but holo, this is C++, why are you using the C API?", you might ask.
// well, my dear reader, it is simple: C APIs are easier to comprehend.
//
// Lua doesn't really do C++-style magic anyhow.
#include <bgfx/c99/bgfx.h>
#include <bgfx/c99/platform.h>
}

#include <bx/fpumath.h>

#include <cstring>
#include <climits>
#include <map>
#include <vector>

// buffer up to 512 mat4's worth of data, we can't even fit that in one send.
#define UNIFORM_BUFFER_SIZE 512*16*sizeof(float)
static bool shutdown = false;
static void *uniform_buffer;

#ifndef luaL_typerror
LUALIB_API int luaL_typerror (lua_State *L, int narg, const char *tname) {
	const char *msg = lua_pushfstring(L,
		"%s expected, got %s", tname, luaL_typename(L, narg)
	);
	return luaL_argerror(L, narg, msg);
}
#endif

#define UD_HANDLE_TYPE2(name) \
static bgfx_##name##_t *to_##name##_ud(lua_State *L, int index) { \
	bgfx_##name##_t *ud = (bgfx_##name##_t*)lua_touserdata(L, index); \
	if (ud == NULL) luaL_typerror(L, index, #name); \
	return ud; \
}

#define UD_HANDLE_TYPE(name) \
static bgfx_##name##_handle_t *to_##name##_ud(lua_State *L, int index) { \
	bgfx_##name##_handle_t *ud = (bgfx_##name##_handle_t*)lua_touserdata(L, index); \
	if (ud == NULL) luaL_typerror(L, index, #name); \
	return ud; \
}

UD_HANDLE_TYPE(program)
UD_HANDLE_TYPE(texture)
UD_HANDLE_TYPE(uniform)
UD_HANDLE_TYPE(vertex_buffer)
UD_HANDLE_TYPE(index_buffer)
UD_HANDLE_TYPE(frame_buffer)

UD_HANDLE_TYPE2(transient_index_buffer)
UD_HANDLE_TYPE2(transient_vertex_buffer)
UD_HANDLE_TYPE2(instance_data_buffer)
UD_HANDLE_TYPE2(vertex_decl)

#undef UD_HANDLE_TYPE
#undef UD_HANDLE_TYPE2

static const luaL_Reg program_fn[] = {
	{ "__gc",  [](lua_State *L) {
		if (shutdown) { return 0; }
		bgfx_program_handle_t *ud = to_program_ud(L, 1);
		bgfx_destroy_program(*ud);
		return 0;
	} },
	{ "__tostring", [](lua_State *L) {
		char buff[64];
		sprintf(buff, "%p", to_program_ud(L, 1));
		lua_pushfstring(L, "bgfx_program (%s)", buff);
		return 1;
	} },
	{ NULL, NULL }
};

static const luaL_Reg texture_fn[] = {
	{ "__gc",  [](lua_State *L) {
		if (shutdown) { return 0; }
		return 0;
		bgfx_texture_handle_t *ud = to_texture_ud(L, 1);
		bgfx_destroy_texture(*ud);
		return 0;
	} },
	{ "__tostring", [](lua_State *L) {
		char buff[64];
		sprintf(buff, "%p", to_texture_ud(L, 1));
		lua_pushfstring(L, "bgfx_texture (%s)", buff);
		return 1;
	} },
	{ NULL, NULL }
};

static const luaL_Reg uniform_fn[] = {
	{ "__gc",  [](lua_State *L) {
		if (shutdown) { return 0; }
		bgfx_uniform_handle_t *ud = to_uniform_ud(L, 1);
		bgfx_destroy_uniform(*ud);
		return 0;
	} },
	{ "__tostring", [](lua_State *L) {
		char buff[64];
		sprintf(buff, "%p", to_uniform_ud(L, 1));
		lua_pushfstring(L, "bgfx_uniform (%s)", buff);
		return 1;
	} },
	{ NULL, NULL }
};

static const luaL_Reg vertex_format_fn[] = {
	{ "__tostring", [](lua_State *L) {
		char buff[64];
		sprintf(buff, "%p", to_vertex_decl_ud(L, 1));
		lua_pushfstring(L, "bgfx_vertex_decl (%s)", buff);
		return 1;
	} },
	{ NULL, NULL }
};

static const luaL_Reg vertex_buffer_fn[] = {
	{ "__gc",  [](lua_State *L) {
		if (shutdown) { return 0; }
		bgfx_vertex_buffer_handle_t *ud = to_vertex_buffer_ud(L, 1);
		bgfx_destroy_vertex_buffer(*ud);
		return 0;
	} },
	{ "__tostring", [](lua_State *L) {
		char buff[64];
		sprintf(buff, "%p", to_vertex_buffer_ud(L, 1));
		lua_pushfstring(L, "bgfx_vertex_buffer (%s)", buff);
		return 1;
	} },
	{ NULL, NULL }
};

static const luaL_Reg index_buffer_fn[] = {
	{ "__gc",  [](lua_State *L) {
		if (shutdown) { return 0; }
		bgfx_index_buffer_handle_t *ud = to_index_buffer_ud(L, 1);
		bgfx_destroy_index_buffer(*ud);
		return 0;
	} },
	{ "__tostring", [](lua_State *L) {
		char buff[64];
		sprintf(buff, "%p", to_index_buffer_ud(L, 1));
		lua_pushfstring(L, "bgfx_index_buffer (%s)", buff);
		return 1;
	} },
	{ NULL, NULL }
};

static const luaL_Reg frame_buffer_fn[] = {
	{ "__gc",  [](lua_State *L) {
		if (shutdown) { return 0; }
		bgfx_frame_buffer_handle_t *ud = to_frame_buffer_ud(L, 1);
		bgfx_destroy_frame_buffer(*ud);
		return 0;
	} },
	{ "__tostring", [](lua_State *L) {
		char buff[64];
		sprintf(buff, "%p", to_frame_buffer_ud(L, 1));
		lua_pushfstring(L, "bgfx_frame_buffer (%s)", buff);
		return 1;
	} },
	{ NULL, NULL }
};


static const luaL_Reg transient_ib_fn[] = {
	{ "__tostring", [](lua_State *L) {
		char buff[64];
		sprintf(buff, "%p", to_frame_buffer_ud(L, 1));
		lua_pushfstring(L, "bgfx_transient_index_buffer (%s)", buff);
		return 1;
	} },
	{ NULL, NULL }
};

static const luaL_Reg transient_vb_fn[] = {
	{ "__tostring", [](lua_State *L) {
		char buff[64];
		sprintf(buff, "%p", to_frame_buffer_ud(L, 1));
		lua_pushfstring(L, "bgfx_transient_vertex_buffer (%s)", buff);
		return 1;
	} },
	{ NULL, NULL }
};

struct fuck_off_cpp {
	bool operator() (char const *a, char const *b) {
		return std::strcmp(a, b) < 0;
	}
};

static std::map<const char*, bgfx_access_t, fuck_off_cpp> access_lookup = {
	{ "read", BGFX_ACCESS_READ },
	{ "write", BGFX_ACCESS_WRITE },
	{ "read_write", BGFX_ACCESS_READWRITE }
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

static std::map<const char*, bgfx_texture_format_t, fuck_off_cpp> texture_format_lookup = {
	{ "bc1", BGFX_TEXTURE_FORMAT_BC1 },
	{ "bc2", BGFX_TEXTURE_FORMAT_BC2 },
	{ "bc3", BGFX_TEXTURE_FORMAT_BC3 },
	{ "bc4", BGFX_TEXTURE_FORMAT_BC4 },
	{ "bc5", BGFX_TEXTURE_FORMAT_BC5 },
	{ "bc6h", BGFX_TEXTURE_FORMAT_BC6H },
	{ "bc7", BGFX_TEXTURE_FORMAT_BC7 },
	{ "etc1", BGFX_TEXTURE_FORMAT_ETC1 },
	{ "etc2", BGFX_TEXTURE_FORMAT_ETC2 },
	{ "etc2a", BGFX_TEXTURE_FORMAT_ETC2A },
	{ "etc2a1", BGFX_TEXTURE_FORMAT_ETC2A1 },
	{ "ptc12", BGFX_TEXTURE_FORMAT_PTC12 },
	{ "ptc14", BGFX_TEXTURE_FORMAT_PTC14 },
	{ "ptc12a", BGFX_TEXTURE_FORMAT_PTC12A },
	{ "ptc14a", BGFX_TEXTURE_FORMAT_PTC14A },
	{ "ptc22", BGFX_TEXTURE_FORMAT_PTC22 },
	{ "ptc24", BGFX_TEXTURE_FORMAT_PTC24 },
	{ "r1", BGFX_TEXTURE_FORMAT_R1 },
	{ "a8", BGFX_TEXTURE_FORMAT_A8 },
	{ "r8", BGFX_TEXTURE_FORMAT_R8 },
	{ "r8i", BGFX_TEXTURE_FORMAT_R8I },
	{ "r8u", BGFX_TEXTURE_FORMAT_R8U },
	{ "r8s", BGFX_TEXTURE_FORMAT_R8S },
	{ "r16", BGFX_TEXTURE_FORMAT_R16 },
	{ "r16i", BGFX_TEXTURE_FORMAT_R16I },
	{ "r16u", BGFX_TEXTURE_FORMAT_R16U },
	{ "r16f", BGFX_TEXTURE_FORMAT_R16F },
	{ "r16s", BGFX_TEXTURE_FORMAT_R16S },
	{ "r32i", BGFX_TEXTURE_FORMAT_R32I },
	{ "r32u", BGFX_TEXTURE_FORMAT_R32U },
	{ "r32f", BGFX_TEXTURE_FORMAT_R32F },
	{ "rg8", BGFX_TEXTURE_FORMAT_RG8 },
	{ "rg8i", BGFX_TEXTURE_FORMAT_RG8I },
	{ "rg8u", BGFX_TEXTURE_FORMAT_RG8U },
	{ "rg8s", BGFX_TEXTURE_FORMAT_RG8S },
	{ "rg16", BGFX_TEXTURE_FORMAT_RG16 },
	{ "rg16i", BGFX_TEXTURE_FORMAT_RG16I },
	{ "rg16u", BGFX_TEXTURE_FORMAT_RG16U },
	{ "rg16f", BGFX_TEXTURE_FORMAT_RG16F },
	{ "rg16s", BGFX_TEXTURE_FORMAT_RG16S },
	{ "rg32i", BGFX_TEXTURE_FORMAT_RG32I },
	{ "rg32u", BGFX_TEXTURE_FORMAT_RG32U },
	{ "rg32f", BGFX_TEXTURE_FORMAT_RG32F },
	{ "rgb8", BGFX_TEXTURE_FORMAT_RGB8 },
	{ "rgb8i", BGFX_TEXTURE_FORMAT_RGB8I },
	{ "rgb8u", BGFX_TEXTURE_FORMAT_RGB8U },
	{ "rgb8s", BGFX_TEXTURE_FORMAT_RGB8S },
	{ "rgb9e5f", BGFX_TEXTURE_FORMAT_RGB9E5F },
	{ "bgra8", BGFX_TEXTURE_FORMAT_BGRA8 },
	{ "rgba8", BGFX_TEXTURE_FORMAT_RGBA8 },
	{ "rgba8i", BGFX_TEXTURE_FORMAT_RGBA8I },
	{ "rgba8u", BGFX_TEXTURE_FORMAT_RGBA8U },
	{ "rgba8s", BGFX_TEXTURE_FORMAT_RGBA8S },
	{ "rgba16", BGFX_TEXTURE_FORMAT_RGBA16 },
	{ "rgba16i", BGFX_TEXTURE_FORMAT_RGBA16I },
	{ "rgba16u", BGFX_TEXTURE_FORMAT_RGBA16U },
	{ "rgba16f", BGFX_TEXTURE_FORMAT_RGBA16F },
	{ "rgba16s", BGFX_TEXTURE_FORMAT_RGBA16S },
	{ "rgba32i", BGFX_TEXTURE_FORMAT_RGBA32I },
	{ "rgba32u", BGFX_TEXTURE_FORMAT_RGBA32U },
	{ "rgba32f", BGFX_TEXTURE_FORMAT_RGBA32F },
	{ "r5g6b5", BGFX_TEXTURE_FORMAT_R5G6B5 },
	{ "rgba4", BGFX_TEXTURE_FORMAT_RGBA4 },
	{ "rgb5a1", BGFX_TEXTURE_FORMAT_RGB5A1 },
	{ "rgb10a2", BGFX_TEXTURE_FORMAT_RGB10A2 },
	{ "r11g11b10f", BGFX_TEXTURE_FORMAT_R11G11B10F },
	{ "d16", BGFX_TEXTURE_FORMAT_D16 },
	{ "d24", BGFX_TEXTURE_FORMAT_D24 },
	{ "d24s8", BGFX_TEXTURE_FORMAT_D24S8 },
	{ "d32", BGFX_TEXTURE_FORMAT_D32 },
	{ "d16f", BGFX_TEXTURE_FORMAT_D16F },
	{ "d24f", BGFX_TEXTURE_FORMAT_D24F },
	{ "d32f", BGFX_TEXTURE_FORMAT_D32F },
	{ "d0s8", BGFX_TEXTURE_FORMAT_D0S8 }
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
	{ "init", [](lua_State *L) {
		uniform_buffer = malloc(UNIFORM_BUFFER_SIZE);
		memset(uniform_buffer, 0, UNIFORM_BUFFER_SIZE);

		bool use_sdl_context = true;
		if (lua_isboolean(L, 1)) {
			use_sdl_context = lua_toboolean(L, 1) ? true : false;
		}
		if (use_sdl_context) {
			_window = SDL_GL_GetCurrentWindow();

			bgfx_platform_data_t data;
			SDL_SysWMinfo wmi;
			SDL_VERSION(&wmi.version);
			if (!SDL_GetWindowWMInfo(_window, &wmi)) {
				return 0;
			}
			memset(&data, 0, sizeof(bgfx_platform_data_t));
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
			data.ndt          = wmi.info.x11.display;
			data.nwh          = (void*)(uintptr_t)wmi.info.x11.window;
#elif BX_PLATFORM_OSX
			data.nwh          = wmi.info.cocoa.window;
#elif BX_PLATFORM_WINDOWS
			data.nwh          = wmi.info.win.window;
#endif // BX_PLATFORM_
			data.context      = SDL_GL_GetCurrentContext();
			SDL_GL_MakeCurrent(_window, NULL);

			bgfx_set_platform_data(&data);
		}
		if (!bgfx_init(BGFX_RENDERER_TYPE_OPENGL, BGFX_PCI_ID_NONE, 0, NULL, NULL)) {
			lua_pushstring(L, "Unable to initialize BGFX.");
			lua_error(L);
			return 0;
		}
		return 0;
	} },

	// bgfx.shutdown()
	{ "shutdown", [](lua_State *) {
		free(uniform_buffer);

		shutdown = true;
		bgfx_shutdown();
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
		bgfx_dbg_text_printf(x, y, attr, "%s", str);
		return 0;
	} },

	// bgfx.debug_text_clear()
	{ "debug_text_clear", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n >= 0 || n <= 2);
		uint8_t attr = n >= 1 ? (uint8_t)lua_tonumber(L, 1) : 0;
		bool small   = n >= 2 ? lua_toboolean(L, 2) > 0 : false;
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
		uint8_t id = (uint8_t)luaL_checkinteger(L, 1);
		unsigned int ret = bgfx_touch(id);
		lua_pushnumber(L, double(ret));
		return 1;
	} },

	// bgfx.frame()
	{ "frame", [](lua_State *L) {
		int r = bgfx_frame(false);
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
		uint8_t id = (uint8_t)luaL_checkinteger(L, 1);
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
		uint8_t id = (uint8_t)luaL_checkinteger(L, 1);
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
		uint8_t id = (uint8_t)luaL_checkinteger(L, 1);

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
		uint8_t id = (uint8_t)luaL_checkinteger(L, 1);
		const char *name = lua_tostring(L, 2);
		bgfx_set_view_name(id, name);
		return 0;
	} },

	// bgfx.set_view_frame_buffer(0, frame_buffer)
	{ "set_view_frame_buffer", [](lua_State *L) {
		uint8_t id = (uint8_t)luaL_checkinteger(L, 1);
		bgfx_frame_buffer_handle_t *fb = to_frame_buffer_ud(L, 2);
		bgfx_set_view_frame_buffer(id, *fb);
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
		(void)n;
		lua_assert(n == 4);

		uint8_t id = (uint8_t)luaL_checkinteger(L, 1);
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
				lua_pushfstring(L, "Invalid table length %d, must be divisible by 16.\n", num);
				lua_error(L);
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

		load_matrix(L, 2, view);
		load_matrix(L, 3, proj);

		bgfx_set_view_transform(id, view, proj);
		// bgfx_set_view_transform_stereo(id, view, proj_l, flags, proj_r);

		return 0;
	} },

	// bgfx.set_transform(mtx)
	// bgfx.set_transform(cache_index, 1)
	{ "set_transform", [](lua_State *L) {
		if (lua_isnumber(L, 1)) {
			uint32_t cache = (uint32_t)luaL_checkinteger(L, 1);
			uint16_t num   = (uint16_t)luaL_checkinteger(L, 2);
			bgfx_set_transform_cached(cache, num);
			return 0;
		}

		// TODO: accept tables of matrices.
		int num = lua_objlen(L, 1);

		// we only accept 4x4 matrices
		if (num % 16 != 0) {
			lua_pushfstring(L, "Invalid table length %d, must be divisible by 16.\n", num);
			lua_error(L);
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

		uint32_t cached = bgfx_set_transform(mtx, num / 16);

		delete[] mtx;

		lua_pushnumber(L, cached);
		return 1;
	} },

	{ "alloc_transform", [](lua_State *L) {
		// TODO: accept tables of matrices.
		int num = lua_objlen(L, 1);

		// we only accept 4x4 matrices
		if (num % 16 != 0) {
			lua_pushfstring(L, "Invalid table length %d, must be divisible by 16.\n", num);
			lua_error(L);
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

		bgfx_transform_t transform;
		uint32_t cached = bgfx_alloc_transform(&transform, num / 16);

		memcpy(transform.data, mtx, sizeof(float)*16);

		delete[] mtx;

		lua_pushnumber(L, cached);
		return 1;
	} },

	// bgfx.new_frame_buffer(width, height, format, flags)
	{ "new_frame_buffer", [](lua_State *L) {
		uint32_t flags = BGFX_TEXTURE_U_CLAMP | BGFX_TEXTURE_V_CLAMP;

		if (lua_istable(L, 1)) {
			std::vector<bgfx_attachment_t> attachments;

			lua_pushvalue(L, 1);
			for (int i=1; ; i++) {
				lua_rawgeti(L, -1, i);
				if (lua_isnil(L,-1)) {
					lua_pop(L, 1);
					break;
				}
				bgfx_attachment_t a;
				a.mip = 0;
				a.layer = 1;

				if (lua_isuserdata(L, -1)) {
					a.handle = *to_texture_ud(L, -1);
				} else {
					table_scan(L, -1, [&](const char *k, const char *) {
						std::string key(k);
						if (key == "mip" || key == "layer") {
							a.mip = (uint16_t)lua_tointeger(L, -2);
						}
						else if (key == "handle") {
							a.handle = *to_texture_ud(L, -2);
						}
					});
				}
				attachments.push_back(a);
				lua_pop(L, 1);
			}
			lua_pop(L, 1);

			bgfx_frame_buffer_handle_t *ud = (bgfx_frame_buffer_handle_t*)lua_newuserdata(L, sizeof(bgfx_frame_buffer_handle_t));
			*ud = bgfx_create_frame_buffer_from_attachment(attachments.size(), attachments.data(), false);

			luaL_getmetatable(L, "bgfx_frame_buffer");
			lua_setmetatable(L, -2);

			return 1;
		}

		uint16_t width = luaL_checkinteger(L, 1);
		uint16_t height = luaL_checkinteger(L, 2);

		bgfx_texture_format_t format = BGFX_TEXTURE_FORMAT_RGBA8;
		const char *_format = "rgba8";
		if (lua_isstring(L, 3)) {
			_format = lua_tostring(L, 3);
		}

		auto val = texture_format_lookup.find(_format);
		if (val != texture_format_lookup.end()) {
			format = val->second;
		}
		else {
			lua_pushboolean(L, 0);
			lua_pushstring(L, "Invalid texture format.");
			return 2;
		}

		if (lua_istable(L, 4)) {
			flags = 0;
			table_scan(L, 4, [&](const char *, const char *v) {
				auto val = texture_lookup.find(v);
				if (val != texture_lookup.end()) {
					flags |= val->second;
				}
			});
		}

		bgfx_frame_buffer_handle_t *ud = (bgfx_frame_buffer_handle_t*)lua_newuserdata(L, sizeof(bgfx_frame_buffer_handle_t));
		*ud = bgfx_create_frame_buffer(width, height, format, flags);

		luaL_getmetatable(L, "bgfx_frame_buffer");
		lua_setmetatable(L, -2);

		return 1;
	} },

	// bgfx.get_texture(frame_buffer, attachment=0)
	{ "get_texture", [](lua_State *L) {
		bgfx_frame_buffer_handle_t *fb = to_frame_buffer_ud(L, 1);

		uint8_t attachment = 0;
		if (lua_isnumber(L, 2)) {
			attachment = (uint8_t)lua_tointeger(L, 2);
		}

		bgfx_texture_handle_t *tex = (bgfx_texture_handle_t*)lua_newuserdata(L, sizeof(bgfx_texture_handle_t));
		*tex = bgfx_get_texture(*fb, attachment);

		luaL_getmetatable(L, "bgfx_texture");
		lua_setmetatable(L, -2);

		return 1;
	} },

	// bgfx.is_texture_valid(format, flags, depth, cubemap, layers)
	{ "is_texture_valid", [](lua_State *L) {
		bgfx_texture_format_t format = BGFX_TEXTURE_FORMAT_RGBA8;

		const char *_format = "rgba8";
		if (lua_isstring(L, 1)) {
			_format = lua_tostring(L, 1);
		}

		auto val = texture_format_lookup.find(_format);
		if (val != texture_format_lookup.end()) {
			format = val->second;
		}
		else {
			lua_pushboolean(L, 0);
			lua_pushstring(L, "Invalid texture format.");
			return 2;
		}

		uint32_t flags = BGFX_TEXTURE_U_CLAMP | BGFX_TEXTURE_V_CLAMP;

		if (lua_istable(L, 2)) {
			flags = 0;
			table_scan(L, 2, [&](const char *, const char *v) {
				auto val = texture_lookup.find(v);
				if (val != texture_lookup.end()) {
					flags |= val->second;
				}
			});
		}

		uint16_t depth  = (uint16_t)luaL_checkinteger(L, 3);
		bool cubemap = false;
		if (lua_isboolean(L, 4)) {
			cubemap = lua_toboolean(L, 4) > 0;
		}

		uint16_t layers = 1;
		if (lua_isnumber(L, 5)) {
			layers = (uint16_t)lua_tointeger(L, 5);
		}

		bool valid = bgfx_is_texture_valid(depth, cubemap, layers, format, flags);
		lua_pushboolean(L, valid ? 1 : 0);
		return 1;
	} },

	// bgfx.new_texture(data, width, height, has_mips, format, flags)
	{ "new_texture", [](lua_State *L) {
		const bgfx_memory_t *mem = NULL;
		if (lua_isstring(L, 1)) {
			size_t size = 0;
			const char *data = lua_tolstring(L, 1, &size);
			mem = bgfx_copy(data, size);
		}

		bool has_mips = false;
		const char *_format = "rgba8";

		int n = lua_gettop(L);
		if (n >= 4) {
			has_mips = lua_toboolean(L, 4) > 0;
		}
		if (n >= 5) {
			_format = luaL_checkstring(L, 5);
		}

		bgfx_texture_format_t format = BGFX_TEXTURE_FORMAT_RGBA8;

		auto val = texture_format_lookup.find(_format);
		if (val != texture_format_lookup.end()) {
			format = val->second;
		}
		else {
			lua_pushboolean(L, 0);
			lua_pushstring(L, "Invalid texture format.");
			return 2;
		}

		uint16_t width  = (uint16_t)luaL_checkinteger(L, 2);
		uint16_t height = (uint16_t)luaL_checkinteger(L, 3);

		uint32_t flags = BGFX_TEXTURE_NONE;
		if (lua_istable(L, 6)) {
			flags = 0;
			table_scan(L, 6, [&](const char *, const char *v) {
				auto val = texture_lookup.find(v);
				if (val != texture_lookup.end()) {
					flags |= val->second;
				}
			});
		}

		bgfx_texture_handle_t *ud = (bgfx_texture_handle_t*)lua_newuserdata(L, sizeof(bgfx_texture_handle_t));
		*ud = bgfx_create_texture_2d(width, height, has_mips, 1, format, flags, mem);

		luaL_getmetatable(L, "bgfx_texture");
		lua_setmetatable(L, -2);

		return 1;
	} },

	{ "new_uniform", [](lua_State *L) {
		const char *name = luaL_checkstring(L, 1);
		const char *type_raw = luaL_checkstring(L, 2);
		uint16_t count = 1;
		if (lua_isnumber(L, 3)) {
			count = lua_tointeger(L, 3);
		}

		bgfx_uniform_type_t type = BGFX_UNIFORM_TYPE_COUNT;
		if (strcmp(type_raw, "int") == 0) {
			type = BGFX_UNIFORM_TYPE_INT1;
		}
		else if (strcmp(type_raw, "vec4") == 0) {
			type = BGFX_UNIFORM_TYPE_VEC4;
		}
		else if (strcmp(type_raw, "mat3") == 0) {
			type = BGFX_UNIFORM_TYPE_MAT3;
		}
		else if (strcmp(type_raw, "mat4") == 0) {
			type = BGFX_UNIFORM_TYPE_MAT4;
		}
		else {
			lua_pushboolean(L, 0);
			lua_pushstring(L, "Invalid uniform type. Must be `int`, `vec4`, `mat3` or `mat4`.");
			return 2;
		}

		bgfx_uniform_handle_t *ud = (bgfx_uniform_handle_t*)lua_newuserdata(L, sizeof(bgfx_uniform_handle_t));
		*ud = bgfx_create_uniform(name, type, count);

		luaL_getmetatable(L, "bgfx_uniform");
		lua_setmetatable(L, -2);

		return 1;
	} },

	{ "set_texture", [](lua_State *L) {
		uint8_t stage = luaL_checkinteger(L, 1);
		bgfx_uniform_handle_t *uniform = to_uniform_ud(L, 2);
		bgfx_texture_handle_t *texture = to_texture_ud(L, 3);

		bgfx_set_texture(stage, *uniform, *texture, UINT32_MAX);

		return 0;
	} },

	{ "set_image", [](lua_State *L) {
		uint8_t stage = luaL_checkinteger(L, 1);
		bgfx_uniform_handle_t *uniform = to_uniform_ud(L, 2);
		bgfx_texture_handle_t *texture = to_texture_ud(L, 3);

		uint8_t mip = (uint8_t)luaL_checkinteger(L, 4);

		const char *_access = luaL_checkstring(L, 5);
		bgfx_access_t access = BGFX_ACCESS_READ;

		auto aval = access_lookup.find(_access);
		if (aval != access_lookup.end()) {
			access = aval->second;
		}
		else {
			lua_pushfstring(L, "Invalid access specifier: '%s'", _access);
			lua_error(L);
		}

		bgfx_texture_format_t format = BGFX_TEXTURE_FORMAT_RGBA8;

		const char *_format = "rgba8";
		if (lua_isstring(L, 6)) {
			_format = lua_tostring(L, 6);
		}

		auto val = texture_format_lookup.find(_format);
		if (val != texture_format_lookup.end()) {
			format = val->second;
		}
		else {
			lua_pushboolean(L, 0);
			lua_pushstring(L, "Invalid texture format.");
			return 2;
		}

		bgfx_set_image(stage, *uniform, *texture, mip, access, format);

		return 0;
	} },

	{ "set_compute_index_buffer", [](lua_State *L) {
		uint8_t stage = luaL_checkinteger(L, 1);
		bgfx_index_buffer_handle_t *ibh = to_index_buffer_ud(L, 2);

		const char *_access = luaL_checkstring(L, 3);
		bgfx_access_t access = BGFX_ACCESS_READ;

		auto val = access_lookup.find(_access);
		if (val != access_lookup.end()) {
			access = val->second;
		}
		else {
			lua_pushfstring(L, "Invalid access specifier: '%s'", _access);
			lua_error(L);
			return 0;
		}

		bgfx_set_compute_index_buffer(stage, *ibh, access);
		return 0;
	} },

	{ "set_compute_vertex_buffer", [](lua_State *L) {
		uint8_t stage = luaL_checkinteger(L, 1);
		bgfx_vertex_buffer_handle_t *vbh = to_vertex_buffer_ud(L, 2);

		const char *_access = luaL_checkstring(L, 3);
		bgfx_access_t access = BGFX_ACCESS_READ;

		auto val = access_lookup.find(_access);
		if (val != access_lookup.end()) {
			access = val->second;
		}
		else {
			lua_pushfstring(L, "Invalid access specifier: '%s'", _access);
			lua_error(L);
			return 0;
		}

		bgfx_set_compute_vertex_buffer(stage, *vbh, access);
		return 0;
	} },

	{ "set_uniform", [](lua_State *L) {
		auto scan_inner = [&](size_t size, int idx, int j) {
			if (lua_istable(L, idx)) {
				lua_pushvalue(L, idx);
				lua_pushnil(L);

				int i = 0;
				while (lua_next(L, -2)) {
					lua_pushvalue(L, -2);

					if (size == 1) {
						int n = lua_tointeger(L, -1);
						int *idata = (int*)uniform_buffer;
						idata[j*size+i] = n;
					}
					else {
						float n = (float)lua_tonumber(L, -2);
						float *fdata = (float*)uniform_buffer;
						fdata[j*size+i] = n;
					}

					lua_pop(L, 2);
					i = i + 1;
				}
				lua_pop(L, 1);
			}
		};

		auto scan_outer = [&](int idx) {
			if (lua_istable(L, idx)) {
				lua_pushvalue(L, idx);
				lua_pushnil(L);

				int i = 0;
				while (lua_next(L, -2)) {
					lua_pushvalue(L, -2);

					// sanity check: we only support float 4, int, mat3 and mat4.
					size_t size = lua_objlen(L, -2);
					if (size != 16 && size != 9 && size != 4 && size != 1) {
						lua_pop(L, 2);
						break;
					}
					scan_inner(size, -2, i);

					lua_pop(L, 2);
					i = i + 1;
				}
				lua_pop(L, 1);
			}
		};

		scan_outer(2);

		uint16_t count = 1;
		if (lua_isnumber(L, 3)) {
			count = lua_tointeger(L, 3);
		}

		bgfx_uniform_handle_t *uniform = to_uniform_ud(L, 1);
		bgfx_set_uniform(*uniform, uniform_buffer, count);

		return 0;
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

		luaL_getmetatable(L, "bgfx_vertex_decl");
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
					normalized = lua_toboolean(L, -2) > 0;
					return;
				}
			});

			bgfx_vertex_decl_add(decl, attrib, size, type, normalized, as_int);
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
		bgfx_vertex_decl_t *decl = to_vertex_decl_ud(L, 2);

		*ud = bgfx_create_vertex_buffer(mem, decl, 0);

		luaL_getmetatable(L, "bgfx_vertex_buffer");
		lua_setmetatable(L, -2);

		return 1;
	} },

	{ "new_index_buffer", [](lua_State *L) {
		size_t vertices = lua_objlen(L, -1);
		size_t bytes    = sizeof(uint16_t);
		bool index32    = false;
		if (vertices > USHRT_MAX) {
			bytes = sizeof(uint32_t);
			index32 = true;
		}
		const bgfx_memory_t *mem = bgfx_alloc(bytes * vertices);

		uint16_t* data16 = (uint16_t*)mem->data;
		uint32_t* data32 = (uint32_t*)mem->data;

		for (int i=1; ; i++) {
			lua_rawgeti(L, -1, i);
			if (lua_isnil(L,-1)) {
				lua_pop(L, 1);
				break;
			}
			if (index32) {
				data32[i-1] = (uint32_t)luaL_checkinteger(L, -1) - 1;
			}
			else {
				data16[i-1] = (uint16_t)luaL_checkinteger(L, -1) - 1;
			}
			lua_pop(L,1);
		}

		bgfx_index_buffer_handle_t *ud = (bgfx_index_buffer_handle_t*)lua_newuserdata(L, sizeof(bgfx_index_buffer_handle_t));
		*ud = bgfx_create_index_buffer(mem, index32 ? BGFX_BUFFER_INDEX32 : 0);

		luaL_getmetatable(L, "bgfx_index_buffer");
		lua_setmetatable(L, -2);
		return 1;
	} },

	// bgfx.check_avail_instance_data_buffer(num, stride)
	{ "check_avail_instance_data_buffer", [](lua_State *L) {
		uint32_t num = (uint32_t)luaL_checkinteger(L, 1);
		uint16_t stride = luaL_checkinteger(L, 2);

		bool avail = bgfx_check_avail_instance_data_buffer(num, stride);

		lua_pushboolean(L, avail ? 1 : 0);
		return 1;
	} },

	// bgfx.check_avail_transient_buffer("index", num)
	// bgfx.check_avail_transient_buffer("vertex", num, decl)
	// bgfx.check_avail_transient_buffer("both", vertices, decl, indices)
	{ "check_avail_transient_buffer", [](lua_State *L) {
		std::string type(luaL_checkstring(L, 1));
		uint32_t num = (uint32_t)luaL_checkinteger(L, 2);

		bool avail = false;
		if (type == "index") {
			avail = bgfx_check_avail_transient_index_buffer(num);
		}
		else if (type == "vertex") {
			bgfx_vertex_decl_t *decl = to_vertex_decl_ud(L, 3);
			avail =bgfx_check_avail_transient_vertex_buffer(num, decl);
		}
		else if (type == "both") {
			bgfx_vertex_decl_t *decl = to_vertex_decl_ud(L, 3);
			uint32_t indices = luaL_checkinteger(L, 4);
			avail = bgfx_check_avail_transient_buffers(num, decl, indices);
		}
		else {
			lua_pushstring(L, "Invalid transient buffer type.");
			lua_error(L);
			return 0;
		}

		lua_pushboolean(L, avail ? 1 : 0);
		return 1;
	} },

	// bgfx.new_transient_buffer("index", indices)
	// bgfx.new_transient_buffer("vertex", num, decl, data)
	// bgfx.new_transient_buffer("both", num, decl, data, indices)
	{ "new_transient_buffer", [](lua_State *L) {
		std::string type(luaL_checkstring(L, 1));

		auto load_tib = [&](bgfx_transient_index_buffer_t *tib, int idx) {
			size_t vertices = lua_objlen(L, idx);
			uint16_t* data16 = (uint16_t*)tib->data;

			for (int i=1; ; i++) {
				lua_rawgeti(L, -1, i);
				if (lua_isnil(L,-1)) {
					lua_pop(L, 1);
					break;
				}
				else {
					data16[i-1] = (uint16_t)luaL_checkinteger(L, -1) - 1;
				}
				lua_pop(L, 1);
			}
		};

		if (type == "index" ) {
			bgfx_transient_index_buffer_t *tib = (bgfx_transient_index_buffer_t*)lua_newuserdata(L, sizeof(bgfx_transient_index_buffer_t));

			if (!lua_istable(L, 2)) {
				lua_pushstring(L, "Indices must be a table");
				lua_error(L);
				return 0;
			}

			uint32_t num = lua_objlen(L, 2);
			bgfx_alloc_transient_index_buffer(tib, num);
			load_tib(tib, 2);

			luaL_getmetatable(L, "bgfx_transient_index_buffer");
			lua_setmetatable(L, -2);
		}
		else if (type == "vertex") {
			bgfx_transient_vertex_buffer_t *tvb = (bgfx_transient_vertex_buffer_t*)lua_newuserdata(L, sizeof(bgfx_transient_vertex_buffer_t));

			uint32_t num = (uint32_t)luaL_checkinteger(L, 2);
			if (!lua_isstring(L, 4)) {
				lua_pushboolean(L, 0);
				return 1;
			}

			bgfx_vertex_decl_t *decl = to_vertex_decl_ud(L, 3);
			bgfx_alloc_transient_vertex_buffer(tvb, num, decl);

			size_t size = 0;
			const char *data = lua_tolstring(L, 4, &size);
			memcpy(tvb->data, data, size);

			luaL_getmetatable(L, "bgfx_transient_vertex_buffer");
			lua_setmetatable(L, -2);
		}
		else if (type == "both") {
			// bgfx_vertex_decl_t *decl = to_vertex_decl_ud(L, 3);

			// if (!lua_istable(L, 4)) {
			// 	lua_pushstring(L, "Indices must be a table.");
			// 	lua_error(L);
			// 	return 0;
			// }

			// uint32_t indices = (uint32_t)lua_objlen(L, 4);

			// bgfx_alloc_transient_buffers(&tvb, decl, num, &tib, indices);

			// load tvb and tib

			// push transient_index_buffer ud
			// push transient_vertex_buffer ud

			return 0;
		}
		else {
			lua_pushstring(L, "Invalid transient buffer type.");
			lua_error(L);
			return 0;
		}

		return 1;
	} },

	// const bgfx_instance_data_buffer_t* bgfx_alloc_instance_data_buffer(uint32_t _num, uint16_t _stride);

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

	// bgfx.set_transient_vertex_buffer(vb, 0, 32)
	{ "set_transient_vertex_buffer", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n == 3);
		(void)n;

		bgfx_transient_vertex_buffer_t* handle = to_transient_vertex_buffer_ud(L, 1);
		uint32_t start = 0;
		uint32_t num = UINT32_MAX;
		if (lua_isnumber(L, 2)) {
			start = (uint32_t)lua_tonumber(L, 2);
		}
		if (lua_isnumber(L, 3)) {
			num = (uint32_t)lua_tonumber(L, 3);
		}

		bgfx_set_transient_vertex_buffer(handle, start, num);
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

	// bgfx.set_transient_index_buffer(tib, 0, 60)
	{ "set_transient_index_buffer", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n == 3);
		(void)n;

		bgfx_transient_index_buffer_t* handle = to_transient_index_buffer_ud(L, 1);
		int first = (int)lua_tonumber(L, 2);
		int num = (int)lua_tonumber(L, 3);
		bgfx_set_transient_index_buffer(handle, first, num);
		return 0;
	} },

	{ "set_stencil", [](lua_State *L) {
		uint32_t front = (uint32_t)luaL_checkinteger(L, 1);
		uint32_t back = BGFX_STENCIL_NONE;
		if (lua_isnumber(L, 2)) {
			back = (uint32_t)lua_tointeger(L, 2);
		}
		bgfx_set_stencil(front, back);
		return 0;
	} },

	{ "set_scissor", [](lua_State *L) {
		if (lua_gettop(L) == 1) {
			uint16_t cache = (uint16_t)luaL_checkinteger(L, 1);
			bgfx_set_scissor_cached(cache);
			return 0;
		}

		uint16_t x = (uint16_t)luaL_checkinteger(L, 1);
		uint16_t y = (uint16_t)luaL_checkinteger(L, 2);
		uint16_t w = (uint16_t)luaL_checkinteger(L, 3);
		uint16_t h = (uint16_t)luaL_checkinteger(L, 4);
		bgfx_set_scissor(x, y, w, h);
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

#define REGISTER_MT(name, functions)  \
	luaL_newmetatable(L, name);        \
	luaL_register(L, NULL, functions); \
	lua_pushvalue(L, -1);              \
	lua_setfield(L, -1, "__index")     \

	REGISTER_MT("bgfx_texture", texture_fn);
	REGISTER_MT("bgfx_uniform", uniform_fn);
	REGISTER_MT("bgfx_program", program_fn);
	REGISTER_MT("bgfx_vertex_decl", vertex_format_fn);
	REGISTER_MT("bgfx_vertex_buffer", vertex_buffer_fn);
	REGISTER_MT("bgfx_index_buffer", index_buffer_fn);
	REGISTER_MT("bgfx_frame_buffer", frame_buffer_fn);
	REGISTER_MT("bgfx_transient_index_buffer", transient_ib_fn);
	REGISTER_MT("bgfx_transient_vertex_buffer", transient_vb_fn);

	lua_pop(L, lua_gettop(L));

	luaL_register(L, "bgfx", m);

	// shut up gcc when we aren't doing any stack dumps
	#ifdef __GCC__
	(void)stack_dump;
	#endif

	return 1;
}
