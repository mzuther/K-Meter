--------------------------------------------------------------------------------
--
--  K-Meter
--  =======
--  Implementation of a K-System meter according to Bob Katz' specifications
--
--  Copyright (c) 2010-2012 Martin Zuther (http://www.mzuther.de/)
--
--  This program is free software: you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation, either version 3 of the License, or
--  (at your option) any later version.
--
--  This program is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with this program.  If not, see <http://www.gnu.org/licenses/>.
--
--  Thank you for using free software!
--
--------------------------------------------------------------------------------

if not _ACTION then
	-- prevent "attempt to ... (a nil value)" errors
elseif _ACTION == "gmake" then
	print ("=== Generating project files (GNU g++, " .. os.get():upper() .. ") ===")
elseif string.startswith(_ACTION, "vs") then
	print "=== Generating project files (Visual C++, WINDOWS) ==="
else
	print "Action not specified\n"
end

solution "kmeter"
	location ("windows/" .. _ACTION .. "/")
	language "C++"

	platforms { "x32" }

	configurations { "Debug", "Release" }

	files {
		"../src/**.h",
		"../src/**.cpp"
	}

	includedirs {
		"../src/juce_library_code/",
		"../libraries/"
	}

	targetdir "../bin/"

	flags {
			"EnableSSE",
			"EnableSSE2",
			"NoMinimalRebuild",
			"StaticRuntime",
			"Unicode",
			"WinMain"
	}

	configuration { "Debug*" }
		defines { "_DEBUG=1", "DEBUG=1", "JUCE_CHECK_MEMORY_LEAKS=1" }
		flags { "Symbols" }
		buildoptions { "" }

	configuration { "Release*" }
		defines { "NDEBUG=1", "JUCE_CHECK_MEMORY_LEAKS=0" }
		flags { "OptimizeSpeed", "NoFramePointer", "NoManifest" }
		buildoptions { "/Zi" }

	configuration { "Debug", "x32" }
		targetsuffix ", Debug)"

	configuration { "Release", "x32" }
		targetsuffix ")"

--------------------------------------------------------------------------------

	project ("Stand-alone (Stereo)")
		kind "WindowedApp"
		targetname "K-Meter (Stereo"
		targetprefix ""

		defines {
			"KMETER_STAND_ALONE=1",
			"KMETER_STEREO=1",
			"JUCETICE_USE_AMALGAMA=1",
			"JUCE_USE_VSTSDK_2_4=0"
		}

		includedirs {
			"../libraries/asiosdk2.2/common"
		}

		files {
			"../libraries/juce/src/audio/plugin_client/Standalone/*.h",
			"../libraries/juce/src/audio/plugin_client/Standalone/*.cpp"
		}

		configuration {"windows"}
			defines {
				"_WINDOWS=1",
				"_USE_MATH_DEFINES=1",
				"WIN32=1",
				"JUCE_USE_XSHM=0",
				"JUCE_ALSA=0",
				"JUCE_JACK=0",
				"JUCE_ASIO=1",
				"JUCE_DIRECTSOUND=1"
			}

			links {
				"kernel32",
				"user32",
				"gdi32",
				"winspool",
				"comdlg32",
				"advapi32",
				"shell32",
				"ole32",
				"oleaut32",
				"uuid",
				"odbc32",
				"odbccp32"
			 }

		configuration "Debug"
			objdir ("../bin/intermediate_" .. os.get() .. "/standalone_stereo_debug")

		configuration "Release"
			objdir ("../bin/intermediate_" .. os.get() .. "/standalone_stereo_release")

--------------------------------------------------------------------------------

	project ("Stand-alone (Surround)")
		kind "WindowedApp"
		targetname "K-Meter (Surround"
		targetprefix ""

		defines {
			"KMETER_STAND_ALONE=1",
			"KMETER_SURROUND=1",
			"JUCETICE_USE_AMALGAMA=1",
			"JUCE_USE_VSTSDK_2_4=0"
		}

		includedirs {
			"../libraries/asiosdk2.2/common"
		}

		files {
			"../libraries/juce/src/audio/plugin_client/Standalone/*.h",
			"../libraries/juce/src/audio/plugin_client/Standalone/*.cpp"
		}

		configuration {"windows"}
			defines {
				"_WINDOWS=1",
				"_USE_MATH_DEFINES=1",
				"WIN32=1",
				"JUCE_USE_XSHM=0",
				"JUCE_ALSA=0",
				"JUCE_JACK=0",
				"JUCE_ASIO=1",
				"JUCE_DIRECTSOUND=1"
			}

			links {
				"kernel32",
				"user32",
				"gdi32",
				"winspool",
				"comdlg32",
				"advapi32",
				"shell32",
				"ole32",
				"oleaut32",
				"uuid",
				"odbc32",
				"odbccp32"
			 }

		configuration "Debug"
			objdir ("../bin/intermediate_" .. os.get() .. "/standalone_surround_debug")

		configuration "Release"
			objdir ("../bin/intermediate_" .. os.get() .. "/standalone_surround_release")

--------------------------------------------------------------------------------

	project ("VST Plug-in (Stereo)")
		kind "SharedLib"
		targetname "K-Meter (Stereo"
		targetprefix ""

		defines {
			"KMETER_VST_PLUGIN=1",
			"KMETER_STEREO=1",
			"JUCETICE_USE_AMALGAMA=1",
			"JUCE_USE_VSTSDK_2_4=1"
		}

		includedirs {
			"../libraries/vstsdk2.4"
		}

		excludes {
			"../src/standalone_application.h",
			"../src/standalone_application.cpp"
		}

		configuration {"windows"}
			defines {
				"_WINDOWS=1",
				"_USE_MATH_DEFINES=1",
				"WIN32=1",
				"JUCE_USE_XSHM=0",
				"JUCE_ALSA=0",
				"JUCE_JACK=0",
				"JUCE_ASIO=0",
				"JUCE_DIRECTSOUND=0"
			}

			links {
				"kernel32",
				"user32",
				"gdi32",
				"winspool",
				"comdlg32",
				"advapi32",
				"shell32",
				"ole32",
				"oleaut32",
				"uuid",
				"odbc32",
				"odbccp32"
			 }

		configuration "Debug"
			objdir ("../bin/intermediate_" .. os.get() .. "/vst_stereo_debug")

		configuration "Release"
			objdir ("../bin/intermediate_" .. os.get() .. "/vst_stereo_release")

--------------------------------------------------------------------------------

	project ("VST Plug-in (Surround)")
		kind "SharedLib"
		targetname "K-Meter (Surround"
		targetprefix ""

		defines {
			"KMETER_VST_PLUGIN=1",
			"KMETER_SURROUND=1",
			"JUCETICE_USE_AMALGAMA=1",
			"JUCE_USE_VSTSDK_2_4=1"
		}

		includedirs {
			"../libraries/vstsdk2.4"
		}

		excludes {
			"../src/standalone_application.h",
			"../src/standalone_application.cpp"
		}

		configuration {"windows"}
			defines {
				"_WINDOWS=1",
				"_USE_MATH_DEFINES=1",
				"WIN32=1",
				"JUCE_USE_XSHM=0",
				"JUCE_ALSA=0",
				"JUCE_JACK=0",
				"JUCE_ASIO=0",
				"JUCE_DIRECTSOUND=0"
			}

			links {
				"kernel32",
				"user32",
				"gdi32",
				"winspool",
				"comdlg32",
				"advapi32",
				"shell32",
				"ole32",
				"oleaut32",
				"uuid",
				"odbc32",
				"odbccp32"
			 }

		configuration "Debug"
			objdir ("../bin/intermediate_" .. os.get() .. "/vst_surround_debug")

		configuration "Release"
			objdir ("../bin/intermediate_" .. os.get() .. "/vst_surround_release")
