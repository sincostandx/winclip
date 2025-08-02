#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <vector>
#include <cstdio>
#include <cstring>

static const char* CLIP_FMT_NAME = "XCLIP_RAW";

int main(int argc, char* argv[])
{
    bool output = (argc == 2 && std::strcmp(argv[1], "-o") == 0);
    _setmode(_fileno(stdin),  _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);

    if (output) {
        if (!OpenClipboard(NULL)) return 1;
        UINT fmt = RegisterClipboardFormatA(CLIP_FMT_NAME);
        HANDLE hData = nullptr;
        if (IsClipboardFormatAvailable(fmt))
            hData = GetClipboardData(fmt);
        else if (IsClipboardFormatAvailable(CF_OEMTEXT))
            hData = GetClipboardData(CF_OEMTEXT);
        if (!hData) { CloseClipboard(); return 1; }

        void* ptr = GlobalLock(hData);
        SIZE_T size = GlobalSize(hData);
        fwrite(ptr, 1, size, stdout);
        GlobalUnlock(hData);
        CloseClipboard();
        return 0;
    }
    else {
        std::vector<char> buffer;
        char chunk[4096];
        while (true) {
            std::cin.read(chunk, sizeof(chunk));
            std::streamsize n = std::cin.gcount();
            if (n <= 0) break;
            buffer.insert(buffer.end(), chunk, chunk + n);
        }
        if (buffer.empty()) return 0;

        if (!OpenClipboard(NULL)) return 1;
        EmptyClipboard();
        UINT fmt = RegisterClipboardFormatA(CLIP_FMT_NAME);
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, buffer.size());
        if (!hMem) { CloseClipboard(); return 1; }

        void* dst = GlobalLock(hMem);
        std::memcpy(dst, buffer.data(), buffer.size());
        GlobalUnlock(hMem);
        SetClipboardData(fmt, hMem);
        CloseClipboard();
        return 0;
    }
}
