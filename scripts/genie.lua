local BASE_DIR   = path.getabsolute("..")
local EXTERN_DIR = path.join(BASE_DIR, "extern")
local OVR_DIR    = path.join(EXTERN_DIR, "LibOVR")
local use_ovr    = false

solution "lua-bgfx" do
	uuid "8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942"
	configurations { "Debug", "Release" }
	platforms { "Native", "x32", "x64" }

	startproject "lua-bgfx"

	targetdir(path.join(BASE_DIR, "bin"))
	objdir(path.join(BASE_DIR, "obj"))

	configuration { "Debug" }
	flags { "Symbols" }

	configuration {}
end

if os.get() == "windows" then
	SDL_DIR = path.join(EXTERN_DIR, "sdl")
	project "SDL2" do
		uuid "3ABE8B7C-26F5-8C0D-CFE1-7210BBF7080F"
		targetname "SDL2"
		kind "SharedLib"
		language "C++"

		local headers = os.matchfiles(path.join(SDL_DIR, "include") .. "/*.h")
		local dir = path.join(BASE_DIR, "scripts/include")
		os.mkdir(dir)
		os.mkdir(path.join(dir, "SDL2"))
		for _, header in pairs(headers) do
			local file = path.getname(header)
			local folder = path.join(dir, "SDL2")
			local path = path.join(folder, file)
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
	uuid "EC77827C-D8AE-830D-819B-69106DB1FF0E"
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
		path.join(BGFX_SRC_DIR, "amalgamated.cpp"),

	}
	configuration {"vs*"}
	if use_ovr and os.isdir(OVR_DIR) then
		defines {
			"BGFX_CONFIG_USE_OVR=1",
			"BGFX_CONFIG_MULTITHREADED=0"
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
		"-std=c++11",
		"-fno-strict-aliasing",
		"-fpic",
		"-mstackrealign"
	}
end

project "lua-bgfx" do
	uuid "BB11813B-A7DE-DB46-D0F7-C9EEBC2311D5"
	targetprefix ""
	targetname "bgfx"
	kind "SharedLib"
	language "C++"

	files {
		path.join(BASE_DIR, "wrap_bgfx.cpp")
	}

	links {
		"BGFX"
	}

	includedirs {
		path.join(EXTERN_DIR, "bgfx/include"),
		path.join(EXTERN_DIR, "bx/include")
	}

	configuration {"vs*"}
	defines { "_CRT_SECURE_NO_WARNINGS" }
	includedirs {
		path.join(EXTERN_DIR, "luajit/src"),
		path.join(BASE_DIR, "scripts/include"),
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
	if use_ovr and os.isdir(OVR_DIR) then
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
