workspace "skygfx_vc"
	configurations { "Release", "DebugIII", "DebugVC" }
	location "build"

	files { "shaders/*.*" }
	files { "src/*.*" }

	includedirs { "shaders" }
	includedirs { "src" }
	includedirs { os.getenv("RWSDK34") }

	includedirs { "external/injector/include" }
	includedirs { "external/rwd3d9/source" }
	includedirs { "../rwd3d9/source" }
	libdirs { "external/rwd3d9/libs" }
	libdirs { "../rwd3d9/libs" }
	links { "rwd3d9.lib" }
   
	prebuildcommands {
		"for /R \"../shaders/ps/\" %%f in (*.hlsl) do \"%DXSDK_DIR%/Utilities/bin/x86/fxc.exe\" /T ps_2_0 /nologo /E main /Fh ../shaders/%%~nf.h %%f",
		"for /R \"../shaders/vs/\" %%f in (*.hlsl) do \"%DXSDK_DIR%/Utilities/bin/x86/fxc.exe\" /T vs_2_0 /nologo /E main /Fh ../shaders/%%~nf.h %%f",
	}
	  
project "skygfx_vc"
	kind "SharedLib"
	language "C++"
	targetname "skygfx"
	targetdir "bin/%{cfg.buildcfg}"
	targetextension ".dll"
	characterset ("MBCS")

	buildoptions { "/Zc:threadSafeInit-" }

	filter "configurations:DebugIII"
		defines { "DEBUG" }
		flags { "StaticRuntime" }
		symbols "On"
		debugdir "C:/Users/aap/games/gta3"
		debugcommand "C:/Users/aap/games/gta3/gta3.exe"
		postbuildcommands "copy /y \"$(TargetPath)\" \"C:\\Users\\aap\\games\\gta3\\plugins\\skygfx.dll\""

	filter "configurations:DebugVC"
		defines { "DEBUG" }
		flags { "StaticRuntime" }
		symbols "On"
		debugdir "C:/Users/aap/games/gtavc"
		debugcommand "C:/Users/aap/games/gtavc/gta_vc.exe"
		postbuildcommands "copy /y \"$(TargetPath)\" \"C:\\Users\\aap\\games\\gtavc\\plugins\\skygfx.dll\""

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		symbols "On"
		flags { "StaticRuntime" }
