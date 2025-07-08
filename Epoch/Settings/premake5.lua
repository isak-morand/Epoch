project "EpochSettings"
    kind "StaticLib"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

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
		"%{wks.location}/vendor/yaml-cpp/include",
		"%{wks.location}/Epoch/Core/src",
    }

    links
	{
        "EpochCore",

		"yaml-cpp",
    }
