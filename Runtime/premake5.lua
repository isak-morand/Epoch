project "EpochRuntime"
	kind "ConsoleApp"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

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
		"%{wks.location}/vendor",
		"%{wks.location}/CommonUtilities/src",
		"%{wks.location}/vendor/spdlog/include",
		"%{wks.location}/vendor/tracy/tracy",
		"%{wks.location}/Epoch/DataTypes/src",
		"%{wks.location}/Epoch/Core/src",
		"%{wks.location}/Epoch/Asset/src",
		"%{wks.location}/Epoch/Scene/src",
		"%{wks.location}/Epoch/Rendering/src",
		"%{wks.location}/Epoch/Engine/src"
    }

    links
	{
        "EpochEngine",
        "ImGui",
    }
	
	filter "configurations:Debug"
		postbuildcommands { "{COPYFILE} %{wks.location}vendor/assimp/bin/Debug/assimp-vc143-mtd.dll %{wks.location}bin/" .. outputdir .. "/%{prj.name}" }

	filter "configurations:Release or configurations:Dist"
		postbuildcommands { "{COPYFILE} %{wks.location}vendor/assimp/bin/Release/assimp-vc143-mt.dll %{wks.location}bin/" .. outputdir .. "/%{prj.name}" }

	