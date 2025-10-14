add_rules("mode.debug", "mode.release")

option("il2cpp_path")
    set_description("il2cpp(即il2cpp+hybridclr)的路径")
option_end()

add_requires("zydis")
add_requires("fmt", {configs = {header_only = true}})

target("HybridClrDetourNative")
    add_packages("zydis", "fmt", { public = true })
    set_kind("shared")
    set_languages("cxx17")
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_links("GameAssembly.dll.lib", {public=true})
    add_defines(
        "_CRT_NONSTDC_NO_DEPRECATE",
        "_CRT_SECURE_NO_DEPRECATE",
        "_HAS_AUTO_PTR_ETC=1",
        "_HAS_ITERATOR_DEBUGGING=0",
        "_ITERATOR_DEBUG_LEVEL=0",
        "_SCL_SECURE_NO_DEPRECATE=0",
        "_SECURE_SCL=0",

        "BASELIB_INLINE_NAMESPACE=il2cpp_baselib"
    )
    on_load(function (target, opt)
        local il2cpp_path = get_config("il2cpp_path")
        print("il2cpp_path: " .. il2cpp_path)

        local il2cpp_include_list = {
            il2cpp_path,
            il2cpp_path .. "/libil2cpp",
            il2cpp_path .. "/external/baselib/Include",
            il2cpp_path .. "/external/bdwgc/include",
            
            il2cpp_path .. "/external/baselib/Platforms/Windows/Include",
        }
        for _, dir in ipairs(il2cpp_include_list) do
            target:add("includedirs", dir, { public = true })
        end
    end)

    add_filegroups("HybridClrDetourNative", {rootdir = "src"})
