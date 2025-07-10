newoption
	{
		trigger = "target",
		description = "Target app",
		allowed =
		{
			{ "editor", "Editor" },
			{ "runtime", "Runtime" },
		}
	}
targetApp = _OPTIONS["target"] or "editor"

newoption
	{
		trigger = "render-api",
		description = "Choose rendering API",
		allowed =
		{
			{ "dx11", "DirectX 11" },
			{ "dx12", "DirectX 12" },
			{ "vulkan", "Vulkan" },
		}
	}
renderapi = _OPTIONS["render-api"] or "dx11"

function apply_simd_flags()
    filter { "configurations:Release or configurations:Dist" }
        vectorextensions "AVX2"
        isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }
    filter {}
end

workspace "Epoch"
	architecture "x64"
	
	if targetApp == "runtime" then
		startproject "EpochRuntime"
	else
		startproject "EpochEditor"
		defines { "WITH_EDITOR" }
	end

	language "C++"
	cppdialect "C++20"
	staticruntime "Off"

	configurations { "Debug", "Release", "Dist" }
	systemversion "latest"

	outputdir = "%{cfg.buildcfg}-%{cfg.architecture}-%{renderapi}"

	flags
	{
		"MultiProcessorCompile"
	}

	defines 
	{
		"_CRT_SECURE_NO_WARNINGS",
		"SPDLOG_USE_STD_FORMAT"
	}

	filter "system:windows"
		defines { "PLATFORM_WINDOWS", "NOMINMAX" }
		buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }
		
	filter "configurations:Debug"
		defines { "_DEBUG" }
		optimize "Off"
		symbols "on"
		
	filter "configurations:Release"
		defines { "_RELEASE", "NDEBUG" }
		optimize "On"
		symbols "default"
		
	filter "configurations:Dist"
		defines { "_DIST", "NDEBUG" }
		optimize "Full"
		symbols "off"
		
	filter {}

	group "Epoch"
		include "Epoch/DataTypes"
		include "Epoch/Core"
		include "Epoch/Assets"
		include "Epoch/Scenes"
		include "Epoch/Rendering"
		include "Epoch/Projects"
		include "Epoch/Engine"
		include "Epoch/Settings"
		include "Epoch/Serialization"
	group ""
	
	group "App"
		include "Editor"
		include "Runtime"
	group ""

	group "Dependencies"
		include "CommonUtilities"
		include "vendor/GLFW"
		include "vendor/Tracy"
		include "vendor/imgui"
		include "vendor/yaml-cpp"
		include "vendor/NFD-Extended"
	group ""
	
	group "Dependencies/NVRHI"
		include "vendor/nvrhi"
	group ""
