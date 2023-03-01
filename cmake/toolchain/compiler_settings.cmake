#[[
   Copyright 2022 The Silkworm Authors
   Copyright 2023 Horizen Labs

   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
]]

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

    message(STATUS "MSVC_VERSION = ${MSVC_VERSION}")
    message(STATUS "MSVC_CXX_ARCHITECTURE_ID = ${MSVC_CXX_ARCHITECTURE_ID}")
    if(MSVC_VERSION LESS_EQUAL 1928)
        message(FATAL_ERROR "Your MSVC version is too low."
                "Please update your Visual Studio to at least release 2019 16.9.2")
    endif()


    add_definitions(-D_WIN32_WINNT=0x0A00)  # Min Windows 10
    add_definitions(-DVC_EXTRALEAN)         # Process windows headers faster ...
    add_definitions(-DWIN32_LEAN_AND_MEAN)  # ... and prevent winsock mismatch with Boost's
    add_definitions(-DNOMINMAX)             # Prevent MSVC to tamper with std::min/std::max
    add_definitions(-DPSAPI_VERSION=2)      # For process info

    # LINK : fatal error LNK1104: cannot open file 'libboost_date_time-vc142-mt-x64-1_72.lib
    # is solved by this (issue only for MVC)
    add_definitions(-DBOOST_DATE_TIME_NO_LIB)

    # Abseil triggers some deprecation warnings
    add_compile_definitions(_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING)
    add_compile_definitions(_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING)
    add_compile_definitions(_SILENCE_CXX20_IS_POD_DEPRECATION_WARNING)
    add_compile_definitions(_SILENCE_CXX20_CISO646_REMOVED_WARNING)

    add_compile_options(/MP)            # Enable parallel compilation
    add_compile_options(/EHa)           # Enable standard C++ unwinding
    add_compile_options(/await:strict)  # Enable coroutine support in std namespace

    #[[
    There is an issue on CLion IDE when toolchain is MSVC. Basically it wrongly parses file(line,column) which
    are meant to point to an error or a warning. Adding the following compile option works around the problem
    but still has to be considered a temporary solution.
    https://youtrack.jetbrains.com/issue/CPP-20259?_ga=2.92522975.312527487.1632161219-1027977455.1629393843&_gac=1.251211380.1631446966.CjwKCAjwyvaJBhBpEiwA8d38vIMQB8b0QfoFeQR5Mf4LHU50RFx3CWeeNzVeCrDOr1QcnfCpUPbFTBoCLEYQAvD_BwE
    ]]
    #add_compile_options(/diagnostics:classic)

    add_compile_options(/wd4127) # Silence warnings about "conditional expression is constant" (abseil mainly)
    add_compile_options(/wd5030) # Silence warnings about GNU attributes
    add_compile_options(/wd4324) # Silence warning C4324: 'xxx': structure was padded due to alignment specifier
    add_compile_options(/wd4068) # Silence warning C4068: unknown pragma
    add_compile_options(/wd5030) # Silence warning C5030: unknown gnu/clang attribute
    add_compile_options(/W4)     # Display all other un-silenced warnings
    add_compile_options(/bigobj) # Increase .obj sections, needed for hard coded pre-verified hashes

    # Required for proper detection of __cplusplus
    # see https://docs.microsoft.com/en-us/cpp/build/reference/zc-cplusplus?view=msvc-160
    add_compile_options(/Zc:__cplusplus)

    add_link_options(/ignore:4099)

    if(CMAKE_BUILD_TYPE MATCHES "Release")
        add_compile_options(/GL)                                                  # Enable LTCG for faster builds
        set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /LTCG")       # Enable LTCG for faster builds
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")             # Enable LTCG for faster builds
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /OPT:REF /OPT:ICF") # Enable unused references removal
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /RELEASE")          # Enable RELEASE so that the executable file has its checksum set
    endif()

    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /VERBOSE /TIME")    # Debug linker
    endif()

elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")

    message(STATUS "${CMAKE_CXX_COMPILER_ID} version ${CMAKE_CXX_COMPILER_VERSION}")

    # Require at least GCC 11
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11.0)
        message(FATAL_ERROR "GCC Version must be at least 13")
    endif()

    if(ZEN_SANITIZE)
        add_compile_options(-fno-omit-frame-pointer -fsanitize=${ZEN_SANITIZE} -DZEN_SANITIZE)
        add_link_options(-fno-omit-frame-pointer -fsanitize=${ZEN_SANITIZE} -DZEN_SANITIZE)
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-g1)
    endif()

elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES ".*Clang$")

    message(STATUS "${CMAKE_CXX_COMPILER_ID} version ${CMAKE_CXX_COMPILER_VERSION}")

    # Require at least Clang 13
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13.0)
        message(FATAL_ERROR "Clang Version must be at least 13")
    endif()

    if(ZEN_CLANG_COVERAGE)
        add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
        add_link_options(-fprofile-instr-generate -fcoverage-mapping)
    endif()

    if(ZEN_SANITIZE)
        add_compile_options(-fno-omit-frame-pointer -fsanitize=${ZEN_SANITIZE} -DZEN_SANITIZE)
        add_link_options(-fno-omit-frame-pointer -fsanitize=${ZEN_SANITIZE} -DZEN_SANITIZE)
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-gline-tables-only)
    endif()

    # coroutines support
    add_compile_options(-stdlib=libc++)
    link_libraries(c++)
    link_libraries(c++abi)

else ()

    message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID} has not been tested or supported."
                        "Please amend compiler_settings.cmake to proceed with build"
                        "or notify the issue on the GitHub repository you've cloned"
                        "this project from")

endif()
