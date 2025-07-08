project "EpochAssets"
    kind "StaticLib"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	apply_simd_flags()
	
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
		"%{wks.location}/vendor/spdlog/include",
		"%{wks.location}/vendor/tracy/tracy",
		"%{wks.location}/vendor/yaml-cpp/include",
		"%{wks.location}/vendor/assimp/include",
		"%{wks.location}/CommonUtilities/src",
		"%{wks.location}/Epoch/DataTypes/src",
		"%{wks.location}/Epoch/Core/src",
		"%{wks.location}/Epoch/Serialization/src",
    }

    links
	{
        "EpochCore",
		"EpochSerialization",
    }

	filter "configurations:Debug"
		links
		{
			"assimp-vc143-mtd"
		}
		libdirs
		{
			"%{wks.location}/vendor/assimp/bin/Debug"
		}

	filter "configurations:Release or configurations:Dist"
		links
		{
			"assimp-vc143-mt"
		}
		libdirs
		{
			"%{wks.location}/vendor/assimp/bin/Release"
		}
