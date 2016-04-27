#include <lua.hpp>
#include <bgfx/c99/bgfx.h>

namespace {
	const luaL_Reg m[] = {
		// TODO: actually take some args for this
		{ "init", [](lua_State *) {
			bgfx_init(BGFX_RENDERER_TYPE_COUNT, BGFX_PCI_ID_NONE, 0, NULL, NULL);
			return 0;
		} },

		{ "shutdown", [](lua_State *) {
			bgfx_shutdown();
			return 0;
		} },

		{ "dbg_text_print", [](lua_State *L) {
			int n = lua_gettop(L);
			lua_assert(n == 4);
			uint16_t x = (uint16_t)lua_tonumber(L, -1);
			uint16_t y = (uint16_t)lua_tonumber(L, -2);
			uint8_t attr = (uint8_t)lua_tonumber(L, -3);
			const char *str = lua_tostring(L, -4);
			bgfx_dbg_text_printf(x, y, attr, "%s", str);
			return 0;
		} },

		{ "dbg_text_clear", [](lua_State *L) {
			int n = lua_gettop(L);
			lua_assert(n >= 0 || n <= 2);
			uint8_t attr = n >= 1 ? (uint8_t)lua_tonumber(L, -1) : 0;
			bool small   = n >= 2 ? (bool)lua_toboolean(L, -2) : false;
			bgfx_dbg_text_clear(attr, small);
			return 0;
		} },

		// TODO: parse stuff
		{ "set_debug", [](lua_State *) {
			bgfx_set_debug(BGFX_DEBUG_WIREFRAME | BGFX_DEBUG_STATS | BGFX_DEBUG_IFH | BGFX_DEBUG_TEXT | BGFX_DEBUG_NONE);
			return 0;
		} },

		// TODO: parse stuff
		{ "reset", [](lua_State *L) {
			int n = lua_gettop(L);
			lua_assert(n == 3);
			int w = (int)lua_tonumber(L, -1);
			int h = (int)lua_tonumber(L, -2);
			//int f = (int)lua_tonumber(L, -3); // TODO
			uint32_t reset_flags = 0
				| BGFX_RESET_VSYNC
				| BGFX_RESET_DEPTH_CLAMP
				| BGFX_RESET_SRGB_BACKBUFFER
				| BGFX_RESET_FLIP_AFTER_RENDER
				| BGFX_RESET_FLUSH_AFTER_RENDER
				//| BGFX_RESET_HMD
				//| BGFX_RESET_MSAA_X16
				;
			bgfx_reset(w, h, reset_flags);
			return 0;
		} },

		{ "touch", [](lua_State *L) {
			int n = lua_gettop(L);
			lua_assert(n == 1);
			uint8_t id = (uint8_t)lua_tonumber(L, -1);
			unsigned int ret = bgfx_touch(id);
			lua_pushnumber(L, double(ret));
			return 1;
		} },

		{ "frame", [](lua_State *L) {
			int r = bgfx_frame();
			lua_pushnumber(L, (double)r);
			return 1;
		} },

		// TODO: parse stuff
		{ "set_state", [](lua_State *L) {
			uint32_t flags = 0
				| BGFX_STATE_MSAA
				| BGFX_STATE_ALPHA_WRITE
				| BGFX_STATE_RGB_WRITE
				| BGFX_STATE_CULL_CCW
				| BGFX_STATE_DEPTH_WRITE
				| BGFX_STATE_DEPTH_TEST_LEQUAL
			;
			uint32_t rgba = 0;
			bgfx_set_state(flags, rgba);
			return 0;
		} },

		{ "set_view_rect", [](lua_State *L) {
			int n = lua_gettop(L);
			lua_assert(n == 5);
			uint8_t id = (uint8_t)lua_tonumber(L, -1);
			uint16_t x = (uint16_t)lua_tonumber(L, -2);
			uint16_t y = (uint16_t)lua_tonumber(L, -3);
			uint16_t w = (uint16_t)lua_tonumber(L, -4);
			uint16_t h = (uint16_t)lua_tonumber(L, -5);
			bgfx_set_view_rect(id, x, y, w, h);
			return 0;
		} },

		{ "set_view_rect_auto", [](lua_State *L) {
			uint8_t id = (uint8_t)lua_tonumber(L, -1);
			uint16_t x = (uint16_t)lua_tonumber(L, -2);
			uint16_t y = (uint16_t)lua_tonumber(L, -3);
			bgfx_set_view_rect_auto(id, x, y, BGFX_BACKBUFFER_RATIO_EQUAL);
			return 0;
		} },

		{ "set_view_scissor", [](lua_State *L) {
			int n = lua_gettop(L);
			lua_assert(n == 5);
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
			lua_assert(n == 999);
			uint8_t id = (uint8_t)lua_tonumber(L, -1);
			uint16_t flags = (uint16_t)lua_tonumber(L, -2);
			uint32_t rgba = (uint32_t)lua_tonumber(L, -3);
			float depth = (float)lua_tonumber(L, -4);
			uint8_t stencil = (uint8_t)lua_tonumber(L, -5);
			bgfx_set_view_clear(id, flags, rgba, depth, stencil);
			return 0;
		} },

		{ "set_view_name", [](lua_State *L) {
			int n = lua_gettop(L);
			lua_assert(n == 2);
			uint8_t id = (uint8_t)lua_tonumber(L, -1);
			const char *name = lua_tostring(L, -2);
			bgfx_set_view_name(id, name);
			return 0;
		} },

		{ "submit", [](lua_State *L) {
			int n = lua_gettop(L);
			lua_assert(n == 4);
			uint8_t id = (uint8_t)lua_tonumber(L, -1);
			bgfx_program_handle_t program = {};
			bool depth = 1.0;
			bool preserve_state = false;
			uint32_t r = bgfx_submit(id, program, depth, preserve_state);
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

		{ "get_hmd", [](lua_State *) {
			// TODO: turn this thing into a table
			const bgfx_hmd_t *hmd = bgfx_get_hmd();
			(void)hmd;
			return 0;
		} },

		/*
			auto destroy_shader = [](lua_State *L) {
				luaL_checkudata(L, 1, "bgfx_shader");
				return 0;
			};
			luaL_Reg shader_reg[] = {
				{ "__gc", destroy_shader },
				{ NULL, NULL }
			};
			luaL_newmetatable(L, "bgfx_shader");
			lua_pushvalue(L, -1);
			lua_setfield(L, -2, "__index");
			luaL_register(L, "bgfx_shader", shader_reg);
		} },
*/
/*	
		bgfx::createIndexBuffer()
		bgfx::createProgram()
		bgfx::createShader()
		bgfx::createVertexBuffer()
		bgfx::destroyProgram(program)
		bgfx::destroyIndexBuffer()
		bgfx::destroyVertexBuffer()
*/
		{ NULL, NULL }
	};

	struct shader_ud_t {
		bgfx_shader_handle_t *handle;
		char *name;
	};

	shader_ud_t *to_shader_ud(lua_State *L, int index) {
		shader_ud_t *ud = (shader_ud_t*)lua_touserdata(L, index);
		if (ud == NULL) luaL_typerror(L, index, "bgfx_shader");
		return ud;
	}

	const luaL_Reg shader_fn[] = {
		{ NULL, NULL }
	};

	const luaL_Reg shader_mt[] = {
		{ "__gc",  [](lua_State *L) {
			shader_ud_t *ud = to_shader_ud(L, 1);
			bgfx_destroy_shader(*ud->handle);
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

}

int luaopen_bgfx(lua_State *L) {
	// barf
	luaL_openlib(L, "bgfx_shader", shader_fn, 0);
	luaL_newmetatable(L, "bgfx_shader");
	luaL_openlib(L, 0, shader_mt, 0);
	lua_pushliteral(L, "__index");
	lua_pushvalue(L, -3);
	lua_rawset(L, -3);
	lua_pushliteral(L, "__metatable");
	lua_pushvalue(L, -3);
	lua_rawset(L, -3);
	lua_pop(L, 1);

	luaL_register(L, "bgfx", m);
	return 1;
}
