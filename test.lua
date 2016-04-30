#!/usr/bin/env luajit
package.cpath = package.cpath .. ";./bin/?.so"

local bgfx = require "bgfx"

require "love"
require "love.filesystem"
love.filesystem.init(arg[0])

bgfx.init(true)
bgfx.reset(1280, 720, { "vsync" })
bgfx.set_debug { "text" }

-- broken
-- local hmd = bgfx.get_hmd()
-- for k, v in pairs(hmd) do
-- 	print(k, v)
-- end

local fmt = bgfx.new_vertex_format {
	{ attrib = "position", type = "float", num = 3 }
}

bgfx.set_view_clear(0, { "color", "depth" }, 0x303030ff, 1.0, 0)
bgfx.set_view_name(0, "igor");

-- local vb = bgfx.new_vertex_buffer()
-- local ib = bgfx.new_index_buffer()
-- local vsh = bgfx.new_shader()
-- local fsh = bgfx.new_shader()
-- local csh = bgfx.new_shader()
-- local p = bgfx.new_program(vsh, fsh)

for i=1,120 do
	bgfx.debug_text_clear()
	bgfx.debug_text_print(0, 0, 0x6f, "ayy lmao")
	bgfx.set_view_rect(0, 0, 0, 1280, 720)
	bgfx.touch(0)

	-- bgfx.set_vertex_buffer(vb, 0, 30)
	-- bgfx.set_index_buffer(ib, 0, 30)
	-- bgfx.set_state { "pt_points" }
	-- bgfx.submit(0, p)

	bgfx.frame()
end

bgfx.shutdown()
