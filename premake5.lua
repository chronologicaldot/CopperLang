-- Copper premake script file by Nicolaus Anderson
-- Copyright 2017, Nicolaus Anderson

-- Usage: premake5 --file=premake5.lua [gmake, vs2015, ..]
-- The make config=[some configuration] requires that the configuration be in lowercase letters.

workspace "Copper"
	configurations { "ConsoleApp", "DebugAll", "DebugFe" }
	location "build"

	filter { "configurations:ConsoleApp" }
		targetdir "bin/ConsoleApp"
		optimize "On"

	filter { "configurations:DebugAll or DebugFe" }
		targetdir "bin/debug"
--		optimize "Debug"

	filter { "configurations:DebugAll" }
		warnings "Extra"

	filter { "configurations:DebugAll", "action:gmake" }
		buildoptions " -g"

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
		"Copper/src/*.h", "Copper/src/*.cpp",
		"Copper/stdlib/*.h", "Copper/stdlib/*.cpp",
		--"exts/**.*"
		"exts/Math/basicmath.h", "exts/Math/cu_basicmath.cpp",
		"exts/Time/systime.h", "exts/Time/cu_systime.cpp",
		"exts/String/cu_stringmap.h", "exts/String/cu_stringmap.cpp",
		"exts/String/cu_stringbasics.h", "exts/String/cu_stringbasics.cpp",
		"exts/System/cu_info.h", "exts/System/cu_info.cpp",
	}

	excludes
	{
		"debug/**.*",
		"exts/excludes/**.*"
	}

	filter { "system:linux or bsd or hurd" }
		linkoptions { " -rdynamic " }

