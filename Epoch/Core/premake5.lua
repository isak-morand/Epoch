project "EpochCore"
    kind "StaticLib"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	apply_simd_flags()
	
	pchheader "epch.h"
	pchsource "src/epch.cpp"

	defines
	{
		"TRACY_ENABLE",
		"TRACY_ON_DEMAND",
		"TRACY_CALLSTACK=10"
	}

    files
	{
		"src/**.h",
		"src/**.hpp",
		"src/**.cpp"
	}

    includedirs
	{
        "src",
		"%{wks.location}/CommonUtilities/src",
		"%{wks.location}/vendor/spdlog/include",
		"%{wks.location}/vendor/GLFW/include",
		"%{wks.location}/vendor/tracy/tracy",
		"%{wks.location}/vendor/NFD-Extended/include",
    }

    links
	{
        "CommonUtilities",
		"GLFW",
		"Tracy",
		"NFD-Extended",
    }

	filter "system:windows"
   		defines { "WIN32_LEAN_AND_MEAN", "GLFW_INCLUDE_NONE" }
	filter ""
