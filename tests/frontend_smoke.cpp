#include "codegen.h"
#include "error_remap.h"
#include "lexer.h"
#include "lsp_server.h"
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

// Run TypeChecker and collect inferred types of all auto-style declarations
// (`o'zgaruvchan x = ...`) keyed by the variable's declared name. Useful for
// pinning that inferType() actually computes a Known type for a given form.
// Phase 2.2: getInferredAutoTypes() now returns map<VarDecl*, Type>; we
// flatten via aniqNomi() so the helper's contract stays the same.
std::unordered_map<std::string, std::string> inferAutoTypes(const std::string& source) {
    uzpp::Lexer lexer(source);
    const auto tokens = lexer.tokenize();
    uzpp::Parser parser(tokens);
    const auto program = parser.parse();
    uzpp::TypeChecker checker;
    checker.check(program.get());
    std::unordered_map<std::string, std::string> out;
    for (const auto& [var, type] : checker.getInferredAutoTypes()) {
        out[var->getName()] = type.aniqNomi();
    }
    return out;
}

// Phase 2.2 helper: collect inferred Types directly (preserves kind info).
// Returns map keyed on variable name → const Type pointer (lives in
// checker's internal map for the lifetime of the call). For smoke pins
// that need to assert kind / structure, not just the flattened string.
std::unordered_map<std::string, uzpp::Type> inferAutoTypeKinds(const std::string& source) {
    uzpp::Lexer lexer(source);
    const auto tokens = lexer.tokenize();
    uzpp::Parser parser(tokens);
    const auto program = parser.parse();
    uzpp::TypeChecker checker;
    checker.check(program.get());
    std::unordered_map<std::string, uzpp::Type> out;
    for (const auto& [var, type] : checker.getInferredAutoTypes()) {
        out[var->getName()] = type;
    }
    return out;
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
        // Phase 12 dan beri `Parser::parse()` istisno tashlamaydi — u xatoliklarni
        // yig'adi va chaqiruvchi `hasErrors()` orqali tekshiradi. Har bir
        // chaqiruvchi (CLI, LSP) shu shartnomaga tayanadi.
        uzpp::Lexer lexer("butun asosiy( {");
        const auto tokens = lexer.tokenize();
        uzpp::Parser parser(tokens);
        auto program = parser.parse();
        (void)program;

        assert(parser.hasErrors());
        assert(!parser.getErrors().empty());

        // Joylashuv xabarda ROSA BIR MARTA bo'lishi kerak — `recordError`
        // ilgari o'z ichida joylashuv bor xabarga yana bittasini qo'shardi.
        const std::string& first = parser.getErrors().front();
        const std::size_t firstPos = first.find("qator:");
        if (firstPos != std::string::npos) {
            assert(first.find("qator:", firstPos + 1) == std::string::npos);
        }
    }

    {
        // `<<=` / `>>=` — siljitish bilan o'zlashtirish.
        const std::string cpp = transpileSnippet(
            "butun asosiy() { butun a = 1; a <<= 3; a >>= 1; qaytarish a; }");
        assert(cpp.find("<<=") != std::string::npos);
        assert(cpp.find(">>=") != std::string::npos);
    }

    {
        // Siljitish darajasi qo'shishdan PAST: `yozish << a + b` →
        // `std::cout << (a + b)`, `(std::cout << a) + b` EMAS.
        const std::string cpp = transpileSnippet(
            "butun asosiy() { butun a = 1; butun b = 2; yozish << a + b; qaytarish 0; }");
        assert(cpp.find("(a + b)") != std::string::npos);
    }

    {
        // `bajar { ... } toki (shart);` → do/while.
        const std::string cpp = transpileSnippet(
            "butun asosiy() { butun i = 0; bajar { i++; } toki (i < 3); qaytarish 0; }");
        assert(cpp.find("do") != std::string::npos);
        assert(cpp.find("while") != std::string::npos);
    }

    {
        // Guruhlangan `holat` yorliqlari `||` ga birlashadi (C `switch` kabi).
        const std::string cpp = transpileSnippet(
            "butun asosiy() { butun o = 1;"
            " moslash (o) { holat 12: holat 1: holat 2: yozish << 1; boshqa: yozish << 2; }"
            " qaytarish 0; }");
        assert(cpp.find("||") != std::string::npos);
    }

    {
        // `holat 1, 2, 3:` — vergul bilan ajratilgan naqshlar.
        const std::string cpp = transpileSnippet(
            "butun asosiy() { butun o = 1;"
            " moslash (o) { holat 1, 2, 3: yozish << 1; boshqa: yozish << 2; }"
            " qaytarish 0; }");
        assert(cpp.find("||") != std::string::npos);
    }

    {
        // Massiv parametri: `belgi* argv[]` → `char**`.
        const std::string cpp = transpileSnippet(
            "butun asosiy(butun argc, belgi* argv[]) { qaytarish 0; }");
        assert(cpp.find("char**") != std::string::npos ||
               cpp.find("char* *") != std::string::npos ||
               cpp.find("char **") != std::string::npos);
    }

    {
        // `moslash` ichidagi `to'xtatish` — aniq, o'rgatuvchi xabar.
        std::vector<uzpp::SemanticError> errors;
        typecheckSnippet(
            "butun asosiy() { butun k = 1;"
            " moslash (k) { holat 1: yozish << 1; to'xtatish; boshqa: yozish << 2; }"
            " qaytarish 0; }",
            &errors);
        bool found = false;
        for (const auto& e : errors) {
            if (e.message.find("moslash") != std::string::npos) found = true;
        }
        assert(found);
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
            "butun asosiy() { o'zgaruvchan f = [](butun a, butun b) -> butun { qaytarish a + b; }; qaytarish f(1, 2); }");
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

    {
        // ErrorRemap: position inside an active #line region — translate to .uzpp
        const std::string cpp =
            "#include <iostream>\n"          // cpp line 1
            "using namespace std;\n"          // cpp line 2
            "#line 1 \"hello.uzpp\"\n"        // cpp line 3 — directive
            "int main() {\n"                  // cpp line 4 → uzpp 1
            "    nomavjud();\n"               // cpp line 5 → uzpp 2
            "    return 0;\n"                 // cpp line 6 → uzpp 3
            "}\n";                            // cpp line 7 → uzpp 4
        const std::string err =
            "hello.generated.cpp:5:5: error: 'nomavjud' was not declared\n"
            "    5 |     nomavjud();\n"
            "      |     ^~~~~~~~\n";
        const std::string out = uzpp::ErrorRemap::remapPositions(
            err, cpp, "hello.generated.cpp");
        assert(out.find("hello.uzpp:2:") != std::string::npos);
        assert(out.find("hello.generated.cpp:5:") == std::string::npos);
    }

    {
        // ErrorRemap: position outside any #line region — pass through, do not crash
        const std::string cpp =
            "#include <iostream>\n"
            "int boot() { return 0; }\n"
            "#line 1 \"x.uzpp\"\n"
            "int main() { return 0; }\n";
        const std::string err =
            "x.generated.cpp:2:6: error: redefinition of boot\n";
        const std::string out = uzpp::ErrorRemap::remapPositions(
            err, cpp, "x.generated.cpp");
        // line 2 is BEFORE any #line directive — must be left untouched
        assert(out.find("x.generated.cpp:2:") != std::string::npos);
    }

    {
        // ErrorRemap: ignores files we did not generate
        const std::string cpp = "#line 1 \"x.uzpp\"\nint main(){}\n";
        const std::string err = "/usr/include/c++/15/format:99:1: note: candidate\n";
        const std::string out = uzpp::ErrorRemap::remapPositions(
            err, cpp, "x.generated.cpp");
        assert(out == err);
    }

    {
        // Inlay hints: `o'zgaruvchan x = 42` should produce `: butun` after `x`.
        const std::string src =
            "butun asosiy() {\n"
            "    o'zgaruvchan x = 42;\n"
            "    o'zgaruvchan s = \"salom\";\n"
            "    qaytarish 0;\n"
            "}\n";
        const std::string hints = uzpp::LspServer::computeInlayHints(src);
        assert(hints.find(": butun") != std::string::npos);
        assert(hints.find(": matn") != std::string::npos);
    }

    {
        // Inlay hints: explicitly-typed declarations should NOT get a hint.
        const std::string src =
            "butun asosiy() {\n"
            "    butun y = 1;\n"
            "    qaytarish y;\n"
            "}\n";
        const std::string hints = uzpp::LspServer::computeInlayHints(src);
        assert(hints == "[]");
    }

    {
        // Code actions: unused variable warning yields both quick-fixes.
        const std::string src =
            "butun asosiy() {\n"
            "    butun foydalanilmagan = 5;\n"
            "    qaytarish 0;\n"
            "}\n";
        // Range covers line 1 (the unused decl) entirely.
        const std::string actions = uzpp::LspServer::computeCodeActions(
            src, "file:///t.uzpp", 0, 0, 3, 0);
        assert(actions.find("_foydalanilmagan") != std::string::npos);
        assert(actions.find("E'lonni o'chirish") != std::string::npos);
        assert(actions.find("file:///t.uzpp") != std::string::npos);
    }

    {
        // Code actions: range that misses the warning line yields no actions.
        const std::string src =
            "butun asosiy() {\n"
            "    butun foydalanilmagan = 5;\n"
            "    qaytarish 0;\n"
            "}\n";
        const std::string actions = uzpp::LspServer::computeCodeActions(
            src, "file:///t.uzpp", 2, 0, 3, 0);
        assert(actions == "[]");
    }

    {
        // Scope shadowing: a local named `yangi` should NOT emit `new` when
        // referenced — the local-name table introduced for keyword-alias
        // collisions is what makes natural variable names work in uz++.
        const std::string cpp = transpileSnippet(
            "butun asosiy() { butun yangi = 5; qaytarish yangi; }");
        assert(cpp.find("return yangi") != std::string::npos);
        assert(cpp.find("return new") == std::string::npos);
    }

    {
        // Same shadowing protection for `bor`, `kasr`, `uzun` (all of which
        // are listed in identifierTranslations / typeMap).
        const std::string cpp = transpileSnippet(
            "butun asosiy() { butun bor = 1; butun kasr = 2; butun uzun = 3; "
            "qaytarish bor + kasr + uzun; }");
        assert(cpp.find("return((bor + kasr) + uzun)") != std::string::npos);
        assert(cpp.find("uzpp::bor") == std::string::npos);
        assert(cpp.find("float") == std::string::npos);
    }

    {
        // `funksiya X(...) o'zgarmas -> T { ... }` — const method with
        // funksiya keyword AND trailing return type, previously rejected.
        const std::string cpp = transpileSnippet(
            "sinf Hisoblagich { ochiq: funksiya hisobla() o'zgarmas -> butun { qaytarish 42; } };");
        // Class methods emit return-type-first (not trailing) form, but
        // the trailing-return after `o'zgarmas` must still be honoured.
        assert(cpp.find("int hisobla() const") != std::string::npos);
    }

    {
        // `agar (...) { ... } yoki { ... }` — `yoki` after if-then must be
        // recognised as `else`, not as a stray binary operator that emits
        // `else; { ... }` (orphan compound statement).
        const std::string cpp = transpileSnippet(
            "butun asosiy() { agar (1) { } yoki { } qaytarish 0; }");
        assert(cpp.find("else;") == std::string::npos);
        assert(cpp.find("else") != std::string::npos);
    }

    {
        // Apostrophe-bearing Uzbek identifiers (`o'lcham`, `g'oya`) must
        // survive transpilation as a *meaningful* C++ identifier. They get
        // mapped to U+02BC MODIFIER LETTER APOSTROPHE (UTF-8: CA BC) — a
        // C++23 XID_Continue character — not flattened to `_` which would
        // mangle `o'lcham` and `o_lcham` into the same name.
        const std::string cpp = transpileSnippet(
            "butun asosiy() { butun o'lcham = 5; qaytarish o'lcham; }");
        // Look for the UTF-8 bytes of U+02BC immediately following 'o'.
        const std::string apos_utf8 = "\xCA\xBC";
        assert(cpp.find("o" + apos_utf8 + "lcham") != std::string::npos);
        // Must NOT use the legacy `_` mangling.
        assert(cpp.find("o_lcham") == std::string::npos);
    }

    {
        // Phase 1.3: inferType covers expression node kinds previously left
        // as "noma'lum". Each case pins the Known result so a regression in
        // inferTypeT() surfaces immediately rather than silently degrading
        // LSP hover and false-positive diagnostics.

        // Unary negation: -5 stays butun.
        {
            auto m = inferAutoTypes("butun asosiy() { o'zgaruvchan x = -5; qaytarish 0; }");
            assert(m["x"] == "butun");
        }
        // Logical not: !true is mantiqiy.
        {
            auto m = inferAutoTypes("butun asosiy() { o'zgaruvchan b = !rost; qaytarish 0; }");
            assert(m["b"] == "mantiqiy");
        }
        // Address-of and dereference round-trip.
        {
            auto m = inferAutoTypes(
                "butun asosiy() { butun y = 5; o'zgaruvchan p = &y; o'zgaruvchan d = *p; qaytarish 0; }");
            assert(m["p"] == "butun*");
            assert(m["d"] == "butun");
        }
        // Nested composite (butun** via double address-of). This stresses
        // Type::intern's stability: a previous vector-backed store would
        // have invalidated the inner Korsatkich pointer when the outer
        // Korsatkich was inserted. With deque/map-backed canon, pointers
        // stay valid and aniqNomi() returns the correct nested string.
        // Pin many nested types in one snippet to force multiple intern
        // calls on composite kinds.
        {
            auto m = inferAutoTypes(
                "butun asosiy() {"
                "  butun a = 1; haqiqiy b = 2.5; matn c = \"x\";"
                "  o'zgaruvchan pa = &a;"
                "  o'zgaruvchan ppa = &pa;"
                "  o'zgaruvchan pb = &b;"
                "  o'zgaruvchan ppb = &pb;"
                "  o'zgaruvchan pc = &c;"
                "  qaytarish 0;"
                "}");
            // After all the composites were created, each must still resolve
            // to its own correct string — proves no use-after-free.
            assert(m["pa"]  == "butun*");
            assert(m["ppa"] == "butun**");
            assert(m["pb"]  == "haqiqiy*");
            assert(m["ppb"] == "haqiqiy**");
            assert(m["pc"]  == "matn*");
        }
        // Ternary with matching branches.
        {
            auto m = inferAutoTypes(
                "butun asosiy() { o'zgaruvchan t = 1 < 2 ? 10 : 20; qaytarish 0; }");
            assert(m["t"] == "butun");
        }
        // Ternary with promotion: butun + haqiqiy → haqiqiy.
        {
            auto m = inferAutoTypes(
                "butun asosiy() { o'zgaruvchan t = 1 < 2 ? 10 : 2.5; qaytarish 0; }");
            assert(m["t"] == "haqiqiy");
        }
        // Assignment expression evaluates to its value.
        {
            auto m = inferAutoTypes(
                "butun asosiy() { butun y = 0; o'zgaruvchan v = (y = 5); qaytarish 0; }");
            assert(m["v"] == "butun");
        }
        // Increment preserves operand type.
        {
            auto m = inferAutoTypes(
                "butun asosiy() { butun y = 0; o'zgaruvchan v = ++y; qaytarish 0; }");
            assert(m["v"] == "butun");
        }
    }

    {
        // Phase 2.2: getInferredAutoType exposes structured Type, not just a
        // string. LSP can now distinguish Aniq vs Polimorf vs Korsatkich/
        // Havola/Shablon. Pin the surface so a regression that flattens back
        // to strings surfaces immediately.

        // Aniq("butun") — simple known type.
        {
            auto k = inferAutoTypeKinds(
                "butun asosiy() { o'zgaruvchan x = 5; qaytarish 0; }");
            assert(k.contains("x"));
            assert(k["x"].isAniq());
            assert(k["x"].aniqNomi() == "butun");
            assert(k["x"].tasvirla() == "butun");
        }
        // Korsatkich(Aniq("butun")) — composite, addressable.
        {
            auto k = inferAutoTypeKinds(
                "butun asosiy() { butun y = 5; o'zgaruvchan p = &y; qaytarish 0; }");
            assert(k.contains("p"));
            assert(k["p"].isAniq());                  // recursive — base is Aniq
            assert(k["p"].kind == uzpp::Type::Kind::Korsatkich);
            assert(k["p"].aniqNomi() == "butun*");
            assert(k["p"].tasvirla() == "butun* (ko'rsatkich)");
        }
        // Polimorf("T") — template parameter; stored separately from Aniq.
        // Body inference inside a template function preserves Polimorf so
        // LSP hover can annotate it.
        {
            auto k = inferAutoTypeKinds(
                "shablon<tur T> funksiya f(T x) -> T {"
                "  o'zgaruvchan q = x;"
                "  qaytarish q;"
                "}"
                "butun asosiy() { qaytarish 0; }");
            assert(k.contains("q"));
            assert(k["q"].isPolimorf());
            assert(!k["q"].isAniq());
            assert(k["q"].aniqNomi() == "T");
            assert(k["q"].tasvirla() == "T (shablon parametri)");
        }
    }

    {
        // Phase 1.4: Polimorf mode — template function bodies must not emit
        // spurious return-type-mismatch warnings, because at instantiation
        // the type parameter could match.

        auto hasReturnMismatchWarning = [](const std::vector<uzpp::SemanticError>& warnings) {
            for (const auto& w : warnings) {
                if (w.message.find("qaytarishi kerak") != std::string::npos) return true;
            }
            return false;
        };

        // (a) Returning a T-typed param from a butun-typed function:
        //     inferred is Polimorf("T") → isAniq() false → no warning.
        {
            std::vector<uzpp::SemanticError> warnings;
            typecheckSnippet(
                "shablon<tur T> funksiya f(T x) -> butun { qaytarish x; } "
                "butun asosiy() { qaytarish f(5); }",
                nullptr, &warnings);
            assert(!hasReturnMismatchWarning(warnings));
        }

        // (b) Returning a concrete butun from a T-returning template function:
        //     currentReturnType_ ("T") is in currentTemplateParams_ → suppressed.
        {
            std::vector<uzpp::SemanticError> warnings;
            typecheckSnippet(
                "shablon<tur T> funksiya g(T x) -> T { qaytarish 5; } "
                "butun asosiy() { qaytarish g(5); }",
                nullptr, &warnings);
            assert(!hasReturnMismatchWarning(warnings));
        }

        // (c) Non-template function still warns on a real mismatch — we didn't
        //     accidentally suppress legitimate diagnostics.
        {
            std::vector<uzpp::SemanticError> warnings;
            typecheckSnippet(
                "funksiya h(matn s) -> butun { qaytarish s; } "
                "butun asosiy() { qaytarish h(\"hi\"); }",
                nullptr, &warnings);
            assert(hasReturnMismatchWarning(warnings));
        }
    }

    {
        // Phase 2.3: Polimorf mode for template *class* bodies. Method
        // bodies inside `shablon<tur T> sinf X { ... }` must not emit
        // false return-type warnings, mirroring Phase 1.4 for functions.

        auto hasReturnMismatchWarning = [](const std::vector<uzpp::SemanticError>& warnings) {
            for (const auto& w : warnings) {
                if (w.message.find("qaytarishi kerak") != std::string::npos) return true;
            }
            return false;
        };

        // (a) Template class with a T-typed field returned from a butun-typed
        //     method: under old code, `qiymat` was Aniq("T") → warning fired.
        //     Now T is in currentTemplateParams_ → Polimorf → silent.
        {
            std::vector<uzpp::SemanticError> warnings;
            typecheckSnippet(
                "shablon<tur T> sinf Yashik {"
                "  ochiq: T qiymat;"
                "  funksiya olish() -> butun { qaytarish qiymat; }"
                "}; "
                "butun asosiy() { qaytarish 0; }",
                nullptr, &warnings);
            assert(!hasReturnMismatchWarning(warnings));
        }

        // (b) Template class returning concrete butun from a T-typed method:
        //     currentReturnType_ ("T") is in template params → suppressed.
        {
            std::vector<uzpp::SemanticError> warnings;
            typecheckSnippet(
                "shablon<tur T> sinf Quti {"
                "  ochiq: funksiya bering() -> T { qaytarish 42; }"
                "}; "
                "butun asosiy() { qaytarish 0; }",
                nullptr, &warnings);
            assert(!hasReturnMismatchWarning(warnings));
        }

        // (c) Non-template class with a real mismatch — STILL warns
        //     (proves Phase 2.3 didn't accidentally silence all class methods).
        {
            std::vector<uzpp::SemanticError> warnings;
            typecheckSnippet(
                "sinf Aniq {"
                "  ochiq: matn s;"
                "  funksiya berish() -> butun { qaytarish s; }"
                "}; "
                "butun asosiy() { qaytarish 0; }",
                nullptr, &warnings);
            assert(hasReturnMismatchWarning(warnings));
        }
    }

    {
        // Phase 2.4: composite types containing template params (vektor<T>,
        // Foo<T>*) are treated as Polimorf. Phase 2.3 only handled bare T —
        // composites slipped through and re-triggered the false warning.

        auto hasReturnMismatchWarning = [](const std::vector<uzpp::SemanticError>& warnings) {
            for (const auto& w : warnings) {
                if (w.message.find("qaytarishi kerak") != std::string::npos) return true;
            }
            return false;
        };

        // (a) Template class with a `vektor<T>` field returned from butun-typed
        //     method. Old: scope type "vektor<T>" → Aniq → warning.
        //     New: typeMentionsTemplateParam(true) → Polimorf → silent.
        {
            std::vector<uzpp::SemanticError> warnings;
            typecheckSnippet(
                "shablon<tur T> sinf Ro'yxat {"
                "  ochiq: vektor<T> elementlar;"
                "  funksiya olish() -> butun { qaytarish elementlar; }"
                "}; "
                "butun asosiy() { qaytarish 0; }",
                nullptr, &warnings);
            assert(!hasReturnMismatchWarning(warnings));
        }

        // (b) Template function with composite return type `-> vektor<T>`
        //     returning concrete value. Suppressed via the second check.
        {
            std::vector<uzpp::SemanticError> warnings;
            typecheckSnippet(
                "shablon<tur T> funksiya bor_qil(T x) -> vektor<T> { qaytarish 0; } "
                "butun asosiy() { qaytarish 0; }",
                nullptr, &warnings);
            assert(!hasReturnMismatchWarning(warnings));
        }

        // (c) Auto-typed variable inferred as composite-with-template-param
        //     surfaces as Polimorf in the LSP API (kind preserved across the
        //     boundary added in Phase 2.2).
        {
            auto k = inferAutoTypeKinds(
                "shablon<tur T> sinf Quti {"
                "  ochiq: vektor<T> e;"
                "  funksiya birinchi() -> T {"
                "    o'zgaruvchan ref = e;"
                "    qaytarish 0;"
                "  }"
                "}; "
                "butun asosiy() { qaytarish 0; }");
            // `ref = e` — e has scope type "vektor<T>" which mentions T.
            // After Phase 2.4: inferTypeT returns Polimorf("vektor<T>").
            // Stored via the isAniq() || isPolimorf() gate from Phase 2.2.
            assert(k.contains("ref"));
            assert(k["ref"].isPolimorf());
            assert(k["ref"].aniqNomi() == "vektor<T>");
        }

        // (d) Negative control: outside a template (no currentTemplateParams_),
        //     a real composite mismatch must still warn. Verifies
        //     typeMentionsTemplateParam doesn't blanket-silence composites.
        {
            std::vector<uzpp::SemanticError> warnings;
            typecheckSnippet(
                "sinf Konteyner {"
                "  ochiq: vektor<butun> m;"
                "  funksiya berish() -> butun { qaytarish m; }"
                "}; "
                "butun asosiy() { qaytarish 0; }",
                nullptr, &warnings);
            assert(hasReturnMismatchWarning(warnings));
        }
    }

    std::cout << "uzpp frontend smoke tests passed\n";
    return 0;
}
