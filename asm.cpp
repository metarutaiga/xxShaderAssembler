//==============================================================================
// xxShaderAssembler : asm Source
//
// Copyright (c) 2024 TAiGA
// https://github.com/metarutaiga/xxshaderassembler
//==============================================================================
#include "pch.h"
#include "asm.h"

//==============================================================================
#define D3DSP_REGTYPE(reg) (((reg << D3DSP_REGTYPE_SHIFT) & D3DSP_REGTYPE_MASK) | ((reg << D3DSP_REGTYPE_SHIFT2) & D3DSP_REGTYPE_MASK2))
//------------------------------------------------------------------------------
// |Destination|P|Type |Shift  |Modifie|WriteMa|Min| |Typ|Number               |
// |           |1| | | | | | | | | | | | | | | | | |?| | | | | | | | | | | | | |
// |Source     |P|Type |Modifie|Swizzle        |Min|A|Typ|Number               |
// |           |1| | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
//------------------------------------------------------------------------------
static uint32_t Register(char const* text, bool source, int shift, bool sat, bool pp)
{
    if (text == nullptr || text[0] == ';' || text[0] == '/')
        return 0;

    bool dot = false;
    bool underline = false;
    uint64_t typeShift = 0;
    uint64_t maskShift = 0;
    uint64_t modShift = 0;
    uint64_t type = 0;
    uint64_t index = 0;
    uint64_t mask = 0;
    uint64_t mod = 0;
    for (uint64_t c; (c = *text); ++text)
    {
        if (c == '.')
        {
            dot = true;
            underline = false;
        }
        else if (c == '_')
        {
            underline = true;
        }
        else if (c == '-' || c == '|' || c == '!')
        {
            mod = mod | (c << modShift);
            modShift += 8;
        }
        else if (c >= '0' && c <= '9')
        {
            if (underline || type == 0)
            {
                mod = mod | (c << modShift);
                modShift += 8;
                continue;
            }
            index = (index * 10) + (c - '0');
        }
        else if (c >= 'a' && c <= 'z')
        {
            if (dot)
            {
                switch (c)
                {
                case 'r':
                case 'g':
                case 'b':
                case 'a':
                case 'x':
                case 'y':
                case 'z':
                case 'w':
                    mask = (mask & ~(0xFFFFFFFF << maskShift)) | (c * (0x01010101 << maskShift));
                    maskShift += 8;
                    break;
                }
                continue;
            }
            if (underline)
            {
                mod = mod | (c << modShift);
                modShift += 8;
                continue;
            }
            type = type | (c << typeShift);
            typeShift += 8;
        }
    }
    uint32_t token = 0x80000000;
    switch (type)
    {
#if (DIRECT3D_VERSION >= 0x0900)
    case "r"_u64:           token |= D3DSP_REGTYPE(D3DSPR_TEMP);        break;
    case "v"_u64:           token |= D3DSP_REGTYPE(D3DSPR_INPUT);       break;
    case "c"_u64:
        switch (index / 2048)
        {
        case 0:             token |= D3DSP_REGTYPE(D3DSPR_CONST);       break;
        case 1:             token |= D3DSP_REGTYPE(D3DSPR_CONST2);      break;
        case 2:             token |= D3DSP_REGTYPE(D3DSPR_CONST3);      break;
        case 3:             token |= D3DSP_REGTYPE(D3DSPR_CONST4);      break;
        }
        break;
    case "a"_u64:           token |= D3DSP_REGTYPE(D3DSPR_ADDR);        break;
    case "t"_u64:           token |= D3DSP_REGTYPE(D3DSPR_TEXTURE);     break;
    case "opos"_u64:        token |= D3DSP_REGTYPE(D3DSPR_RASTOUT);     break;
    case "ofog"_u64:        token |= D3DSP_REGTYPE(D3DSPR_RASTOUT);     break;
    case "opts"_u64:        token |= D3DSP_REGTYPE(D3DSPR_RASTOUT);     break;
    case "od"_u64:          token |= D3DSP_REGTYPE(D3DSPR_ATTROUT);     break;
    case "ot"_u64:          token |= D3DSP_REGTYPE(D3DSPR_TEXCRDOUT);   break;
    case "o"_u64:           token |= D3DSP_REGTYPE(D3DSPR_OUTPUT);      break;
    case "i"_u64:           token |= D3DSP_REGTYPE(D3DSPR_CONSTINT);    break;
    case "oc"_u64:          token |= D3DSP_REGTYPE(D3DSPR_COLOROUT);    break;
    case "odepth"_u64:      token |= D3DSP_REGTYPE(D3DSPR_DEPTHOUT);    break;
    case "s"_u64:           token |= D3DSP_REGTYPE(D3DSPR_SAMPLER);     break;
    case "b"_u64:           token |= D3DSP_REGTYPE(D3DSPR_CONSTBOOL);   break;
    case "al"_u64:          token |= D3DSP_REGTYPE(D3DSPR_LOOP);        break;
    case "h"_u64:           token |= D3DSP_REGTYPE(D3DSPR_TEMPFLOAT16); break;
    case "vface"_u64:       token |= D3DSP_REGTYPE(D3DSPR_MISCTYPE);    break;
    case "vpos"_u64:        token |= D3DSP_REGTYPE(D3DSPR_MISCTYPE);    break;
    case "l"_u64:           token |= D3DSP_REGTYPE(D3DSPR_LABEL);       break;
    case "p"_u64:           token |= D3DSP_REGTYPE(D3DSPR_PREDICATE);   break;
#else
    case "r"_u64:           token |= D3DSPR_TEMP;       break;
    case "v"_u64:           token |= D3DSPR_INPUT;      break;
    case "c"_u64:           token |= D3DSPR_CONST;      break;
    case "a"_u64:           token |= D3DSPR_ADDR;       break;
    case "t"_u64:           token |= D3DSPR_TEXTURE;    break;
    case "opos"_u64:        token |= D3DSPR_RASTOUT;    break;
    case "ofog"_u64:        token |= D3DSPR_RASTOUT;    break;
    case "opts"_u64:        token |= D3DSPR_RASTOUT;    break;
    case "od"_u64:          token |= D3DSPR_ATTROUT;    break;
    case "ot"_u64:          token |= D3DSPR_TEXCRDOUT;  break;
#endif
    }
    switch (type)
    {
    case "opos"_u64:        index = D3DSRO_POSITION;    break;
    case "ofog"_u64:        index = D3DSRO_FOG;         break;
    case "opts"_u64:        index = D3DSRO_POINT_SIZE;  break;
#if (DIRECT3D_VERSION >= 0x0900)
    case "c"_u64:           index %= 2048;              break;
    case "vface"_u64:       index = D3DSMO_FACE;        break;
    case "vpos"_u64:        index = D3DSMO_POSITION;    break;
#endif
    }
    token |= index;
    if (source)
    {
        switch (mod)
        {
        case "-"_u64:       token |= D3DSPSM_NEG;       break;
        case "bias"_u64:    token |= D3DSPSM_BIAS;      break;
        case "-bias"_u64:   token |= D3DSPSM_BIASNEG;   break;
        case "bx2"_u64:     token |= D3DSPSM_SIGN;      break;
        case "-bx2"_u64:    token |= D3DSPSM_SIGNNEG;   break;
        case "1-"_u64:      token |= D3DSPSM_COMP;      break;
        case "x2"_u64:      token |= D3DSPSM_X2;        break;
        case "-x2"_u64:     token |= D3DSPSM_X2NEG;     break;
        case "dz"_u64:
        case "db"_u64:      token |= D3DSPSM_DZ;        break;
        case "dw"_u64:
        case "da"_u64:      token |= D3DSPSM_DW;        break;
#if (DIRECT3D_VERSION >= 0x0900)
        case "||"_u64:      token |= D3DSPSM_ABS;       break;
        case "-||"_u64:     token |= D3DSPSM_ABSNEG;    break;
        case "!"_u64:       token |= D3DSPSM_NOT;       break;
#endif
        }
    }
    else
    {
        token |= (shift << D3DSP_DSTSHIFT_SHIFT);
        if (sat)
        {
            token |= D3DSPDM_SATURATE;
        }
#if (DIRECT3D_VERSION >= 0x0900)
        if (pp)
        {
            token |= D3DSPDM_PARTIALPRECISION;
        }
#endif
    }
    if (mask)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            switch (uint8_t(mask))
            {
            case 'r': case 'x': token |= source ? (0 << (D3DSP_SWIZZLE_SHIFT + i * 2)) : D3DSP_WRITEMASK_0; break;
            case 'g': case 'y': token |= source ? (1 << (D3DSP_SWIZZLE_SHIFT + i * 2)) : D3DSP_WRITEMASK_1; break;
            case 'b': case 'z': token |= source ? (2 << (D3DSP_SWIZZLE_SHIFT + i * 2)) : D3DSP_WRITEMASK_2; break;
            case 'a': case 'w': token |= source ? (3 << (D3DSP_SWIZZLE_SHIFT + i * 2)) : D3DSP_WRITEMASK_3; break;
            }
            mask >>= 8;
        }
    }
    else
    {
        token |= source ? D3DSP_NOSWIZZLE : D3DSP_WRITEMASK_ALL;
    }

    return token;
}
//------------------------------------------------------------------------------
size_t AssembleD3DSI(uint32_t tokens[8], uint32_t version, char const* text)
{
    char temp[256];
    strncpy(temp, text, 256);
    for (char& c : temp)
    {
        if (c == 0)
            break;
        if (c >= 'A' && c <= 'Z')
            c += 0x20;
    }

    // Tokenize
    char* lasts = NULL;
    char* opcode = strtok_r(temp, ", \t", &lasts);
    char* dst = strtok_r(nullptr, ", \t", &lasts);
    char* src[4] =
    {
        strtok_r(nullptr, ", \t", &lasts),
        strtok_r(nullptr, ", \t", &lasts),
        strtok_r(nullptr, ", \t", &lasts),
        strtok_r(nullptr, ", \t", &lasts),
    };

    // Skip
    size_t index = 0;
    if (opcode == nullptr || opcode[0] == ';' || opcode[0] == '/')
        return 0;

    // Co-issue
    bool coissue = false;
    if (opcode[0] == '+')
    {
        opcode++;
        coissue = true;
    }

    // Saturate
    bool sat = false;
    bool pp = false;
    if (char* suffix = strstr(opcode, "_sat")) { suffix[0] = 0; sat = true; }
#if (DIRECT3D_VERSION >= 0x0900)
    if (char* suffix = strstr(opcode, "_pp")) { suffix[0] = 0; pp = true; }
#endif

    // Shift
    int shift = 0;
    if (char* suffix = strstr(opcode, "_x2")) { suffix[0] = 0; shift = 1; }
    if (char* suffix = strstr(opcode, "_x4")) { suffix[0] = 0; shift = 2; }
    if (char* suffix = strstr(opcode, "_x8")) { suffix[0] = 0; shift = 3; }
    if (char* suffix = strstr(opcode, "_d2")) { suffix[0] = 0; shift = 15; }
    if (char* suffix = strstr(opcode, "_d4")) { suffix[0] = 0; shift = 14; }
    if (char* suffix = strstr(opcode, "_d8")) { suffix[0] = 0; shift = 13; }

    // Opcode
    uint32_t token = D3DSIO_END;
    uint32_t dcl = UINT32_MAX;
    switch (operator ""_u64(opcode, strlen(opcode)))
    {
    case "vs_1_0"_u64:  case "vs.1.0"_u64:  tokens[index++] = D3DVS_VERSION(1, 0);  return index;
    case "vs_1_1"_u64:  case "vs.1.1"_u64:  tokens[index++] = D3DVS_VERSION(1, 1);  return index;
#if (DIRECT3D_VERSION >= 0x0900)
    case "vs_2_0"_u64:  case "vs.2.0"_u64:  tokens[index++] = D3DVS_VERSION(2, 0);  return index;
    case "vs_2_a"_u64:  case "vs.2.a"_u64:  tokens[index++] = D3DVS_VERSION(2, 1);  return index;
    case "vs_3_0"_u64:  case "vs.3.0"_u64:  tokens[index++] = D3DVS_VERSION(3, 0);  return index;
#endif
    case "ps_1_0"_u64:  case "ps.1.0"_u64:  tokens[index++] = D3DPS_VERSION(1, 0);  return index;
    case "ps_1_1"_u64:  case "ps.1.1"_u64:  tokens[index++] = D3DPS_VERSION(1, 1);  return index;
    case "ps_1_2"_u64:  case "ps.1.2"_u64:  tokens[index++] = D3DPS_VERSION(1, 2);  return index;
    case "ps_1_3"_u64:  case "ps.1.3"_u64:  tokens[index++] = D3DPS_VERSION(1, 3);  return index;
    case "ps_1_4"_u64:  case "ps.1.4"_u64:  tokens[index++] = D3DPS_VERSION(1, 4);  return index;
#if (DIRECT3D_VERSION >= 0x0900)
    case "ps_2_0"_u64:  case "ps.2.0"_u64:  tokens[index++] = D3DPS_VERSION(2, 0);  return index;
    case "ps_2_a"_u64:  case "ps.2.a"_u64:  tokens[index++] = D3DPS_VERSION(2, 1);  return index;
    case "ps_2_b"_u64:  case "ps.2.b"_u64:  tokens[index++] = D3DPS_VERSION(2, 2);  return index;
    case "ps_3_0"_u64:  case "ps.3.0"_u64:  tokens[index++] = D3DPS_VERSION(3, 0);  return index;
#endif
    case "nop"_u64:                 token = D3DSIO_NOP;             break;
    case "mov"_u64:                 token = D3DSIO_MOV;             break;
    case "add"_u64:                 token = D3DSIO_ADD;             break;
    case "sub"_u64:                 token = D3DSIO_SUB;             break;
    case "mad"_u64:                 token = D3DSIO_MAD;             break;
    case "mul"_u64:                 token = D3DSIO_MUL;             break;
    case "rcp"_u64:                 token = D3DSIO_RCP;             break;
    case "rsq"_u64:                 token = D3DSIO_RSQ;             break;
    case "dp3"_u64:                 token = D3DSIO_DP3;             break;
    case "dp4"_u64:                 token = D3DSIO_DP4;             break;
    case "min"_u64:                 token = D3DSIO_MIN;             break;
    case "max"_u64:                 token = D3DSIO_MAX;             break;
    case "slt"_u64:                 token = D3DSIO_SLT;             break;
    case "sge"_u64:                 token = D3DSIO_SGE;             break;
    case "exp"_u64:                 token = D3DSIO_EXP;             break;
    case "log"_u64:                 token = D3DSIO_LOG;             break;
    case "lit"_u64:                 token = D3DSIO_LIT;             break;
    case "dst"_u64:                 token = D3DSIO_DST;             break;
    case "lrp"_u64:                 token = D3DSIO_LRP;             break;
    case "frc"_u64:                 token = D3DSIO_FRC;             break;
    case "m4x4"_u64:                token = D3DSIO_M4x4;            break;
    case "m4x3"_u64:                token = D3DSIO_M4x3;            break;
    case "m3x4"_u64:                token = D3DSIO_M3x4;            break;
    case "m3x3"_u64:                token = D3DSIO_M3x3;            break;
    case "m3x2"_u64:                token = D3DSIO_M3x2;            break;
#if (DIRECT3D_VERSION >= 0x0900)
    case "call"_u64:                token = D3DSIO_CALL;            break;
    case "callnz"_u64:              token = D3DSIO_CALLNZ;          break;
    case "loop"_u64:                token = D3DSIO_LOOP;            break;
    case "ret"_u64:                 token = D3DSIO_RET;             break;
    case "endloop"_u64:             token = D3DSIO_ENDLOOP;         break;
    case "label"_u64:               token = D3DSIO_LABEL;           break;
    case "dcl"_u64:                 token = D3DSIO_DCL;             break;
    case "dcl_position"_u64:        token = D3DSIO_DCL; dcl = D3DDECLUSAGE_POSITION;        break;
    case "dcl_blendweight"_u64:     token = D3DSIO_DCL; dcl = D3DDECLUSAGE_BLENDWEIGHT;     break;
    case "dcl_blendindices"_u64:    token = D3DSIO_DCL; dcl = D3DDECLUSAGE_BLENDINDICES;    break;
    case "dcl_normal"_u64:          token = D3DSIO_DCL; dcl = D3DDECLUSAGE_NORMAL;          break;
    case "dcl_psize"_u64:           token = D3DSIO_DCL; dcl = D3DDECLUSAGE_PSIZE;           break;
    case "dcl_texcoord"_u64:        token = D3DSIO_DCL; dcl = D3DDECLUSAGE_TEXCOORD;        break;
    case "dcl_tangent"_u64:         token = D3DSIO_DCL; dcl = D3DDECLUSAGE_TANGENT;         break;
    case "dcl_binormal"_u64:        token = D3DSIO_DCL; dcl = D3DDECLUSAGE_BINORMAL;        break;
    case "dcl_tessfactor"_u64:      token = D3DSIO_DCL; dcl = D3DDECLUSAGE_TESSFACTOR;      break;
    case "dcl_positiont"_u64:       token = D3DSIO_DCL; dcl = D3DDECLUSAGE_POSITIONT;       break;
    case "dcl_color"_u64:           token = D3DSIO_DCL; dcl = D3DDECLUSAGE_COLOR;           break;
    case "dcl_fog"_u64:             token = D3DSIO_DCL; dcl = D3DDECLUSAGE_FOG;             break;
    case "dcl_depth"_u64:           token = D3DSIO_DCL; dcl = D3DDECLUSAGE_DEPTH;           break;
    case "dcl_sample"_u64:          token = D3DSIO_DCL; dcl = D3DDECLUSAGE_SAMPLE;          break;
    case "pow"_u64:                 token = D3DSIO_POW;             break;
    case "crs"_u64:                 token = D3DSIO_CRS;             break;
    case "sgn"_u64:                 token = D3DSIO_SGN;             break;
    case "abs"_u64:                 token = D3DSIO_ABS;             break;
    case "nrm"_u64:                 token = D3DSIO_NRM;             break;
    case "sincos"_u64:              token = D3DSIO_SINCOS;          break;
    case "rep"_u64:                 token = D3DSIO_REP;             break;
    case "endrep"_u64:              token = D3DSIO_ENDREP;          break;
    case "if"_u64:                  token = D3DSIO_IF;              break;
    case "ifc"_u64:                 token = D3DSIO_IFC;             break;
    case "else"_u64:                token = D3DSIO_ELSE;            break;
    case "endif"_u64:               token = D3DSIO_ENDIF;           break;
    case "break"_u64:               token = D3DSIO_BREAK;           break;
    case "breakc"_u64:              token = D3DSIO_BREAKC;          break;
    case "mova"_u64:                token = D3DSIO_MOVA;            break;
    case "defb"_u64:                token = D3DSIO_DEFB;            break;
    case "defi"_u64:                token = D3DSIO_DEFI;            break;
#endif
    case "texcoord"_u64:
    case "texcrd"_u64:              token = D3DSIO_TEXCOORD;        break;
    case "texkill"_u64:             token = D3DSIO_TEXKILL;         break;
    case "tex"_u64:
    case "texld"_u64:               token = D3DSIO_TEX;             break;
    case "texbem"_u64:              token = D3DSIO_TEXBEM;          break;
    case "texbeml"_u64:             token = D3DSIO_TEXBEML;         break;
    case "texreg2ar"_u64:           token = D3DSIO_TEXREG2AR;       break;
    case "texreg2gb"_u64:           token = D3DSIO_TEXREG2GB;       break;
    case "texm3x2pad"_u64:          token = D3DSIO_TEXM3x2PAD;      break;
    case "texm3x2tex"_u64:          token = D3DSIO_TEXM3x2TEX;      break;
    case "texm3x3pad"_u64:          token = D3DSIO_TEXM3x3PAD;      break;
    case "texm3x3tex"_u64:          token = D3DSIO_TEXM3x3TEX;      break;
    case "texm3x3diff"_u64:         token = D3DSIO_TEXM3x3DIFF;     break;
    case "texm3x3spec"_u64:         token = D3DSIO_TEXM3x3SPEC;     break;
    case "texm3x3vspec"_u64:        token = D3DSIO_TEXM3x3VSPEC;    break;
    case "expp"_u64:                token = D3DSIO_EXPP;            break;
    case "logp"_u64:                token = D3DSIO_LOGP;            break;
    case "cnd"_u64:                 token = D3DSIO_CND;             break;
    case "def"_u64:                 token = D3DSIO_DEF;             break;
    case "texreg2rgb"_u64:          token = D3DSIO_TEXREG2RGB;      break;
    case "texdp3tex"_u64:           token = D3DSIO_TEXDP3TEX;       break;
    case "texm3x2depth"_u64:        token = D3DSIO_TEXM3x2DEPTH;    break;
    case "texdp3"_u64:              token = D3DSIO_TEXDP3;          break;
    case "texm3x3"_u64:             token = D3DSIO_TEXM3x3;         break;
    case "texdepth"_u64:            token = D3DSIO_TEXDEPTH;        break;
    case "cmp"_u64:                 token = D3DSIO_CMP;             break;
    case "bem"_u64:                 token = D3DSIO_BEM;             break;
#if (DIRECT3D_VERSION >= 0x0900)
    case "dp2add"_u64:              token = D3DSIO_DP2ADD;          break;
    case "dsx"_u64:                 token = D3DSIO_DSX;             break;
    case "dsy"_u64:                 token = D3DSIO_DSY;             break;
    case "texldd"_u64:              token = D3DSIO_TEXLDD;          break;
    case "setp"_u64:                token = D3DSIO_SETP;            break;
    case "texldl"_u64:              token = D3DSIO_TEXLDL;          break;
    case "breakp"_u64:              token = D3DSIO_BREAKP;          break;
#endif
    case "phase"_u64:               token = D3DSIO_PHASE;           break;
    case "end"_u64:                 token = D3DSIO_END;             break;
    default:
        return SIZE_MAX;
    }
    if (coissue)
    {
        token |= D3DSI_COISSUE;
    }
    tokens[index++] = token;
    if (dcl != UINT32_MAX)
    {
        tokens[index++] = (dcl | 0x80000000);
    }

    // Constant
    switch (token)
    {
    case D3DSIO_DEF:
        tokens[index++] = Register(dst, false, 0, false, false);
        for (size_t i = 0; i < 4; ++i)
        {
            float value = strtof(src[i] ? src[i] : "", NULL);
            memcpy(&tokens[index++], &value, sizeof(float));
        }
        return index;
#if (DIRECT3D_VERSION >= 0x0900)
    case D3DSIO_DEFB:
        tokens[index++] = Register(dst, false, 0, false, false);
        for (size_t i = 0; i < 4; ++i)
        {
            int value = strcmp(src[i] ? src[i] : "false", "true") ? 0 : 1;
            memcpy(&tokens[index++], &value, sizeof(int));
        }
        return index;
    case D3DSIO_DEFI:
        tokens[index++] = Register(dst, false, 0, false, false);
        for (size_t i = 0; i < 4; ++i)
        {
            long value = strtol(src[i] ? src[i] : "", NULL, 10);
            memcpy(&tokens[index++], &value, sizeof(int));
        }
        return index;
#endif
    }

    tokens[index++] = Register(dst, false, shift, sat, pp);
    for (size_t i = 0; i < 3; ++i)
    {
        tokens[index] = Register(src[i], true, 0, false, false);
        if ((tokens[index] & 0x80000000) == 0)
            break;
        index++;
    }

    return index;
}
//==============================================================================
