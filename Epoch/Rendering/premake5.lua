project "EpochRendering"
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

	removefiles
	{
		"src/EpochRendering/Backends/**",
	}

	function IncludeBackend(name, nvrhiLib, define)
		files
		{
			"src/EpochRendering/Backends/" .. name .. "/**.h",
			"src/EpochRendering/Backends/" .. name .. "/**.hpp",
			"src/EpochRendering/Backends/" .. name .. "/**.cpp"
		}
		links { nvrhiLib }
		defines { define }
	end

    if renderapi == "dx11" then
		IncludeBackend("DX11", "NVRHI-D3D11", "USE_DX11")
    	includedirs { "%{wks.location}/vendor/NVRHI/thirdparty/DirectX-Headers/include" }
	elseif renderapi == "dx12" then
		IncludeBackend("DX12", "NVRHI-D3D12", "USE_DX12")
    	includedirs { "%{wks.location}/vendor/NVRHI/thirdparty/DirectX-Headers/include" }
	elseif renderapi == "vulkan" then
		IncludeBackend("Vulkan", "NVRHI-Vulkan", "USE_VULKAN")
    	includedirs { "%{wks.location}/vendor/NVRHI/thirdparty/Vulkan-Headers/include" }
	else
        --No render-api specified, defaulting to DX11
		IncludeBackend("DX11", "NVRHI-D3D11", "USE_DX11")
    	includedirs { "%{wks.location}/vendor/NVRHI/thirdparty/DirectX-Headers/include" }
	end

    includedirs
	{
        "src",
		"%{wks.location}/vendor",
		"%{wks.location}/CommonUtilities/src",
		"%{wks.location}/vendor/spdlog/include",
		"%{wks.location}/vendor/tracy/tracy",
		"%{wks.location}/vendor/NVRHI/include",
		"%{wks.location}/Epoch/DataTypes/src",
		"%{wks.location}/Epoch/Core/src",
		"%{wks.location}/Epoch/Assets/src",
		"%{wks.location}/Epoch/Scenes/src",
    }

    links
	{
        "EpochCore",
        "EpochAssets",
        "EpochScenes",

        "ImGui",
    }
