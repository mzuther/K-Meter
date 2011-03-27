--------------------------------------------------------------------------------
--
--  K-Meter
--  =======
--  Implementation of a K-System meter according to Bob Katz' specifications
--
--  Copyright (c) 2010-2011 Martin Zuther (http://www.mzuther.de/)
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
	language "C++"

	platforms { "x32", "x64" }

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

	configuration { "x32" }
		linkoptions {
			-- force static linking to FFTW
			"../../../libraries/fftw3/bin/i386/libfftw3f.a"
		}

	configuration { "x64" }
		linkoptions {
			-- force static linking to FFTW
			"../../../libraries/fftw3/bin/amd64/libfftw3f.a"
		}

	configuration { "Debug*" }
		defines { "_DEBUG=1", "DEBUG=1" }
		flags { "Symbols", "ExtraWarnings" }
		buildoptions { "-fno-inline" }

	configuration { "Release*" }
		defines { "NDEBUG" }
		flags { "OptimizeSpeed", "NoFramePointer", "ExtraWarnings" }
		buildoptions { "-pipe", "-fvisibility=hidden" }

	configuration { "Debug", "x32" }
		targetsuffix "_debug"

	configuration { "Debug", "x64" }
		targetsuffix "_debug_x64"

	configuration { "Release", "x32" }
		targetsuffix ""

	configuration { "Release", "x64" }
		targetsuffix "_x64"

--------------------------------------------------------------------------------

	project (os.get() .. "_standalone_stereo")
		kind "WindowedApp"
		location (os.get() .. "/standalone_stereo")
		targetname "kmeter_stereo"
		targetprefix ""

		defines {
			"KMETER_STAND_ALONE=1",
			"KMETER_STEREO=1",
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
				"JUCE_JACK=1",
				"JUCE_ASIO=0",
				"JUCE_DIRECTSOUND=0"
			}

			links {
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

		configuration "Debug"
			objdir ("../bin/intermediate_" .. os.get() .. "/standalone_stereo_debug")

		configuration "Release"
			objdir ("../bin/intermediate_" .. os.get() .. "/standalone_stereo_release")

--------------------------------------------------------------------------------

	project (os.get() .. "_standalone_surround")
		kind "WindowedApp"
		location (os.get() .. "/standalone_surround")
		targetname "kmeter_surround"
		targetprefix ""

		defines {
			"KMETER_STAND_ALONE=1",
			"KMETER_SURROUND=1",
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
				"JUCE_JACK=1",
				"JUCE_ASIO=0",
				"JUCE_DIRECTSOUND=0"
			}

			links {
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

		configuration "Debug"
			objdir ("../bin/intermediate_" .. os.get() .. "/standalone_surround_debug")

		configuration "Release"
			objdir ("../bin/intermediate_" .. os.get() .. "/standalone_surround_release")

--------------------------------------------------------------------------------

	project (os.get() .. "_vst_stereo")
		kind "SharedLib"
		location (os.get() .. "/vst_stereo")
		targetname "kmeter_stereo"
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

		configuration {"linux"}
			defines {
				"LINUX=1",
				"JUCE_USE_XSHM=1",
				"JUCE_ALSA=0",
				"JUCE_JACK=0",
				"JUCE_ASIO=0",
				"JUCE_DIRECTSOUND=0"
			}

			includedirs {
				"/usr/include",
				"/usr/include/freetype2"
			}

			links {
				"freetype",
				"pthread",
				"rt",
				"X11",
				"Xext"
			}

		configuration "Debug"
			objdir ("../bin/intermediate_" .. os.get() .. "/vst_stereo_debug")

		configuration "Release"
			objdir ("../bin/intermediate_" .. os.get() .. "/vst_stereo_release")

--------------------------------------------------------------------------------

	project (os.get() .. "_vst_surround")
		kind "SharedLib"
		location (os.get() .. "/vst_surround")
		targetname "kmeter_surround"
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

		configuration {"linux"}
			defines {
				"LINUX=1",
				"JUCE_USE_XSHM=1",
				"JUCE_ALSA=0",
				"JUCE_JACK=0",
				"JUCE_ASIO=0",
				"JUCE_DIRECTSOUND=0"
			}

			includedirs {
				"/usr/include",
				"/usr/include/freetype2"
			}

			links {
				"freetype",
				"pthread",
				"rt",
				"X11",
				"Xext"
			}

		configuration "Debug"
			objdir ("../bin/intermediate_" .. os.get() .. "/vst_surround_debug")

		configuration "Release"
			objdir ("../bin/intermediate_" .. os.get() .. "/vst_surround_release")
