solution "gm_roc2"
	language "C++"
	location "./proj"

	architecture "x86"

	configurations "Release"

	configuration "Release"
		defines {"NDEBUG"}
		optimize "On"

	project "gmsv_roc2_win32"
		flags "StaticRuntime"
		kind "SharedLib"

		targetdir "bin"

		buildoptions {"/Os", "/MP", "/arch:SSE2"}

		includedirs {
			"include",
			"src"
		}

		files {
			"src/**.cpp",
			"src/**.hpp",
			"src/**.h",
		}

		defines {
			"GMMODULE",
			"WIN32",
			"_WINDOWS",
			"_USRDLL"
		}
