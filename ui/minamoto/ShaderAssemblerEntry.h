//==============================================================================
// Minamoto : ShaderAssemblerEntry Header
//
// Copyright (c) 2023-2025 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include <Interface.h>

#include <assert.h>

#include <functional>
#include <string>
#include <vector>

#include <xxGraphic/xxGraphic.h>
#include <Editor/ImGuiHelper.h>
#include <Runtime/Tools/WindowsHeader.h>

#define xxFourCC
#define xxEightCC
constexpr uint32_t operator ""_cc(char const* text, size_t length)
{
    uint32_t value = 0;
    for (size_t i = 0; i < length; ++i)
        value += uint32_t(uint8_t(text[i])) << (i * 8);
    return value;
};
constexpr uint64_t operator ""_CC(char const* text, size_t length)
{
    uint64_t value = 0;
    for (size_t i = 0; i < length; ++i)
        value += uint64_t(uint8_t(text[i])) << (i * 8);
    return value;
};
