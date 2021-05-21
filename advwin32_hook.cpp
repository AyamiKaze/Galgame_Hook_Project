// dllmain.cpp : 定义 DLL 应用程序的入口点。
// for こなたよりかなたまで
#include "framework.h"

PVOID g_pOldCreateFontA = CreateFontA;
typedef HFONT(WINAPI* PfuncCreateFontA)(
	int nHeight, // logical height of font height
	int nWidth, // logical average character width
	int nEscapement, // angle of escapement
	int nOrientation, // base-line orientation angle
	int fnWeight, // font weight
	DWORD fdwItalic, // italic attribute flag
	DWORD fdwUnderline, // underline attribute flag
	DWORD fdwStrikeOut, // strikeout attribute flag
	DWORD fdwCharSet, // character set identifier
	DWORD fdwOutputPrecision, // output precision
	DWORD fdwClipPrecision, // clipping precision
	DWORD fdwQuality, // output quality
	DWORD fdwPitchAndFamily, // pitch and family
	LPCSTR lpszFace // pointer to typeface name string
	);
HFONT WINAPI HookCreateFontA(
	int nHeight, // logical height of font height
	int nWidth, // logical average character width
	int nEscapement, // angle of escapement
	int nOrientation, // base-line orientation angle
	int fnWeight, // font weight
	DWORD fdwItalic, // italic attribute flag
	DWORD fdwUnderline, // underline attribute flag
	DWORD fdwStrikeOut, // strikeout attribute flag
	DWORD fdwCharSet, // character set identifier
	DWORD fdwOutputPrecision, // output precision
	DWORD fdwClipPrecision, // clipping precision
	DWORD fdwQuality, // output quality
	DWORD fdwPitchAndFamily, // pitch and family
	LPCSTR lpszFace // pointer to typeface name string
)
{
	return ((PfuncCreateFontA)g_pOldCreateFontA)(
		nHeight, // logical height of font height
		nWidth, // logical average character width
		nEscapement, // angle of escapement
		nOrientation, // base-line orientation angle
		fnWeight, // font weight
		fdwItalic, // italic attribute flag
		fdwUnderline, // underline attribute flag
		fdwStrikeOut, // strikeout attribute flag
		GB2312_CHARSET, // character set identifier
		fdwOutputPrecision, // output precision
		fdwClipPrecision, // clipping precision
		fdwQuality, // output quality
		fdwPitchAndFamily, // pitch and family
		"黑体" // pointer to typeface name string
		);
}

PVOID g_pOldCreateFontIndirectA = CreateFontIndirectA;
typedef HFONT(WINAPI* PfuncCreateFontIndirectA)(LOGFONTA* lplf);
HFONT WINAPI HookCreateFontIndirectA(LOGFONTA* lplf)
{
    lplf->lfCharSet = GB2312_CHARSET;
    strcpy(lplf->lfFaceName, "黑体");
    return ((PfuncCreateFontIndirectA)g_pOldCreateFontIndirectA)(lplf);
}

/*
02D1F65A 82 B1 82 C8 82 BD 82 E6 82 E8 82 A9 82 C8 82 BD  偙側偨傛傝偐側偨
02D1F66A 82 DC 82 C5 20 83 70 83 62 83 50 81 5B 83 57 83  傑偱 僷僢働乕僕
02D1F67A 8A 83 6A 83 85 81 5B 83 41 83 8B 94 C5 00 E5 4B  妰j儏乕傾儖斉
*/
PVOID g_pOldSetWindowTextA = SetWindowTextA;
typedef bool (WINAPI* PfuncSetWindowTextA)(HWND hWnd, LPCSTR lpString);
bool WINAPI HookSetWindowTextA(HWND hw, LPCSTR lpString)
{
	if(memcmp(lpString,"\x82\xB1\x82\xC8\x82\xBD\x82\xE6\x82\xE8\x82\xA9\x82\xC8\x82\xBD\x82\xDC\x82\xC5\x20\x83\x70\x83\x62\x83\x50\x81\x5B\x83\x57\x83\x8A\x83\x6A\x83\x85\x81\x5B\x83\x41\x83\x8B\x94\xC5",0x2D))
		return ((PfuncSetWindowTextA)g_pOldSetWindowTextA)(hw, lpString);
	wchar_t newtitle[] = L"【伊甸学院×葵海计划】从此方到彼方 - v1.0";
	return SetWindowTextW(hw, newtitle);
}

PVOID g_pOldMessageBoxA = MessageBoxA;
typedef int (WINAPI* PfuncMessageboxA)(
	_In_opt_ HWND    hWnd,
	_In_opt_ LPCSTR lpText,
	_In_opt_ LPCSTR lpCaption,
	_In_     UINT    uType
	);
int WINAPI HookMessageBoxA(
	_In_opt_ HWND    hWnd,
	_In_opt_ LPCSTR lpText,
	_In_opt_ LPCSTR lpCaption,
	_In_     UINT    uType)
{
	wchar_t* lpText_W = ctow((char*)lpText, 932);
	wchar_t* lpCaption_W = ctow((char*)lpCaption, 932);
	if(!lstrcmpW(lpText_W,L"ゲームを終了しますか？"))
		return MessageBoxW(hWnd, L"确认结束游戏吗？", L"提示", uType);
	return MessageBoxW(hWnd, lpText_W, lpCaption_W, uType);
}

// System.unt里面搜0101010101010101010101，有四个函数，全部下hook。
// 这里不能把边界里面的00改成01，否则文字计算会错位
int __fastcall TextProc1(BYTE* a1)
{
	int result; // eax

	for (result = 0; *a1; ++result)
	{
		switch (*a1 & 0xF0)
		{
		case 0x80:
		case 0x90:
		case 0xA0:
		case 0xB0:
		case 0xC0:
		case 0xD0:
		case 0xE0:
		case 0xF0:
			++a1;
			break;
		default:
			break;
		}
		++a1;
	}
	return result;
}

int __fastcall TextProc4(BYTE* a1)
{
	int result; // eax

	result = 0;
	if (a1 && *a1)
	{
		do
		{
			switch (*a1 & 0xF0)
			{
			case 0x80:
			case 0x90:
			case 0xA0:
			case 0xB0:
			case 0xC0:
			case 0xD0:
			case 0xE0:
			case 0xF0:
				++a1;
				break;
			default:
				break;
			}
			++a1;
			++result;
		} while (*a1);
	}
	return result;
}

// 这里包装的时候不能使用stdcall，因为stdcall需要额外保存寄存器。
int TextProc2(int a1, unsigned int a2)
{
	int v2; // ecx
	unsigned int v3; // edx

	v2 = 0;
	v3 = 0;
	if (!a2)
		return v2 + a1;
	while (*(BYTE*)(v2 + a1))
	{
		switch (*(BYTE*)(v2 + a1) & 0xF0)
		{
		case 0x80:
		case 0x90:
		case 0xA0:
		case 0xB0:
		case 0xC0:
		case 0xD0:
		case 0xE0:
		case 0xF0:
			++v2;
			break;
		default:
			break;
		}
		++v3;
		++v2;
		if (v3 >= a2)
			return v2 + a1;
	}
	return 0;
}



_declspec(naked) void _TextProc2()
{
	_asm
	{
		push esi;
		push edi;
		call TextProc2;
		pop edi;
		pop esi;
		ret;
	}
}

int TextProc3(BYTE* a1, unsigned int a2)
{
	int result; // eax
	unsigned int v3; // esi

	result = 0;
	if (a1)
	{
		if (a2)
		{
			v3 = 0;
			if (*a1)
			{
				do
				{
					if (v3 >= a2)
						break;
					switch (a1[result] & 0xF0)
					{
					case 0x80:
					case 0x90:
					case 0xA0:
					case 0xB0:
					case 0xC0:
					case 0xD0:
					case 0xE0:
					case 0xF0:
						++result;
						break;
					default:
						break;
					}
					++result;
					++v3;
				} while (a1[result]);
			}
		}
		else if (*a1)
		{
			do
				++result;
			while (a1[result]);
		}
	}
	return result;
}

_declspec(naked) void _TextProc3()
{
	_asm
	{
		push edi;
		push ecx;
		call TextProc3;
		pop ecx;
		pop edi;
		ret;
	}
}

/***************************** MCA *****************************/

char* FileName = nullptr;
char* MCGDIBBuffer = nullptr;
BOOL IsMCG = FALSE;

#pragma pack (1)
struct MCG_Header {
	char Magic[8]; // "MCG 2.00"
	int Zero;
	int One;
	int HeaderSize;
	int OffsetX;
	int OffsetY;
	int Width;
	int Height;
	int Bpp;
	int DIBSize; // "Width * Heaight * 3"
	int Unk1;
	int Unk2;
	int Unk3;
	int FileSize;
	int Unk4;
};
#pragma pack ()

MCG_Header* Header = nullptr;

// 断下CreateFileA，当读取文件名是mcg时往回跟出函数，在一个CharUpper前下hook。
PVOID GetMCGFileName_addr = 0;
_declspec(naked) void GetMCGFileName()
{
	_asm
	{
		mov FileName, esi;
		jmp GetMCGFileName_addr;
	}
}

void __stdcall GetMCGInfo(BYTE* ImageData)
{
	Header = (MCG_Header*)ImageData;
	if (memcmp(Header->Magic, "MCG 2.00", 8))
	{
		//MessageBox(NULL, L"Magic Not Match.", L"GetMCGInfo", MB_OK | MB_ICONERROR);
		//ExitProcess(-1);
		return;
	}
	IsMCG = TRUE;
}

// DIBWORK里mcg的解压函数前
PVOID GetMCGDIB_addr = 0;
_declspec(naked) void GetMCGDIB()
{
	_asm
	{
		mov MCGDIBBuffer, eax;
		pushad;
		pushfd;
		push edi;
		call GetMCGInfo;
		popfd;
		popad;
		jmp GetMCGDIB_addr;
	}
}

#define copy

#ifdef copy
void __stdcall MCGProc()
{
	if (FileName && MCGDIBBuffer && IsMCG)
	{
		char ofnm[MAX_PATH] = { 0 };
		strcpy(ofnm, "_MCGDump\\");
		strcat(ofnm, FileName);
		strcat(ofnm, ".bmp");
		//strcat(ofnm, ".DIB");
		FILE* fp = fopen(ofnm, "rb");
		if (!fp)
			return;
		fseek(fp, 0, SEEK_END);
		DWORD size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		BYTE* buff = new BYTE[size];
		fread(buff, size, 1, fp);
		fclose(fp);
		buff += sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		memcpy(MCGDIBBuffer, buff, Header->DIBSize);
		delete[] buff;
		IsMCG = FALSE;
		FileName = nullptr;
		MCGDIBBuffer = nullptr;
	}
}

#else
void __stdcall MCGProc()
{
	if (FileName && MCGDIBBuffer && IsMCG)
	{
		cout << "DumpMCG:" << FileName << endl;
		char ofnm[MAX_PATH] = { 0 };
		strcpy(ofnm, "_MCGDump\\");
		strcat(ofnm, FileName);
		cout << Header->Width << endl;
		cout << Header->Height << endl;
		cout << Header->Bpp << endl;
		write_bmp(ofnm, MCGDIBBuffer, Header->DIBSize,
			Header->Width, Header->Height, Header->Bpp);
		//strcat(ofnm, ".DIB");
		//FILE* fp = fopen(ofnm, "wb");
		//fwrite(MCGDIBBuffer, Header->DIBSize, 1, fp);
		//fclose(fp);
		IsMCG = FALSE;
		FileName = nullptr;
		MCGDIBBuffer = nullptr;
	}
}

#endif

// MCA里解压函数后
PVOID ReadMCGDIB_addr = 0;
_declspec(naked) void ReadMCGDIB()
{
	_asm
	{
		pushad;
		pushfd;
		call MCGProc;
		popfd;
		popad;
		jmp ReadMCGDIB_addr;
	}
}

void WINAPI SystemUntHook()
{
	PVOID TextProc1_addr = (PVOID)(SystemUntAddr + 0x12C60);
	PVOID TextProc2_addr = (PVOID)(SystemUntAddr + 0x12D20);
	PVOID TextProc4_addr = (PVOID)(SystemUntAddr + 0x1E750);
	PVOID TextProc3_addr = (PVOID)(SystemUntAddr + 0x1E810);
	GetMCGFileName_addr = (PVOID)(SystemUntAddr + 0x27933);

	cout << hex << "TextProc1_addr:" << TextProc1_addr << endl;
	cout << hex << "TextProc2_addr:" << TextProc2_addr << endl;
	cout << hex << "TextProc4_addr:" << TextProc4_addr << endl;
	cout << hex << "TextProc3_addr:" << TextProc3_addr << endl;
	cout << hex << "GetMCGFileName_addr:" << GetMCGFileName_addr << endl;
	DetourTransactionBegin();
	DetourAttach((void**)&TextProc1_addr, TextProc1);
	DetourAttach((void**)&TextProc2_addr, _TextProc2); // naked
	DetourAttach((void**)&TextProc4_addr, TextProc4);
	DetourAttach((void**)&TextProc3_addr, _TextProc3); // naked
	DetourAttach((void**)&GetMCGFileName_addr, GetMCGFileName); // naked
	if (DetourTransactionCommit() != NOERROR)
	{
		MessageBox(NULL, L"SystemUnt Hook Error", L"SystemUntHook", MB_OK | MB_ICONERROR);
		ExitProcess(-1);
	}
}

void WINAPI DIBWORKHook()
{
	GetMCGDIB_addr = (PVOID)(DIBWORKFTLAddr + 0x1F04);
	ReadMCGDIB_addr = (PVOID)(DIBWORKFTLAddr + 0x1F2B);
	cout << hex << "GetMCGDIB_addr:" << GetMCGDIB_addr << endl;
	cout << hex << "ReadMCGDIB_addr:" << ReadMCGDIB_addr << endl;

	DetourTransactionBegin();
	DetourAttach((void**)&GetMCGDIB_addr, GetMCGDIB);
	DetourAttach((void**)&ReadMCGDIB_addr, ReadMCGDIB);
	if (DetourTransactionCommit() != NOERROR)
	{
		MessageBox(NULL, L"DIBWORKFTL Hook Error", L"DIBWORKHook", MB_OK | MB_ICONERROR);
		ExitProcess(-1);
	}
}

PVOID g_pOldLoadLibraryExA = LoadLibraryExA;
typedef HMODULE (WINAPI* PfuncLoadLibraryExA)(
	LPCSTR lpLibFileName,
	HANDLE hFile,
	DWORD  dwFlags
);
HMODULE WINAPI HookLoadLibraryExA(
	LPCSTR lpLibFileName,
	HANDLE hFile,
	DWORD  dwFlags
)
{
	HMODULE ret = ((PfuncLoadLibraryExA)g_pOldLoadLibraryExA)(lpLibFileName, hFile, dwFlags);
	if (!lstrcmpA(lpLibFileName, "system.unt"))
	{
		SystemUntAddr = (DWORD)GetModuleHandleA("system.unt");
		if (SystemUntAddr == 0)
		{
			MessageBox(NULL, L"SystemUntAddr is 0", L"HookLoadLibraryExA", MB_OK | MB_ICONERROR);
			ExitProcess(-1);
		}
		cout << hex << "SystemUntAddr:" << SystemUntAddr << endl;
		SystemUntHook();
	}
	return ret;
}

PVOID g_pOldLoadLibraryA = LoadLibraryA;
typedef HMODULE (WINAPI* PfuncLoadLibraryA)(
	LPCSTR lpLibFileName
);
HMODULE WINAPI HookLoadLibraryA(
	LPCSTR lpLibFileName
)
{
	HMODULE ret = ((PfuncLoadLibraryA)g_pOldLoadLibraryA)(lpLibFileName);
	string strOldName((LPCSTR)lpLibFileName);
	string LibName = strOldName.substr(strOldName.find_last_of("\\") + 1);
	if (LibName == "DIBWORK.FIL")
	{
		DIBWORKFTLAddr = (DWORD)GetModuleHandleA(lpLibFileName);
		if (DIBWORKFTLAddr == 0)
		{
			MessageBox(NULL, L"DIBWORKFTLAddr is 0", L"HookLoadLibraryA", MB_OK | MB_ICONERROR);
			ExitProcess(-1);
		}
		cout << hex << "DIBWORKFTLAddr:" << DIBWORKFTLAddr << endl;
		DIBWORKHook();
	}
	return ret;
}

void WINAPI APIHook()
{
	DetourTransactionBegin();
	DetourAttach(&g_pOldCreateFontA, HookCreateFontA);
	DetourAttach(&g_pOldCreateFontIndirectA, HookCreateFontIndirectA);
	DetourAttach(&g_pOldSetWindowTextA, HookSetWindowTextA);
	DetourAttach(&g_pOldMessageBoxA, HookMessageBoxA);
	DetourAttach(&g_pOldLoadLibraryExA, HookLoadLibraryExA);
	DetourAttach(&g_pOldLoadLibraryA, HookLoadLibraryA);
	if (DetourTransactionCommit() != NOERROR)
	{
		MessageBox(NULL, L"API Hook Error", L"APIHook", MB_OK | MB_ICONERROR);
		ExitProcess(-1);
	}
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		make_console();
		APIHook();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

__declspec(dllexport)void WINAPI AyamiKaze()
{
}