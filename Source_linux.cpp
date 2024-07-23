#include <cups/cups.h>
#include <string>
#include <iostream>

class Printer {
public:
    Printer(const std::string& printerName);
    ~Printer();

    bool print(const std::string& text);

private:
    std::string printerName;
    cups_dest_t* printer;
    cups_dinfo_t* dinfo;

    bool initialize();
    void cleanup();
};

Printer::Printer(const std::string& printerName) : printerName(printerName), printer(nullptr), dinfo(nullptr) {
    if (!initialize()) {
        std::cerr << "Failed to initialize printer: " << printerName << std::endl;
    }
}

Printer::~Printer() {
    cleanup();
}

bool Printer::initialize() {
    int num_dests;
    cups_dest_t* dests = cupsGetDests(&num_dests);
    printer = cupsGetDest(printerName.c_str(), nullptr, num_dests, dests);
    cupsFreeDests(num_dests, dests);

    if (!printer) {
        std::cerr << "Unable to find printer: " << printerName << std::endl;
        return false;
    }

    dinfo = cupsCopyDestInfo(CUPS_HTTP_DEFAULT, printer);
    if (!dinfo) {
        std::cerr << "Unable to get printer information for: " << printerName << std::endl;
        return false;
    }

    return true;
}

void Printer::cleanup() {
    if (dinfo) {
        cupsFreeDestInfo(dinfo);
        dinfo = nullptr;
    }
    if (printer) {
        cupsFreeDests(1, printer);
        printer = nullptr;
    }
}

bool Printer::print(const std::string& text) {
    if (!printer || !dinfo) {
        std::cerr << "Printer handle is invalid." << std::endl;
        return false;
    }

    std::cout << "Starting document print..." << std::endl;

    cups_file_t* temp_file = cupsTempFile2(nullptr, nullptr);
    if (!temp_file) {
        std::cerr << "Unable to create temporary file." << std::endl;
        return false;
    }

    cupsFilePuts(temp_file, text.c_str());
    cupsFileClose(temp_file);

    const char* file_name = cupsFileGetName(temp_file);
    int job_id = cupsPrintFile(printerName.c_str(), file_name, "Ticket", 0, nullptr);

    if (job_id == 0) {
        std::cerr << "Failed to print file: " << cupsLastErrorString() << std::endl;
        return false;
    }

    std::cout << "Print job sent with ID: " << job_id << std::endl;

    return true;
}

int main() {
    std::string printerName = "hp_LaserJet_1320"; // update printer name
    Printer printer(printerName);

    std::string ticketText = "This is a test ticket.\nThank you for your purchase!";
    if (printer.print(ticketText)) {
        std::cout << "Ticket printed successfully." << std::endl;
    } else {
        std::cerr << "Failed to print ticket." << std::endl;
    }

    return 0;
}
