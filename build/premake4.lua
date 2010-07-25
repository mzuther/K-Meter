--------------------------------------------------------------------------------
--
--  K-Meter
--  =======
--  Implementation of a K-System meter according to Bob Katz' specifications
--
--  Copyright (c) 2010 Martin Zuther (http://www.mzuther.de/)
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
	print "=== Generating project files (Visual C++) ==="
else
	print "Action not specified\n"
end

solution "kmeter"
	language "C++"
	configurations { "Debug", "Release" }
	targetdir "../bin"

	files {
		"../src/**.h",
		"../src/**.cpp"
	}

	includedirs {
		"../src",
		"../src/juce_library_code/",
		"../libraries/"
	}

	libdirs {
		"../build",
		"../libraries/juce/bin",
		"../libraries/fftw3/bin"
	}

	configuration { "Debug*" }
		defines { "_DEBUG=1", "DEBUG=1" }
		flags { "Symbols", "ExtraWarnings" }
		buildoptions { "-fno-inline" }

	configuration { "Release*" }
		defines { "NDEBUG" }
		flags { "OptimizeSpeed", "NoFramePointer", "ExtraWarnings" }
		buildoptions { "-pipe", "-fvisibility=hidden" }

--------------------------------------------------------------------------------

	project (os.get() .. "_standalone")
		kind "WindowedApp"
		location (os.get() .. "/standalone")
		targetprefix ""

		platforms { "x32" }

		defines {
			"KMETER_STAND_ALONE=1",
			"JUCETICE_USE_AMALGAMA=1",
			"JUCE_USE_VSTSDK_2_4=0"
		}

		files {
			"../libraries/juce/extras/audio plugins/wrapper/Standalone/*.h",
			"../libraries/juce/extras/audio plugins/wrapper/Standalone/*.cpp"
		}

		configuration {"linux"}
			defines {
				"LINUX=1",
				"JUCE_USE_XSHM=1",
				"JUCE_ALSA=1",
				"JUCE_JACK=1"
			}

			links {
				"fftw3f",
				"freetype",
				"pthread",
				"rt",
				"X11",
				"Xext",
				"asound"
			}

			includedirs {
				"/usr/include",
				"/usr/include/freetype2"
			}
			
			libdirs {
				"/usr/X11R6/lib32/"
			}

		configuration {"windows"}
			defines {
				"WIN32=1",
				"JUCE_USE_XSHM=0",
				"JUCE_ALSA=0",
				"JUCE_JACK=0"
			}

		configuration "Debug"
			targetname "kmeter_debug"
			objdir ("../bin/intermediate_" .. os.get() .. "/standalone_debug")
			links { "juce_debug32" }

      configuration "Release"
			targetname "kmeter"
			objdir ("../bin/intermediate_" .. os.get() .. "/standalone_release")
			links { "juce32" }

--------------------------------------------------------------------------------

	project (os.get() .. "_vst")
		kind "SharedLib"
		location (os.get() .. "/vst")
		targetprefix ""

		platforms { "x32" }

		defines {
			"KMETER_VST_PLUGIN=1",
			"JUCETICE_USE_AMALGAMA=1",
			"JUCE_USE_VSTSDK_2_4=1"
		}

		excludes {
			"../src/standalone_application.h",
			"../src/standalone_application.cpp"
		}

		configuration {"linux"}
			defines {
				"LINUX=1",
				"JUCE_USE_XSHM=1",
				"JUCE_ALSA=0",
				"JUCE_JACK=0"
			}

			includedirs {
				"/usr/include",
				"/usr/include/freetype2",
				"../libraries/vstsdk2.4"
			}

			libdirs {
				"/usr/X11R6/lib32/"
			}

			links {
				"fftw3f",
				"freetype",
				"pthread",
				"rt",
				"X11",
				"Xext"
			}

		configuration {"windows"}
			defines {
				"WIN32=1",
				"JUCE_USE_XSHM=0",
				"JUCE_ALSA=0",
				"JUCE_JACK=0"
			}

		configuration "Debug"
			targetname "kmeter_vst_debug"
			objdir ("../bin/intermediate_" .. os.get() .. "/vst_debug")
			links { "juce_debug32" }

      configuration "Release"
			targetname "kmeter_vst"
			objdir ("../bin/intermediate_" .. os.get() .. "/vst_release")
			links { "juce32" }
