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
    size_t count = 0;
    uint8_t RLU_TEMP = 7;
    uint8_t RLU_WRITEMASK_ALL = 0b1111;
    struct CheopsMicrocode microcodes[4] = {};

    switch (D3DSI_GETOPCODE(d3dsi[0]))
    {
    case D3DSIO_MOV:
        DefaultCheops(&microcodes[0]);
        DestinationRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[1]);
        SourceRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[2]);
        count = 1;
        break;
    case D3DSIO_ADD:
        DefaultCheops(&microcodes[0]);
        DefaultCheops(&microcodes[1]);
        DestinationRegisterCheopsFromD3DSI(&microcodes[1], d3dsi[1]);
        SourceRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[2]);
        SourceRegisterCheopsFromD3DSI(&microcodes[1], d3dsi[3]);
        microcodes[1].aia = CHEOPS_CALU_AA_A;
        microcodes[1].alu = CHEOPS_CALU_ADDA;
        count = 2;
        break;
    case D3DSIO_MAD:
        DefaultCheops(&microcodes[0]);
        DefaultCheops(&microcodes[1]);
        DefaultCheops(&microcodes[2]);
        DestinationRegisterCheopsFromD3DSI(&microcodes[2], d3dsi[1]);
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
        SourceRegisterCheopsFromD3DSI(&microcodes[2], d3dsi[4]);
        microcodes[0].rwa = RLU_TEMP;
        microcodes[0].rwm = RLU_WRITEMASK_ALL;
        microcodes[1].mlu = CHEOPS_CMLU_MULT;
        microcodes[1].rra = RLU_TEMP;
        microcodes[2].aia = CHEOPS_CALU_AA_A;
        microcodes[2].alu = CHEOPS_CALU_ADDA;
        count = 3;
        break;
    case D3DSIO_MUL:
        DefaultCheops(&microcodes[0]);
        DefaultCheops(&microcodes[1]);
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
        microcodes[0].oa = CHEOPS_COUT_TVW_NOP;
        microcodes[1].mlu = CHEOPS_CMLU_MULT;
        microcodes[1].rra = RLU_TEMP;
        count = 2;
        break;
    case D3DSIO_RCP:
        DefaultCheops(&microcodes[0]);
        DefaultCheops(&microcodes[1]);
        DestinationRegisterCheopsFromD3DSI(&microcodes[1], d3dsi[1]);
        SourceRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[2]);
        microcodes[0].ilu = CHEOPS_CILU_INV;
        microcodes[1].mib = CHEOPS_CMLU_MB_I;
        microcodes[1].mlu = CHEOPS_CMLU_PASB;
        count = 2;
        break;
    case D3DSIO_RSQ:
        DefaultCheops(&microcodes[0]);
        DefaultCheops(&microcodes[1]);
        DestinationRegisterCheopsFromD3DSI(&microcodes[1], d3dsi[1]);
        SourceRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[2]);
        microcodes[0].ilu = CHEOPS_CILU_ISQ;
        microcodes[1].mib = CHEOPS_CMLU_MB_I;
        microcodes[1].mlu = CHEOPS_CMLU_PASB;
        count = 2;
        break;
    case D3DSIO_DP3:
        DefaultCheops(&microcodes[0]);
        DefaultCheops(&microcodes[1]);
        DestinationRegisterCheopsFromD3DSI(&microcodes[1], d3dsi[1]);
        SourceRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[2]);
        SourceRegisterCheopsFromD3DSI(&microcodes[1], d3dsi[3]);
        microcodes[0].rwa = RLU_TEMP;
        microcodes[0].rwm = RLU_WRITEMASK_ALL;
        microcodes[1].alu = CHEOPS_CALU_SUM3B;
        microcodes[1].rra = RLU_TEMP;
        count = 2;
        break;
    case D3DSIO_DP4:
        DefaultCheops(&microcodes[0]);
        DefaultCheops(&microcodes[1]);
        DestinationRegisterCheopsFromD3DSI(&microcodes[1], d3dsi[1]);
        SourceRegisterCheopsFromD3DSI(&microcodes[0], d3dsi[2]);
        SourceRegisterCheopsFromD3DSI(&microcodes[1], d3dsi[3]);
        microcodes[0].rwa = RLU_TEMP;
        microcodes[0].rwm = RLU_WRITEMASK_ALL;
        microcodes[1].alu = CHEOPS_CALU_SUM4B;
        microcodes[1].rra = RLU_TEMP;
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
        break;
    }

    memcpy(cheops, microcodes, sizeof(uint64_t) * count);
    return count;
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
    if (microcode.rwm != 0b0000)
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
    else
    {
        _PRINTF("%s", "alu");
    }
}

void DisasembleCheops(char* text, size_t size, uint64_t cheops)
{
    struct CheopsMicrocode microcode;
    memcpy(&microcode, &cheops, sizeof(uint64_t));

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
            case CHEOPS_CALU_SUM3B: _PRINTF("%s", "dp3");   break;
            case CHEOPS_CALU_SUM4B: _PRINTF("%s", "dp4");   break;
            default:                _PRINTF("%s", "mul");   break;
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
            case CHEOPS_CALU_SUM3B:
            case CHEOPS_CALU_SUM4B: _PRINTF("%s", "sum");   break;
            default:                _PRINTF("%s", "mov");   break;
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
