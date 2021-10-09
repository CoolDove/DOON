project "glad"
	kind "StaticLib"
	language "C"
	targetdir "%{wks.location}/bin/%{cfg.buildcfg}/%{prj.name}"
	objdir "%{wks.location}/obj/%{cfg.buildcfg}/%{prj.name}"

	staticruntime "on"
	includedirs {
		"include",
	}
	files { "src/glad.c", }

filter "configurations:Debug"
	symbols "On"
	defines {"DEBUG"}