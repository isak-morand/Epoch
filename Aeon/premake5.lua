project "Aeon"
	kind "ConsoleApp"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"../CommonUtilities/src",
		"src",
		"../Epoch/src",
		"../Epoch/vendor",
		"../Epoch/vendor/GLFW/include",
		"../Epoch/vendor/filewatch/include",
		"../Epoch/vendor/yaml-cpp/include",
		"../Epoch/vendor/NFD-Extended/include",
		"../Epoch/vendor/magic_enum/include"
	}

	links
	{
		"Epoch"
	}

	filter "configurations:Debug"
		postbuildcommands { "xcopy \"$(SolutionDir)Epoch\\vendor\\mono\\bin\\Debug\\mono-2.0-sgen.dll\" \"$(SolutionDir)bin\\$(Configuration)-$(LlvmPlatformName)\\$(ProjectName)\" /e /y /i /r" }
		postbuildcommands { "xcopy \"$(SolutionDir)Epoch\\vendor\\assimp\\bin\\Debug\\assimp-vc143-mtd.dll\" \"$(SolutionDir)bin\\$(Configuration)-$(LlvmPlatformName)\\$(ProjectName)\" /e /y /i /r" }

	filter "configurations:Release or configurations:Dist"
		postbuildcommands { "xcopy \"$(SolutionDir)Epoch\\vendor\\mono\\bin\\Release\\mono-2.0-sgen.dll\" \"$(SolutionDir)bin\\$(Configuration)-$(LlvmPlatformName)\\$(ProjectName)\" /e /y /i /r" }
		postbuildcommands { "xcopy \"$(SolutionDir)Epoch\\vendor\\assimp\\bin\\Release\\assimp-vc143-mt.dll\" \"$(SolutionDir)bin\\$(Configuration)-$(LlvmPlatformName)\\$(ProjectName)\" /e /y /i /r" }

	filter "configurations:Dist"
		kind "WindowedApp"
