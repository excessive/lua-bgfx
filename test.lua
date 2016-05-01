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

local cpml = require "cpml"
local iqm  = require "iqm"
local model, program

local function load()
	bgfx.init(true)
	bgfx.reset(1280, 720, { "vsync" })
	bgfx.set_debug { "text", "wireframe" }

	local info = bgfx.get_renderer_info()
	print(info.name, info.type)

	local hmd = bgfx.get_hmd() or {}
	for k, v in pairs(hmd) do
		print(k, v)
	end

	model = iqm.load("chair.iqm")

	bgfx.set_view_clear(0, { "color", "depth" }, 0x303030ff, 1.0, 0)
	bgfx.set_view_name(0, "igor")

	local vsb = love.filesystem.newFileData("test.vs.bin")
	local fsb = love.filesystem.newFileData("test.fs.bin")
	program = bgfx.new_program(
		bgfx.new_shader(vsb:getPointer(), vsb:getSize()),
		bgfx.new_shader(fsb:getPointer(), fsb:getSize())
	)
	vsb = nil
	fsb = nil
end

local function draw()
	bgfx.set_marker("miku")
	bgfx.debug_text_clear()
	bgfx.debug_text_print(0, 0, 0x6f, "ayy lmao")
	bgfx.set_view_rect(0, 0, 0, 1280, 720)
	bgfx.touch(0)

	local p = cpml.mat4():perspective(60, 1280/720, 0.1, 100.0)
	local v = cpml.mat4():look_at(
		cpml.vec3(0, -5, 0),
		cpml.vec3(0, 0.5, 0),
		cpml.vec3.unit_z
	)
	bgfx.set_view_transform(0, v, p)

	for _, mesh in ipairs(model) do
		local m = cpml.mat4()
		bgfx.set_vertex_buffer(model.mesh)
		bgfx.set_index_buffer(model.ibo, mesh.first, mesh.count)
		bgfx.set_state {
			"msaa",
			"alpha_write",
			"rgb_write",
			"cull_ccw",
			"depth_write",
			"depth_test_lequal"
		}
		bgfx.set_transform(m)
		bgfx.submit(0, program)
	end

	bgfx.frame()
end

local function quit()
	bgfx.shutdown()
end

require "love.event"
require "love.timer"
require "love.keyboard"

love.event.pump()
if load then load() end
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
				if not quit() then
					return
				end
			end
		end
	end
	draw()
	collectgarbage("step")
	love.timer.sleep(0.001)
end
