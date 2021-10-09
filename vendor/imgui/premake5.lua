project "ImGui"
	kind "StaticLib"
	language "C++"

	targetdir "%{wks.location}/bin/%{cfg.buildcfg}/%{prj.name}"
	objdir "%{wks.location}/obj/%{cfg.buildcfg}/%{prj.name}"

	files
	{
		"*.cpp",
		"*.h",
		"backends/*.cpp",
		"backends/*.h"
	}

	includedirs
	{
		"backends",
		"%{wks.location}/vendor/glad/include",
		"%{wks.location}/vendor/SDL2/include/SDL2",
		"./"
	}
	defines
	{
		"IMGUI_IMPL_OPENGL_LOADER_GLAD"
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"