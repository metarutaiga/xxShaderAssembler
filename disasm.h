#pragma once

#if __cplusplus
extern "C" {
#endif

size_t TokenD3DSI(uint32_t tokens[8], uint32_t const* binary, size_t index, size_t count);
size_t CommentD3DSI(char* text, size_t size, uint32_t const* binary, size_t index, size_t count);
void DisassembleD3DSI(char* text, size_t size, size_t width, uint32_t version, uint32_t const tokens[8], size_t count);

#if __cplusplus
}
#endif
