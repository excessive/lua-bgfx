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

project "lua-bgfx" do
	targetname "lua-bgfx"
	kind "SharedLib"
	language "C++"

	files {
		"wrap_bgfx.cpp"
	}

	links {
		"luajit-5.1"
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
