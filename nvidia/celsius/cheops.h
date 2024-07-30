// Patent No.: US 6,198,488 B1
// Appl. No.: 09/454,516
// Filed: Dec. 6, 1999
// Date of Patent: Mar. 6, 2001
// Assignee: NVidia, Santa Clara, CA (US)
#pragma once

#if __cplusplus
extern "C" {
#endif

extern int (*PrintfCheops)(char const* format, ...);
size_t CompileCheopsFromD3DSI(uint64_t cheops[8], uint32_t const d3dsi[8]);
void DisasembleCheops(char* text, size_t size, uint64_t cheops);

struct CheopsMicrocode
{
    uint32_t  oa:6; // 0:00 OUTPUT BUFFER WRITE ADDRESS
    uint32_t rra:3; // 0:06 RLU READ ADDRESS
    uint32_t rwm:4; // 0:09 RLU WRITE MASK
    uint32_t rwa:3; // 0:13 RLU WRITE ADDRESS
    uint32_t ilu:2; // 0:16 ILU OPERATION
    uint32_t alu:4; // 0:18 ALU OPERATION
    uint32_t ais:2; // 0:22 ALU SIGN CONTROL
    uint32_t aia:1; // 0:24 ALU INPUT A MUX
    uint32_t mlu:3; // 0:25 MLU OPERATION
    uint32_t mib:2; // 0:28 MLU INPUT B MUX
    uint32_t mia:2; // 0:30 MLU INPUT A MUX
    uint32_t  va:3; // 1:32 INPUT BUFFER READ ADDRESS
    uint32_t  ce:1; // 1:35 CONTEXT MEMORY READ/WRITE
    uint32_t  ca:6; // 1:36 CONTEXT MEMORY ADDRESS
    uint32_t  mr:2; // 1:42 MLU INPUT VECTOR ROTATE
};

// MLU OPERATION
#define CHEOPS_CMLU_MULT    0   // o[0] = a[0]*b[0],o[1] = a[1]*b[1],o[2] = a[2]*b[2],o[3] = a[3]*b[3]
#define CHEOPS_CMLU_MULA    1   // o[0] = a[0]*b[0],o[1] = a[1]*b[1],o[2] = a[2]*b[2],o[3] = a[3]
#define CHEOPS_CMLU_MULB    2   // o[0] = a[0]*b[0],o[1] = a[1]*b[1],o[2] = a[2]*b[2],o[3] = b[3]
#define CHEOPS_CMLU_PASA    3   // o[0] = a[0],o[1] = a[1],o[2] = a[2],o[3] = a[3]
#define CHEOPS_CMLU_PASB    4   // o[0] = b[0],o[1] = b[1],o[2] = b[2],o[3] = b[3]

// MLU INPUT
#define CHEOPS_CMLU_MA_M    0   // MLU
#define CHEOPS_CMLU_MA_V    1   // Input Buffer
#define CHEOPS_CMLU_MA_R    2   // RLU (shared with MB_R)
#define CHEOPS_CMLU_MB_I    0   // ILU
#define CHEOPS_CMLU_MB_C    1   // Context Memory
#define CHEOPS_CMLU_MB_R    2   // RLU (shared with MA_R)

// MLU ROTATE
#define CHEOPS_CMLU_MR_NONE 0   // No change
#define CHEOPS_CMLU_MR_ALBR 1   // Rotate A[XYZ] vector left, B[XYZ] vector right
#define CHEOPS_CMLU_MR_ARBL 2   // Rotate A[XYZ] vector right, B[XYZ] vector left

// ALU OPERATION
#define CHEOPS_CALU_ADDA    0   // o[0] = a[0]+b[0],o[1] = a[1]+b[1],o[2] = a[2]+b[2],o[3] = a[3]
#define CHEOPS_CALU_ADDB    1   // o[0] = a[0]+b[0],o[1] = a[1]+b[1],o[2] = a[2]+b[2],o[3] = b[3]
#define CHEOPS_CALU_SUM3B   2   // o[0123] = b[0] + b[1] + b[2]
#define CHEOPS_CALU_SUM4B   3   // o[0123] = b[0] + b[1] + b[2] + b[3]
#define CHEOPS_CALU_SMRB0   4   // o[0123] = b[0]
#define CHEOPS_CALU_SMRB1   5   // o[0123] = b[1]
#define CHEOPS_CALU_SMRB2   6   // o[0123] = b[2]
#define CHEOPS_CALU_SMRB3   7   // o[0123] = b[3]
#define CHEOPS_CALU_PASA    8   // o[0] = a[0],o[1] = a[1],o[2] = a[2],o[3] = a[3]
#define CHEOPS_CALU_PASB    9   // o[0] = b[0],o[1] = b[1],o[2] = b[2],o[3] = b[3]

// ALU INPUT
#define CHEOPS_CALU_AA_A    0   // ALU (one instruction delay)
#define CHEOPS_CALU_AA_C    1   // Context Memory
#define CHEOPS_CALU_AB_M    0   // MLU

// ILU OPERATION
#define CHEOPS_CILU_INV     0   // o = 1.0/a
#define CHEOPS_CILU_ISQ     1   // o = 1.0/sqrt(a)
#define CHEOPS_CILU_CINV    2   // o = 1.0/a (with range clamp)
#define CHEOPS_CILU_NOP     3   // no output

// INPUT BUFFER - S1E8M23 - 32bit Floating point
#define CHEOPS_CIN_POS      0   // Position: x,y,z,w
#define CHEOPS_CIN_C0       1   // Diffuse Color: r,g,b,a
#define CHEOPS_CIN_C1       2   // Specular Color: r,g,b
#define CHEOPS_CIN_FOG      3   // Fog: f
#define CHEOPS_CIN_T0       4   // Texture0: s,t,r,q
#define CHEOPS_CIN_T1       5   // Texture1: s,t,r,q
#define CHEOPS_CIN_NRM      6   // Normal: nx,ny,nz
#define CHEOPS_CIN_SW       7   // Skin Weight: w

// OUTPUT BUFFER - S1E8M13 - 22bit Floating point
#define CHEOPS_COUT_TPOS    0   // Position
#define CHEOPS_COUT_TT0     1   // Texture0
#define CHEOPS_COUT_TT1     2   // Texture1
#define CHEOPS_COUT_WEV     3   // Eye vector
#define CHEOPS_COUT_WLV0    4   // Light0 direction vector
#define CHEOPS_COUT_WLV1    5   // Light1 direction vector
#define CHEOPS_COUT_WLV2    6   // Light2 direction vector
#define CHEOPS_COUT_WLV3    7   // Light3 direction vector
#define CHEOPS_COUT_WLV4    8   // Light4 direction vector
#define CHEOPS_COUT_WLV5    9   // Light5 direction vector
#define CHEOPS_COUT_WLV6    10  // Light6 direction vector
#define CHEOPS_COUT_WLV7    11  // Light7 direction vector
#define CHEOPS_COUT_WSL0    12  // Spotlight0 cos
#define CHEOPS_COUT_WSL1    13  // Spotlight1 cos
#define CHEOPS_COUT_WSL2    14  // Spotlight2 cos
#define CHEOPS_COUT_WSL3    15  // Spotlight3 cos
#define CHEOPS_COUT_WSL4    16  // Spotlight4 cos
#define CHEOPS_COUT_WSL5    17  // Spotlight5 cos
#define CHEOPS_COUT_WSL6    18  // Spotlight6 cos
#define CHEOPS_COUT_WSL7    19  // Spotlight7 cos
#define CHEOPS_COUT_VED     20  // Eye radial distance vector
#define CHEOPS_COUT_VLD0    21  // Light0 distance vector
#define CHEOPS_COUT_VLD1    22  // Light1 distance vector
#define CHEOPS_COUT_VLD2    23  // Light2 distance vector
#define CHEOPS_COUT_VLD3    24  // Light3 distance vector
#define CHEOPS_COUT_VLD4    25  // Light4 distance vector
#define CHEOPS_COUT_VLD5    26  // Light5 distance vector
#define CHEOPS_COUT_VLD6    27  // Light6 distance vector
#define CHEOPS_COUT_VLD7    28  // Light7 distance vector
#define CHEOPS_COUT_VC0     29  // Diffuse color
#define CHEOPS_COUT_VC1     30  // Specular color
#define CHEOPS_COUT_VNRM    31  // Normal vector
#define CHEOPS_COUT_VED2    32  // Eye planar distance vector
#define CHEOPS_COUT_TVW_NOP 33  // No valid output

#if __cplusplus
}
#endif
