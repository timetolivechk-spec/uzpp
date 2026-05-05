#pragma once

#include "tarmoq_tuzilmalar.hpp"
#include "asinxron.hpp"
#include "xavfsizlik.hpp"
#include "xatoliklar.hpp"
#include "jurnal.hpp"
#include "fayl_tizimi.hpp"
#include "kripto.hpp"

#include <iostream>
#include <functional>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#undef min
#undef max
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#endif

#if __has_include(<curl/curl.h>)
#include <curl/curl.h>
#endif

namespace uzpp::Tarmoq {

enum class SoketTuri { TCP, UDP };

class Soket {
private:
#ifdef _WIN32
    SOCKET fd_ = INVALID_SOCKET;
#else
    int fd_ = -1;
#endif
    SoketTuri turi_;

    void initWSA() {
#ifdef _WIN32
        static bool wsaInited = false;
        if (!wsaInited) {
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
            wsaInited = true;
        }
#endif
    }

public:
    Soket(SoketTuri turi = SoketTuri::TCP) : turi_(turi) {
        initWSA();
        int type = (turi == SoketTuri::TCP) ? SOCK_STREAM : SOCK_DGRAM;
#ifdef _WIN32
        fd_ = socket(AF_INET, type, (turi == SoketTuri::UDP) ? IPPROTO_UDP : IPPROTO_TCP);
#else
        fd_ = socket(AF_INET, type, 0);
#endif
    }
    
#ifdef _WIN32
    explicit Soket(SOCKET fd, SoketTuri turi) : fd_(fd), turi_(turi) {}
#else
    explicit Soket(int fd, SoketTuri turi) : fd_(fd), turi_(turi) {}
#endif

    ~Soket() { yopish(); }

    Soket(const Soket&) = delete;
    Soket& operator=(const Soket&) = delete;
    
    Soket(Soket&& boshqa) noexcept : fd_(boshqa.fd_), turi_(boshqa.turi_) {
#ifdef _WIN32
        boshqa.fd_ = INVALID_SOCKET;
#else
        boshqa.fd_ = -1;
#endif
    }
    
    Soket& operator=(Soket&& boshqa) noexcept {
        if (this != &boshqa) {
            yopish();
            fd_ = boshqa.fd_;
            turi_ = boshqa.turi_;
#ifdef _WIN32
            boshqa.fd_ = INVALID_SOCKET;
#else
            boshqa.fd_ = -1;
#endif
        }
        return *this;
    }

    bool yaroqlimi() const {
#ifdef _WIN32
        return fd_ != INVALID_SOCKET;
#else
        return fd_ >= 0;
#endif
    }

    Xavfsizlik::Natija<bool> ulanish(const std::string& ip, int port) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

        if (connect(fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
            return Xavfsizlik::xato(std::string("Ulanish xatosi (Connection failed)"));
        }
        return Xavfsizlik::muvaffaqiyat(true);
    }

    Xavfsizlik::Natija<bool> tinglash(int port) {
        int opt = 1;
        setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
            return Xavfsizlik::xato(std::string("Soketni portga bog'lashda xato (Bind error)"));
        }

        if (turi_ == SoketTuri::TCP) {
            if (listen(fd_, SOMAXCONN) < 0) {
                return Xavfsizlik::xato(std::string("Tinglash rejimiga o'tishda xato (Listen error)"));
            }
        }
        return Xavfsizlik::muvaffaqiyat(true);
    }

    Xavfsizlik::Natija<std::shared_ptr<Soket>> qabulQilish() {
        if (turi_ != SoketTuri::TCP) return Xavfsizlik::xato(std::string("Faqat TCP soketlar ulanishlarni qabul qilishi mumkin"));
        
        sockaddr_in clientAddr;
#ifdef _WIN32
        int addrLen = sizeof(clientAddr);
        SOCKET clientFd = accept(fd_, (sockaddr*)&clientAddr, &addrLen);
        if (clientFd == INVALID_SOCKET) return Xavfsizlik::xato(std::string("Qabul qilish xatosi (Accept error)"));
#else
        socklen_t addrLen = sizeof(clientAddr);
        int clientFd = accept(fd_, (sockaddr*)&clientAddr, &addrLen);
        if (clientFd < 0) return Xavfsizlik::xato(std::string("Qabul qilish xatosi (Accept error)"));
#endif
        return Xavfsizlik::muvaffaqiyat(std::make_shared<Soket>(clientFd, SoketTuri::TCP));
    }

    int yuborish(const std::string& malumot) {
        if (!yaroqlimi()) return -1;
        return send(fd_, malumot.c_str(), static_cast<int>(malumot.size()), 0);
    }
    
    int udpYuborish(const std::string& ip, int port, const std::string& malumot) {
        if (!yaroqlimi() || turi_ != SoketTuri::UDP) return -1;
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
        return sendto(fd_, malumot.c_str(), static_cast<int>(malumot.size()), 0, (sockaddr*)&addr, sizeof(addr));
    }

    std::string oqish(int buffer_size = 4096) {
        if (!yaroqlimi()) return "";
        std::vector<char> buf(buffer_size, 0);
        int bytesRead = recv(fd_, buf.data(), buffer_size - 1, 0);
        if (bytesRead > 0) {
            return std::string(buf.data(), bytesRead);
        }
        return "";
    }

    void yopish() {
#ifdef _WIN32
        if (fd_ != INVALID_SOCKET) {
            closesocket(fd_);
            fd_ = INVALID_SOCKET;
        }
#else
        if (fd_ >= 0) {
            close(fd_);
            fd_ = -1;
        }
#endif
    }
};

inline void Javob::matnQaytarish(int statusKodi, const std::string& matn) {
    std::ostringstream ss;
    ss << "HTTP/1.1 " << statusKodi << " OK\r\n";
    ss << "Content-Type: text/plain; charset=UTF-8\r\n";
    ss << "Content-Length: " << matn.size() << "\r\n";
    ss << "Connection: close\r\n\r\n";
    ss << matn;
    
    std::string response = ss.str();
#ifdef _WIN32
    if (klientSoketi != INVALID_SOCKET) {
        send(klientSoketi, response.c_str(), static_cast<int>(response.size()), 0);
        closesocket(klientSoketi);
        klientSoketi = INVALID_SOCKET;
    }
#else
    if (klientSoketi >= 0) {
        send(klientSoketi, response.c_str(), response.size(), 0);
        close(klientSoketi);
        klientSoketi = -1;
    }
#endif
}

inline void Javob::jsonQaytarish(int statusKodi, const uzpp::FaylTizimi::JsonObyekt& json) {
    std::string jsonStr = uzpp::FaylTizimi::jsonGaAylantirish(json);
    std::ostringstream ss;
    ss << "HTTP/1.1 " << statusKodi << " OK\r\n";
    ss << "Content-Type: application/json; charset=UTF-8\r\n";
    ss << "Content-Length: " << jsonStr.size() << "\r\n";
    ss << "Connection: close\r\n\r\n";
    ss << jsonStr;
    
    std::string response = ss.str();
#ifdef _WIN32
    if (klientSoketi != INVALID_SOCKET) {
        send(klientSoketi, response.c_str(), static_cast<int>(response.size()), 0);
        closesocket(klientSoketi);
        klientSoketi = INVALID_SOCKET;
    }
#else
    if (klientSoketi >= 0) {
        send(klientSoketi, response.c_str(), response.size(), 0);
        close(klientSoketi);
        klientSoketi = -1;
    }
#endif
}

inline void Javob::htmlQaytarish(int statusKodi, const std::string& html) {
    std::ostringstream ss;
    ss << "HTTP/1.1 " << statusKodi << " OK\r\n";
    ss << "Content-Type: text/html; charset=UTF-8\r\n";
    ss << "Content-Length: " << html.size() << "\r\n";
    ss << "Connection: close\r\n\r\n";
    ss << html;
    
    std::string response = ss.str();
#ifdef _WIN32
    if (klientSoketi != INVALID_SOCKET) {
        send(klientSoketi, response.c_str(), static_cast<int>(response.size()), 0);
        closesocket(klientSoketi);
        klientSoketi = INVALID_SOCKET;
    }
#else
    if (klientSoketi >= 0) {
        send(klientSoketi, response.c_str(), response.size(), 0);
        close(klientSoketi);
        klientSoketi = -1;
    }
#endif
}

inline void Javob::uiQaytarish(int statusKodi, const uzpp::Xotira::Ulashilgan<uzpp::VebUI::Element>& ui) {
    std::string html = "<!DOCTYPE html>\n<html lang=\"uz\">\n<head>\n<meta charset=\"UTF-8\">\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n<title>uz++ VebUI</title>\n</head>\n<body>\n";
    html += ui->chizish();
    html += "\n</body>\n</html>";
    htmlQaytarish(statusKodi, html);
}

inline void Javob::webSocketQabulQilish(const Sorov& sorov) {
    if (!sorov.sarlavhalar.contains("Sec-WebSocket-Key")) return;
    std::string key = sorov.sarlavhalar.at("Sec-WebSocket-Key");
    std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    auto sha1hash = uzpp::Kripto::sha1_baytlar(key + magic);
    std::string hashStr(sha1hash.begin(), sha1hash.end());
    std::string acceptKey = uzpp::Kripto::base64Kodlash(hashStr);

    std::ostringstream ss;
    ss << "HTTP/1.1 101 Switching Protocols\r\n";
    ss << "Upgrade: websocket\r\n";
    ss << "Connection: Upgrade\r\n";
    ss << "Sec-WebSocket-Accept: " << acceptKey << "\r\n\r\n";
    
    std::string response = ss.str();
#ifdef _WIN32
    send(klientSoketi, response.c_str(), static_cast<int>(response.size()), 0);
#else
    send(klientSoketi, response.c_str(), response.size(), 0);
#endif
}

class WebSocket {
    int fd_;
public:
    explicit WebSocket(int fd) : fd_(fd) {}
    
    void yuborish(const std::string& matn) {
        std::vector<uint8_t> frame;
        frame.push_back(0x81); // Text frame, FIN
        if (matn.size() <= 125) {
            frame.push_back(static_cast<uint8_t>(matn.size()));
        } else if (matn.size() <= 65535) {
            frame.push_back(126);
            frame.push_back((matn.size() >> 8) & 0xFF);
            frame.push_back(matn.size() & 0xFF);
        } else {
            frame.push_back(127);
            for(int i=7; i>=0; --i) frame.push_back((matn.size() >> (i*8)) & 0xFF);
        }
        frame.insert(frame.end(), matn.begin(), matn.end());
#ifdef _WIN32
        send(fd_, reinterpret_cast<const char*>(frame.data()), frame.size(), 0);
#else
        send(fd_, frame.data(), frame.size(), 0);
#endif
    }
    
    std::string oqish() {
        uint8_t head[2];
#ifdef _WIN32
        if (recv(fd_, reinterpret_cast<char*>(head), 2, 0) <= 0) return "";
#else
        if (recv(fd_, head, 2, 0) <= 0) return "";
#endif
        uint64_t len = head[1] & 0x7F;
        bool masked = (head[1] & 0x80) != 0;
        if (len == 126) { uint8_t ext[2]; recv(fd_, reinterpret_cast<char*>(ext), 2, 0); len = (ext[0] << 8) | ext[1]; }
        else if (len == 127) { uint8_t ext[8]; recv(fd_, reinterpret_cast<char*>(ext), 8, 0); len = 0; for(int i=0; i<8; ++i) len = (len << 8) | ext[i]; }
        
        uint8_t mask[4] = {0};
        if (masked) recv(fd_, reinterpret_cast<char*>(mask), 4, 0);
        
        std::vector<uint8_t> payload(len);
        recv(fd_, reinterpret_cast<char*>(payload.data()), len, 0);
        
        if (masked) {
            for (size_t i = 0; i < len; ++i) payload[i] ^= mask[i % 4];
        }
        return std::string(payload.begin(), payload.end());
    }
};

class Server {
private:
    int port_;
#ifdef _WIN32
    SOCKET serverSoketi_ = INVALID_SOCKET;
#else
    int serverSoketi_ = -1;
#endif
    bool ishlamoqda_ = false;
    std::unordered_map<std::string, std::function<void(Sorov, Javob)>> yonalishlar_;

    void setupSocket() {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        serverSoketi_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        u_long mode = 1;
        ioctlsocket(serverSoketi_, FIONBIO, &mode); // TRUE NON-BLOCKING!
#else
        serverSoketi_ = socket(AF_INET, SOCK_STREAM, 0);
        int flags = fcntl(serverSoketi_, F_GETFL, 0);
        fcntl(serverSoketi_, F_SETFL, flags | O_NONBLOCK); // TRUE NON-BLOCKING!
#endif

        int opt = 1;
        setsockopt(serverSoketi_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);

        bind(serverSoketi_, (sockaddr*)&address, sizeof(address));
        listen(serverSoketi_, SOMAXCONN);
    }

public:
    explicit Server(int port) : port_(port) {
        setupSocket();
    }

    ~Server() {
        ishlamoqda_ = false;
#ifdef _WIN32
        if (serverSoketi_ != INVALID_SOCKET) {
            closesocket(serverSoketi_);
        }
        WSACleanup();
#else
        if (serverSoketi_ >= 0) {
            close(serverSoketi_);
        }
#endif
    }

    void yonalish(const std::string& metod, const std::string& yol, std::function<void(Sorov, Javob)> handler) {
        std::string kalit = metod + " " + yol;
        yonalishlar_[kalit] = std::move(handler);
    }

    Asinxron::Vazifa<void> ishlash() {
        ishlamoqda_ = true;
        
        while (ishlamoqda_) {
            sockaddr_in clientAddr;
#ifdef _WIN32
            int addrLen = sizeof(clientAddr);
            SOCKET clientSocket = accept(serverSoketi_, (sockaddr*)&clientAddr, &addrLen);
            if (clientSocket == INVALID_SOCKET) {
                int err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5)); // 5ms mikropauza CPU kuymasligi uchun
                    continue; // Kutishda davom etamiz
                }
            }
#else
            socklen_t addrLen = sizeof(clientAddr);
            int clientSocket = accept(serverSoketi_, (sockaddr*)&clientAddr, &addrLen);
            if (clientSocket < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5)); // 5ms mikropauza CPU kuymasligi uchun
                    continue; // Kutishda davom etamiz
                }
            }
#endif

            // Agar ulanish kelsa, faqat shu klientni o'qiymiz
#ifdef _WIN32
            if (clientSocket != INVALID_SOCKET) {
#else
            if (clientSocket >= 0) {
#endif
#ifdef _WIN32
                u_long clientMode = 1;
                ioctlsocket(clientSocket, FIONBIO, &clientMode);
#else
                int flags = fcntl(clientSocket, F_GETFL, 0);
                fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);
#endif
                
                char buffer[65536] = {0}; // Katta JSON va fayllar uchun 64KB bufer
                int bytesRead = recv(clientSocket, buffer, 65535, 0);
                
                if (bytesRead > 0) {
                    std::string req(buffer);
                    std::istringstream stream(req);
                    Sorov sorov;
                    stream >> sorov.metod >> sorov.yol;

                    std::string line;
                    while (std::getline(stream, line) && line != "\r" && line != "\r\n") {
                        if (!line.empty() && line.back() == '\r') line.pop_back();
                        size_t colon = line.find(':');
                        if (colon != std::string::npos) {
                            std::string key = line.substr(0, colon);
                            std::string val = line.substr(colon + 1);
                            size_t start = val.find_first_not_of(" \t");
                            if (start != std::string::npos) val = val.substr(start);
                            sorov.sarlavhalar[key] = val;
                        }
                    }

                    std::ostringstream bodyStream;
                    bodyStream << stream.rdbuf();
                    sorov.tana = bodyStream.str();

                    Javob javob;
                    javob.klientSoketi = clientSocket;

                    std::string kalit = sorov.metod + " " + sorov.yol;
                    if (yonalishlar_.contains(kalit)) {
                        // Requestlarni parallel oqimlarga (threads) ajratamiz
                        std::thread([handler = yonalishlar_[kalit], sorov, javob]() mutable {
                            handler(sorov, javob);
                        }).detach();
                    } else {
                        javob.matnQaytarish(404, "404 Not Found");
                    }
                } else {
                    // Agar xat kelsa yoki client birdan yopilsa memory leak bo'lmasligi uchun
#ifdef _WIN32
                    closesocket(clientSocket);
#else
                    close(clientSocket);
#endif
                }
            }
        }
        co_return; 
    }
};

class Router {
    std::unordered_map<std::string, std::function<void(Sorov, Javob)>> yonalishlar_;
public:
    void get(const std::string& yol, std::function<void(Sorov, Javob)> handler) { yonalishlar_["GET " + yol] = std::move(handler); }
    void post(const std::string& yol, std::function<void(Sorov, Javob)> handler) { yonalishlar_["POST " + yol] = std::move(handler); }
    void put(const std::string& yol, std::function<void(Sorov, Javob)> handler) { yonalishlar_["PUT " + yol] = std::move(handler); }
    void ochirish(const std::string& yol, std::function<void(Sorov, Javob)> handler) { yonalishlar_["DELETE " + yol] = std::move(handler); }
    
    void ulash(Server* server) {
        for (const auto& [kalit, handler] : yonalishlar_) {
            size_t space = kalit.find(' ');
            server->yonalish(kalit.substr(0, space), kalit.substr(space + 1), handler);
        }
    }
};

#if __has_include(<curl/curl.h>)
class Mijoz {
    static size_t yozishKallbeki(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

public:
    static Xavfsizlik::Natija<std::string> get(const std::string& url) {
        CURL* curl = curl_easy_init();
        if (!curl) return Xavfsizlik::xato(std::string("cURL initsializatsiya xatosi"));
        
        std::string javob;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, yozishKallbeki);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &javob);
        
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) return Xavfsizlik::xato(std::string(curl_easy_strerror(res)));
        return Xavfsizlik::muvaffaqiyat(javob);
    }

    static Xavfsizlik::Natija<std::string> post(const std::string& url, const std::string& payload) {
        CURL* curl = curl_easy_init();
        if (!curl) return Xavfsizlik::xato(std::string("cURL initsializatsiya xatosi"));
        
        std::string javob;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, yozishKallbeki);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &javob);
        
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) return Xavfsizlik::xato(std::string(curl_easy_strerror(res)));
        return Xavfsizlik::muvaffaqiyat(javob);
    }
};
#endif

} // namespace uzpp::Tarmoq
