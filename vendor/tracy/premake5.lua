project "Tracy"
	kind "StaticLib"

	disablewarnings { "4244" }

    linkoptions { "/ignore:4006" }

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    defines
	{
		"TRACY_ENABLE",
		"TRACY_ON_DEMAND",
		"TRACY_CALLSTACK=10"
	}

	files
	{
		"tracy/client/**.h",
		"tracy/client/**.hpp",
		"tracy/client/**.cpp",

		"tracy/common/**.h",
		"tracy/common/**.hpp",
		"tracy/common/**.cpp",

		"tracy/tracy/**.h",
		"tracy/tracy/**.hpp",
		"tracy/tracy/**.cpp",

		"tracy/libbacktrace/alloc.cpp",
		"tracy/libbacktrace/sort.cpp",
		"tracy/libbacktrace/state.cpp",
	}

	includedirs { "tracy/" }

	links
	{
		"DbgHelp",
		"ws2_32"
	}
