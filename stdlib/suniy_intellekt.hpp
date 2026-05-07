#pragma once
// status: REAL — LLM integratsiyasi cURL orqali amalga oshiriladi (yoki stub qaytaradi).
// This module provides LLM integration with an appropriate fallback if cURL is missing.

#include "platforma.hpp"
#include "tarmoq.hpp"
#include "fayl_tizimi.hpp"
#include "xavfsizlik.hpp"
#include "matn.hpp"

#include <string>
#include <utility>

namespace uzpp::SuniyIntellekt {

#if !defined(UZPP_EMBEDDED)
#if __has_include(<curl/curl.h>)
class LLM {
    std::string apiUrl_;
    std::string model_;
public:
    // Standart Ollama API (localhost:11434) yoki OpenAI mos API
    explicit LLM(std::string model = "llama3", std::string apiUrl = "http://localhost:11434/api/generate")
        : apiUrl_(std::move(apiUrl)), model_(std::move(model)) {}

    Xavfsizlik::Natija<std::string> sorash(const std::string& prompt) {
        FaylTizimi::JsonQiymat req;
        req.objektQosh("model", FaylTizimi::JsonQiymat(model_));
        req.objektQosh("prompt", FaylTizimi::JsonQiymat(prompt));
        req.objektQosh("stream", FaylTizimi::JsonQiymat(false));
        
        std::string payload = FaylTizimi::jsonQiymatStringKilish(req);
        auto res = Tarmoq::Mijoz::post(apiUrl_, payload);
        if (!res) return Xavfsizlik::xato(res.error());
        
        std::string javobJson = res.value();
        std::string qidiruv = "\"response\":\""; // Ollama javob formati
        size_t pos = javobJson.find(qidiruv);
        if (pos != std::string::npos) {
            pos += qidiruv.length();
            size_t end = javobJson.find("\"", pos);
            std::string text = javobJson.substr(pos, end - pos);
            text = Matn::almashtirish(text, "\\n", "\n");
            text = Matn::almashtirish(text, "\\\"", "\"");
            return Xavfsizlik::muvaffaqiyat(text);
        }
        return Xavfsizlik::muvaffaqiyat(javobJson); // Fallback: to'liq JSON qaytarish
    }
};
#else
class LLM {
    std::string apiUrl_;
    std::string model_;
public:
    explicit LLM(std::string model = "llama3", std::string apiUrl = "http://localhost:11434/api/generate")
        : apiUrl_(std::move(apiUrl)), model_(std::move(model)) {}

    Xavfsizlik::Natija<std::string> sorash(const std::string& prompt) {
        return Xavfsizlik::xato(std::string("cURL kutubxonasi topilmadi. LLM so'rovi bajarilmadi."));
    }
};
#endif
#endif

} // namespace uzpp::SuniyIntellekt