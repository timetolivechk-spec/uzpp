#include "codegen.h"
#include "lexer.h"
#include "package_manager.h"
#include "parser.h"
#include "type_checker.hpp"

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

bool typecheckSnippet(const std::string& source,
                      std::vector<uzpp::SemanticError>* errors = nullptr,
                      std::vector<uzpp::SemanticError>* warnings = nullptr) {
    uzpp::Lexer lexer(source);
    const auto tokens = lexer.tokenize();
    uzpp::Parser parser(tokens);
    const auto program = parser.parse();
    uzpp::TypeChecker checker;
    bool ok = checker.check(program.get());
    if (errors)   *errors   = checker.getErrors();
    if (warnings) *warnings = checker.getWarnings();
    return ok;
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

    // ------------------------------------------------------------
    // Lexer unit tests
    // ------------------------------------------------------------

    {
        // Triple-quoted string literal
        uzpp::Lexer lexer("matn s = \"\"\"hello\nworld\"\"\";");
        const auto tokens = lexer.tokenize();
        bool foundTriple = false;
        for (const auto& t : tokens) {
            if (t.type == uzpp::TokenType::StringLiteral &&
                t.value.find("hello") != std::string::npos &&
                t.value.find("world") != std::string::npos) {
                foundTriple = true;
            }
        }
        assert(foundTriple);
    }

    {
        // String escape sequences
        uzpp::Lexer lexer("\"line\\nbreak\\ttab\\\"quote\"");
        const auto tokens = lexer.tokenize();
        bool found = false;
        for (const auto& t : tokens) {
            if (t.type == uzpp::TokenType::StringLiteral) {
                assert(t.value.find("\\n") != std::string::npos);
                assert(t.value.find("\\t") != std::string::npos);
                found = true;
            }
        }
        assert(found);
    }

    {
        // Numeric literals: integer, float, hex
        uzpp::Lexer lexer("butun a = 42; haqiqiy b = 3.14; butun c = 0xFF;");
        const auto tokens = lexer.tokenize();
        int intCount = 0, floatCount = 0;
        for (const auto& t : tokens) {
            if (t.type == uzpp::TokenType::IntegerLiteral) ++intCount;
            else if (t.type == uzpp::TokenType::FloatLiteral) ++floatCount;
        }
        assert(intCount >= 2);   // 42, 0xFF
        assert(floatCount >= 1); // 3.14
    }

    {
        // Single-line and block comments are stripped
        uzpp::Lexer lexer("// izoh\nbutun /* inside */ x = 1; // tail");
        const auto tokens = lexer.tokenize();
        for (const auto& t : tokens) {
            assert(t.value.find("izoh") == std::string::npos);
            assert(t.value.find("inside") == std::string::npos);
            assert(t.value.find("tail") == std::string::npos);
        }
    }

    {
        // Compound operators are tokenized as single tokens
        uzpp::Lexer lexer("a += 1; b -= 2; c == 3; d != 4; e <= 5; f >= 6;");
        const auto tokens = lexer.tokenize();
        bool plusEq=false, minusEq=false, eqEq=false, neq=false, leq=false, geq=false;
        for (const auto& t : tokens) {
            plusEq  = plusEq  || t.value == "+=";
            minusEq = minusEq || t.value == "-=";
            eqEq    = eqEq    || t.value == "==";
            neq     = neq     || t.value == "!=";
            leq     = leq     || t.value == "<=";
            geq     = geq     || t.value == ">=";
        }
        assert(plusEq && minusEq && eqEq && neq && leq && geq);
    }

    {
        // Empty input produces only EOF — tokenize must not crash
        uzpp::Lexer lexer("");
        const auto tokens = lexer.tokenize();
        assert(tokens.empty() || tokens.back().type == uzpp::TokenType::EndOfFile);
    }

    // ------------------------------------------------------------
    // Parser unit tests
    // ------------------------------------------------------------

    {
        // Class with public/private members and constructor
        const std::string cpp = transpileSnippet(
            "sinf Hisob {"
            " yopiq: butun balans;"
            " ochiq: Hisob(butun b) : balans(b) {} butun olish() { qaytarish balans; }"
            "};");
        assert(cpp.find("class Hisob") != std::string::npos);
        assert(cpp.find("private:") != std::string::npos);
        assert(cpp.find("public:") != std::string::npos);
    }

    {
        // moslash (switch/pattern matching) with multiple cases
        const std::string cpp = transpileSnippet(
            "butun f(butun x) { moslash (x) { holat 1: qaytarish 10; holat 2: qaytarish 20; boshqa: qaytarish 0; } }");
        assert(cpp.find("if") != std::string::npos);
        assert(cpp.find("else if") != std::string::npos);
        assert(cpp.find("else") != std::string::npos);
    }

    {
        // try / catch (urinish / ushlash)
        const std::string cpp = transpileSnippet(
            "butun asosiy() { urinish { irgitish std::runtime_error(\"x\"); } ushlash (std::exception& e) { qaytarish 1; } qaytarish 0; }");
        assert(cpp.find("try") != std::string::npos);
        assert(cpp.find("catch") != std::string::npos);
        assert(cpp.find("throw") != std::string::npos);
    }

    {
        // Lambda with explicit return type
        const std::string cpp = transpileSnippet(
            "butun asosiy() { ozgaruvchan f = [](butun a, butun b) -> butun { qaytarish a + b; }; qaytarish f(1, 2); }");
        assert(cpp.find("[](") != std::string::npos);
        assert(cpp.find("-> int") != std::string::npos || cpp.find("->int") != std::string::npos);
    }

    {
        // sanab_olish (enum class)
        const std::string cpp = transpileSnippet(
            "sanab_olish Rang { qizil, yashil, kok };");
        assert(cpp.find("enum class Rang") != std::string::npos);
    }

    // ------------------------------------------------------------
    // TypeChecker unit tests
    // ------------------------------------------------------------

    {
        // Wrong arg count — error
        std::vector<uzpp::SemanticError> errs;
        bool ok = typecheckSnippet(
            "butun qosh(butun a, butun b) { qaytarish a + b; }"
            "butun asosiy() { butun s = qosh(1); qaytarish 0; }",
            &errs);
        assert(!ok);
        bool sawArgCount = false;
        for (const auto& e : errs) {
            if (e.message.find("argument") != std::string::npos) sawArgCount = true;
        }
        assert(sawArgCount);
    }

    {
        // Correct arg count — no errors
        std::vector<uzpp::SemanticError> errs;
        bool ok = typecheckSnippet(
            "butun qosh(butun a, butun b) { qaytarish a + b; }"
            "butun asosiy() { butun s = qosh(1, 2); yozish << s; qaytarish 0; }",
            &errs);
        assert(ok);
        assert(errs.empty());
    }

    {
        // Undefined identifier reported (as warning currently, but it must show up)
        std::vector<uzpp::SemanticError> errs, warns;
        typecheckSnippet(
            "butun asosiy() { yozish << notanish; qaytarish 0; }",
            &errs, &warns);
        bool sawUndef = false;
        for (const auto& d : warns) {
            if (d.message.find("notanish") != std::string::npos) sawUndef = true;
        }
        for (const auto& d : errs) {
            if (d.message.find("notanish") != std::string::npos) sawUndef = true;
        }
        assert(sawUndef);
    }

    {
        // Unused variable produces a warning
        std::vector<uzpp::SemanticError> errs, warns;
        typecheckSnippet(
            "butun asosiy() { butun ishlatilmaydi = 7; qaytarish 0; }",
            &errs, &warns);
        bool sawUnused = false;
        for (const auto& w : warns) {
            if (w.message.find("ishlatilmaydi") != std::string::npos ||
                w.message.find("ishlatilmagan") != std::string::npos) {
                sawUnused = true;
            }
        }
        assert(sawUnused);
    }

    {
        // Errors carry line/column info
        std::vector<uzpp::SemanticError> errs;
        typecheckSnippet(
            "butun qosh(butun a, butun b) { qaytarish a + b; }\n"
            "butun asosiy() { qosh(1); qaytarish 0; }",
            &errs);
        assert(!errs.empty());
        assert(errs.front().line >= 1);
    }

    // ------------------------------------------------------------
    // Codegen unit tests
    // ------------------------------------------------------------

    {
        // Test mode marker — generate() should emit a main when none provided
        const std::string cpp = transpileSnippet(
            "butun asosiy() { yozish << \"x\" << qator_oxiri; qaytarish 0; }");
        assert(cpp.find("int main") != std::string::npos);
    }

    {
        // #line directives are emitted for the source file (debug aid)
        const std::string cpp = transpileSnippet("butun asosiy() { qaytarish 0; }");
        assert(cpp.find("#line") != std::string::npos);
    }

    {
        // Range-based for (`uchun (T x : c)`) lowers to `for (...:...)`
        const std::string cpp = transpileSnippet(
            "butun asosiy() { vektor<butun> v = {1,2,3}; uchun (butun x : v) { yozish << x; } qaytarish 0; }");
        assert(cpp.find("for") != std::string::npos);
        assert(cpp.find(": ") != std::string::npos);
    }

    {
        // Abstract method (mavhum) emits virtual ... = 0
        const std::string cpp = transpileSnippet(
            "sinf Hayvon { ochiq: mavhum bosh ovoz(); };");
        assert(cpp.find("virtual") != std::string::npos);
        assert(cpp.find("= 0") != std::string::npos);
    }

    {
        // statik metod
        const std::string cpp = transpileSnippet(
            "sinf Util { ochiq: statik butun ikki(butun x) { qaytarish x * 2; } };");
        assert(cpp.find("static") != std::string::npos);
    }

    std::cout << "uzpp frontend smoke tests passed\n";
    return 0;
}
