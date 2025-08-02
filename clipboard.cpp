#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static const char* CLIP_FMT_NAME = "XCLIP_RAW";

int main(int argc, char* argv[])
{
    int output = (argc == 2 && std::strcmp(argv[1], "-o") == 0);

    if (output) {
        if (!OpenClipboard(NULL)) return 1;
        UINT fmt = RegisterClipboardFormatA(CLIP_FMT_NAME);
        HANDLE hData = NULL;
        if (IsClipboardFormatAvailable(fmt))
            hData = GetClipboardData(fmt);
        else if (IsClipboardFormatAvailable(CF_OEMTEXT))
            hData = GetClipboardData(CF_OEMTEXT);
        if (!hData) { CloseClipboard(); return 1; }

        void* ptr = GlobalLock(hData);
        SIZE_T size = GlobalSize(hData);

        DWORD written = 0;
        HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        WriteFile(hStdout, ptr, (DWORD)size, &written, NULL);

        GlobalUnlock(hData);
        CloseClipboard();
        return 0;
    } else {
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        char* buf = NULL;
        size_t bufsize = 0, total = 0;
        DWORD n;

        while (1) {
            if (total + 4096 > bufsize) {
                bufsize = bufsize ? bufsize * 2 : 4096;
                buf = (char*)realloc(buf, bufsize);
                if (!buf) return 1;
            }
            if (!ReadFile(hStdin, buf + total, 4096, &n, NULL) || n == 0)
                break;
            total += n;
        }

        if (total == 0) { free(buf); return 0; }

        if (!OpenClipboard(NULL)) { free(buf); return 1; }
        EmptyClipboard();
        UINT fmt = RegisterClipboardFormatA(CLIP_FMT_NAME);
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, total);
        if (!hMem) { CloseClipboard(); free(buf); return 1; }

        void* dst = GlobalLock(hMem);
        memcpy(dst, buf, total);
        GlobalUnlock(hMem);
        SetClipboardData(fmt, hMem);
        CloseClipboard();
        free(buf);
        return 0;
    }
}
