local EXTERN_DIR = "extern"
local OVR_DIR = path.join(EXTERN_DIR, "LibOVR")

solution "lua-bgfx" do
	configurations { "Debug", "Release" }
	platforms { "Native", "x32", "x64" }

	startproject "lua-bgfx"

	targetdir "bin"
	objdir "obj"

	configuration { "Debug" }
	flags { "Symbols" }

	configuration {}
end

if os.get() == "windows" then
	SDL_DIR = path.join(EXTERN_DIR, "sdl")
	project "SDL2" do
		targetname "SDL2"
		kind "SharedLib"
		language "C++"

		local headers = os.matchfiles(path.join(SDL_DIR, "include") .. "/*.h")
		os.mkdir("include")
		os.mkdir("include/SDL2")
		for _, header in pairs(headers) do
			local file = path.getname(header)
			local path = path.join("include/SDL2", file)
			os.copyfile(header, path)
		end

		local SDL_SRC = path.join(SDL_DIR, "src")

		includedirs {
			path.join(SDL_DIR, "include")
		}

		-- common files
		files {
			path.join(SDL_SRC, "*.c"),
			path.join(SDL_SRC, "atomic/*.c"),
			path.join(SDL_SRC, "audio/*.c"),
			path.join(SDL_SRC, "audio/dummy/*.c"),
			path.join(SDL_SRC, "audio/disk/*.c"),
			path.join(SDL_SRC, "core/*.c"),
			path.join(SDL_SRC, "cpuinfo/*.c"),
			path.join(SDL_SRC, "dynapi/*.c"),
			path.join(SDL_SRC, "events/*.c"),
			path.join(SDL_SRC, "file/*.c"),
			path.join(SDL_SRC, "filesystem/dummy/*.c"),
			path.join(SDL_SRC, "haptic/*.c"),
			path.join(SDL_SRC, "haptic/dummy/*.c"),
			path.join(SDL_SRC, "joystick/*.c"),
			path.join(SDL_SRC, "joystick/dummy/*.c"),
			path.join(SDL_SRC, "libm/*.c"),
			path.join(SDL_SRC, "loadso/*.c"),
			path.join(SDL_SRC, "main/*.c"),
			path.join(SDL_SRC, "power/*.c"),
			path.join(SDL_SRC, "render/*.c"),
			path.join(SDL_SRC, "stdlib/*.c"),
			path.join(SDL_SRC, "thread/*.c"),
			path.join(SDL_SRC, "timer/*.c"),
			path.join(SDL_SRC, "timer/dummy/*.c"),
			path.join(SDL_SRC, "video/*.c"),
			path.join(SDL_SRC, "video/dummy/*.c")
		}
		configuration { "windows", "vs*" }
		files {
			path.join(SDL_SRC, "audio/directsound/*.c"),
			-- this is, apparently, possible.
			path.join(SDL_SRC, "audio/pulseaudio/*.c"),
			path.join(SDL_SRC, "audio/xaudio2/*.c"),
			path.join(SDL_SRC, "audio/winmm/*.c"),
			path.join(SDL_SRC, "core/windows/*.c"),
			path.join(SDL_SRC, "filesystem/windows/*.c"),
			path.join(SDL_SRC, "haptic/windows/*.c"),
			path.join(SDL_SRC, "joystick/windows/*.c"),
			path.join(SDL_SRC, "loadso/windows/*.c"),
			path.join(SDL_SRC, "power/windows/*.c"),
			path.join(SDL_SRC, "render/direct3d/*.c"),
			path.join(SDL_SRC, "render/direct3d11/*.c"),
			path.join(SDL_SRC, "render/opengl/*.c"),
			path.join(SDL_SRC, "render/opengles/*.c"),
			path.join(SDL_SRC, "render/opengles2/*.c"),
			path.join(SDL_SRC, "render/software/*.c"),
			path.join(SDL_SRC, "thread/generic/SDL_syscond.c"),
			path.join(SDL_SRC, "thread/windows/*.c"),
			path.join(SDL_SRC, "timer/windows/*.c"),
			path.join(SDL_SRC, "video/windows/*.c")
		}
		links {
			"version",
			"imm32",
			"dxguid",
			"xinput",
			"winmm"
		}
	end
end

local function link_ovr()
	-- 32-bit
	configuration {"vs2013"}
	libdirs { path.join(OVR_DIR, "Lib/Windows/Win32/Release/VS2013") }
	configuration {"vs2015"}
	libdirs { path.join(OVR_DIR, "Lib/Windows/Win32/Release/VS2015") }

	-- 64-bit
	-- configuration {"vs2013", "x64"}
	-- libdirs { path.join(OVR_DIR, "Lib/Windows/x64/Release/VS2013") }
	-- configuration {"vs2015", "x64"}
	-- libdirs { path.join(OVR_DIR, "Lib/Windows/x64/Release/VS2015") }

	configuration {"vs*"}
	includedirs { path.join(OVR_DIR, "Include") }
	links { "LibOVR" }
end

project "BGFX" do
	targetname "bgfx_s"
	kind "StaticLib"
	language "C++"
	local BX_DIR = path.join(EXTERN_DIR, "bx/include")
	local BGFX_DIR = path.join(EXTERN_DIR, "bgfx")
	local BGFX_SRC_DIR = path.join(BGFX_DIR, "src")
	includedirs {
		path.join(BGFX_DIR, "include"),
		path.join(BGFX_DIR, "3rdparty/khronos"),
		path.join(BGFX_DIR, "3rdparty"),
		BGFX_SRC_DIR,
		BX_DIR
	}
	files {
		path.join(BGFX_SRC_DIR, "bgfx.cpp"),
		path.join(BGFX_SRC_DIR, "debug_renderdoc.cpp"),
		path.join(BGFX_SRC_DIR, "glcontext_egl.cpp"),
		path.join(BGFX_SRC_DIR, "glcontext_glx.cpp"),
		path.join(BGFX_SRC_DIR, "glcontext_ppapi.cpp"),
		path.join(BGFX_SRC_DIR, "glcontext_wgl.cpp"),
		path.join(BGFX_SRC_DIR, "hmd_openvr.cpp"),
		path.join(BGFX_SRC_DIR, "hmd_ovr.cpp"),
		path.join(BGFX_SRC_DIR, "image.cpp"),
		path.join(BGFX_SRC_DIR, "renderer_d3d11.cpp"),
		path.join(BGFX_SRC_DIR, "renderer_d3d12.cpp"),
		path.join(BGFX_SRC_DIR, "renderer_d3d9.cpp"),
		path.join(BGFX_SRC_DIR, "renderer_gl.cpp"),
		path.join(BGFX_SRC_DIR, "renderer_null.cpp"),
		path.join(BGFX_SRC_DIR, "renderer_vk.cpp"),
		path.join(BGFX_SRC_DIR, "shader_dx9bc.cpp"),
		path.join(BGFX_SRC_DIR, "shader_dxbc.cpp"),
		path.join(BGFX_SRC_DIR, "shader_spirv.cpp"),
		path.join(BGFX_SRC_DIR, "shader.cpp"),
		path.join(BGFX_SRC_DIR, "topology.cpp"),
		path.join(BGFX_SRC_DIR, "vertexdecl.cpp")
	}
	defines {
		-- this was causing crashes on some systems.
		"BGFX_CONFIG_MULTITHREADED=0"
	}
	configuration {"vs*"}
	if os.isdir(OVR_DIR) then
		defines {
			"BGFX_CONFIG_USE_OVR=1",
		}
		link_ovr()
	end
	configuration {"vs*"}
	defines { "_CRT_SECURE_NO_WARNINGS" }
	links {
		"psapi"
	}
	includedirs {
		path.join(BGFX_DIR, "3rdparty/dxsdk/include"),
		path.join(BX_DIR, "compat/msvc")
	}
	configuration {"gmake"}
	buildoptions {
		"-fpic",
		"-mstackrealign"
	}
end

project "lua-bgfx" do
	targetprefix ""
	targetname "bgfx"
	kind "SharedLib"
	language "C++"

	files {
		"wrap_bgfx.cpp"
	}

	links {
		"BGFX"
	}

	includedirs {
		"extern/bgfx/include",
		"extern/bx/include"
	}

	configuration {"vs*"}
	defines { "_CRT_SECURE_NO_WARNINGS" }
	includedirs {
		path.join(EXTERN_DIR, "luajit/src"),
		"include",
		EXTERN_DIR
	}
	libdirs {
		path.join(EXTERN_DIR, "luajit/src"),
	}
	links {
		"version",
		"imm32",
		"dxguid",
		"xinput",
		"winmm",

		"SDL2",
		"lua51",
		"psapi"
	}
	if os.isdir(OVR_DIR) then
		link_ovr()
	end

	configuration {"linux"}
	includedirs {
		"/usr/include/luajit-2.0"
	}
	links {
		"luajit-5.1",
		"GL",
		"SDL2"
	}
	linkoptions {
		"-pthread"
	}

	configuration {"gmake"}
	buildoptions {
		"-std=c++11",
		"-fno-strict-aliasing",
		"-Wall",
		"-Wextra",
		"-mstackrealign"
	}
end
