#include "codegen.h"
#include "lexer.h"
#include "package_manager.h"
#include "parser.h"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>

namespace {

namespace fs = std::filesystem;

std::string transpileSnippet(const std::string& source) {
    uzpp::Lexer lexer(source);
    const auto tokens = lexer.tokenize();

    uzpp::Parser parser(tokens);
    const auto program = parser.parse();

    uzpp::CodeGen codegen;
    return codegen.generate(program.get(), "smoke.uzpp");
}

} // namespace

int main() {
    {
        uzpp::Lexer lexer("Demo::Type ptr = foo->bar + ++i;");
        const auto tokens = lexer.tokenize();

        bool hasDoubleColon = false;
        bool hasArrow = false;
        bool hasIncrement = false;

        for (const auto& token : tokens) {
            hasDoubleColon = hasDoubleColon || token.value == "::";
            hasArrow = hasArrow || token.value == "->";
            hasIncrement = hasIncrement || token.value == "++";
        }

        assert(hasDoubleColon);
        assert(hasArrow);
        assert(hasIncrement);
    }

    {
        const std::string cpp = transpileSnippet(
            "butun asosiy() { uchun (butun i = 0; i < 3; i++) { yozish << i << qator_oxiri; } qaytarish 0; }");
        assert(cpp.find("int main") != std::string::npos);
        assert(cpp.find("for") != std::string::npos);
        assert(cpp.find("std::cout") != std::string::npos);
        assert(cpp.find("std::endl") != std::string::npos);
    }

    {
        const std::string cpp =
            transpileSnippet("nomlar_fazosi Demo { sinf A { ochiq butun get() { qaytarish 1; } }; }");
        assert(cpp.find("namespace Demo") != std::string::npos);
        assert(cpp.find("class A") != std::string::npos);
        assert(cpp.find("public:") != std::string::npos);
    }

    {
        const std::string cpp = transpileSnippet("oqim worker(task, 1); qulf m; fayl_yozish out(\"log.txt\");");
        assert(cpp.find("uzpp::Oqim") != std::string::npos);
        assert(cpp.find("std::mutex") != std::string::npos);
        assert(cpp.find("std::ofstream") != std::string::npos);
    }

    {
        bool thrown = false;
        try {
            uzpp::Lexer lexer("butun asosiy( {");
            const auto tokens = lexer.tokenize();
            uzpp::Parser parser(tokens);
            auto program = parser.parse();
            (void)program;
        } catch (const uzpp::ParseError&) {
            thrown = true;
        }

        assert(thrown);
    }

    {
        uzpp::TomlParser parser(
            "[loyiha]\n"
            "nomi = \"mening_loyiham\"\n"
            "versiya = \"0.1.0\"\n"
            "muallif = \"Ism Familiya\"\n"
            "asosiy_fayl = \"src/asosiy.uzpp\"\n"
            "\n"
            "[kutubxonalar]\n"
            "tarmoq = \"1.0.0\"\n"
            "fayl_tizimi = \"1.0.0\"\n");
        const uzpp::ProjectManifest manifest = parser.parse();

        assert(manifest.name == "mening_loyiham");
        assert(manifest.version == "0.1.0");
        assert(manifest.author == "Ism Familiya");
        assert(manifest.entryFile == "src/asosiy.uzpp");
        assert(manifest.dependencies.size() == 2);
        assert(manifest.dependencies[0].name == "tarmoq");
        assert(manifest.dependencies[1].name == "fayl_tizimi");
    }

    {
        const fs::path tempRoot = fs::temp_directory_path() / "uzpp_phase12_smoke";
        std::error_code ec;
        fs::remove_all(tempRoot, ec);

        assert(uzpp::ProjectManager::createProjectStructure(tempRoot, "demo"));

        const auto project = uzpp::ProjectManager::loadProject(tempRoot);
        assert(project.has_value());
        assert(project->manifest.name == "demo");
        assert(project->manifest.entryFile == "src/asosiy.uzpp");
        assert(project->entryFile == (tempRoot / "src" / "asosiy.uzpp"));
        assert(fs::exists(project->manifestPath));
        assert(fs::exists(project->entryFile));

        assert(uzpp::ProjectManager::addDependency(tempRoot, "tarmoq", "1.0.0"));
        const auto updatedManifest = uzpp::ProjectManager::loadManifest(tempRoot);
        assert(updatedManifest.has_value());
        assert(updatedManifest->dependencies.size() == 1);
        assert(updatedManifest->dependencies[0].name == "tarmoq");

        fs::remove_all(tempRoot, ec);
    }

    std::cout << "uzpp frontend smoke tests passed\n";
    return 0;
}
