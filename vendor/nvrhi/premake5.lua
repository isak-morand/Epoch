project "NVRHI-D3D11"
	kind "StaticLib"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/nvrhi/d3d11.h",

		"src/common/dxgi-format.h",
		"src/common/dxgi-format.cpp",

		"src/d3d11/**.h",
		"src/d3d11/**.cpp",
	}
	
	defines
	{
		"NVRHI_WITH_RTXMU=1",
		"NOMINMAX"
	}

	includedirs
	{
		"include",

		"rtxmu/include",

		"thirdparty/DirectX-Headers/include",
	}
	
    links { "NVRHI", "d3d11", "dxgi", "dxguid", "d3dcompiler" }

project "NVRHI-D3D12"
	kind "StaticLib"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/nvrhi/d3d12.h",

		"src/common/dxgi-format.h",
		"src/common/dxgi-format.cpp",
		"src/common/versioning.h",

		"src/d3d12/**.h",
		"src/d3d12/**.cpp",
	}
	
	defines
	{
		"NVRHI_WITH_RTXMU=1",
		"NOMINMAX"
	}

	includedirs
	{
		"include",

		"rtxmu/include",

		"thirdparty/DirectX-Headers/include",
	}
	
    links { "NVRHI", "d3d12", "dxgi", "dxguid", "d3dcompiler" }

project "NVRHI-Vulkan"
	kind "StaticLib"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/nvrhi/vulkan.h",
		"src/vulkan/**.h",
		"src/vulkan/**.cpp",
	}
	
	defines
	{
		"NVRHI_WITH_RTXMU=1",
		"VK_USE_PLATFORM_WIN32_KHR",
		"NOMINMAX"
	}

	--VULKAN_SDK = os.getenv("VULKAN_SDK")

	includedirs
	{
		"include",

		"rtxmu/include",

		--"%{VULKAN_SDK}/Include",
		"thirdparty/Vulkan-Headers/include",
	}
	
    links { "NVRHI" }

project "NVRHI"
	kind "StaticLib"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/nvrhi/nvrhi.h",
		"include/nvrhi/utils.h",

		"include/nvrhi/common/**.h",
		"src/common/**.cpp",

		"src/validation/**.h",
		"src/validation/**.cpp",

		"tools/nvrhi.natvis"
	}

	includedirs
	{
		"include",

		"rtxmu/include",
	}
	
	defines
	{
		"NVRHI_WITH_RTXMU=1",
		"NOMINMAX"
	}