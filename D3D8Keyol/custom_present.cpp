#include"custom_present.h"
#include "d3dx8.h"
#include<map>
#include<string>
#include<ctime>
#include"keyol.h"

#pragma comment(lib,"d3d8/lib/d3dx8.lib")

#ifdef _DEBUG
#define C(x) if(FAILED(x)){MessageBox(NULL,TEXT(_CRT_STRINGIZE(x)),NULL,MB_ICONERROR);throw E_FAIL;}
#else
#define C(x) x
#endif

#ifndef _X86_
#error x64 platform is not supported.
#endif

ID3DXFont* pFont = nullptr;//经测试D3DXFont在此处不能创建多次
class D3DXCustomPresent
{
private:
	unsigned t1, t2, fcount;
	std::wstring display_text;
	int current_fps;
	TCHAR time_text[32], fps_text[32], width_text[32], height_text[32];

	TCHAR font_name[256], font_size[16],text_x[16],text_y[16],text_align[16],text_valign[16],display_text_fmt[256],fps_fmt[32],time_fmt[32], width_fmt[32], height_fmt[32];
	TCHAR font_red[16], font_green[16], font_blue[16], font_alpha[16];
	TCHAR font_shadow_red[16], font_shadow_green[16], font_shadow_blue[16], font_shadow_alpha[16], font_shadow_distance[16];
	UINT font_weight,period_frames;
	RECT rText, rTextShadow;
	int formatFlag;
	D3DCOLOR color_text, color_shadow;
	D3DVIEWPORT8 viewport;
	static WNDPROC oldWndProc;
public:
	D3DXCustomPresent():t1(0),t2(0),fcount(0),formatFlag(0)
	{
	}
	D3DXCustomPresent(D3DXCustomPresent &&other)
	{
		t1 = std::move(other.t1);
		t2 = std::move(other.t2);
		fcount = std::move(other.fcount);
		formatFlag = std::move(other.formatFlag);
	}
	~D3DXCustomPresent()
	{
		Uninit();
	}
	static LRESULT CALLBACK ExtraProcess(HWND h, UINT m, WPARAM w, LPARAM l)
	{
		return KeyOverlayExtraProcess(oldWndProc, h, m, w, l);
	}
	BOOL Init(LPDIRECT3DDEVICE8 pDev)
	{
		Uninit();
		TCHAR szConfPath[MAX_PATH];
		GetDLLPath(szConfPath, ARRAYSIZE(szConfPath));
		lstrcpy(wcsrchr(szConfPath, '.'), TEXT(".ini"));
#define GetInitConfStr(key,def) GetPrivateProfileString(TEXT("Init"),TEXT(_STRINGIZE(key)),def,key,ARRAYSIZE(key),szConfPath)
#define GetInitConfInt(key,def) key=GetPrivateProfileInt(TEXT("Init"),TEXT(_STRINGIZE(key)),def,szConfPath)
#define F(_i_str) (float)_wtof(_i_str)
		GetInitConfStr(font_name, TEXT("宋体"));
		GetInitConfStr(font_size, TEXT("48"));
		GetInitConfStr(font_red, TEXT("1"));
		GetInitConfStr(font_green, TEXT("1"));
		GetInitConfStr(font_blue, TEXT("0"));
		GetInitConfStr(font_alpha, TEXT("1"));
		GetInitConfStr(font_shadow_red, TEXT("0.5"));
		GetInitConfStr(font_shadow_green, TEXT("0.5"));
		GetInitConfStr(font_shadow_blue, TEXT("0"));
		GetInitConfStr(font_shadow_alpha, TEXT("1"));
		GetInitConfStr(font_shadow_distance, TEXT("2"));
		GetInitConfInt(font_weight, 400);
		GetInitConfStr(text_x, TEXT("0"));
		GetInitConfStr(text_y, TEXT("0"));
		GetInitConfStr(text_align, TEXT("left"));
		GetInitConfStr(text_valign, TEXT("top"));
		GetInitConfInt(period_frames, 60);
		GetInitConfStr(time_fmt, TEXT("%H:%M:%S"));
		GetInitConfStr(fps_fmt, TEXT("FPS:%3d"));
		GetInitConfStr(width_fmt, TEXT("%d"));
		GetInitConfStr(height_fmt, TEXT("%d"));
		GetInitConfStr(display_text_fmt, TEXT("{fps}"));

		LOGFONT df;
		ZeroMemory(&df, sizeof(LOGFONT));
		df.lfHeight = -MulDiv(_wtoi(font_size), USER_DEFAULT_SCREEN_DPI, 72);//此处不能直接用字体大小，需要将字体大小换算成GDI的逻辑单元大小
		df.lfWidth = 0;
		df.lfWeight = font_weight;
		df.lfItalic = false;
		df.lfCharSet = DEFAULT_CHARSET;
		df.lfOutPrecision = 0;
		df.lfQuality = 0;
		df.lfPitchAndFamily = 0;
		lstrcpy(df.lfFaceName, font_name);
		if (!pFont)
			C(D3DXCreateFontIndirect(pDev, &df, &pFont));
		C(pDev->GetViewport(&viewport));
		if (lstrcmpi(text_align, TEXT("right")) == 0)
		{
			formatFlag |= DT_RIGHT;
			rText.left = 0;
			rText.right = (LONG)(F(text_x)*viewport.Width);
		}
		else if (lstrcmpi(text_align, TEXT("center")) == 0)
		{
			formatFlag |= DT_CENTER;
			if (F(text_x) > 0.5f)
			{
				rText.left = 0;
				rText.right = (LONG)(2.0f*viewport.Width*F(text_x));
			}
			else
			{
				rText.left = (LONG)(2.0f*viewport.Width*F(text_x) - viewport.Width);
				rText.right = (LONG)viewport.Width;
			}
		}
		else
		{
			formatFlag |= DT_LEFT;
			rText.left = (LONG)(F(text_x)*viewport.Width);
			rText.right = (LONG)viewport.Width;
		}
		if (lstrcmpi(text_valign, TEXT("bottom")) == 0)
		{
			formatFlag |= DT_BOTTOM;
			rText.top = 0;
			rText.bottom = (LONG)(F(text_y)*viewport.Height);
		}
		else if (lstrcmpi(text_valign, TEXT("center")) == 0)
		{
			formatFlag |= DT_VCENTER;
			if (F(text_y) > 0.5f)
			{
				rText.top = 0;
				rText.bottom = (LONG)(2.0f*viewport.Height*F(text_y));
			}
			else
			{
				rText.top = (LONG)(2.0f*viewport.Height*F(text_y) - viewport.Height);
				rText.bottom = (LONG)viewport.Height;
			}
		}
		else
		{
			formatFlag |= DT_TOP;
			rText.top = (LONG)(F(text_y)*viewport.Height);
			rText.bottom = (LONG)viewport.Height;
		}
		rText.left += (LONG)viewport.X;
		rText.top += (LONG)viewport.Y;
		rText.right += (LONG)viewport.X;
		rText.bottom += (LONG)viewport.Y;
		rTextShadow.left = rText.left + (LONG)F(font_shadow_distance);
		rTextShadow.top = rText.top + (LONG)F(font_shadow_distance);
		rTextShadow.right = rText.right + (LONG)F(font_shadow_distance);
		rTextShadow.bottom = rText.bottom + (LONG)F(font_shadow_distance);
		color_shadow = D3DCOLOR_ARGB((DWORD)(255.0f*F(font_shadow_alpha)),
			(DWORD)(255.0f*F(font_shadow_red)),
			(DWORD)(255.0f*F(font_shadow_green)),
			(DWORD)(255.0f*F(font_shadow_blue)));
		color_text = D3DCOLOR_ARGB((DWORD)(255.0f*F(font_alpha)),
			(DWORD)(255.0f*F(font_red)),
			(DWORD)(255.0f*F(font_green)),
			(DWORD)(255.0f*F(font_blue)));

		D3DDEVICE_CREATION_PARAMETERS dcp;
		C(pDev->GetCreationParameters(&dcp));
		if (!KeyOverlayInit(dcp.hFocusWindow, pDev))
			return FALSE;
		oldWndProc = (WNDPROC)GetWindowLongPtr(dcp.hFocusWindow, GWLP_WNDPROC);
		SetWindowLongPtr(dcp.hFocusWindow, GWLP_WNDPROC, (LONG_PTR)ExtraProcess);
		return TRUE;
	}
	void Uninit()
	{
		if (pFont)
		{
			LPDIRECT3DDEVICE8 pDev;
			C(pFont->GetDevice(&pDev));
			D3DDEVICE_CREATION_PARAMETERS dcp;
			C(pDev->GetCreationParameters(&dcp));
			SetWindowLongPtr(dcp.hFocusWindow, GWLP_WNDPROC, (LONG_PTR)oldWndProc);
			KeyOverlayUninit();
			pFont->Release();
			pFont = nullptr;
		}
	}
	void Draw()
	{
		if (fcount--==0)
		{
			fcount = period_frames;
			t1 = t2;
			t2 = GetTickCount();
			if (t1 == t2)
				t1--;
			current_fps = period_frames * 1000 / (t2 - t1);
			wsprintf(fps_text, fps_fmt, current_fps);//注意wsprintf不支持浮点数格式化
			time_t t1 = time(NULL);
			tm tm1;
			localtime_s(&tm1, &t1);
			wcsftime(time_text, ARRAYSIZE(time_text), time_fmt, &tm1);
			wsprintf(width_text, width_fmt, viewport.Width);
			wsprintf(height_text, height_fmt, viewport.Height);
			display_text = display_text_fmt;
			size_t pos = display_text.find(TEXT("\\n"));
			if (pos != std::wstring::npos)
				display_text.replace(pos, 2, TEXT("\n"));
			pos = display_text.find(TEXT("{fps}"));
			if (pos != std::wstring::npos)
				display_text.replace(pos, 5, fps_text);
			pos = display_text.find(TEXT("{time}"));
			if (pos != std::wstring::npos)
				display_text.replace(pos, 6, time_text);
			pos = display_text.find(TEXT("{width}"));
			if (pos != std::wstring::npos)
				display_text.replace(pos, 7, width_text);
			pos = display_text.find(TEXT("{height}"));
			if (pos != std::wstring::npos)
				display_text.replace(pos, 8, height_text);
		}
		KeyOverlayDraw();
		pFont->DrawText(display_text.c_str(), (int)display_text.length(), &rTextShadow, formatFlag, color_shadow);
		pFont->DrawText(display_text.c_str(), (int)display_text.length(), &rText, formatFlag, color_text);
	}
};

WNDPROC D3DXCustomPresent::oldWndProc = nullptr;

static std::map<LPDIRECT3DDEVICE8, D3DXCustomPresent> cp;

void CustomPresent(LPDIRECT3DDEVICE8 p,HRESULT hrLast)
{
	if (cp.find(p) == cp.end())
	{
		if (hrLast == D3D_OK && p->TestCooperativeLevel() == D3D_OK)
		{
			cp.insert(std::make_pair(p, D3DXCustomPresent()));
			cp[p].Init(p);
		}
	}
	else if (hrLast != D3D_OK || p->TestCooperativeLevel() != D3D_OK)
		cp.erase(p);
	else
		cp[p].Draw();
}