#pragma once
#include"d3d8.h"
#ifdef __cplusplus
extern "C" {
#endif
//�Զ���Present�ĸ��Ӳ���
void CustomPresent(LPDIRECT3DDEVICE8,HRESULT);
DWORD GetDLLPath(LPTSTR path, DWORD max_length);
#ifdef __cplusplus
}
#endif
