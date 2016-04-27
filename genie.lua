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

project "BGFX" do
	targetname "bgfx"
	if _OPTIONS["force-gl3"] then
		defines { "BGFX_CONFIG_RENDERER_OPENGL=33" }
	end
	kind "StaticLib"
	language "C++"
	local EXTERN_DIR = "."
	local OVR_DIR = "LibOVR"
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
		path.join(BGFX_SRC_DIR, "glcontext_egl.cpp"),
		path.join(BGFX_SRC_DIR, "glcontext_glx.cpp"),
		path.join(BGFX_SRC_DIR, "glcontext_ppapi.cpp"),
		path.join(BGFX_SRC_DIR, "glcontext_wgl.cpp"),
		path.join(BGFX_SRC_DIR, "image.cpp"),
		path.join(BGFX_SRC_DIR, "shader.cpp"),
		path.join(BGFX_SRC_DIR, "topology.cpp"),
		path.join(BGFX_SRC_DIR, "hmd_ovr.cpp"),
		path.join(BGFX_SRC_DIR, "hmd_openvr.cpp"),
		path.join(BGFX_SRC_DIR, "debug_renderdoc.cpp"),
		path.join(BGFX_SRC_DIR, "renderer_d3d9.cpp"),
		path.join(BGFX_SRC_DIR, "renderer_d3d11.cpp"),
		path.join(BGFX_SRC_DIR, "renderer_d3d12.cpp"),
		path.join(BGFX_SRC_DIR, "renderer_null.cpp"),
		path.join(BGFX_SRC_DIR, "renderer_gl.cpp"),
		path.join(BGFX_SRC_DIR, "renderer_vk.cpp"),
		path.join(BGFX_SRC_DIR, "shader_dxbc.cpp"),
		path.join(BGFX_SRC_DIR, "shader_dx9bc.cpp"),
		path.join(BGFX_SRC_DIR, "shader_spirv.cpp"),
		path.join(BGFX_SRC_DIR, "vertexdecl.cpp")
	}
	configuration {"vs*"}
	if os.isdir(OVR_DIR) then
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
		"-fpic"
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
		"luajit-5.1",
		"BGFX",
		"GL"
	}

	includedirs {
		"/usr/include/luajit-2.0",
		"bgfx/include"
	}

	configuration {"gmake"}
	buildoptions {
		"-std=c++11",
		"-fno-strict-aliasing",
		"-Wall",
		"-Wextra"
	}
end
