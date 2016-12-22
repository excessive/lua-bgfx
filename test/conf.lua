function love.conf(t)
	t.modules.graphics = false
	t.modules.window   = true
	t.modules.audio    = false
	t.modules.sound    = false
	-- t.console          = true
	io.stdout:setvbuf("no") -- Don't delay prints.
end
