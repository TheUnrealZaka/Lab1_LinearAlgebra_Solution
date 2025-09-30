newoption
{
    trigger = "sdl_backend",
    value = "BACKEND",
    description = "SDL backend to use",
    allowed = {
        { "auto", "Auto-detect best backend" },
        { "opengl", "OpenGL backend" },
        { "vulkan", "Vulkan backend" },
        { "d3d11", "Direct3D 11 backend" },
        { "d3d12", "Direct3D 12 backend" }
    },
    default = "auto"
}

function download_progress(total, current)
    local ratio = current / total
    ratio = math.min(math.max(ratio, 0), 1)
    local percent = math.floor(ratio * 100)
    print("Download progress (" .. percent .. "%/100%)")
end

function get_latest_sdl3_version()
    -- Fetch latest SDL3 release information
    local content, result = http.get("https://api.github.com/repos/libsdl-org/SDL/releases/latest")
    if result == "OK" then
        local tag = content:match('"tag_name"%s*:%s*"([^"]+)"')
        if tag then
            return tag:gsub("release%-", "")
        end
    end
    -- Fallback to hardcoded version if API fails
    print("Warning: Could not fetch latest SDL3 version, using fallback version 3.2.22")
    return "3.2.22"
end

function get_latest_imgui_version()
    -- Fetch latest ImGui release information
    local content, result = http.get("https://api.github.com/repos/ocornut/imgui/releases/latest")
    if result == "OK" then
        local tag = content:match('"tag_name"%s*:%s*"([^"]+)"')
        if tag then
            return tag
        end
    end
    -- Fallback to hardcoded version if API fails
    print("Warning: Could not fetch latest ImGui version, using fallback version v1.92.3")
    return "v1.92.3"
end

function get_latest_glew_version()
    -- Fetch latest GLEW release information
    local content, result = http.get("https://api.github.com/repos/nigels-com/glew/releases/latest")
    if result == "OK" then
        local tag = content:match('"tag_name"%s*:%s*"([^"]+)"')
        if tag then
            return tag
        end
    end
    -- Fallback to hardcoded version if API fails
    print("Warning: Could not fetch latest GLEW version, using fallback version glew-2.2.0")
    return "glew-2.2.0"
end

function check_sdl3()
    os.chdir("external")
    if(os.isdir("SDL3") == false) then
        local version = get_latest_sdl3_version()
        local filename = "SDL3-devel-" .. version .. "-VC.zip"
        local url = "https://github.com/libsdl-org/SDL/releases/download/release-" .. version .. "/" .. filename
        
        if(not os.isfile(filename)) then
            print("SDL3 not found, downloading version " .. version .. " from GitHub")
            local result_str, response_code = http.download(
                url, 
                filename, 
                {
                    progress = download_progress,
                    headers = { "From: Premake", "Referer: Premake" }
                }
            )
        end
        print("Unzipping SDL3 to " .. os.getcwd())
        zip.extract(filename, os.getcwd())
        -- Rename the extracted folder to SDL3 for consistency
        local extracted_folder = "SDL3-" .. version
        if os.isdir(extracted_folder) then
            os.rename(extracted_folder, "SDL3")
        end
        os.remove(filename)
    else
        print("SDL3 found in external/SDL3/")
    end
    os.chdir("../")
end

function check_imgui()
    os.chdir("external")
    if(os.isdir("ImGui") == false) then
        local version = get_latest_imgui_version()
        local filename = "imgui-" .. version .. ".zip"
        local url = "https://github.com/ocornut/imgui/archive/refs/tags/" .. version .. ".zip"
        
        if(not os.isfile(filename)) then
            print("ImGui not found, downloading version " .. version .. " from GitHub")
            local result_str, response_code = http.download(
                url,
                filename,
                {
                    progress = download_progress,
                    headers = { "From: Premake", "Referer: Premake" }
                }
            )
        end
        print("Unzipping ImGui to " .. os.getcwd())
        zip.extract(filename, os.getcwd())
        -- Rename the extracted folder to ImGui for consistency
        local extracted_folder = "imgui-" .. version:gsub("v", "")
        if os.isdir(extracted_folder) then
            os.rename(extracted_folder, "ImGui")
        end
        os.remove(filename)
    else
        print("ImGui found in external/ImGui/")
    end
    os.chdir("../")
end

function check_glew()
    os.chdir("external")
    if(os.isdir("glew") == false) then
        local version = get_latest_glew_version()
        local filename = version .. "-win32.zip"
        local url = "https://github.com/nigels-com/glew/releases/download/" .. version .. "/" .. filename
        
        if(not os.isfile(filename)) then
            print("GLEW not found, downloading version " .. version .. " from GitHub")
            local result_str, response_code = http.download(
                url,
                filename,
                {
                    progress = download_progress,
                    headers = { "From: Premake", "Referer: Premake" }
                }
            )
        end
        print("Unzipping GLEW to " .. os.getcwd())
        zip.extract(filename, os.getcwd())
        -- Rename and reorganize GLEW structure
        local extracted_folder = version
        if os.isdir(extracted_folder) then
            -- Simply rename the extracted folder to glew for consistency
            if os.isdir("glew") then
                os.rmdir("glew", true)
            end
            os.rename(extracted_folder, "glew")
        end
        os.remove(filename)
    else
        print("GLEW found in external/glew/")
    end
    os.chdir("../")
end

function build_externals()
    print("calling externals")
    check_sdl3()
    check_imgui()
    check_glew()
end

function platform_defines()
    filter {"configurations:Debug"}
        defines{"DEBUG", "_DEBUG"}
        symbols "On"

    filter {"configurations:Release"}
        defines{"NDEBUG", "RELEASE"}
        optimize "On"

    filter {"system:windows"}
        defines {"_WIN32", "_WINDOWS"}
        systemversion "latest"

    filter {"system:linux"}
        defines {"_GNU_SOURCE"}
        
    filter{}
end

-- Configuration
-- All external dependencies will be auto-downloaded if not present
downloadSDL3 = true
sdl3_dir = "external/SDL3"
imgui_dir = "external/ImGui"
glew_dir = "external/glew"

baseName = path.getbasename(path.getdirectory(os.getcwd()))
workspaceName = baseName

if (os.isdir('build_files') == false) then
    os.mkdir('build_files')
end

if (os.isdir('external') == false) then
    os.mkdir('external')
end

workspace (workspaceName)
    location "../"
    configurations { "Debug", "Release" }
    platforms { "x64", "x86" }

    defaultplatform ("x64")

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

    filter { "platforms:x64" }
        architecture "x86_64"

    filter { "platforms:x86" }
        architecture "x86"

    filter {}

    targetdir "bin/%{cfg.buildcfg}/"

if (downloadSDL3) then
    build_externals()
end

    startproject(workspaceName)

    -- Main project (combined app + bench + core library)
    project (workspaceName)
        kind "ConsoleApp"
        location "build_files/"
        targetdir "../bin/%{cfg.buildcfg}"

        filter "action:vs*"
            debugdir "$(SolutionDir)"

        filter{}

        vpaths 
        {
            ["Header Files/*"] = { "../include/**.h", "../include/**.hpp", "../src/**.h", "../src/**.hpp"},
            ["Source Files/*"] = {"../src/**.c", "../src/**.cpp"},
            ["App Files/*"] = {"../app/**.cpp"},
            ["Bench Files/*"] = {"../bench/**.cpp"},
            ["ImGui Files/*"] = {
                imgui_dir .. "/imgui*.cpp", 
                imgui_dir .. "/imgui*.h",
                imgui_dir .. "/backends/imgui_impl_sdl3.*",
                imgui_dir .. "/backends/imgui_impl_opengl3.*"
            },
        }
        
        files {
            "../src/**.c", 
            "../src/**.cpp", 
            "../src/**.h", 
            "../src/**.hpp", 
            "../include/**.h", 
            "../include/**.hpp",
            "../app/**.cpp",
            "../bench/**.cpp",
            -- ImGui core files
            imgui_dir .. "/imgui.cpp",
            imgui_dir .. "/imgui_demo.cpp",
            imgui_dir .. "/imgui_draw.cpp",
            imgui_dir .. "/imgui_tables.cpp",
            imgui_dir .. "/imgui_widgets.cpp",
            -- ImGui backend files for SDL3 and OpenGL3
            imgui_dir .. "/backends/imgui_impl_sdl3.cpp",
            imgui_dir .. "/backends/imgui_impl_opengl3.cpp",
            -- ImGui headers
            imgui_dir .. "/imgui*.h",
            imgui_dir .. "/backends/imgui_impl_sdl3.h",
            imgui_dir .. "/backends/imgui_impl_opengl3.h",
            imgui_dir .. "/backends/imgui_impl_opengl3_loader.h"
        }

        includedirs { "../src" }
        includedirs { "../include" }
        includedirs { sdl3_dir .. "/include" }
        includedirs { imgui_dir }
        includedirs { imgui_dir .. "/backends" }
        includedirs { glew_dir .. "/include" }

        cdialect "C17"
        cppdialect "C++17"
        platform_defines()

        filter "action:vs*"
            defines{"_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS"}
            links {"SDL3.lib", "glew32.lib", "opengl32.lib"}
            characterset ("Unicode")

        filter "system:windows"
            defines{"_WIN32"}
            links {"winmm", "gdi32", "opengl32"}
            
            -- SDL3 and GLEW x64 specific
            filter { "system:windows", "platforms:x64" }
                libdirs { sdl3_dir .. "/lib/x64", glew_dir .. "/lib/Release/x64" }
                postbuildcommands {
                    -- Path correcte: des de build/build_files/ cap a build/external/SDL3/lib/x64/
                    "{COPY} \"$(SolutionDir)build\\external\\SDL3\\lib\\x64\\SDL3.dll\" \"$(SolutionDir)bin\\%{cfg.buildcfg}\\\"",
                    -- Copy GLEW DLL
                    "{COPY} \"$(SolutionDir)build\\external\\glew\\bin\\Release\\x64\\glew32.dll\" \"$(SolutionDir)bin\\%{cfg.buildcfg}\\\""
                }
                
            -- SDL3 and GLEW x86 specific
            filter { "system:windows", "platforms:x86" }
                libdirs { sdl3_dir .. "/lib/x86", glew_dir .. "/lib/Release/Win32" }
                postbuildcommands {
                    -- Path correcte: des de build/build_files/ cap a build/external/SDL3/lib/x86/
                    "{COPY} \"$(SolutionDir)build\\external\\SDL3\\lib\\x86\\SDL3.dll\" \"$(SolutionDir)bin\\%{cfg.buildcfg}\\\"",
                    -- Copy GLEW DLL
                    "{COPY} \"$(SolutionDir)build\\external\\glew\\bin\\Release\\Win32\\glew32.dll\" \"$(SolutionDir)bin\\%{cfg.buildcfg}\\\""
                }

        filter "system:linux"
            links {"pthread", "m", "dl", "rt", "X11"}

        filter{}