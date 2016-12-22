local cpml   = require "cpml"
local camera = {}

local camera_mt = {
	__index = camera
}

local function new(options)
	local t = {
		fov      = options.fov      or 45,
		near     = options.near     or 0.1,   -- 10cm
		far      = options.far      or 150.0, -- 150m
		exposure = options.exposure or 1.0,

		position    = options.position    or cpml.vec3(0, 0, 0),
		orientation = options.orientation or cpml.quat(0, 0, 0, 1),

		target = options.target or false,
		up     = cpml.vec3.unit_z,

		orbit_offset = options.orbit_offset or cpml.vec3(0, 0, 0),
		offset       = options.offset or cpml.vec3(0, 0, 0),

		view       = cpml.mat4(),
		projection = cpml.mat4(),

		mouse_sensitivity = 0.1,
		pitch_limit_up    = 0.3,
		pitch_limit_down  = 0.9
	}
	t.direction = t.orientation * cpml.vec3.unit_y

	return setmetatable(t, camera_mt)
end

function camera:rotate_xy(mx, my)
	local sensitivity     = self.mouse_sensitivity
	local mouse_direction = {
		x = math.rad(-mx * sensitivity),
		y = math.rad(-my * sensitivity)
	}

	-- get the axis to rotate around the x-axis.
	local axis = cpml.vec3():cross(self.direction, self.up)
	axis:normalize(axis)

	-- First, we apply a left/right rotation.
	self.orientation = cpml.quat.rotate(mouse_direction.x, self.up) * self.orientation

	-- Next, we apply up/down rotation.
	-- up/down rotation is applied after any other rotation (so that other rotations are not affected by it),
	-- hence we post-multiply it.
	local new_orientation = self.orientation * cpml.quat.rotate(mouse_direction.y, cpml.vec3.unit_x)
	local new_pitch       = (new_orientation * cpml.vec3.unit_y):dot(self.up)

	-- Don't rotate up/down more than self.pitch_limit.
	-- We need to limit pitch, but the only reliable way we're going to get away with this is if we
	-- calculate the new orientation twice. If the new rotation is going to be over the threshold and
	-- Y will send you out any further, cancel it out. This prevents the camera locking up at +/-PITCH_LIMIT
	if new_pitch >= self.pitch_limit_up then
		mouse_direction.y = math.min(0, mouse_direction.y)
	elseif new_pitch <= -self.pitch_limit_down then
		mouse_direction.y = math.max(0, mouse_direction.y)
	end

	self.orientation = self.orientation * cpml.quat.rotate(mouse_direction.y, cpml.vec3.unit_x)

	-- Apply rotation to camera direction
	self.direction = self.orientation * cpml.vec3.unit_y
end

function camera:update(w, h)
	local aspect = math.max(w / h, h / w)
	local target = self.target and self.target or (self.position + self.direction)
	local look = cpml.mat4()
	look:look_at(look, self.position, target, cpml.vec3.unit_z)
	self.view:identity(self.view)
		:translate(self.view, self.orbit_offset)
		:mul(look, self.view)
		:translate(self.view, self.offset)

	self.projection = cpml.mat4.from_perspective(self.fov, aspect, self.near, self.far)
end

return setmetatable(
	{ new = new },
	{ __call = function(_, ...) return new(...) end }
)
