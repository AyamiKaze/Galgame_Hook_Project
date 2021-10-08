// dllmain.cpp : 定义 DLL 应用程序的入口点。
// 有很多垃圾代码，我就不清理了。或许又是另一条思路。
// 十分感谢天殇大佬和展鸿大佬提供的建议和帮助，没有你们我是写不来字形替换的。
// 使用的库有FreeType和opencv2
// 游戏：《痕》
// 由于Leaf曾公开过ToHeart2源代码，虽然不是一个游戏，但是核心基础没变，所以以下hook建议配套源代码使用
#include "framework.h"
#include <math.h>
DWORD BaseAddr = (DWORD)GetModuleHandle(NULL);
unordered_map<unsigned int, wstring>TextList;


HDC hdc;
TEXTMETRICA tm;
DWORD moji;
HWND hwnd;

PVOID g_pOldCreateWindowExA = CreateWindowExA;
typedef HWND(WINAPI* pfuncCreateWindowExA)(
    DWORD dwExStyle,
    LPCTSTR lpClassName,
    LPCTSTR lpWindowName,
    DWORD dwStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam);
HWND WINAPI NewCreateWindowExA(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
    hwnd = ((pfuncCreateWindowExA)g_pOldCreateWindowExA)(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    return hwnd;
}

// 这个Hook在子指令里面，不在文本op里面。也就是hook GetScriptParam_str
// 子指令会把sdt里面的脚本复制到一个很大的内存中，然后返回他的长度。
// 这里我们可以等拿到返回值了，再对内存进行操作，返回值不变。
int (*sub_4012A0)(char* Dst);
int GetScriptParam_str(char* Dst)
{
    // 由于这里会不停的读取，建议最好准备缓存机制。
    _asm
    {
        pushad;
        pushfd;
    }
    int ret = sub_4012A0(Dst);
    _asm
    {
        popfd;
        popad;
    }
    int offset_num = 0;
    _asm
    {
        mov offset_num, esi;
    }
    if (offset_num == 0x40)
    {
        //cout << "offset:0x" << offset_num << endl;
        char* Text = (char*)"这是我的中文显示测试自部自分。\\k\\n";
        //char* Text = (char*)"leaf";
        int len = strlen(Text);
        memcpy(Dst, Text, len);
        Dst[len] = 0;
    }
    return ret;
}

void InlineHook()
{
    *(DWORD*)& sub_4012A0 = BaseAddr + 0x12A0;
    cout << hex << "sub_4012A0_Hook_Pos:0x" << (BaseAddr + 0x12A0) << endl;
    DetourTransactionBegin();
    DetourAttach((void**)&sub_4012A0, GetScriptParam_str); // naked
    DetourAttach(&g_pOldCreateWindowExA, NewCreateWindowExA);
    if (DetourTransactionCommit() != NOERROR)
    {
        MessageBox(NULL, L"Inline Hook Error", L"InlineHook", MB_OK | MB_ICONERROR);
        ExitProcess(-1);
    }
}

// 获取当前汉字，由于这个汉字后面还要做查表运算所以之后要塞一个fake的SJIS值
// fake必须是汉字值
/*
004A8EA8 | 8B4424 38         | MOV     EAX, DWORD PTR SS:[ESP + 38]            |
004A8EAC | 33DB              | XOR     EBX, EBX                                | HERE
004A8EAE | 8AF8              | MOV     BH, AL                                  |
004A8EB0 | C1F8 08           | SAR     EAX, 8                                  |
004A8EB3 | 0BD8              | OR      EBX, EAX                                |
004A8EB5 | 33C0              | XOR     EAX, EAX                                |
*/
PVOID pHookCinit1 = (PVOID)(BaseAddr + 0xA8EAC);
__declspec(naked)void gHookCinit1()
{
    _asm
    {
        mov moji, eax;
        mov eax, 0xB296;
        jmp pHookCinit1;
    }
}

FT_Error error1;
FT_Library  library;
FT_Face face;
//std::vector<BYTE> output;
BYTE output[4096];

// 这里要同时处理阴影和文字。然后根据字号和汉字生成对应的字形。

/*
004A8F44 | 0FAF4C24 14       | IMUL    ECX, DWORD PTR SS:[ESP + 14]            | 
004A8F49 | 0FAFF2            | IMUL    ESI, EDX                                | 
004A8F4C | 8B4424 18         | MOV     EAX, DWORD PTR SS:[ESP + 18]            |
004A8F50 | 8D0480            | LEA     EAX, DWORD PTR DS:[EAX + EAX * 4]       |
004A8F53 | 03C0              | ADD     EAX, EAX                                |
004A8F55 | 8B9400 A0DDCE00   | MOV     EDX, DWORD PTR DS:[EAX + EAX + CEDDA0]  | 
004A8F5C | 03C0              | ADD     EAX, EAX                                |
004A8F5E | 8B80 B0DDCE00     | MOV     EAX, DWORD PTR DS:[EAX + CEDDB0]        |
004A8F64 | 03D6              | ADD     EDX, ESI                                | 
004A8F66 | 03C8              | ADD     ECX, EAX                                | 阴影
004A8F68 | 03D3              | ADD     EDX, EBX                                | 字
004A8F6A | 85C0              | TEST    EAX, EAX                                | HOOK HERE
004A8F6C | 8D4C29 04         | LEA     ECX, DWORD PTR DS:[ECX + EBP + 4]       | （EDI = 当前使用的字库模型大小）
*/

BYTE* WINAPI HookFont1(BYTE* ret, UINT FONT_WIDTH, UINT FONT_HEIGHT, BOOL IsKage)
{
    //cout << dec << FONT_WIDTH << "x" << FONT_HEIGHT << endl;
    if (IsKage)
    {
        if (FONT_WIDTH == 30)
        {
            FONT_WIDTH = 34;
            FONT_HEIGHT = 34;
        }
        else if (FONT_WIDTH == 20)
        {
            FONT_WIDTH = 24;
            FONT_HEIGHT = 24;
        }
        else if (FONT_WIDTH == 18)
        {
            FONT_WIDTH = 20;
            FONT_HEIGHT = 20;
        }
        else if (FONT_WIDTH == 14)
        {
            FONT_WIDTH = 16;
            FONT_HEIGHT = 16;
        }
        else if (FONT_WIDTH == 12)
        {
            FONT_WIDTH = 14;
            FONT_HEIGHT = 14;
        }
    }

    error1 = FT_Set_Pixel_Sizes(face, FONT_WIDTH * 0.8, FONT_HEIGHT * 0.8);
    if (error1)
    {
        std::cout << "Failed to set font sizes." << std::endl;
        return ret;
    }


    error1 = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
    //cout << hex << "1:0x" << moji << endl;
    wchar_t* text = ctow((char*)&moji, 936);
    //cout << hex << "2:0x" << *text << endl;
    auto glyph_index = FT_Get_Char_Index(face, (FT_ULong)*text);

    error1 = FT_Load_Glyph(face, glyph_index, FT_LOAD_TARGET_NORMAL);
    if (error1)
    {
        std::cout << "Failed to load font glyph." << std::endl;
        return ret;
    }

    error1 = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
    if (error1)
    {
        std::cout << "Failed to render font glyph." << std::endl;
        return ret;
    }

    if (face->glyph->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY)
    {
        std::cout << "Pixel mode not supported." << std::endl;
        return ret;
    }

    auto& bitmap = face->glyph->bitmap;
    if (IsKage) 
    {
        FT_Outline_EmboldenXY(&face->glyph->outline, FONT_WIDTH, FONT_HEIGHT);

        FT_Bitmap_Embolden(library, &bitmap, FONT_WIDTH, FONT_HEIGHT);
    }

    size_t output_width = std::ceil(FONT_WIDTH / 2.0);
    size_t output_height = FONT_HEIGHT;

    //output.resize(output_width * output_height);
    memset(output, 0, 4096);

    // 字形bitmap在output中的偏移量，太大会导致越界
    auto bearingX = face->glyph->metrics.horiBearingX >> 6;
    auto bearingY = face->glyph->metrics.horiBearingY >> 6;

    int originX = 0;
    int originY = 21;
    
    //int offset_x = originX + bearingX;
    //int offset_y = originY - bearingY;

    int offset_x = 1;
    int offset_y = 1;

    int offset_x2 = 3;
    int offset_y2 = 3;

    //cout << hex << offset_x << "|" << offset_y << endl;

    // 从bitmap中指定坐标读取像素
    auto get_pixel = [&bitmap](size_t x, size_t y) -> BYTE
    {
        // 检查是否越界，调试用
        assert(x * y < bitmap.width* bitmap.rows);

        return bitmap.buffer[y * bitmap.width + x];
    };

    cv::Mat out;
    cv::Mat image(cv::Size(face->glyph->bitmap.pitch, face->glyph->bitmap.rows),
        CV_8UC1, face->glyph->bitmap.buffer, cv::Mat::AUTO_STEP);

    int w = face->glyph->bitmap.pitch;
    int h = face->glyph->bitmap.rows;
    int base_y = FONT_HEIGHT - 4;
    int t = base_y - face->glyph->bitmap_top;
    int b = FONT_HEIGHT - t - h;
    int l = face->glyph->bitmap_left;
    int r = FONT_WIDTH - w - l;
    cv::copyMakeBorder(image, out, t, b, l, r, cv::BORDER_CONSTANT, 0);

    cout << dec << FONT_WIDTH << "x" << FONT_HEIGHT << endl;

    for (uint32_t y = 0; y < FONT_HEIGHT; y++)
    {
        for (uint32_t x = 0; x < FONT_WIDTH; x += 2)
        {
            // 从bitmap中读取像素值
            /*
            BYTE pixel0 = (x + 0 < bitmap.width) ? get_pixel(x + 0, y) : 0;
            BYTE pixel1 = (x + 1 < bitmap.width) ? get_pixel(x + 1, y) : 0;

            pixel0 = pixel0 * 0xF / 0xFF;
            pixel1 = (pixel1 * 0xF / 0xFF) << 4;

            BYTE val = pixel1 | pixel0;

            size_t pos = FONT_WIDTH + y * output_width + (FONT_HEIGHT + x / 2);

            output[pos]= val;
            */
            auto PixCol1 = out.at<BYTE>(y, x);
            auto PixCol2 = out.at<BYTE>(y, x + 1);
            PixCol1 = PixCol1 * 0xF / 0xFF;
            PixCol2 = (PixCol2 * 0xF / 0xFF) << 4;
            BYTE val = (BYTE)PixCol2 | PixCol1;
            size_t pos = 0;
            if(!IsKage)
                pos = offset_y + y * output_width + (offset_x + x / 2);
            else
                pos = offset_y2 + y * output_width + (offset_x2 + x / 2);

            output[pos] = val;
            //output.push_back(val);
        }
    }

    /**/
    memset(ret, 0, FONT_WIDTH* FONT_HEIGHT);

    memcpy(ret, output, FONT_WIDTH* FONT_HEIGHT);
    /*
    if (!IsKage) 
    {
        if (*text == L'自')
        {
            FILE* fp = fopen("自.dmp", "wb");
            fwrite(output.data(), output.size(), 1, fp);
            fclose(fp);
        }
        else if (*text == L'这')
        {
            FILE* fp = fopen("这.dmp", "wb");
            fwrite(output.data(), output.size(), 1, fp);
            fclose(fp);
        }
        else if (*text == L'显')
        {
            FILE* fp = fopen("显.dmp", "wb");
            fwrite(output.data(), output.size(), 1, fp);
            fclose(fp);
        }
        else if (*text == L'试')
        {
            FILE* fp = fopen("试.dmp", "wb");
            fwrite(output.data(), output.size(), 1, fp);
            fclose(fp);
        }
    }
    */
}


DWORD ret = 0;

PVOID pHookCinit0 = (PVOID)(BaseAddr + 0xA8F6A);
__declspec(naked)void gHookCinit0()
{
    _asm
    {
        pushad
        pushfd
        push 0;
        push edi;
        push edi;
        push edx;
        call HookFont1;
        popfd;
        popad;

        /**/
        pushad;
        pushfd;
        push 1
        push edi;
        push edi;
        push ecx;
        call HookFont1;
        popfd;
        popad;

        jmp pHookCinit0
    }
}


void FontMain()
{
    //hdc = CreateDCA("DISPLAY", 0, 0, 0);
    /*
    hdc = GetDC(hwnd);
    HFONT v0 = CreateFontA(28, 0, 0, 0, 400, 0, 0, 0, 0x86u, 2u, 1u, 0, 0, "黑体");
    SelectObject(hdc, v0);
    GetTextMetricsA(hdc, &tm);
    */


    //cout << FONT_WIDTH << "|" << FONT_HEIGHT << endl;

    error1 = FT_Init_FreeType(&library);
    if (error1)
    {
        std::cout << "Failed to initialize freetype library." << std::endl;
        return;
    }

    error1 = FT_New_Face(library, "CNFONT.otf", 0, &face);
    if (error1)
    {
        std::cout << "Failed to create font face." << std::endl;
        return;
    }

    DetourTransactionBegin();
    DetourAttach((void**)&pHookCinit0, gHookCinit0); // naked
    DetourAttach((void**)&pHookCinit1, gHookCinit1); // naked
    if (DetourTransactionCommit() != NOERROR)
    {
        MessageBox(NULL, L"FontMain Hook Error", L"FontMain", MB_OK | MB_ICONERROR);
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
        setlocale(0, "chinese");
        make_console();
        InlineHook();
        FontMain();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

