#include <windows.h>
#include <string>
#include <iostream>

class Printer {
public:
    Printer(const std::wstring& printerName);
    ~Printer();

    bool print(const std::wstring& text);

private:
    HANDLE hPrinter;
    DOC_INFO_1W docInfo;
    std::wstring printerName;

    bool initialize();
    void cleanup();
};

Printer::Printer(const std::wstring& printerName) : printerName(printerName), hPrinter(nullptr) {
    if (!initialize()) {
        std::wcerr << L"Failed to initialize printer: " << printerName << std::endl;
    }
}

Printer::~Printer() {
    cleanup();
}

bool Printer::initialize() {
    if (!OpenPrinterW(const_cast<LPWSTR>(printerName.c_str()), &hPrinter, nullptr)) {
        std::wcerr << L"OpenPrinter failed with error code: " << GetLastError() << std::endl;
        return false;
    }

    docInfo.pDocName = const_cast<LPWSTR>(L"Ticket");
    docInfo.pOutputFile = nullptr;
    docInfo.pDatatype = const_cast<LPWSTR>(L"RAW");

    return true;
}

void Printer::cleanup() {
    if (hPrinter) {
        ClosePrinter(hPrinter);
        hPrinter = nullptr;
    }
}

bool Printer::print(const std::wstring& text) {
    if (!hPrinter) {
        std::wcerr << L"Printer handle is invalid." << std::endl;
        return false;
    }

    std::wcout << L"Starting document print..." << std::endl;
    std::wcout << L"Document Name: " << docInfo.pDocName << std::endl;
    std::wcout << L"Data Type: " << docInfo.pDatatype << std::endl;

    if (StartDocPrinterW(hPrinter, 1, reinterpret_cast<LPBYTE>(&docInfo)) == 0) {
        DWORD error = GetLastError();
        std::wcerr << L"StartDocPrinter failed with error code: " << error << std::endl;

        // Additional error code interpretation
        LPWSTR messageBuffer = nullptr;
        size_t size = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, nullptr);
        std::wcerr << L"Error Message: " << messageBuffer << std::endl;
        LocalFree(messageBuffer);

        return false;
    }

    if (StartPagePrinter(hPrinter) == 0) {
        DWORD error = GetLastError();
        std::wcerr << L"StartPagePrinter failed with error code: " << error << std::endl;
        EndDocPrinter(hPrinter);
        return false;
    }

    DWORD bytesWritten;
    if (WritePrinter(hPrinter, const_cast<LPWSTR>(text.c_str()), text.size() * sizeof(wchar_t), &bytesWritten) == 0) {
        DWORD error = GetLastError();
        std::wcerr << L"WritePrinter failed with error code: " << error << std::endl;
        EndPagePrinter(hPrinter);
        EndDocPrinter(hPrinter);
        return false;
    }

    if (EndPagePrinter(hPrinter) == 0) {
        DWORD error = GetLastError();
        std::wcerr << L"EndPagePrinter failed with error code: " << error << std::endl;
        EndDocPrinter(hPrinter);
        return false;
    }

    if (EndDocPrinter(hPrinter) == 0) {
        DWORD error = GetLastError();
        std::wcerr << L"EndDocPrinter failed with error code: " << error << std::endl;
        return false;
    }

    return true;
}

int main() {
    std::wstring printerName = L"hp LaserJet 1320 PCL 5"; // update printer name
    Printer printer(printerName);

    std::wstring ticketText = L"This is a test ticket.\nThank you for your purchase!";
    if (printer.print(ticketText)) {
        std::wcout << L"Ticket printed successfully." << std::endl;
    }
    else {
        std::wcerr << L"Failed to print ticket." << std::endl;
    }

    return 0;
}
