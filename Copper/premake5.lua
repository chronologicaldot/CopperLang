-- Copper premake script file by Nicolaus Anderson
-- Copyright 2017, Nicolaus Anderson

-- Usage: premake5 --file=premake5.lua [gmake, vs2015, ..]

workspace "Copper"
	configurations { "ConsoleApp", "Debug_all", "Debug_fe" }
	location "build"

	filter { "configurations:ConsoleApp" }
		targetdir "bin/ConsoleApp"
		optimize "Off"

	filter { "configurations:Debug_all or Debug_fe" }
		targetdir "bin/debug"
--		optimize "Debug"

	filter { "configurations:Debug_all" }
		warnings "Extra"

	filter { "configurations:Debug_all", "action:gmake" }
		buildoptions " -g "

	filter { "configurations:Debug_fe" }
		flags { "FatalWarnings" }

	filter { "configurations:Debug_fe", "action:gmake" }
		buildoptions " -Wfatal-errors -g "

project "Copper"
	targetname	"copper"
	language	"C++"
	kind		"ConsoleApp"
	includedirs { "src", "stdlib" }

	files
	{
		"console.cpp",
		"src/*.h", "src/*.cpp",
		"stdlib/*.h", "stdlib/*.cpp",
	}

	excludes
	{
		"debug/RHHash_Driver.cpp"
	}

	filter { "system:linux or bsd or hurd" }
		linkoptions { " -rdynamic " }

	filter { "configurations:Debug_all" }
		includedirs { "debug" }
		-- Um... I need to just append files somehow
		files { "debug/driver.cpp" }
		excludes { "console.cpp" }

	filter { "configurations:Debug_fe" }
		includedirs { "debug" }
		-- Um... I need to just append files somehow
		files { "debug/driver.cpp" }
		excludes { "console.cpp" }
