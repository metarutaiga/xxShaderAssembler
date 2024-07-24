// Patent No.: US 6,198,488 B1
// Appl. No.: 09/454,516
// Filed: Dec. 6, 1999
// Date of Patent: Mar. 6, 2001
// Assignee: NVidia, Santa Clara, CA (US)
#include "pch.h"
#include "cheops.h"

#define D3DSI_GETREGNUM(token)      (token & D3DSP_REGNUM_MASK)
#define D3DSI_GETOPCODE(token)      (token & D3DSI_OPCODE_MASK)
#define D3DSI_GETWRITEMASK(token)   (token & D3DSP_WRITEMASK_ALL)
#define D3DVS_GETSWIZZLE(token)     (token & D3DVS_SWIZZLE_MASK)
#define D3DVS_GETSRCMODIFIER(token) (token & D3DSP_SRCMOD_MASK)
#define D3DSI_GETREGTYPE(token)     ((D3DSHADER_PARAM_REGISTER_TYPE)(((token & D3DSP_REGTYPE_MASK) >> D3DSP_REGTYPE_SHIFT) | ((token & D3DSP_REGTYPE_MASK2) >> D3DSP_REGTYPE_SHIFT2)))

#define _PRINTF(format, ...) snprintf(text, size, "%s" format, text, __VA_ARGS__)
#define _OPTIMIZE 1

int (*PrintfCheops)(char const* format, ...) = printf;

static void PackCheops(void* destination, void* source)
{
    uint32_t* left = (uint32_t*)source;
    uint32_t* right = (uint32_t*)destination;
    right[0] = left[1];
    right[1] = left[0];
}

static void DefaultCheops(struct CheopsMicrocode* microcode)
{
    microcode->mr = 0;
    microcode->ca = 0;
    microcode->ce = 0;
    microcode->va = 0;
    microcode->mia = CHEOPS_CMLU_MA_R;
    microcode->mib = CHEOPS_CMLU_MB_R;
    microcode->mlu = CHEOPS_CMLU_PASA;
    microcode->aia = CHEOPS_CALU_AA_A;
    microcode->ais = 0;
    microcode->alu = CHEOPS_CALU_PASB;
    microcode->ilu = CHEOPS_CILU_NOP;
    microcode->rwa = 0;
    microcode->rwm = 0;
    microcode->rra = 0;
    microcode->oa = CHEOPS_COUT_TVW_NOP;
}

static void SourceRegisterCheopsFromD3DSI(struct CheopsMicrocode* microcode, uint32_t source)
{
    if (D3DSI_GETREGTYPE(source) == D3DSPR_CONST)
    {
        microcode->mib = CHEOPS_CMLU_MB_C;
        microcode->mlu = CHEOPS_CMLU_PASB;
        microcode->ca = D3DSI_GETREGNUM(source);
    }
    else if (D3DSI_GETREGTYPE(source) == D3DSPR_INPUT)
    {
        microcode->mia = CHEOPS_CMLU_MA_V;
        microcode->mlu = CHEOPS_CMLU_PASA;
        microcode->va = D3DSI_GETREGNUM(source);
    }
    else
    {
        microcode->mia = CHEOPS_CMLU_MA_R;
        microcode->mlu = CHEOPS_CMLU_PASA;
        microcode->rra = D3DSI_GETREGNUM(source);
    }
    microcode->ais = D3DVS_GETSRCMODIFIER(source) == D3DSPSM_NEG ? 2 : 0;
    microcode->alu = CHEOPS_CALU_PASB;
    switch (D3DVS_GETSWIZZLE(source))
    {
    case D3DSP_NOSWIZZLE:
        break;
    case D3DSP_REPLICATERED:
        microcode->alu = CHEOPS_CALU_SMRB0;
        break;
    case D3DSP_REPLICATEGREEN:
        microcode->alu = CHEOPS_CALU_SMRB1;
        break;
    case D3DSP_REPLICATEBLUE:
        microcode->alu = CHEOPS_CALU_SMRB2;
        break;
    case D3DSP_REPLICATEALPHA:
        microcode->alu = CHEOPS_CALU_SMRB3;
        break;
    default:
        PrintfCheops("Unknown swizzle mask (%08x)\n", D3DVS_GETSWIZZLE(source));
        break;
    }
}

static void DestinationRegisterCheopsFromD3DSI(struct CheopsMicrocode* microcode, uint32_t destination)
{
    switch (D3DSI_GETREGTYPE(destination))
    {
    case D3DSPR_TEMP:
        microcode->rwa = D3DSI_GETREGNUM(destination);
        microcode->rwm = D3DSI_GETWRITEMASK(destination) >> 16;
        microcode->oa = CHEOPS_COUT_TVW_NOP;
        break;
    case D3DSPR_RASTOUT:
        microcode->rwa = 0;
        microcode->rwm = 0;
        microcode->oa = CHEOPS_COUT_TPOS;
        break;
    case D3DSPR_ATTROUT:
        microcode->rwa = 0;
        microcode->rwm = 0;
        microcode->oa = CHEOPS_COUT_VC0 + D3DSI_GETREGNUM(destination);
        break;
    case D3DSPR_TEXCRDOUT:
        microcode->rwa = 0;
        microcode->rwm = 0;
        microcode->oa = CHEOPS_COUT_TT0 + D3DSI_GETREGNUM(destination);
        break;
    }
}

size_t CompileCheopsFromD3DSI(uint64_t cheops[4], uint32_t d3dsi[8])
{
    size_t index = 0;
    size_t count = 0;
    uint8_t MLU_TEMP = 63;
    uint8_t RLU_TEMP = 7;
    uint8_t RLU_WRITEMASK_ALL = 0b1111;
    struct CheopsMicrocode NOP = {};
    struct CheopsMicrocode EMPTY = {};
    struct CheopsMicrocode microcodes[4];
    DefaultCheops(&NOP);
    memset(&EMPTY, 0xFF, sizeof(EMPTY));
    microcodes[0] = EMPTY;
    microcodes[1] = EMPTY;
    microcodes[2] = EMPTY;
    microcodes[3] = EMPTY;

    switch (D3DSI_GETOPCODE(d3dsi[0]))
    {
    case D3DSIO_MOV:
        microcodes[0] = NOP;
        DestinationRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[1]);
        SourceRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[2]);
        count = 1;
        break;
    case D3DSIO_ADD:
        // [2] ADD r0, r0, r0
        // [2] ADD r0, r0, v0
        // [1] ADD r0, r0, c0
        // [2] ADD r0, v0, v0
        // [1] ADD r0, v0, c0
        // [2] ADD r0, c0, c0 // 1 instruction when context read address is the same
#if _OPTIMIZE
        if (D3DVS_GETSWIZZLE(d3dsi[2]) != D3DVS_NOSWIZZLE || D3DVS_GETSWIZZLE(d3dsi[3]) != D3DVS_NOSWIZZLE)
        {
            // Fallback
        }
        else if (D3DSI_GETREGTYPE(d3dsi[2]) == D3DSPR_CONST && D3DSI_GETREGTYPE(d3dsi[3]) == D3DSPR_CONST && D3DSI_GETREGNUM(d3dsi[2]) != D3DSI_GETREGNUM(d3dsi[3]))
        {
            // Fallback
        }
        else if (D3DSI_GETREGTYPE(d3dsi[2]) == D3DSPR_CONST || D3DSI_GETREGTYPE(d3dsi[3]) == D3DSPR_CONST)
        {
            microcodes[0] = NOP;
            DestinationRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[1]);
            if (D3DSI_GETREGTYPE(d3dsi[2]) == D3DSPR_CONST)
            {
                SourceRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[3]);
                microcodes[0].ais |= D3DVS_GETSRCMODIFIER(d3dsi[2]) == D3DSPSM_NEG ? 1 : 0;
                microcodes[0].ca = D3DSI_GETREGNUM(d3dsi[2]);
            }
            else
            {
                SourceRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[2]);
                microcodes[0].ais |= D3DVS_GETSRCMODIFIER(d3dsi[3]) == D3DSPSM_NEG ? 1 : 0;
                microcodes[0].ca = D3DSI_GETREGNUM(d3dsi[3]);
            }
            microcodes[0].aia = CHEOPS_CALU_AA_C;
            microcodes[0].alu = CHEOPS_CALU_ADDA;
            count = 1;
            break;
        }
#endif
        if (D3DVS_GETSWIZZLE(d3dsi[2]) != D3DSP_NOSWIZZLE && D3DVS_GETSWIZZLE(d3dsi[3]) != D3DSP_NOSWIZZLE)
        {
            microcodes[0] = NOP;
            microcodes[1] = NOP;
            microcodes[2] = NOP;
            DestinationRegisterCheopsFromD3DSI(&microcodes[2], d3dsi[1]);
            SourceRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[2]);
            SourceRegisterCheopsFromD3DSI(&microcodes[1], d3dsi[3]);
            microcodes[0].rwa = RLU_TEMP;
            microcodes[0].rwm = RLU_WRITEMASK_ALL;
            microcodes[2].aia = CHEOPS_CALU_AA_A;
            microcodes[2].alu = CHEOPS_CALU_ADDA;
            microcodes[2].rra = RLU_TEMP;
            count = 3;
        }
        else
        {
            microcodes[0] = NOP;
            microcodes[1] = NOP;
            DestinationRegisterCheopsFromD3DSI(&microcodes[1], d3dsi[1]);
            if (D3DVS_GETSWIZZLE(d3dsi[2]) != D3DSP_NOSWIZZLE)
            {
                SourceRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[2]);
                SourceRegisterCheopsFromD3DSI(&microcodes[1], d3dsi[3]);
            }
            else
            {
                SourceRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[3]);
                SourceRegisterCheopsFromD3DSI(&microcodes[1], d3dsi[2]);
            }
            microcodes[1].aia = CHEOPS_CALU_AA_A;
            microcodes[1].alu = CHEOPS_CALU_ADDA;
            count = 2;
        }
        break;
    case D3DSIO_MAD:
        // [2] MAD r0, r0, r0, r0
        // [2] MAD r0, r0, r0, v0
        // [2] MAD r0, r0, r0, c0 // 1 instruction when RLU read address is the same
        // [2] MAD r0, r0, v0, r0
        // [2] MAD r0, r0, v0, v0
        // [1] MAD r0, r0, v0, c0
        // [2] MAD r0, r0, c0, r0
        // [2] MAD r0, r0, c0, v0
        // [2] MAD r0, r0, c0, c0 // 1 instruction when context read address is the same
        // [3] MAD r0, v0, v0, r0
        // [3] MAD r0, v0, v0, v0
        // [2] MAD r0, v0, v0, c0
        // [2] MAD r0, v0, c0, r0
        // [2] MAD r0, v0, c0, v0
        // [2] MAD r0, v0, c0, c0 // 1 instruction when context read address is the same
        // [3] MAD r0, c0, c0, r0
        // [3] MAD r0, c0, c0, v0
        // [3] MAD r0, c0, c0, c0 // 2 instructions when context read address is the same
    case D3DSIO_MUL:
    case D3DSIO_DP3:
    case D3DSIO_DP4:
        // [2] MUL r0, r0, r0 // 1 instruction when RLU read address is the same
        // [1] MUL r0, r0, v0
        // [1] MUL r0, r0, c0
        // [2] MUL r0, v0, v0
        // [1] MUL r0, v0, c0
        // [2] MUL r0, c0, c0
#if _OPTIMIZE
        if (D3DVS_GETSWIZZLE(d3dsi[2]) != D3DVS_NOSWIZZLE || D3DVS_GETSWIZZLE(d3dsi[3]) != D3DVS_NOSWIZZLE)
        {
            // Fallback
        }
        else if (D3DSI_GETREGTYPE(d3dsi[2]) == D3DSPR_TEMP && D3DSI_GETREGTYPE(d3dsi[3]) == D3DSPR_TEMP && D3DSI_GETREGNUM(d3dsi[2]) != D3DSI_GETREGNUM(d3dsi[3]))
        {
            // Fallback
        }
        else if (D3DSI_GETREGTYPE(d3dsi[2]) == D3DSPR_INPUT && D3DSI_GETREGTYPE(d3dsi[3]) == D3DSPR_INPUT)
        {
            // Fallback
        }
        else if (D3DSI_GETREGTYPE(d3dsi[2]) == D3DSPR_CONST && D3DSI_GETREGTYPE(d3dsi[3]) == D3DSPR_CONST)
        {
            // Fallback
        }
        else if ((D3DSI_GETREGTYPE(d3dsi[2]) == D3DSPR_TEMP || D3DSI_GETREGTYPE(d3dsi[2]) == D3DSPR_INPUT || D3DSI_GETREGTYPE(d3dsi[2]) == D3DSPR_CONST) &&
                 (D3DSI_GETREGTYPE(d3dsi[3]) == D3DSPR_TEMP || D3DSI_GETREGTYPE(d3dsi[3]) == D3DSPR_INPUT || D3DSI_GETREGTYPE(d3dsi[3]) == D3DSPR_CONST))
        {
            microcodes[0] = NOP;
            DestinationRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[1]);
            if ((D3DSI_GETREGTYPE(d3dsi[2]) == D3DSPR_TEMP || D3DSI_GETREGTYPE(d3dsi[2]) == D3DSPR_INPUT) && D3DSI_GETREGTYPE(d3dsi[3]) != D3DSPR_INPUT)
            {
                SourceRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[2]);
                if (D3DSI_GETREGTYPE(d3dsi[3]) == D3DSPR_CONST)
                {
                    microcodes[0].mib = CHEOPS_CMLU_MB_C;
                    microcodes[0].ca = D3DSI_GETREGNUM(d3dsi[3]);
                }
                else
                {
                    microcodes[0].mib = CHEOPS_CMLU_MB_R;
                    microcodes[0].rra = D3DSI_GETREGNUM(d3dsi[3]);
                }
            }
            else
            {
                SourceRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[3]);
                if (D3DSI_GETREGTYPE(d3dsi[2]) == D3DSPR_CONST)
                {
                    microcodes[0].mib = CHEOPS_CMLU_MB_C;
                    microcodes[0].ca = D3DSI_GETREGNUM(d3dsi[2]);
                }
                else
                {
                    microcodes[0].mib = CHEOPS_CMLU_MB_R;
                    microcodes[0].rra = D3DSI_GETREGNUM(d3dsi[2]);
                }
            }
            microcodes[0].ais = (D3DVS_GETSRCMODIFIER(d3dsi[2]) + D3DVS_GETSRCMODIFIER(d3dsi[3])) == D3DSPSM_NEG ? 2 : 0;
            microcodes[0].mlu = CHEOPS_CMLU_MULT;
            switch (D3DSI_GETOPCODE(d3dsi[0]))
            {
            case D3DSIO_MAD:    goto mad;                               break;
            case D3DSIO_DP3:    microcodes[0].alu = CHEOPS_CALU_SUM3B;  break;
            case D3DSIO_DP4:    microcodes[0].alu = CHEOPS_CALU_SUM4B;  break;
            }
            count = 1;
            break;
        }
#endif
        if (D3DVS_GETSWIZZLE(d3dsi[2]) != D3DSP_NOSWIZZLE && D3DVS_GETSWIZZLE(d3dsi[3]) != D3DSP_NOSWIZZLE)
        {
            if (D3DSI_GETREGTYPE(d3dsi[2]) == D3DSPR_CONST && D3DSI_GETREGTYPE(d3dsi[3]) == D3DSPR_CONST)
            {
                PrintfCheops("Unimplemented\n");
                return 0;
            }
            microcodes[0] = NOP;
            microcodes[1] = NOP;
            microcodes[2] = NOP;
            DestinationRegisterCheopsFromD3DSI(&microcodes[2], d3dsi[1]);
            if (D3DSI_GETREGTYPE(d3dsi[2]) == D3DSPR_CONST)
            {
                SourceRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[2]);
                SourceRegisterCheopsFromD3DSI(&microcodes[1], d3dsi[3]);
            }
            else
            {
                SourceRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[3]);
                SourceRegisterCheopsFromD3DSI(&microcodes[1], d3dsi[2]);
            }
            microcodes[0].rwa = RLU_TEMP;
            microcodes[0].rwm = RLU_WRITEMASK_ALL;
            microcodes[1].ca = MLU_TEMP;
            microcodes[1].ce = TRUE;
            microcodes[2].mlu = CHEOPS_CMLU_MULT;
            microcodes[2].mib = CHEOPS_CMLU_MB_C;
            microcodes[2].ca = MLU_TEMP;
            microcodes[2].rra = RLU_TEMP;
            switch (D3DSI_GETOPCODE(d3dsi[0]))
            {
            case D3DSIO_DP3:    microcodes[2].alu = CHEOPS_CALU_SUM3B;  break;
            case D3DSIO_DP4:    microcodes[2].alu = CHEOPS_CALU_SUM4B;  break;
            }
            count = 3;
        }
        else
        {
            microcodes[0] = NOP;
            microcodes[1] = NOP;
            DestinationRegisterCheopsFromD3DSI(&microcodes[1], d3dsi[1]);
            if (D3DVS_GETSWIZZLE(d3dsi[2]) != D3DSP_NOSWIZZLE)
            {
                SourceRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[2]);
                SourceRegisterCheopsFromD3DSI(&microcodes[1], d3dsi[3]);
            }
            else
            {
                SourceRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[3]);
                SourceRegisterCheopsFromD3DSI(&microcodes[1], d3dsi[2]);
            }
            microcodes[0].rwa = RLU_TEMP;
            microcodes[0].rwm = RLU_WRITEMASK_ALL;
            microcodes[1].mlu = CHEOPS_CMLU_MULT;
            switch (D3DSI_GETOPCODE(d3dsi[0]))
            {
            case D3DSIO_DP3:    microcodes[1].alu = CHEOPS_CALU_SUM3B;  break;
            case D3DSIO_DP4:    microcodes[1].alu = CHEOPS_CALU_SUM4B;  break;
            }
            microcodes[1].rra = RLU_TEMP;
            count = 2;
        }
        if (D3DSI_GETOPCODE(d3dsi[0]) == D3DSIO_MAD)
        {
            index = count - 1;
mad:
            if (D3DSI_GETREGTYPE(d3dsi[4]) == D3DSPR_CONST &&
                ((D3DSI_GETREGTYPE(d3dsi[2]) != D3DSPR_CONST || D3DSI_GETREGNUM(d3dsi[2]) == D3DSI_GETREGNUM(d3dsi[4])) &&
                 (D3DSI_GETREGTYPE(d3dsi[3]) != D3DSPR_CONST || D3DSI_GETREGNUM(d3dsi[3]) == D3DSI_GETREGNUM(d3dsi[4]))))
            {
                microcodes[index].aia = CHEOPS_CALU_AA_C;
                microcodes[index].ais |= D3DVS_GETSRCMODIFIER(d3dsi[4]) == D3DSPSM_NEG ? 1 : 0;
                microcodes[index].alu = CHEOPS_CALU_ADDA;
                microcodes[index].ca = D3DSI_GETREGNUM(d3dsi[4]);
                count = index + 1;
            }
            else
            {
                microcodes[index + 1] = NOP;
                SourceRegisterCheopsFromD3DSI(&microcodes[index + 1], d3dsi[4]);
                microcodes[index + 1].aia = CHEOPS_CALU_AA_A;
                microcodes[index + 1].ais |= D3DVS_GETSRCMODIFIER(d3dsi[4]) == D3DSPSM_NEG ? 1 : 0;
                microcodes[index + 1].alu = CHEOPS_CALU_ADDA;
                microcodes[index + 1].rwa = microcodes[index + 0].rwa;
                microcodes[index + 1].rwm = microcodes[index + 0].rwm;
                microcodes[index + 0].rwa = 0;
                microcodes[index + 0].rwm = 0;
                count = index + 2;
            }
        }
        break;
    case D3DSIO_RCP:
    case D3DSIO_RSQ:
        microcodes[0] = NOP;
        microcodes[1] = NOP;
        DestinationRegisterCheopsFromD3DSI(&microcodes[1], d3dsi[1]);
        SourceRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[2]);
        switch (D3DSI_GETOPCODE(d3dsi[0]))
        {
        case D3DSIO_RCP:    microcodes[0].ilu = CHEOPS_CILU_INV;    break;
        case D3DSIO_RSQ:    microcodes[0].ilu = CHEOPS_CILU_ISQ;    break;
        }
        microcodes[1].mib = CHEOPS_CMLU_MB_I;
        microcodes[1].mlu = CHEOPS_CMLU_PASB;
        count = 2;
        break;
    case D3DSIO_MIN:
    case D3DSIO_MAX:
    case D3DSIO_SLT:
    case D3DSIO_SGE:
    case D3DSIO_EXP:
    case D3DSIO_LOG:
    case D3DSIO_LIT:
    case D3DSIO_DST:
    case D3DSIO_FRC:
    case D3DSIO_M4x4:
    case D3DSIO_M4x3:
    case D3DSIO_M3x4:
    case D3DSIO_M3x3:
    case D3DSIO_M3x2:
        PrintfCheops("Unimplemented\n");
        return 0;
    }

    size_t reduced = 0;
    for (size_t i = 0; i < count; ++i)
    {
        if (memcmp(microcodes + i, &EMPTY, sizeof(EMPTY)) == 0)
            continue;
        PackCheops(cheops + reduced, microcodes + i);
        reduced++;
    }
    return reduced;
}

static void PrintMLUInputA(char* text, size_t size, struct CheopsMicrocode microcode)
{
    switch (microcode.mia)
    {
    case CHEOPS_CMLU_MA_M:  _PRINTF("%s", "mlu");                   break;
    case CHEOPS_CMLU_MA_V:  _PRINTF("%s%d", "v", microcode.va);     break;
    case CHEOPS_CMLU_MA_R:  _PRINTF("%s%d", "r", microcode.rra);    break;
    }
}

static void PrintMLUInputB(char* text, size_t size, struct CheopsMicrocode microcode)
{
    switch (microcode.mib)
    {
    case CHEOPS_CMLU_MB_I:  _PRINTF("%s", "ilu");                   break;
    case CHEOPS_CMLU_MB_C:  _PRINTF("%s%d", "c", microcode.ca);     break;
    case CHEOPS_CMLU_MB_R:  _PRINTF("%s%d", "r", microcode.rra);    break;
    }
}

static void PrintALUInputA(char* text, size_t size, struct CheopsMicrocode microcode)
{
    switch (microcode.aia)
    {
    case CHEOPS_CALU_AA_A:  _PRINTF("%s", "alu");                   break;
    case CHEOPS_CALU_AA_C:  _PRINTF("%s%d", "c", microcode.ca);     break;
    }
}

static void PrintALUSwizzle(char* text, size_t size, struct CheopsMicrocode microcode)
{
    if (microcode.alu == CHEOPS_CALU_SMRB0) _PRINTF("%s", ".xxxx");
    if (microcode.alu == CHEOPS_CALU_SMRB1) _PRINTF("%s", ".yyyy");
    if (microcode.alu == CHEOPS_CALU_SMRB2) _PRINTF("%s", ".zzzz");
    if (microcode.alu == CHEOPS_CALU_SMRB3) _PRINTF("%s", ".wwww");
}

static void PrintALUOutput(char* text, size_t size, struct CheopsMicrocode microcode)
{
    if (microcode.ilu != CHEOPS_CILU_NOP)
    {
        _PRINTF("%s", "ilu");
        if (microcode.rwm != 0b0000)
        {
            PrintfCheops("%s is found\n", "RLU ADDRESS");
        }
        if (microcode.oa < CHEOPS_COUT_TVW_NOP)
        {
            PrintfCheops("%s is found\n", "OUTPUT BUFFER ADDRESS");
        }
    }
    else if (microcode.rwm != 0b0000)
    {
        _PRINTF("%s%d", "r", microcode.rwa);
        if (microcode.rwm != 0b1111)
        {
            _PRINTF("%s", ".");
            if (microcode.rwm & 0b0001) _PRINTF("%s", "x");
            if (microcode.rwm & 0b0010) _PRINTF("%s", "y");
            if (microcode.rwm & 0b0100) _PRINTF("%s", "z");
            if (microcode.rwm & 0b1000) _PRINTF("%s", "w");
        }
    }
    else if (microcode.oa < CHEOPS_COUT_TVW_NOP)
    {
        static char const* const address[CHEOPS_COUT_TVW_NOP] =
        {
            "tpos", "tt0",  "tt1",  "wev",  "wlv0", "wlv1", "wlv2", "wlv3",
            "wlv4", "wlv5", "wlv6", "wlv7", "wsl0", "wsl1", "wsl2", "wsl3",
            "wsl4", "wsl5", "wsl6", "wsl7", "ved",  "vld0", "vld1", "vld2",
            "vld3", "vld4", "vld5", "vld6", "vld7", "vc0",  "vc1",  "vnrm",
            "ved2",
        };
        _PRINTF("%s", address[microcode.oa]);
    }
    else if (microcode.ce == TRUE)
    {
        _PRINTF("%s%d", "c", microcode.ca);
    }
    else
    {
        _PRINTF("%s", "alu");
    }
}

void DisasembleCheops(char* text, size_t size, uint64_t cheops)
{
    struct CheopsMicrocode microcode;
    PackCheops(&microcode, &cheops);

    // Reset
    text[0] = 0;

    // Operation
    switch (microcode.alu)
    {
    case CHEOPS_CALU_ADDA:
    case CHEOPS_CALU_ADDB:
        switch (microcode.mlu)
        {
        case CHEOPS_CMLU_MULT:
        case CHEOPS_CMLU_MULA:
        case CHEOPS_CMLU_MULB:
            _PRINTF("%s", "mad");
            _PRINTF("%s", " ");
            PrintALUOutput(text, size, microcode);
            _PRINTF("%s", ", ");
            PrintMLUInputB(text, size, microcode);
            _PRINTF("%s", ", ");
            _PRINTF("%s", microcode.ais & 0b10 ? "-" : "");
            PrintMLUInputA(text, size, microcode);
            _PRINTF("%s", ", ");
            _PRINTF("%s", microcode.ais & 0b01 ? "-" : "");
            PrintALUInputA(text, size, microcode);
            break;
        case CHEOPS_CMLU_PASA:
        case CHEOPS_CMLU_PASB:
            _PRINTF("%s", "add");
            _PRINTF("%s", " ");
            PrintALUOutput(text, size, microcode);
            _PRINTF("%s", ", ");
            _PRINTF("%s", microcode.ais & 0b01 ? "-" : "");
            PrintALUInputA(text, size, microcode);
            _PRINTF("%s", ", ");
            _PRINTF("%s", microcode.ais & 0b10 ? "-" : "");
            switch (microcode.mlu)
            {
            case CHEOPS_CMLU_PASA:  PrintMLUInputA(text, size, microcode);  break;
            case CHEOPS_CMLU_PASB:  PrintMLUInputB(text, size, microcode);  break;
            }
            break;
        }
        break;
    case CHEOPS_CALU_PASA:
        _PRINTF("%s", "mov");
        _PRINTF("%s", " ");
        PrintALUOutput(text, size, microcode);
        _PRINTF("%s", ", ");
        _PRINTF("%s", microcode.ais & 0b01 ? "-" : "");
        PrintALUInputA(text, size, microcode);
        break;
    case CHEOPS_CALU_SUM3B:
    case CHEOPS_CALU_SUM4B:
    case CHEOPS_CALU_SMRB0:
    case CHEOPS_CALU_SMRB1:
    case CHEOPS_CALU_SMRB2:
    case CHEOPS_CALU_SMRB3:
    case CHEOPS_CALU_PASB:
        switch (microcode.mlu)
        {
        case CHEOPS_CMLU_MULT:
        case CHEOPS_CMLU_MULA:
        case CHEOPS_CMLU_MULB:
            switch (microcode.alu)
            {
            default:                _PRINTF("%s", "mul");   break;
            case CHEOPS_CALU_SUM3B: _PRINTF("%s", "dp3");   break;
            case CHEOPS_CALU_SUM4B: _PRINTF("%s", "dp4");   break;
            }
            _PRINTF("%s", " ");
            PrintALUOutput(text, size, microcode);
            _PRINTF("%s", ", ");
            PrintMLUInputB(text, size, microcode);
            PrintALUSwizzle(text, size, microcode);
            _PRINTF("%s", ", ");
            _PRINTF("%s", microcode.ais & 0b10 ? "-" : "");
            PrintMLUInputA(text, size, microcode);
            PrintALUSwizzle(text, size, microcode);
            break;
        case CHEOPS_CMLU_PASA:
        case CHEOPS_CMLU_PASB:
            switch (microcode.alu)
            {
            default:                _PRINTF("%s", "mov");   break;
            case CHEOPS_CALU_SUM3B: _PRINTF("%s", "sum3");  break;
            case CHEOPS_CALU_SUM4B: _PRINTF("%s", "sum4");  break;
            case CHEOPS_CALU_SMRB0:
            case CHEOPS_CALU_SMRB1:
            case CHEOPS_CALU_SMRB2:
            case CHEOPS_CALU_SMRB3:
            case CHEOPS_CALU_PASB:
                switch (microcode.ilu)
                {
                default:                _PRINTF("%s", "mov");   break;
                case CHEOPS_CILU_INV:   _PRINTF("%s", "rcp");   break;
                case CHEOPS_CILU_ISQ:   _PRINTF("%s", "rsq");   break;
                case CHEOPS_CILU_CINV:  _PRINTF("%s", "rcp");   break;
                }
                break;
            }
            _PRINTF("%s", " ");
            PrintALUOutput(text, size, microcode);
            _PRINTF("%s", ", ");
            _PRINTF("%s", microcode.ais & 0b10 ? "-" : "");
            switch (microcode.mlu)
            {
            case CHEOPS_CMLU_PASA:  PrintMLUInputA(text, size, microcode);  break;
            case CHEOPS_CMLU_PASB:  PrintMLUInputB(text, size, microcode);  break;
            }
            PrintALUSwizzle(text, size, microcode);
            break;
        }
        break;
    }
}
