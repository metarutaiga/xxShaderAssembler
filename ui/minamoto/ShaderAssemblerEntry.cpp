//==============================================================================
// xxShaderAssembler : ShaderAssemblerEntry Source
//
// Copyright (c) 2023-2025 TAiGA
// https://github.com/metarutaiga/xxshaderassembler
//==============================================================================
#include "ShaderAssemblerEntry.h"
#include "ShaderAssembler.h"
#include "ShaderCompiler.h"
#include "ShaderDisassembler.h"

#define PLUGIN_NAME     "ShaderAssembler"
#define PLUGIN_MAJOR    1
#define PLUGIN_MINOR    0

//------------------------------------------------------------------------------
moduleAPI const char* Create(const CreateData& createData)
{
    return PLUGIN_NAME;
}
//------------------------------------------------------------------------------
moduleAPI void Shutdown(const ShutdownData& shutdownData)
{

}
//------------------------------------------------------------------------------
moduleAPI void Message(const MessageData& messageData)
{
    if (messageData.length == 1)
    {
        switch (xxHash(messageData.data[0]))
        {
        case xxHash("INIT"):
            ShaderDisassembler::Initialize();
            break;
        case xxHash("SHUTDOWN"):
            ShaderDisassembler::Shutdown();
            break;
        default:
            break;
        }
    }
}
//------------------------------------------------------------------------------
moduleAPI bool Update(const UpdateData& updateData)
{
    static bool showAbout = false;
    static bool showShaderAssembler = false;
    static bool showShaderCompiler = false;
    static bool showShaderDisassembler = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(PLUGIN_NAME))
        {
            ImGui::MenuItem("Shader Assembler", nullptr, &showShaderAssembler);
            ImGui::MenuItem("Shader Compiler", nullptr, &showShaderCompiler);
            ImGui::MenuItem("Shader Disassembler", nullptr, &showShaderDisassembler);
            ImGui::Separator();
            ImGui::MenuItem("About " PLUGIN_NAME, nullptr, &showAbout);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    bool updated = false;
    updated |= ShaderAssembler::Update(updateData, showShaderAssembler);
    updated |= ShaderCompiler::Update(updateData, showShaderCompiler);
    updated |= ShaderDisassembler::Update(updateData, showShaderDisassembler);

    return updated;
}
//------------------------------------------------------------------------------
moduleAPI void Render(const RenderData& renderData)
{

}
//------------------------------------------------------------------------------
