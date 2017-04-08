-- Copper premake script file by Nicolaus Anderson
-- Copyright 2017, Nicolaus Anderson

-- Usage: premake5 --file=premake5.lua [gmake, vs2015, ..]
-- The make config=[some configuration] requires that the configuration be in lowercase letters.

workspace "Copper"
	configurations { "consoleapp", "debugall", "debugfe" }
	location "build"

	filter { "configurations:consoleapp" }
		targetdir "bin/ConsoleApp"
		optimize "Off"

	filter { "configurations:debugall or debugfe" }
		targetdir "bin/debug"
--		optimize "Debug"

	filter { "configurations:debugall" }
		warnings "Extra"

	filter { "configurations:debugall", "action:gmake" }
		buildoptions " -Wall -g "

	filter { "configurations:debugfe" }
		flags { "FatalWarnings" }

	filter { "configurations:debugfe", "action:gmake" }
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

	filter { "configurations:debugall" }
		includedirs { "debug" }
		files { "debug/driver.cpp" }
		excludes { "console.cpp" }

	filter { "configurations:debugfe" }
		includedirs { "debug" }
		files { "debug/driver.cpp" }
		excludes { "console.cpp" }
