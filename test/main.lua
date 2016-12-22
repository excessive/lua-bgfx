local bgfx = require "bgfx"
local cpml = require "cpml"
local iqm  = require "iqm"
local model, program, tex, sampler

local camera = require "camera" {
	orbit_offset = cpml.vec3(0, 0, -3),
	offset       = cpml.vec3(0, 0, -1)
}

local w, h = love.window.getMode()

function love.load()
	bgfx.init()
	bgfx.reset(w, h, { "vsync" })
	bgfx.set_debug { "text" }

	local info = bgfx.get_renderer_info()
	print(info.name, info.type)

	-- local hmd = bgfx.get_hmd() or {}
	-- for k, v in pairs(hmd) do
	-- 	print(k, v)
	-- end

	local _tex = love.image.newImageData("assets/textures/grid.png")
	tex = bgfx.new_texture(_tex:getString(), _tex:getWidth(), _tex:getHeight(), false)
	sampler = bgfx.new_uniform("s_tex", "int", 1)

	model = iqm.load("assets/models/StagePolish0.11.iqm")
	-- model = iqm.load("assets/models/chair.iqm")

	bgfx.set_view_clear(0, { "color", "depth" }, 0x303030ff, 1.0, 0)
	bgfx.set_view_name(0, "igor")

	program = bgfx.new_program(
		love.filesystem.read("assets/shaders/bin/test.vs.bin"),
		love.filesystem.read("assets/shaders/bin/test.fs.bin")
	)

	love.mouse.setRelativeMode(true)
end

function love.mousemoved(_, _, dx, dy)
	camera:rotate_xy(dx, dy)
end

function love.draw()
	bgfx.set_marker("miku")
	bgfx.debug_text_clear()
	bgfx.debug_text_print(0, 0, 0x6f, "ayy lmao")
	bgfx.set_view_rect(0, 0, 0, w, h)
	bgfx.touch(0)

	camera:update(w, h)
	bgfx.set_view_transform(0, camera.view, camera.projection)

	for _, mesh in ipairs(model) do
		local m = cpml.mat4()
		bgfx.set_vertex_buffer(model.mesh)
		bgfx.set_index_buffer(model.ibo, mesh.first, mesh.count)
		bgfx.set_state {
			"msaa",
			"alpha_write",
			"cull_ccw",
			"rgb_write",
			"depth_write",
			"depth_test_lequal"
		}
		bgfx.set_texture(0, sampler, tex)
		bgfx.set_transform(m)
		bgfx.submit(0, program)
	end

	bgfx.frame()
end

function love.quit()
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
