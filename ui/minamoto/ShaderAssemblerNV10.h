//==============================================================================
// xxShaderAssembler : ShaderAssemblerNV10 Header
//
// Copyright (c) 2023-2025 TAiGA
// https://github.com/metarutaiga/xxshaderassembler
//==============================================================================
#pragma once

struct ShaderAssemblerNV10
{
    static int DebugPrintf(char const* format, ...);
    static std::vector<uint32_t> CompileCheops(std::vector<uint32_t> const& binary, std::string& message);
    static std::vector<uint32_t> CompileCelsius(std::vector<uint32_t> const& binary, std::string& message);
    static std::string DisassembleCheops(std::vector<uint32_t> const& code, std::string& message);
    static std::string DisassembleCelsius(std::vector<uint32_t> const& code, std::string& message);
};
