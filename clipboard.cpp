#include <iostream>
#include <vector>
#include <windows.h>

void copyToClipboard(const std::vector<char>& data) {
    // Open the clipboard
    if (!OpenClipboard(nullptr)) {
        std::cerr << "Failed to open clipboard!" << std::endl;
        return;
    }

    // Empty the clipboard before setting new data
    EmptyClipboard();

    // Allocate memory for the data
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, data.size());
    if (hMem == nullptr) {
        std::cerr << "Failed to allocate memory for clipboard!" << std::endl;
        CloseClipboard();
        return;
    }

    // Lock the memory and copy the data
    void* pMem = GlobalLock(hMem);
    memcpy(pMem, data.data(), data.size());
    GlobalUnlock(hMem);

    // Set the data to the clipboard
    if (SetClipboardData(CF_BINARY, hMem) == nullptr) {
        std::cerr << "Failed to set clipboard data!" << std::endl;
    }

    // Close the clipboard
    CloseClipboard();
}

std::vector<char> getClipboardData() {
    // Open the clipboard
    if (!OpenClipboard(nullptr)) {
        std::cerr << "Failed to open clipboard!" << std::endl;
        return {};
    }

    // Get the handle to the clipboard data in binary format
    HANDLE hData = GetClipboardData(CF_BINARY);
    if (hData == nullptr) {
        std::cerr << "Failed to get clipboard data!" << std::endl;
        CloseClipboard();
        return {};
    }

    // Lock the memory and get the data
    void* pData = GlobalLock(hData);
    if (pData == nullptr) {
        std::cerr << "Failed to lock clipboard data!" << std::endl;
        CloseClipboard();
        return {};
    }

    // Get the size of the data
    DWORD dataSize = GlobalSize(hData);

    // Copy the data into a vector
    std::vector<char> clipboardContent(static_cast<char*>(pData), static_cast<char*>(pData) + dataSize);

    // Unlock and close clipboard
    GlobalUnlock(hData);
    CloseClipboard();

    return clipboardContent;
}

int main(int argc, char* argv[]) {
    // Check if '-o' flag is passed for outputting clipboard content
    if (argc == 2 && std::string(argv[1]) == "-o") {
        std::vector<char> clipboardContent = getClipboardData();
        if (!clipboardContent.empty()) {
            // Output the binary data as-is (raw)
            std::cout.write(clipboardContent.data(), clipboardContent.size());
        } else {
            std::cerr << "Clipboard is empty or failed to retrieve data." << std::endl;
        }
    } else {
        // Otherwise, copy from stdin to clipboard
        std::vector<char> inputData;
        char buffer[1024];

        // Read everything from stdin (supports binary data)
        while (std::cin.read(buffer, sizeof(buffer))) {
            inputData.insert(inputData.end(), buffer, buffer + std::cin.gcount());
        }

        // Handle the last partial block if exists
        if (std::cin.gcount() > 0) {
            inputData.insert(inputData.end(), buffer, buffer + std::cin.gcount());
        }

        copyToClipboard(inputData);
    }

    return 0;
}
