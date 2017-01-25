-- Copper premake script file by Nicolaus Anderson
-- Copyright 2017, Nicolaus Anderson

-- Usage: premake5 --file=premake5.lua [gmake, vs2015, ..]
-- The make config=[some configuration] requires that the configuration be in lowercase letters.

workspace "Copper"
	configurations { "ConsoleApp", "DebugAll", "DebugFe" }
	location "build"

	filter { "configurations:ConsoleApp" }
		targetdir "bin/ConsoleApp"
		optimize "Off"

	filter { "configurations:DebugAll or DebugFe" }
		targetdir "bin/debug"
--		optimize "Debug"

	filter { "configurations:DebugAll" }
		warnings "Extra"

	filter { "configurations:DebugAll", "action:gmake" }
		buildoptions " -g "

	filter { "configurations:DebugFe" }
		flags { "FatalWarnings" }

	filter { "configurations:DebugFe", "action:gmake" }
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
		"debug/**.*",
	}

	filter { "system:linux or bsd or hurd" }
		linkoptions { " -rdynamic " }

	filter { "configurations:DebugAll" }
		includedirs { "debug" }
		-- Um... I need to just append files somehow
		files { "debug/driver.cpp" }
		excludes { "console.cpp" }

	filter { "configurations:DebugFe" }
		includedirs { "debug" }
		-- Um... I need to just append files somehow
		files { "debug/driver.cpp" }
		excludes { "console.cpp" }
