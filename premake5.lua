workspace "Dove"
	platforms {"Win", "Linux"}
	configurations {"Debug", "Release"}
	architecture "x86_64"
	language "c++"
	cppdialect "c++17"

include "./vendor/glad"
include "./vendor/imgui"

project "DOON"
	kind "ConsoleApp"
	targetdir "%{wks.location}/bin/%{cfg.buildcfg}/%{prj.name}"
	objdir "%{wks.location}/obj/%{cfg.buildcfg}/%{prj.name}"

	includedirs {
		"src/",
		"vendor/",
		"vendor/glad/include",
		"vendor/SDL2/include",
		"vendor/SDL2-img/include",
	}
	libdirs {
		"vendor/SDL2/lib/x64/",
		"vendor/SDL2-img/lib/x64/"
	}
	files {
		"*.cpp", 
		"src/*.cpp", 
		"src/DGLCore/*.cpp",
	}
	links {
		"SDL2main", 
		"SDL2", 
		"SDL2_image", 
		"opengl32",
		"ImGui",
		"glad",
	}
	staticruntime "On"

filter "platforms:Win"
	postbuildcommands {
		{("{COPY} vendor/SDL2/lib/x64/*.dll %{cfg.buildtarget.directory}")},
		{("{COPY} vendor/SDL2-img/lib/x64/*.dll %{cfg.buildtarget.directory}")},
		{("{COPY} res/ %{cfg.buildtarget.directory}/res")}
	}

filter "configurations:Debug"
	symbols "On"
	defines {"DEBUG"}