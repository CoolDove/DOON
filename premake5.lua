require "export-compile-commands"

workspace "Dove"
	platforms {"Win"}
	configurations {"Debug", "Release"}
	architecture "x86_64"
	language "c++"
	cppdialect "c++17"
	characterset ("ASCII")

include "./vendor/glad"
include "./vendor/imgui"

project "DOON"
	kind "WindowedApp"
	targetdir "%{wks.location}/bin/%{cfg.buildcfg}/%{prj.name}"
	objdir "%{wks.location}/obj/%{cfg.buildcfg}/%{prj.name}"

	includedirs {
		"src/",
		"vendor/",
		"vendor/glad/include",
	}

	files {
		"src/*.h", 
		"src/*/*.h", 
		"src/*/*/*.h", 
		"vendor/stb_image/*.h",

		"src/*.cpp", 
		"src/*/*.cpp", 
		"src/*/*/*.cpp", 
		"vendor/stb_image/*.cpp",
	}

	links {
		"opengl32",
		"ImGui",
		"glad",
	}

	staticruntime "On"

	postbuildcommands {
		{("{COPY} res/ %{cfg.buildtarget.directory}/res")}
	}

filter "configurations:Debug"
	symbols "On"
	defines {"DEBUG"}