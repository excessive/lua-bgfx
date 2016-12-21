local bgfx = require "bgfx"
local cpml = require "cpml"
local iqm  = require "iqm"
local model, program

function love.load()
	bgfx.init(true)
	bgfx.reset(1280, 720, { "vsync" })
	bgfx.set_debug {
		"stats"
	}

	local info = bgfx.get_renderer_info()
	print(info.name, info.type)

	-- local hmd = bgfx.get_hmd() or {}
	-- for k, v in pairs(hmd) do
	-- 	print(k, v)
	-- end

	model = iqm.load("assets/models/chair.iqm")

	bgfx.set_view_clear(0, { "color", "depth" }, 0x303030ff, 1.0, 0)
	bgfx.set_view_name(0, "igor")

	program = bgfx.new_program(
		love.filesystem.read("assets/shaders/bin/test.vs.bin"),
		love.filesystem.read("assets/shaders/bin/test.fs.bin")
	)
end

function love.draw()
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

function love.quit()
	model   = nil
	program = nil

	bgfx.shutdown()
end

function love.run()
	love.event.pump()

	if love.load then love.load(arg) end

	collectgarbage "collect"

	if love.timer then love.timer.step() end

	local dt = 0
	while true do
		if love.event then
			love.event.pump()
			for name, a,b,c,d,e,f in love.event.poll() do
				if name == "keypressed" and a == "escape" and
					(love.keyboard.isDown "lshift" or love.keyboard.isDown "rshift")
				then
					love.event.quit()
				elseif name == "quit" then
					if not love.quit or not love.quit() then
						return a
					end
				end
				love.handlers[name](a, b, c, d, e, f)
			end
		end

		-- Update dt, as we'll be passing it to update
		if love.timer then
			love.timer.step()
			dt = love.timer.getDelta()
		end

		if love.update then
			love.update(dt)
		end

		if love.draw then
			love.draw()
		end

		collectgarbage("step")
		love.timer.sleep(0.001)
	end
end
