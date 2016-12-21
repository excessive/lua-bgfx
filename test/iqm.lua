-- This is like the upstream version of the file, except +1's have been removed
-- and output format changes have been made such that it works with lua-bgfx.
local base = (...):gsub('%.init$', '') .. "."
local ffi  = require "ffi"

local iqm = {
	_LICENSE     = "Inter-Quake Model Loader is distributed under the terms of the MIT license. See LICENSE.md.",
   _FULL_LICENSE = [[
   # Inter-Quake Model Loader is licensed under the MIT Open Source License.
   (http://www.opensource.org/licenses/mit-license.html)

   Copyright (c) 2015 excessive ❤ moé

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
   ]],
	_URL         = "https://github.com/excessive/iqm",
	_VERSION     = "2.0.0",
	_DESCRIPTION = "Load an IQM 3D model into BGFX.",
}

ffi.cdef([[
typedef uint32_t uint;
typedef uint8_t uchar;
typedef uint8_t byte;

struct iqmheader {
    char magic[16];
    uint version;
    uint filesize;
    uint flags;
    uint num_text, ofs_text;
    uint num_meshes, ofs_meshes;
    uint num_vertexarrays, num_vertexes, ofs_vertexarrays;
    uint num_triangles, ofs_triangles, ofs_adjacency;
    uint num_joints, ofs_joints;
    uint num_poses, ofs_poses;
    uint num_anims, ofs_anims;
    uint num_frames, num_framechannels, ofs_frames, ofs_bounds;
    uint num_comment, ofs_comment;
    uint num_extensions, ofs_extensions;
};

struct iqmmesh {
    uint name;
    uint material;
    uint first_vertex, num_vertexes;
    uint first_triangle, num_triangles;
};

enum {
    IQM_POSITION     = 0,
    IQM_TEXCOORD     = 1,
    IQM_NORMAL       = 2,
    IQM_TANGENT      = 3,
    IQM_BLENDINDEXES = 4,
    IQM_BLENDWEIGHTS = 5,
    IQM_COLOR        = 6,
    IQM_CUSTOM       = 0x10
};

enum {
    IQM_BYTE   = 0,
    IQM_UBYTE  = 1,
    IQM_SHORT  = 2,
    IQM_USHORT = 3,
    IQM_INT    = 4,
    IQM_UINT   = 5,
    IQM_HALF   = 6,
    IQM_FLOAT  = 7,
    IQM_DOUBLE = 8,
};

struct iqmvertexarray {
    uint type;
    uint flags;
    uint format;
    uint size;
    uint offset;
};

struct iqmtriangle {
    uint vertex[3];
};

struct iqmadjacency {
    uint triangle[3];
};

struct iqmjoint {
    uint name;
    int parent;
    float translate[3], rotate[4], scale[3];
};

struct iqmpose {
    int parent;
    uint channelmask;
    float channeloffset[10], channelscale[10];
};

struct iqmanim {
    uint name;
    uint first_frame, num_frames;
    float framerate;
    uint flags;
};

enum {
    IQM_LOOP = 1<<0
};

struct iqmbounds {
    float bbmins[3], bbmaxs[3];
    float xyradius, radius;
};

]])

local c = ffi.C

iqm.lookup = {}

local function check_magic(magic)
	return magic:sub(1,16) == "INTERQUAKEMODEL\0"
end

local function load_data(file)
	local is_buffer = check_magic(file)

	-- Make sure it's a valid IQM file
	if not is_buffer then
		assert(love.filesystem.isFile(file))
		assert(check_magic(love.filesystem.read(file, 16)))
	end

	-- Decode the header, it's got all the offsets
	local iqm_header  = ffi.typeof("struct iqmheader*")
	local size        = ffi.sizeof("struct iqmheader")
	local header_data
	if is_buffer then
		header_data = file
	else
		header_data = love.filesystem.read(file, size)
	end
	local header = ffi.cast(iqm_header, header_data)[0]

	-- We only support IQM version 2
	assert(header.version == 2)

	-- Only read the amount of data declared by the header, just in case!
	local data = is_buffer and file or love.filesystem.read(file, header.filesize)

	return header, data
end

-- Read `num` element from `data` at `offset` and convert it to a table.
local function read_offset(data, type, offset, num)
	local decoded = {}
	local type_ptr = ffi.typeof(type.."*")
	local size = ffi.sizeof(type)
	local ptr = ffi.cast(type_ptr, data:sub(offset+1))
	for i = 1, num do
		table.insert(decoded, ptr[i-1])
	end
	return decoded
end

-- a bit simpler than read_offset, as we don't bother converting to a table.
local function read_ptr(data, type, offset)
	local type_ptr = ffi.typeof(type.."*")
	local size = ffi.sizeof(type)
	local ptr = ffi.cast(type_ptr, data:sub(offset+1))
	return ptr
end

-- Collect all text data in the file.
-- Not needed for meshes because everything is byte offsets - but very
-- useful for debugging.
local function dump_strings(text)
	local strings = {}
	local advance = 1

	-- header.num_text is the length of the text block in bytes.
	repeat
		local str = ffi.string(text + advance)
		table.insert(strings, str)
		advance = advance + str:len() + 1
		print(str)
	until advance >= header.num_text

	return strings
end

-- 'file' can be either a filename or IQM data (as long as the magic is intact)
function iqm.load(file, save_data)
	-- HACK: Workaround for a bug in LuaJIT's GC - we need to turn it off for the
	-- rest of the function or we'll get a segfault shortly into these loops.
	--
	-- I've got no idea why the GC thinks it can pull the rug out from under us,
	-- but I sure as hell don't appreciate it. Do NOT restart until the end. -ss
	collectgarbage("stop")

	local header, data = load_data(file)

	-- Decode the vertex arrays
	local vertex_arrays = read_offset(
		data,
		"struct iqmvertexarray",
		header.ofs_vertexarrays,
		header.num_vertexarrays
	)

	local function translate_va(type)
		local types = {
			[c.IQM_POSITION]     = "position",
			[c.IQM_TEXCOORD]     = "texcoord",
			[c.IQM_NORMAL]       = "normal",
			[c.IQM_TANGENT]      = "tangent",
			[c.IQM_COLOR]        = "color",
			[c.IQM_BLENDINDEXES] = "bone",
			[c.IQM_BLENDWEIGHTS] = "weight"
		}
		return types[type] or false
	end

	local function translate_format(type)
		local types = {
			[c.IQM_FLOAT] = "float",
			[c.IQM_UBYTE] = "byte",
		}
		return types[type] or false
	end

	local function translate_bgfx(type)
		local types = {
			position = "position",
			texcoord = "texcoord0",
			normal   = "normal",
			tangent  = "tangent",
			bone     = "indices",
			weight   = "weight",
			color    = "color0",
		}
		return assert(types[type])
	end

	-- Build iqm_vertex struct out of whatever is in this file
	local found = {}
	local found_names = {}
	local found_types = {}

	for _, va in ipairs(vertex_arrays) do
		while true do

		local type = translate_va(va.type)
		if not type then
			break
		end

		local format = assert(translate_format(va.format))

		table.insert(found, string.format("%s %s[%d]", format, type, va.size))
		table.insert(found_names, type)
		table.insert(found_types, {
			type        = type,
			size        = va.size,
			offset      = va.offset,
			format      = format,
			bgfx_type   = translate_bgfx(type)
		})

		break end
	end
	table.sort(found_names)
	local title = "iqm_vertex_" .. table.concat(found_names, "_")

	-- If we've already got a struct of this type, reuse it.
	local type = iqm.lookup[title]
	if not type then
		local def = string.format("struct %s {\n\t%s;\n};", title, table.concat(found, ";\n\t"))
		ffi.cdef(def)

		local ct = ffi.typeof("struct " .. title)
		iqm.lookup[title] = ct
		type = ct
	end

	local filedata = love.filesystem.newFileData(("\0"):rep(header.num_vertexes * ffi.sizeof(type)), "dummy")
	local vertices = ffi.cast("struct " .. title .. "*", filedata:getPointer())

	-- TODO: Compute XY + spherical radiuses
	local computed_bbox = { min = {}, max = {} }

	-- Interleave vertex data
	for _, va in ipairs(found_types) do
		local ptr = read_ptr(data, va.format, va.offset)
		for i = 0, header.num_vertexes-1 do
			for j = 0, va.size-1 do
				vertices[i][va.type][j] = ptr[i*va.size+j]
			end
			if va.type == "position" then
				local v = vertices[i][va.type]
				for i = 1, 3 do
					computed_bbox.min[i] = math.min(computed_bbox.min[i] or v[i-1], v[i-1])
					computed_bbox.max[i] = math.max(computed_bbox.max[i] or v[i-1], v[i-1])
				end
			end
		end
	end

	-- Decode triangle data (index buffer)
	local triangles = read_offset(
		data,
		"struct iqmtriangle",
		header.ofs_triangles,
		header.num_triangles
	)
	assert(#triangles == header.num_triangles)

	-- Translate indices for bgfx
	local indices = {}
	for _, triangle in ipairs(triangles) do
		table.insert(indices, triangle.vertex[0] + 1)
		-- IQM uses CW winding, but we want CCW. Reverse.
		table.insert(indices, triangle.vertex[2] + 1)
		table.insert(indices, triangle.vertex[1] + 1)
	end

	-- re-read the vertex data :(
	local save_buffer = {}
	if save_data then
		local buffer = {}
		for _, va in ipairs(found_types) do
			local ptr = read_ptr(data, va.format, va.offset)
			for i = 1, header.num_vertexes do
				buffer[i] = buffer[i] or {}
				buffer[i][va.type] = {}
				for j = 0, va.size-1 do
					buffer[i][va.type][j+1] = ptr[(i-1)*va.size+j]
				end
			end
		end
		for i, triangle in ipairs(triangles) do
			save_buffer[i] = {
				buffer[triangle.vertex[0] + 1],
				buffer[triangle.vertex[2] + 1],
				buffer[triangle.vertex[1] + 1]
			}
		end
	end

	local layout = {}
	for i, va in ipairs(found_types) do
		layout[i] = {
			attrib     = va.bgfx_type,
			type       = va.format,
			size       = va.size,
			normalized = va.bgfx_type == "color0"
		}
	end

	local fmt = bgfx.new_vertex_format(layout)
	local vb  = bgfx.new_vertex_buffer(filedata:getString(), fmt)
	local ib  = bgfx.new_index_buffer(indices)

	-- Decode mesh/material names.
	local text = read_ptr(
		data,
		"char",
		header.ofs_text
	)

	local objects = {}
	objects.bounds = {}
	objects.bounds.base = computed_bbox
	objects.triangles = save_buffer

	if header.ofs_bounds > 0 then
		local bounds = read_offset(
			data,
			"struct iqmbounds",
			header.ofs_bounds,
			header.num_frames
		)
		for i, bb in ipairs(bounds) do
			table.insert(objects.bounds, {
				min = { bb.bbmins[0], bb.bbmins[1], bb.bbmins[2] },
				max = { bb.bbmaxs[0], bb.bbmaxs[1], bb.bbmaxs[2] }
			})
		end
	end

	-- Decode meshes
	local meshes = read_offset(
		data,
		"struct iqmmesh",
		header.ofs_meshes,
		header.num_meshes
	)

	objects.has_joints = header.ofs_joints > 0
	objects.has_anims  = header.ofs_anims  > 0
	objects.mesh = vb
	objects.ibo  = ib
	for i, mesh in ipairs(meshes) do
		local add = {
			first    = mesh.first_triangle * 3,
			count    = mesh.num_triangles * 3,
			material = ffi.string(text+mesh.material),
			name     = ffi.string(text+mesh.name)
		}
		add.last = add.first + add.count
		table.insert(objects, add)
	end

	collectgarbage("restart")

	return objects
end

function iqm.load_anims(file)
	-- Require CPML here because loading the mesh does not depend on it.
	local cpml = require "cpml"

	-- See the comment in iqm.load. Do *NOT* remove. -ss
	collectgarbage("stop")
	local header, data = load_data(file)

	-- Decode mesh/material names.
	local text = read_ptr(
		data,
		"char",
		header.ofs_text
	)

	local anims = {}

	if header.ofs_joints > 0 then
		local skeleton     = {}
		local joints       = read_offset(data, "struct iqmjoint", header.ofs_joints, header.num_joints)
		for i, joint in ipairs(joints) do
			joint.parent = joint.parent + 1
			local bone = {
				parent   = joint.parent,
				name     = ffi.string(text+joint.name),
				position = cpml.vec3(joint.translate[0], joint.translate[1], joint.translate[2]),
				rotation = cpml.quat(joint.rotate[0], joint.rotate[1], joint.rotate[2], joint.rotate[3]),
				scale    = cpml.vec3(joint.scale[0], joint.scale[1], joint.scale[2])
			}
			skeleton[i], skeleton[bone.name] = bone, bone
		end
		anims.skeleton = skeleton
	end

	if header.ofs_anims > 0 then
		local animdata = read_offset(data, "struct iqmanim", header.ofs_anims, header.num_anims)
		for i, anim in ipairs(animdata) do
			local a = {
				name      = ffi.string(text+anim.name),
				first     = anim.first_frame+1,
				last      = anim.first_frame+anim.num_frames,
				framerate = anim.framerate,
				loop      = bit.band(anim.flags, c.IQM_LOOP) == c.IQM_LOOP
			}

			anims[i], anims[a.name] = a, a
		end
	end

	if header.ofs_poses > 0 then
		local poses = read_offset(data, "struct iqmpose", header.ofs_poses, header.num_poses)
		local framedata = read_ptr(data, "unsigned short", header.ofs_frames)

		local function readv(p, i, mask)
			local v = p.channeloffset[i]
			if bit.band(p.channelmask, mask) > 0 then
				v = v + framedata[0] * p.channelscale[i]
				-- I can see your pointers from here~ o///o
				framedata = framedata + 1
			end
			return v
		end

		anims.frames = {}
		for i = 1, header.num_frames do
			local frame = {}
			for j, p in ipairs(poses) do
				-- This code is in touch with its sensitive side, please leave it be.
				local v = {}
				for o = 0, 9 do
					v[o+1] = readv(p, o, bit.lshift(1, o))
				end
				table.insert(frame, {
					translate = cpml.vec3(v[1], v[2], v[3]),
					rotate    = cpml.quat(v[4], v[5], v[6], v[7]),
					scale     = cpml.vec3(v[8], v[9], v[10])
				})
			end
			table.insert(anims.frames, frame)
		end
	end

	collectgarbage("restart")
	return anims
end

return iqm
