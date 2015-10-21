-- http://industriousone.com/scripting-reference

local action = _ACTION or ""

solution "assimp2gltf"
    location ("_project")
    configurations { "Debug", "Release" }
    platforms {"x64", "x32"}
    language "C++"
    targetdir ("bin")
    kind "ConsoleApp"

    configuration "vs*"
        defines { "_CRT_SECURE_NO_WARNINGS" }

    configuration "Debug"
        targetdir ("bin")
        defines { "DEBUG" }
        flags { "Symbols"}
        targetsuffix "-d"

    configuration "Release"
        defines { "NDEBUG" }
        flags { "Optimize"}

    project "assimp2gltf"
        includedirs {
            "3rdparty/assimp/include",
            "3rdparty/assimp/code/BoostWorkaround",
            "3rdparty/assimp/contrib/irrXML",
            "3rdparty/assimp/contrib/zlib",
            "3rdparty/assimp/contrib/openddlparser/include",
            "3rdparty/assimp/contrib/clipper",
        }
        files { 
            "assimp2gltf/*",
            "3rdparty/assimp/code/*.h",
            "3rdparty/assimp/code/*.cpp",
            "3rdparty/assimp/contrib/irrXML/*.cpp",
            "3rdparty/assimp/contrib/zlib/*.c",
            "3rdparty/assimp/contrib/clipper/*",
            "3rdparty/assimp/contrib/openddlparser/code/*",
            "3rdparty/assimp/contrib/openddlparser/include/*",
        }

        defines {
            "OPENDDLPARSER_BUILD",
            "ASSIMP_BUILD_NO_C4D_IMPORTER",
        }
