extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

// "but holo, this is C++, why are you using the C API?", you might ask.
// well, my dear reader, it is simple: C APIs are easier to comprehend.
//
// Lua doesn't really do C++-style magic anyhow.
#include <bgfx/c99/bgfx.h>
}

#include <unordered_map>

static bgfx_shader_handle_t *to_shader_ud(lua_State *L, int index) {
	bgfx_shader_handle_t **ud = (bgfx_shader_handle_t**)lua_touserdata(L, index);
	if (ud == NULL) luaL_typerror(L, index, "bgfx_shader");
	return *ud;
}

static bgfx_program_handle_t *to_program_ud(lua_State *L, int index) {
	bgfx_program_handle_t **ud = (bgfx_program_handle_t**)lua_touserdata(L, index);
	if (ud == NULL) luaL_typerror(L, index, "bgfx_program");
	return *ud;
}

static bgfx_vertex_buffer_handle_t *to_vertex_buffer_ud(lua_State *L, int index) {
	bgfx_vertex_buffer_handle_t **ud = (bgfx_vertex_buffer_handle_t**)lua_touserdata(L, index);
	if (ud == NULL) luaL_typerror(L, index, "bgfx_vertex_buffer");
	return *ud;
}

static bgfx_index_buffer_handle_t *to_index_buffer_ud(lua_State *L, int index) {
	bgfx_index_buffer_handle_t **ud = (bgfx_index_buffer_handle_t**)lua_touserdata(L, index);
	if (ud == NULL) luaL_typerror(L, index, "bgfx_index_buffer");
	return *ud;
}

static const luaL_Reg shader_fn[] = {
	{ "new", [](lua_State *L) {
		bgfx_shader_handle_t **ud = (bgfx_shader_handle_t**)lua_newuserdata(L, sizeof(bgfx_shader_handle_t*));

		const void *data = lua_topointer(L, -1);
		unsigned int size = (unsigned int)lua_tonumber(L, -2);
		lua_assert(data != NULL);
		const bgfx_memory_t *mem = bgfx_make_ref(data, size);
		*(*ud) = bgfx_create_shader(mem);

		luaL_getmetatable(L, "bgfx_shader");
		lua_setmetatable(L, -2);
		return 1;
	} },
	{ "__gc", [](lua_State *L) {
		bgfx_shader_handle_t *ud = to_shader_ud(L, 1);
		bgfx_destroy_shader(*ud);
		return 0;
	} },
	{ "__tostring", [](lua_State *L) {
		char buff[32];
		sprintf(buff, "%p", to_shader_ud(L, 1));
		lua_pushfstring(L, "bgfx_shader (%s)", buff);
		return 0;
	} },
	{ NULL, NULL }
};

static const luaL_Reg program_fn[] = {
	{ "new", [](lua_State *L) {
		bgfx_program_handle_t **ud = (bgfx_program_handle_t**)lua_newuserdata(L, sizeof(bgfx_program_handle_t*));

		// lua_assert(data != NULL);
		bgfx_shader_handle_t vsh = *to_shader_ud(L, -1);
		bgfx_shader_handle_t fsh = *to_shader_ud(L, -1);

		*(*ud) = bgfx_create_program(vsh, fsh, false);

		luaL_getmetatable(L, "bgfx_program");
		lua_setmetatable(L, -2);
		return 1;
	} },
	{ "__gc",  [](lua_State *L) {
		bgfx_program_handle_t *ud = to_program_ud(L, 1);
		bgfx_destroy_program(*ud);
		return 0;
	} },
	{ "__tostring", [](lua_State *L) {
		char buff[32];
		sprintf(buff, "%p", to_program_ud(L, 1));
		lua_pushfstring(L, "bgfx_program (%s)", buff);
		return 0;
	} },
	{ NULL, NULL }
};

static const luaL_Reg vertex_buffer_fn[] = {
	{ "new", [](lua_State *L) {
		bgfx_vertex_buffer_handle_t **ud = (bgfx_vertex_buffer_handle_t**)lua_newuserdata(L, sizeof(bgfx_vertex_buffer_handle_t*));

		// lua_assert(data != NULL);

		const bgfx_memory_t *mem = NULL;
		uint16_t flags = 0;
		bgfx_vertex_decl_t *decl = NULL;

		*(*ud) = bgfx_create_vertex_buffer(mem, decl, flags);

		luaL_getmetatable(L, "bgfx_vertex_buffer");
		lua_setmetatable(L, -2);
		return 1;
	} },
	{ "__gc",  [](lua_State *L) {
		bgfx_vertex_buffer_handle_t *ud = to_vertex_buffer_ud(L, 1);
		bgfx_destroy_vertex_buffer(*ud);
		return 0;
	} },
	{ "__tostring", [](lua_State *L) {
		char buff[32];
		sprintf(buff, "%p", to_vertex_buffer_ud(L, 1));
		lua_pushfstring(L, "bgfx_vertex_buffer (%s)", buff);
		return 0;
	} },
	{ NULL, NULL }
};

static const luaL_Reg index_buffer_fn[] = {
	{ "new", [](lua_State *L) {
		bgfx_index_buffer_handle_t **ud = (bgfx_index_buffer_handle_t**)lua_newuserdata(L, sizeof(bgfx_index_buffer_handle_t*));

		lua_assert(data != NULL);
		const bgfx_memory_t *mem = NULL;
		uint16_t flags = 0;

		*(*ud) = bgfx_create_index_buffer(mem, flags);

		luaL_getmetatable(L, "bgfx_index_buffer");
		lua_setmetatable(L, -2);
		return 1;
	} },
	{ "__gc",  [](lua_State *L) {
		bgfx_index_buffer_handle_t *ud = to_index_buffer_ud(L, 1);
		bgfx_destroy_index_buffer(*ud);
		return 0;
	} },
	{ "__tostring", [](lua_State *L) {
		char buff[32];
		sprintf(buff, "%p", to_index_buffer_ud(L, 1));
		lua_pushfstring(L, "bgfx_index_buffer (%s)", buff);
		return 0;
	} },
	{ NULL, NULL }
};

static std::unordered_map<const char*, uint32_t> debug_lookup = {
	{ "none",      BGFX_DEBUG_NONE },
	{ "wireframe", BGFX_DEBUG_WIREFRAME },
	{ "stats",     BGFX_DEBUG_STATS },
	{ "ifh",       BGFX_DEBUG_IFH },
	{ "text",      BGFX_DEBUG_TEXT }
};

static std::unordered_map<const char*, uint32_t> reset_lookup = {
	{ "none",               BGFX_RESET_NONE },
	{ "fullscreen",         BGFX_RESET_FULLSCREEN },
	{ "msaa_x2",            BGFX_RESET_MSAA_X2 },
	{ "msaa_x4",            BGFX_RESET_MSAA_X4 },
	{ "msaa_x8",            BGFX_RESET_MSAA_X8 },
	{ "msaa_x16",           BGFX_RESET_MSAA_X16 },
	{ "vsync",              BGFX_RESET_VSYNC },
	{ "maxanisotropy",      BGFX_RESET_MAXANISOTROPY },
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
static std::unordered_map<const char*, uint32_t> state_lookup = {
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

	{ "msaa",   BGFX_STATE_MSAA },
	{ "lineaa", BGFX_STATE_LINEAA },
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

template <typename F>
static void table_scan(lua_State *L, int index, F fn) {
	lua_pushvalue(L, index);
	lua_pushnil(L);

	while (lua_next(L, -2)) {
		lua_pushvalue(L, -2);
		const char *key = lua_tostring(L, -1);
		const char *value = lua_tostring(L, -2);
		lua_pop(L, 2);

		if (!fn(key, value)) {
			break;
		}
	}

	lua_pop(L, 1);
}

static const luaL_Reg m[] = {
	// bgfx.init()
	// TODO: actually take some args for this
	{ "init", [](lua_State *) {
		bgfx_init(BGFX_RENDERER_TYPE_OPENGL, BGFX_PCI_ID_NONE, 0, NULL, NULL);
		return 0;
	} },

	// bgfx.shutdown()
	{ "shutdown", [](lua_State *) {
		bgfx_shutdown();
		return 0;
	} },

	// bgfx.debug_text_print(1, 1, 0x7f, string.format("hello %s!", "world"))
	{ "debug_text_print", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n == 4);
		(void)n;
		uint16_t x = (uint16_t)lua_tonumber(L, -1);
		uint16_t y = (uint16_t)lua_tonumber(L, -2);
		uint8_t attr = (uint8_t)lua_tonumber(L, -3);
		const char *str = lua_tostring(L, -4);
		bgfx_dbg_text_printf(x, y, attr, "%s", str);
		return 0;
	} },

	// bgfx.debug_text_clear()
	{ "debug_text_clear", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n >= 0 || n <= 2);
		uint8_t attr = n >= 1 ? (uint8_t)lua_tonumber(L, -1) : 0;
		bool small   = n >= 2 ? (bool)lua_toboolean(L, -2) : false;
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

		table_scan(L, -1, [&](const char *, const char *v) {
			auto val = debug_lookup.find(v);
			if (val != debug_lookup.end()) {
				debug |= val->second;
			}
			return true;
		});

		bgfx_set_debug(debug);

		return 0;
	} },

	// bgfx.reset (1280, 720 {
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
		int w = (int)lua_tonumber(L, -1);
		int h = (int)lua_tonumber(L, -2);
		uint32_t reset = 0;

		table_scan(L, -3, [&](const char *, const char *v) {
			auto val = reset_lookup.find(v);
			if (val != reset_lookup.end()) {
				reset |= val->second;
			}
			return true;
		});

		bgfx_reset(w, h, reset);
		return 0;
	} },

	// bgfx.touch(0)
	{ "touch", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n == 1);
		(void)n;
		uint8_t id = (uint8_t)lua_tonumber(L, -1);
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

		table_scan(L, -1, [&](const char *, const char *v) {
			auto val = state_lookup.find(v);
			if (val != state_lookup.end()) {
				flags |= val->second;
			}
			return true;
		});

		bgfx_set_state((uint32_t)flags, rgba);

		return 0;
	} },

	// bgfx.set_view_rect(0, 0, 0, 1280, 720)
	// bgfx.set_view_rect(0) -- auto set to full extents
	{ "set_view_rect", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n >= 1);
		uint8_t id = (uint8_t)lua_tonumber(L, -1);
		uint16_t x = 0;
		uint16_t y = 0;
		if (n >= 3) {
			x = (uint16_t)lua_tonumber(L, -2);
			y = (uint16_t)lua_tonumber(L, -3);
		}
		if (n <= 3) {
			bgfx_set_view_rect_auto(id, x, y, BGFX_BACKBUFFER_RATIO_EQUAL);
			return 0;
		}
		if (n == 5) {
			uint16_t w = (uint16_t)lua_tonumber(L, -4);
			uint16_t h = (uint16_t)lua_tonumber(L, -5);
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
		uint8_t id = (uint8_t)lua_tonumber(L, -1);
		uint16_t x = (uint16_t)lua_tonumber(L, -2);
		uint16_t y = (uint16_t)lua_tonumber(L, -3);
		uint16_t w = (uint16_t)lua_tonumber(L, -4);
		uint16_t h = (uint16_t)lua_tonumber(L, -5);
		bgfx_set_view_scissor(id, x, y, w, h);
		return 0;
	} },

	{ "set_view_clear", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n == 5);
		(void)n;
		uint8_t id = (uint8_t)lua_tonumber(L, -1);
		uint16_t flags = (uint16_t)lua_tonumber(L, -2);
		uint32_t rgba = (uint32_t)lua_tonumber(L, -3);
		float depth = (float)lua_tonumber(L, -4);
		uint8_t stencil = (uint8_t)lua_tonumber(L, -5);
		bgfx_set_view_clear(id, flags, rgba, depth, stencil);
		return 0;
	} },

	// bgfx.set_view_name("steve")
	{ "set_view_name", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n == 2);
		(void)n;
		uint8_t id = (uint8_t)lua_tonumber(L, -1);
		const char *name = lua_tostring(L, -2);
		bgfx_set_view_name(id, name);
		return 0;
	} },

	// bgfx.submit(0, program)
	// bgfx.submit(0, program, 0, false)
	{ "submit", [](lua_State *L) {
		int n = lua_gettop(L);
		lua_assert(n == 4);
		(void)n;
		uint8_t id = (uint8_t)lua_tonumber(L, -1);
		bgfx_program_handle_t *program = to_program_ud(L, -2);
		int32_t depth = 0;
		bool preserve_state = false;
		if (n >= 3) {
			depth = (int32_t)lua_tonumber(L, -3);
		}
		if (n >= 4) {
			preserve_state = lua_toboolean(L, -4) ? true : false;
		}
		uint32_t r = bgfx_submit(id, *program, depth, preserve_state);
		lua_pushnumber(L, r);
		return 1;
	} },

	{ "set_view_transform", [](lua_State *) {
		// bgfx_set_view_transform(id, view, proj);
		// bgfx_set_view_transform_stereo(id, view, proj_l, flags, proj_r);
		return 0;
	} },

	{ "set_transform", [](lua_State *) {
		// bgfx_set_transform(mtx, num);
		return 0;
	} },

	{ "set_vertex_buffer", [](lua_State *) {
		// bgfx_set_vertex_buffer(handle, start, num);
		return 0;
	} },

	{ "set_index_buffer", [](lua_State *) {
		// bgfx_set_index_buffer(handle, first, num);
		return 0;
	} },

	// local hmd = bgfx.get_hmd()
	{ "get_hmd", [](lua_State *L) {
		const bgfx_hmd_t *hmd = bgfx_get_hmd();

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
	luaL_newmetatable(L, "bgfx_shader");
	luaL_register(L, NULL, shader_fn);
	lua_pushvalue(L, -1);
	lua_setfield(L, -1, "__index");

	luaL_newmetatable(L, "bgfx_program");
	luaL_register(L, NULL, program_fn);
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

	luaL_register(L, "bgfx", m);

	return 1;
}
