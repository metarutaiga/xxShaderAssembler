//==============================================================================
// xxShaderAssembler : disasm Source
//
// Copyright (c) 2024 TAiGA
// https://github.com/metarutaiga/xxshaderassembler
//==============================================================================
#include "pch.h"
#include "disasm.h"

#define D3DSHADER_MASK D3DVS_VERSION(0, 0)

//==============================================================================
#define _PRINTF(format, ...) snprintf(text, size, "%s" format, text, __VA_ARGS__)
//------------------------------------------------------------------------------
size_t TokenD3DSI(uint32_t tokens[8], uint32_t const* binary, size_t index, size_t count)
{
    uint32_t token = binary[index];
    tokens[0] = token;
    if ((token & D3DSHADER_MASK) == D3DSHADER_MASK)
        return 1;
    if ((token & D3DSI_OPCODE_MASK) == D3DSIO_COMMENT)
        return 0;
    for (size_t i = 1; i < 8; ++i)
    {
        if (index + i >= count)
            return i;
        tokens[i] = binary[index + i];
        switch (token & D3DSI_OPCODE_MASK)
        {
        case D3DSIO_DEF:
#if (DIRECT3D_VERSION >= 0x0900)
        case D3DSIO_DEFB:
        case D3DSIO_DEFI:
#endif
            if (i == 6)
                return 6;
            continue;
        }
        if ((tokens[i] & 0x80000000) == 0)
            return i;
    }
    return 8;
}
//------------------------------------------------------------------------------
size_t CommentD3DSI(char* text, size_t size, uint32_t const* binary, size_t index, size_t count)
{
    uint32_t token = binary[index];
    if ((token & D3DSI_OPCODE_MASK) == D3DSIO_COMMENT)
    {
        if (text == NULL)
            return ((token & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT) + 1;
        if (count > index)
            count = count - index - 1;
        else
            count = 0;
        if (count > ((token & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT) * 4)
            count = ((token & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT) * 4;
        if (count > size)
            count = size;
        memcpy(text, binary + index + 1, size);
        return size;
    }
    return 0;
}
//------------------------------------------------------------------------------
void DisassembleD3DSI(char* text, size_t size, size_t width, uint32_t version, uint32_t const tokens[8], size_t count)
{
    // Reset
    text[0] = 0;

    // Token
    size_t i = 0;
    uint32_t token = tokens[i];

    // Width
    if (width)
    {
        for (size_t i = 0; i < width; ++i)
        {
            if (i >= count)
            {
                _PRINTF("%8s ", "");
            }
            else
            {
                _PRINTF("%08X ", tokens[i]);
            }
        }
    }

    // Version
    if ((token & D3DSHADER_MASK) == D3DSHADER_MASK)
    {
        char const* shader = "";
        switch (token & D3DSHADER_MASK)
        {
        case D3DVS_VERSION(0, 0):
            shader = "vs";
            break;
        case D3DPS_VERSION(0, 0):
            shader = "ps";
            break;
        }
        _PRINTF("%s.%d.%d", shader, D3DSHADER_VERSION_MAJOR(token), D3DSHADER_VERSION_MINOR(token));
        return;
    }
    bool vs = true;
    bool ps14 = false;
    switch (version)
    {
    case D3DVS_VERSION(1, 0):
    case D3DVS_VERSION(1, 1):
        vs = true;
        ps14 = false;
        break;
    case D3DPS_VERSION(1, 0):
    case D3DPS_VERSION(1, 1):
    case D3DPS_VERSION(1, 2):
    case D3DPS_VERSION(1, 3):
        vs = false;
        ps14 = false;
        break;
    case D3DPS_VERSION(1, 4):
        vs = false;
        ps14 = true;
        break;
    }

    // Instruction
    char const* instruction;
    switch (token & D3DSI_OPCODE_MASK)
    {
    case D3DSIO_NOP:            instruction = "nop";                        break;
    case D3DSIO_MOV:            instruction = "mov";                        break;
    case D3DSIO_ADD:            instruction = "add";                        break;
    case D3DSIO_SUB:            instruction = "sub";                        break;
    case D3DSIO_MAD:            instruction = "mad";                        break;
    case D3DSIO_MUL:            instruction = "mul";                        break;
    case D3DSIO_RCP:            instruction = "rcp";                        break;
    case D3DSIO_RSQ:            instruction = "rsq";                        break;
    case D3DSIO_DP3:            instruction = "dp3";                        break;
    case D3DSIO_DP4:            instruction = "dp4";                        break;
    case D3DSIO_MIN:            instruction = "min";                        break;
    case D3DSIO_MAX:            instruction = "max";                        break;
    case D3DSIO_SLT:            instruction = "slt";                        break;
    case D3DSIO_SGE:            instruction = "sge";                        break;
    case D3DSIO_EXP:            instruction = "exp";                        break;
    case D3DSIO_LOG:            instruction = "log";                        break;
    case D3DSIO_LIT:            instruction = "lit";                        break;
    case D3DSIO_DST:            instruction = "dst";                        break;
    case D3DSIO_LRP:            instruction = "lrp";                        break;
    case D3DSIO_FRC:            instruction = "frc";                        break;
    case D3DSIO_M4x4:           instruction = "m4x4";                       break;
    case D3DSIO_M4x3:           instruction = "m4x3";                       break;
    case D3DSIO_M3x4:           instruction = "m3x4";                       break;
    case D3DSIO_M3x3:           instruction = "m3x3";                       break;
    case D3DSIO_M3x2:           instruction = "m3x2";                       break;
#if (DIRECT3D_VERSION >= 0x0900)
    case D3DSIO_CALL:           instruction = "call";                       break;
    case D3DSIO_CALLNZ:         instruction = "callnz";                     break;
    case D3DSIO_LOOP:           instruction = "loop";                       break;
    case D3DSIO_RET:            instruction = "ret";                        break;
    case D3DSIO_ENDLOOP:        instruction = "endloop";                    break;
    case D3DSIO_LABEL:          instruction = "label";                      break;
    case D3DSIO_DCL:            instruction = "dcl";                        break;
    case D3DSIO_POW:            instruction = "pow";                        break;
    case D3DSIO_CRS:            instruction = "crs";                        break;
    case D3DSIO_SGN:            instruction = "sgn";                        break;
    case D3DSIO_ABS:            instruction = "abs";                        break;
    case D3DSIO_NRM:            instruction = "nrm";                        break;
    case D3DSIO_SINCOS:         instruction = "sincos";                     break;
    case D3DSIO_REP:            instruction = "rep";                        break;
    case D3DSIO_ENDREP:         instruction = "endrep";                     break;
    case D3DSIO_IF:             instruction = "if";                         break;
    case D3DSIO_IFC:            instruction = "ifc";                        break;
    case D3DSIO_ELSE:           instruction = "else";                       break;
    case D3DSIO_ENDIF:          instruction = "endif";                      break;
    case D3DSIO_BREAK:          instruction = "break";                      break;
    case D3DSIO_BREAKC:         instruction = "breakc";                     break;
    case D3DSIO_MOVA:           instruction = "mova";                       break;
    case D3DSIO_DEFB:           instruction = "defb";                       break;
    case D3DSIO_DEFI:           instruction = "defi";                       break;
#endif
    case D3DSIO_TEXCOORD:       instruction = ps14 ? "texcrd" : "texcoord"; break;
    case D3DSIO_TEXKILL:        instruction = "texkill";                    break;
    case D3DSIO_TEX:            instruction = ps14 ? "texld" : "tex";       break;
    case D3DSIO_TEXBEM:         instruction = "texbem";                     break;
    case D3DSIO_TEXBEML:        instruction = "texbeml";                    break;
    case D3DSIO_TEXREG2AR:      instruction = "texreg2ar";                  break;
    case D3DSIO_TEXREG2GB:      instruction = "texreg2gb";                  break;
    case D3DSIO_TEXM3x2PAD:     instruction = "texm3x2pad";                 break;
    case D3DSIO_TEXM3x2TEX:     instruction = "texm3x2tex";                 break;
    case D3DSIO_TEXM3x3PAD:     instruction = "texm3x3pad";                 break;
    case D3DSIO_TEXM3x3TEX:     instruction = "texm3x3tex";                 break;
    case D3DSIO_TEXM3x3DIFF:    instruction = "texm3x3diff";                break;
    case D3DSIO_TEXM3x3SPEC:    instruction = "texm3x3spec";                break;
    case D3DSIO_TEXM3x3VSPEC:   instruction = "texm3x3vspec";               break;
    case D3DSIO_EXPP:           instruction = "expp";                       break;
    case D3DSIO_LOGP:           instruction = "logp";                       break;
    case D3DSIO_CND:            instruction = "cnd";                        break;
    case D3DSIO_DEF:            instruction = "def";                        break;
    case D3DSIO_TEXREG2RGB:     instruction = "texreg2rgb";                 break;
    case D3DSIO_TEXDP3TEX:      instruction = "texdp3tex";                  break;
    case D3DSIO_TEXM3x2DEPTH:   instruction = "texm3x2depth";               break;
    case D3DSIO_TEXDP3:         instruction = "texdp3";                     break;
    case D3DSIO_TEXM3x3:        instruction = "texm3x3";                    break;
    case D3DSIO_TEXDEPTH:       instruction = "texdepth";                   break;
    case D3DSIO_CMP:            instruction = "cmp";                        break;
    case D3DSIO_BEM:            instruction = "bem";                        break;
#if (DIRECT3D_VERSION >= 0x0900)
    case D3DSIO_DP2ADD:         instruction = "dp2add";                     break;
    case D3DSIO_DSX:            instruction = "dsx";                        break;
    case D3DSIO_DSY:            instruction = "dsy";                        break;
    case D3DSIO_TEXLDD:         instruction = "texldd";                     break;
    case D3DSIO_SETP:           instruction = "setp";                       break;
    case D3DSIO_TEXLDL:         instruction = "texldl";                     break;
    case D3DSIO_BREAKP:         instruction = "breakp";                     break;
#endif
    case D3DSIO_PHASE:          instruction = "phase";                      break;
    case D3DSIO_END:            instruction = "end";                        break;
    default:                    instruction = "?";                          break;
    }
    i++;

#if (DIRECT3D_VERSION >= 0x0900)
    // Declaration
    if ((token & D3DSI_OPCODE_MASK) == D3DSIO_DCL)
    {
        switch (tokens[i] & D3DSP_DCL_USAGE_MASK)
        {
        case D3DDECLUSAGE_POSITION:     instruction = "dcl_position";       break;
        case D3DDECLUSAGE_BLENDWEIGHT:  instruction = "dcl_blendweight";    break;
        case D3DDECLUSAGE_BLENDINDICES: instruction = "dcl_blendindices";   break;
        case D3DDECLUSAGE_NORMAL:       instruction = "dcl_normal";         break;
        case D3DDECLUSAGE_PSIZE:        instruction = "dcl_psize";          break;
        case D3DDECLUSAGE_TEXCOORD:     instruction = "dcl_texcoord";       break;
        case D3DDECLUSAGE_TANGENT:      instruction = "dcl_tangent";        break;
        case D3DDECLUSAGE_BINORMAL:     instruction = "dcl_binormal";       break;
        case D3DDECLUSAGE_TESSFACTOR:   instruction = "dcl_tessfactor";     break;
        case D3DDECLUSAGE_POSITIONT:    instruction = "dcl_positiont";      break;
        case D3DDECLUSAGE_COLOR:        instruction = "dcl_color";          break;
        case D3DDECLUSAGE_FOG:          instruction = "dcl_fog";            break;
        case D3DDECLUSAGE_DEPTH:        instruction = "dcl_depth";          break;
        case D3DDECLUSAGE_SAMPLE:       instruction = "dcl_sample";         break;
        }
        i++;
    }
#endif

    // Shift / Saturate
    char const* shift = "";
    char const* modifier = "";
    if ((token & D3DSI_OPCODE_MASK) != D3DSIO_DEF)
    {
        switch (tokens[i] & D3DSP_DSTSHIFT_MASK)
        {
        case 1  << D3DSP_DSTSHIFT_SHIFT:    shift = "_x2";  break;
        case 2  << D3DSP_DSTSHIFT_SHIFT:    shift = "_x4";  break;
        case 3  << D3DSP_DSTSHIFT_SHIFT:    shift = "_x8";  break;
        case 15 << D3DSP_DSTSHIFT_SHIFT:    shift = "_d2";  break;
        case 14 << D3DSP_DSTSHIFT_SHIFT:    shift = "_d4";  break;
        case 13 << D3DSP_DSTSHIFT_SHIFT:    shift = "_d8";  break;
        }
        switch (tokens[i] & D3DSP_DSTMOD_MASK)
        {
        case D3DSPDM_SATURATE:          modifier = "_sat";  break;
#if (DIRECT3D_VERSION >= 0x0900)
        case D3DSPDM_PARTIALPRECISION:  modifier = "_pp";   break;
#endif
        }
    }
    if (token & D3DSI_COISSUE)
    {
        _PRINTF("%s%s%s%s", "+", instruction, shift, modifier);
    }
    else
    {
        _PRINTF("%s%s%s%s", "", instruction, shift, modifier);
    }

    // End
    if ((token & D3DSI_OPCODE_MASK) == D3DSIO_END)
    {
        return;
    }

    // Parameter
    for (size_t j = 0; j < 5; ++j)
    {
        // Constant
        if (j != 0)
        {
            switch (token & D3DSI_OPCODE_MASK)
            {
            case D3DSIO_DEF:
            {
                float value;
                memcpy(&value, &tokens[i + j], sizeof(float));
                _PRINTF("%s", (j == 0) ? " " : ", ");
                _PRINTF("%.1f", value);
                continue;
            }
#if (DIRECT3D_VERSION >= 0x0900)
            case D3DSIO_DEFB:
            {
                bool value;
                memcpy(&value, &tokens[i + j], sizeof(bool));
                _PRINTF("%s", (j == 0) ? " " : ", ");
                _PRINTF("%s", value ? "true" : "false");
                continue;
            }
            case D3DSIO_DEFI:
            {
                int value;
                memcpy(&value, &tokens[i + j], sizeof(int));
                _PRINTF("%s", (j == 0) ? " " : ", ");
                _PRINTF("%d", value);
                continue;
            }
#endif
            default:
                break;
            }
        }

        // Token
        uint32_t token = tokens[i + j];

        // Register
        if ((token & 0x80000000) == 0)
        {
            break;
        }
        _PRINTF("%s", (j == 0) ? " " : ", ");

        // Type
        char const* reg = "";
        uint32_t type = 0;
        uint32_t index = 0;
#if (DIRECT3D_VERSION >= 0x0900)
        type |= (token & D3DSP_REGTYPE_MASK) >> D3DSP_REGTYPE_SHIFT;
        type |= (token & D3DSP_REGTYPE_MASK2) >> D3DSP_REGTYPE_SHIFT2;
#else
        type |= (token & D3DSP_REGTYPE_MASK);
#endif
        index |= (token & D3DSP_REGNUM_MASK);
        switch (type)
        {
        case D3DSPR_TEMP:           reg = "r";              break;
        case D3DSPR_INPUT:          reg = "v";              break;
        case D3DSPR_CONST:          reg = "c";              break;
//      case D3DSPR_TEXTURE:
        case D3DSPR_ADDR:           reg = vs ? "a" : "t";   break;
        case D3DSPR_RASTOUT:
            switch (index)
            {
            case D3DSRO_POSITION:   reg = "oPos";           break;
            case D3DSRO_FOG:        reg = "oFog";           break;
            case D3DSRO_POINT_SIZE: reg = "oPts";           break;
            }
            break;
        case D3DSPR_ATTROUT:        reg = "oD";             break;
//      case D3DSPR_OUTPUT:
        case D3DSPR_TEXCRDOUT:      reg = "oT";             break;
#if (DIRECT3D_VERSION >= 0x0900)
        case D3DSPR_CONSTINT:       reg = "i";              break;
        case D3DSPR_COLOROUT:       reg = "oC";             break;
        case D3DSPR_DEPTHOUT:       reg = "oDepth";         break;
        case D3DSPR_SAMPLER:        reg = "s";              break;
        case D3DSPR_CONST2:         reg = "c";              break;
        case D3DSPR_CONST3:         reg = "c";              break;
        case D3DSPR_CONST4:         reg = "c";              break;
        case D3DSPR_CONSTBOOL:      reg = "c";              break;
        case D3DSPR_LOOP:           reg = "aL";             break;
        case D3DSPR_TEMPFLOAT16:    reg = "h";              break;
        case D3DSPR_MISCTYPE:
            switch (index)
            {
            case D3DSMO_FACE:       reg = "vFace";          break;
            case D3DSMO_POSITION:   reg = "vPos";           break;
            }
            break;
        case D3DSPR_LABEL:          reg = "l";              break;
        case D3DSPR_PREDICATE:      reg = "p";              break;
#endif
        default:                    reg = "?";              break;
        }
#if (DIRECT3D_VERSION >= 0x0900)
        switch (type)
        {
        case D3DSPR_CONST2:         index += 2048;          break;
        case D3DSPR_CONST3:         index += 4096;          break;
        case D3DSPR_CONST4:         index += 6144;          break;
        }
#endif

        // Source / Destination
        char const* pre = "";
        char const* post = "";
        if (j != 0)
        {
            switch (token & D3DSP_SRCMOD_MASK)
            {
            case D3DSPSM_NEG:
            case D3DSPSM_BIASNEG:
            case D3DSPSM_SIGNNEG:
            case D3DSPSM_X2NEG:     pre = "-";          break;
            case D3DSPSM_COMP:      pre = "1-";         break;
#if (DIRECT3D_VERSION >= 0x0900)
            case D3DSPSM_ABS:       pre = "|";          break;
            case D3DSPSM_ABSNEG:    pre = "-|";         break;
            case D3DSPSM_NOT:       pre = "!";          break;
#endif
            }
            switch (token & D3DSP_SRCMOD_MASK)
            {
            case D3DSPSM_BIAS:
            case D3DSPSM_BIASNEG:   post = "_bias";     break;
            case D3DSPSM_SIGN:
            case D3DSPSM_SIGNNEG:   post = "_bx2";      break;
            case D3DSPSM_X2:
            case D3DSPSM_X2NEG:     post = "_x2";       break;
            case D3DSPSM_DZ:        post = "_dz";       break;
            case D3DSPSM_DW:        post = "_dw";       break;
#if (DIRECT3D_VERSION >= 0x0900)
            case D3DSPSM_ABS:
            case D3DSPSM_ABSNEG:    post = "|";         break;
#endif
            }
        }
        switch (type)
        {
        case D3DSPR_RASTOUT:
#if (DIRECT3D_VERSION >= 0x0900)
        case D3DSPR_MISCTYPE:
#endif
            _PRINTF("%s%s%s", pre, reg, post);
            break;
        default:
            _PRINTF("%s%s%d%s", pre, reg, index, post);
            break;
        }

        // Swizzle / WriteMask
        if (j != 0 && (token & D3DSP_SWIZZLE_MASK) != D3DSP_NOSWIZZLE)
        {
            _PRINTF("%s", ".");
            for (size_t i = 0; i < 4; ++i)
            {
                switch ((token >> (D3DSP_SWIZZLE_SHIFT + i * 2)) & 0x3)
                {
                case 0: _PRINTF("%s", "x"); break;
                case 1: _PRINTF("%s", "y"); break;
                case 2: _PRINTF("%s", "z"); break;
                case 3: _PRINTF("%s", "w"); break;
                }
            }
        }
        else if (j == 0 && (token & D3DSP_WRITEMASK_ALL) != D3DSP_WRITEMASK_ALL)
        {
            _PRINTF("%s", ".");
            if (token & D3DSP_WRITEMASK_0) _PRINTF("%s", "x");
            if (token & D3DSP_WRITEMASK_1) _PRINTF("%s", "y");
            if (token & D3DSP_WRITEMASK_2) _PRINTF("%s", "z");
            if (token & D3DSP_WRITEMASK_3) _PRINTF("%s", "w");
        }
    }
}
//==============================================================================
