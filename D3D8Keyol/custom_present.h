#pragma once
#include"d3d8.h"
#ifdef __cplusplus
extern "C" {
#endif
//自定义Present的附加操作
void CustomPresent(LPDIRECT3DDEVICE8,HRESULT);
DWORD GetDLLPath(LPTSTR path, DWORD max_length);
#ifdef __cplusplus
}
#endif
