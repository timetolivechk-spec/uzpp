#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

namespace fs = std::filesystem;

namespace uzpp {

class DocGen {
public:
    bool generateDocs(const fs::path& inputDir, const fs::path& outputDir) {
        if (!fs::exists(inputDir)) {
            std::cerr << "XATO: Manba (src) katalogi topilmadi -> " << inputDir.string() << "\n";
            return false;
        }

        fs::create_directories(outputDir);
        fs::path outMd = outputDir / "API_Qollanma.md";
        std::ofstream md(outMd, std::ios::binary);

        if (!md.is_open()) {
            std::cerr << "XATO: Hujjat faylini yaratib bo'lmadi.\n";
            return false;
        }

        md << "# Dastur API Qo'llanmasi (uz++)\n\n";
        md << "Ushbu hujjat avtomatik ravishda `uzpp hujjat` buyrug'i yordamida yaratilgan.\n\n";

        int processedFiles = 0;

        for (const auto& entry : fs::recursive_directory_iterator(inputDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".uzpp") {
                processFile(entry.path(), md);
                processedFiles++;
            }
        }

        std::cout << "MUVAFFAQIYAT: " << processedFiles << " ta fayl o'qildi. Hujjat saqlandi -> " << outMd.string() << "\n";
        return true;
    }

private:
    void processFile(const fs::path& file, std::ofstream& md) {
        std::ifstream in(file);
        if (!in) return;

        std::string line;
        std::vector<std::string> currentDoc;
        std::string moduleName = file.filename().string();

        std::vector<std::pair<std::string, std::string>> extractedItems;

        while (std::getline(in, line)) {
            size_t start = line.find_first_not_of(" \t");
            if (start == std::string::npos) continue;
            
            std::string trimmed = line.substr(start);

            if (trimmed.starts_with("///")) {
                std::string docLine = trimmed.substr(3);
                if (!docLine.empty() && docLine[0] == ' ') docLine = docLine.substr(1);
                currentDoc.push_back(docLine);
            } else if (!currentDoc.empty()) {
                std::string declaration = trimmed;
                size_t bracePos = declaration.find('{');
                if (bracePos != std::string::npos) declaration = declaration.substr(0, bracePos);
                size_t semiPos = declaration.find(';');
                if (semiPos != std::string::npos) declaration = declaration.substr(0, semiPos);
                
                size_t end = declaration.find_last_not_of(" \t");
                if (end != std::string::npos && end + 1 <= declaration.length()) declaration = declaration.substr(0, end + 1);

                std::string docText;
                for (const auto& d : currentDoc) docText += d + "\n";
                
                extractedItems.push_back({declaration, docText});
                currentDoc.clear();
            }
        }

        if (!extractedItems.empty()) {
            md << "## Modul: `" << moduleName << "`\n\n";
            for (const auto& item : extractedItems) {
                md << "### `" << item.first << "`\n\n";
                md << item.second << "\n";
            }
            md << "---\n\n";
        }
    }
};

} // namespace uzpp