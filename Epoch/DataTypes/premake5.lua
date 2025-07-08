project "EpochDataTypes"
    kind "StaticLib"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
	{
		"src/**.h",
		"src/**.hpp",
		"src/**.cpp",
	}

    includedirs
	{
        "src",
		"%{wks.location}/CommonUtilities/src",
		"%{wks.location}/Epoch/Core/src",
    }

    links
	{
        "CommonUtilities",
		"EpochCore"
    }
