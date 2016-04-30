#!/usr/bin/env luajit
package.cpath = package.cpath .. ";./bin/?.so"

local bgfx = require "bgfx"

require "love"
require "love.filesystem"

local function get_low(a)
	local m = math.huge
	for k,v in pairs(a) do
		if k < m then
			m = k
		end
	end
	return a[m]
end
love.filesystem.init(get_low(arg))
love.filesystem.setSource(love.filesystem.getWorkingDirectory())

local function load()
	bgfx.init(true)
	bgfx.reset(1280, 720, { "vsync" })
	bgfx.set_debug { "text" }

	local info = bgfx.get_renderer_info()
	print(info.name, info.type)

	local hmd = bgfx.get_hmd() or {}
	for k, v in pairs(hmd) do
		print(k, v)
	end

	local fmt = bgfx.new_vertex_format {
		{ attrib = "position", type = "float", num = 3 }
	}
	print(fmt)

	local ib = bgfx.new_index_buffer({1, 2, 3, 4, 5, 6})
	print(ib)

	local data, err = love.filesystem.newFileData("bin/bgfx.so")
	local vb = bgfx.new_vertex_buffer(data:getPointer(), data:getSize(), fmt)
	print(vb)

	bgfx.set_view_clear(0, { "color", "depth" }, 0x303030ff, 1.0, 0)
	bgfx.set_view_name(0, "igor");
end

-- local vsh = bgfx.new_shader()
-- local fsh = bgfx.new_shader()
-- local csh = bgfx.new_shader()
-- local p = bgfx.new_program(vsh, fsh)

local function draw()
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

local function quit()
	bgfx.shutdown()
end

require "love.math"
require "love.event"
require "love.timer"
require "love.keyboard"

love.math.setRandomSeed(os.time())
love.event.pump()
if load then load() end
love.timer.step()
local dt = 0
while true do
	if love.event then
		love.event.pump()
		for name, a,b,c,d,e,f in love.event.poll() do
			if name == "keypressed" and a == "escape" and
				(love.keyboard.isDown "lshift" or love.keyboard.isDown "rshift")
			then
				love.event.quit()
			end
			if name == "quit" then
				if not quit or not quit() then
					return
				end
			end
		end
	end

	if love.timer then
		love.timer.step()
		dt = love.timer.getDelta()
		if love.keyboard.isDown "tab" then
			dt = dt * 4
		end
	end

	if update then update(dt) end
	if draw then draw() end

	collectgarbage("step")

	if love.timer then love.timer.sleep(0.001) end
end
