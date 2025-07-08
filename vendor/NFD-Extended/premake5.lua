project "NFD-Extended"
	kind "StaticLib"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/nfd.h",
		"include/nfd.hpp",

		"nfd_win.cpp"
	}

	includedirs
	{
		"include"
	}