//==============================================================================
// xxShaderAssembler : ShaderDisassembler Header
//
// Copyright (c) 2023-2025 TAiGA
// https://github.com/metarutaiga/xxshaderassembler
//==============================================================================
#pragma once

struct ShaderDisassembler
{
    static void Initialize();
    static void Shutdown();
    static bool Update(const UpdateData& updateData, bool& show);
};
