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
	-- libdirs {
	-- 	"vendor/SDL2/lib/x64/",
	-- 	"vendor/SDL2-img/lib/x64/"
	-- }
	files {
		"*.cpp", 
		"src/*.cpp", 
		"src/DGLCore/*.cpp",
		"src/Core/*.cpp",
	}
	links {
		"opengl32",
		"ImGui",
		"glad",
	}
	staticruntime "On"

filter "platforms:Win"
	postbuildcommands {
		{("{COPY} res/ %{cfg.buildtarget.directory}/res")}
	}

filter "configurations:Debug"
	symbols "On"
	defines {"DEBUG"}