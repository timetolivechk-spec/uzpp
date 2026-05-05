#pragma once

#include "platforma.hpp"

#if !defined(UZPP_EMBEDDED) && !defined(UZPP_NO_WINDOW)
#include "grafika/tuzilmalar.hpp"
#include "grafika/hodisalar.hpp"
#include "grafika/vidjetlar.hpp"

#include <chrono>
#include <cmath>
#include <fstream>
#include <cctype>
#include <vector>
#include <thread>
#include <sstream>
#include <algorithm>

#if defined(UZPP_OS_WINDOWS)
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #include <windows.h>
    #include <GL/gl.h>

    typedef ptrdiff_t UZ_GLsizeiptr;
    typedef void (APIENTRY * UZ_PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
    typedef void (APIENTRY * UZ_PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
    typedef void (APIENTRY * UZ_PFNGLBUFFERDATAPROC) (GLenum target, UZ_GLsizeiptr size, const GLvoid *data, GLenum usage);
    #define UZ_GL_ARRAY_BUFFER 0x8892
    #define UZ_GL_STATIC_DRAW 0x88E4

    static UZ_PFNGLGENBUFFERSPROC uz_glGenBuffers = nullptr;
    static UZ_PFNGLBINDBUFFERPROC uz_glBindBuffer = nullptr;
    static UZ_PFNGLBUFFERDATAPROC uz_glBufferData = nullptr;

#elif defined(UZPP_OS_LINUX) || defined(UZPP_OS_UNIX)
    #define GL_GLEXT_PROTOTYPES
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <X11/keysym.h>
    #include <GL/gl.h>
    #include <GL/glx.h>
#endif

namespace uzpp::Grafika {

class Oyna {
private:
    int kengligi_;
    int balandligi_;
    std::string sarlavha_;
    bool yopildimi_ = false;
    
    SichqonchaHolati sichqoncha_;
    KlaviaturaHolati klaviatura_;
    
    std::shared_ptr<Vidjet> asosiyVidjet_; // Declarative ROOT
    
    GLuint fontBase_ = 0;
    GLuint vbo_ = 0;

    double deltaVaqt_ = 0.0;
    double umumiyVaqt_ = 0.0;
    std::chrono::time_point<std::chrono::high_resolution_clock> oxirgiKadr_;

#if defined(UZPP_OS_WINDOWS)
    HWND hwnd_ = nullptr;
    HDC hdc_ = nullptr;
    HGLRC hrc_ = nullptr;
#elif defined(UZPP_OS_LINUX) || defined(UZPP_OS_UNIX)
    Display* display_ = nullptr;
    Window window_ = 0;
    GLXContext context_ = nullptr;
#endif

public:
    Oyna(int k, int b, const std::string& s) : kengligi_(k), balandligi_(b), sarlavha_(s) {
        yaratish();
        oxirgiKadr_ = std::chrono::high_resolution_clock::now();
    }

    ~Oyna() { yopish(); }

    [[nodiscard]] double deltaVaqt() const { return deltaVaqt_; }
    [[nodiscard]] double umumiyVaqt() const { return umumiyVaqt_; }
    [[nodiscard]] char oxirgiBelgi() const { return klaviatura_.oxirgiBelgi; }
    
    [[nodiscard]] bool tugmaBosilgami(char tugma) { 
        int upper = toupper(tugma);
        return upper < 256 ? klaviatura_.tugmalar[upper] : false; 
    }

    // Markazlashtirish (Task 2: Layout Centering)
    void markazlashtirish(Turtburchak& vidjet, int ekranKengligi = 0, int ekranBalandligi = 0) {
        int w = ekranKengligi > 0 ? ekranKengligi : kengligi_;
        int h = ekranBalandligi > 0 ? ekranBalandligi : balandligi_;
        vidjet.x = (w / 2.0f) - (vidjet.kenglik / 2.0f);
        vidjet.y = (h / 2.0f) - (vidjet.balandlik / 2.0f);
    }

    [[nodiscard]] bool aktivmi() {
        xabarlarniUqish(); 
        return !yopildimi_;
    }

    void sahifa(std::shared_ptr<Vidjet> root) {
        asosiyVidjet_ = root;
    }

    void ishgaTushirish() {
        while(aktivmi()) {
            if (asosiyVidjet_) {
                asosiyVidjet_->hodisa(sichqoncha_);
            }
            tozalash({30, 30, 30, 255}); // Dark mode
            if (asosiyVidjet_) {
                Turtburchak tola = { 0, 0, static_cast<float>(kengligi_), static_cast<float>(balandligi_) };
                asosiyVidjet_->chizish(this, tola);
            }
            yangilash();
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS update logic lock
        }
    }

    void tozalash(Rang rang) {
        glClearColor(rang.r / 255.0f, rang.g / 255.0f, rang.b / 255.0f, rang.a / 255.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void yangilash() {
#if defined(UZPP_OS_WINDOWS)
        SwapBuffers(hdc_);
#elif defined(UZPP_OS_LINUX) || defined(UZPP_OS_UNIX)
        if (display_ && window_) glXSwapBuffers(display_, window_);
#endif
        auto xozirgiVaqt = std::chrono::high_resolution_clock::now();
        deltaVaqt_ = std::chrono::duration<double>(xozirgiVaqt - oxirgiKadr_).count();
        oxirgiKadr_ = xozirgiVaqt;
        umumiyVaqt_ += deltaVaqt_;

        sichqoncha_.chapTugmaQoyibYuborildi = false;
        klaviatura_.oxirgiBelgi = 0;
    }

    // Transformatsiyalar (Animatsiya uchun)
    void saqlashMatritsa() { glPushMatrix(); }
    void tiklashMatritsa() { glPopMatrix(); }
    void burish(float burchak, float markazX, float markazY) {
        glTranslatef(markazX, markazY, 0.0f);
        glRotatef(burchak, 0.0f, 0.0f, 1.0f);
        glTranslatef(-markazX, -markazY, 0.0f);
    }
    void surish(float dX, float dY) { glTranslatef(dX, dY, 0.0f); }
    void masshtab(float sX, float sY, float markazX, float markazY) {
        glTranslatef(markazX, markazY, 0.0f);
        glScalef(sX, sY, 1.0f);
        glTranslatef(-markazX, -markazY, 0.0f);
    }

    void surish3D(float dX, float dY, float dZ) { glTranslatef(dX, dY, dZ); }
    void burish3D(float burchak, float oqX, float oqY, float oqZ) { glRotatef(burchak, oqX, oqY, oqZ); }

    void uchOchlamliRejim(float fov = 45.0f, float zYaqin = 0.1f, float zUzoq = 1000.0f) {
        glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect = static_cast<float>(kengligi_) / static_cast<float>(balandligi_);
        float fH = std::tan(fov / 360.0f * 3.14159265f) * zYaqin;
        float fW = fH * aspect;
        glFrustum(-fW, fW, -fH, fH, zYaqin, zUzoq);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    void ikkiOchlamliRejim() {
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, kengligi_, balandligi_, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
    
    Model3D modelYuklash(const std::string& faylYoli, GLuint tekstura = 0) {
        Model3D model;
        model.tekstura = tekstura;
        std::ifstream fayl(faylYoli);
        if (!fayl.is_open()) return model;

        std::vector<float> v, vt, vn;
        std::string line;
        while (std::getline(fayl, line)) {
            std::istringstream iss(line);
            std::string type;
            iss >> type;
            if (type == "v") {
                float x, y, z; iss >> x >> y >> z;
                v.push_back(x); v.push_back(y); v.push_back(z);
            } else if (type == "vt") {
                float u, v_tex; iss >> u >> v_tex;
                vt.push_back(u); vt.push_back(1.0f - v_tex); // OpenGL V invert
            } else if (type == "vn") {
                float x, y, z; iss >> x >> y >> z;
                vn.push_back(x); vn.push_back(y); vn.push_back(z);
            } else if (type == "f") {
                std::vector<std::string> faceTokens;
                std::string token;
                while (iss >> token) faceTokens.push_back(token);
                
                // Triangulyatsiya: ko'pburchaklarni uchburchaklarga bo'lish
                for (size_t i = 1; i + 1 < faceTokens.size(); ++i) {
                    std::string tri[3] = {faceTokens[0], faceTokens[i], faceTokens[i+1]};
                    for (int j = 0; j < 3; ++j) {
                        std::vector<std::string> parts;
                        std::stringstream ss(tri[j]);
                        std::string part;
                        while (std::getline(ss, part, '/')) parts.push_back(part);
                        
                        int vi = 0, vti = 0, vni = 0;
                        if (parts.size() > 0 && !parts[0].empty()) vi = std::stoi(parts[0]);
                        if (parts.size() > 1 && !parts[1].empty()) vti = std::stoi(parts[1]);
                        if (parts.size() > 2 && !parts[2].empty()) vni = std::stoi(parts[2]);
                        
                        if (vi > 0) {
                            model.vertices.push_back(v[(vi - 1) * 3]);
                            model.vertices.push_back(v[(vi - 1) * 3 + 1]);
                            model.vertices.push_back(v[(vi - 1) * 3 + 2]);
                        }
                        if (vti > 0 && vti * 2 <= vt.size()) {
                            model.texCoords.push_back(vt[(vti - 1) * 2]);
                            model.texCoords.push_back(vt[(vti - 1) * 2 + 1]);
                        }
                        if (vni > 0 && vni * 3 <= vn.size()) {
                            model.normals.push_back(vn[(vni - 1) * 3]);
                            model.normals.push_back(vn[(vni - 1) * 3 + 1]);
                            model.normals.push_back(vn[(vni - 1) * 3 + 2]);
                        }
                        model.vertexCount++;
                    }
                }
            }
        }
        return model;
    }

    void modelChizish(const Model3D& model) {
        if (model.vertexCount == 0) return;

        if (model.tekstura > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, model.tekstura);
            glColor4ub(255, 255, 255, 255);
        } else {
            glDisable(GL_TEXTURE_2D);
        }

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, model.vertices.data());

        if (!model.texCoords.empty()) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, 0, model.texCoords.data());
        }

        if (!model.normals.empty()) {
            glEnableClientState(GL_NORMAL_ARRAY);
            glNormalPointer(GL_FLOAT, 0, model.normals.data());
        }

        glDrawArrays(GL_TRIANGLES, 0, model.vertexCount);

        glDisableClientState(GL_VERTEX_ARRAY);
        if (!model.texCoords.empty()) glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        if (!model.normals.empty()) glDisableClientState(GL_NORMAL_ARRAY);
        
        glDisable(GL_TEXTURE_2D);
    }

    void chiziqChizish(Nuqta n1, Nuqta n2, float qalinlik, Rang r) {
        glColor4ub(r.r, r.g, r.b, r.a);
        glLineWidth(qalinlik);
        float vertices[] = { n1.x, n1.y, n2.x, n2.y };
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, vertices);
        glDrawArrays(GL_LINES, 0, 2);
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    void aylanaChizish(Nuqta markaz, float radius, Rang r) {
        glColor4ub(r.r, r.g, r.b, r.a);
        const int segmentlar = 32;
        float vertices[(segmentlar + 2) * 2];
        vertices[0] = markaz.x;
        vertices[1] = markaz.y;
        for (int i = 0; i <= segmentlar; ++i) {
            float burchak = 2.0f * 3.14159265f * float(i) / float(segmentlar);
            vertices[(i + 1) * 2] = markaz.x + radius * std::cos(burchak);
            vertices[(i + 1) * 2 + 1] = markaz.y + radius * std::sin(burchak);
        }
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, vertices);
        glDrawArrays(GL_TRIANGLE_FAN, 0, segmentlar + 2);
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    void turtburchakChizish(Turtburchak t, Rang r) {
        struct Vertex { float x, y; unsigned char cr, cg, cb, ca; };
        Vertex vertices[4] = {
            {t.x, t.y, r.r, r.g, r.b, r.a},
            {t.x + t.kenglik, t.y, r.r, r.g, r.b, r.a},
            {t.x + t.kenglik, t.y + t.balandlik, r.r, r.g, r.b, r.a},
            {t.x, t.y + t.balandlik, r.r, r.g, r.b, r.a}
        };

#if defined(UZPP_OS_WINDOWS)
        if (uz_glBindBuffer) {
            uz_glBindBuffer(UZ_GL_ARRAY_BUFFER, vbo_);
            uz_glBufferData(UZ_GL_ARRAY_BUFFER, sizeof(vertices), vertices, UZ_GL_STATIC_DRAW);
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_COLOR_ARRAY);
            glVertexPointer(2, GL_FLOAT, sizeof(Vertex), (void*)0);
            glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), (void*)(sizeof(float)*2));
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_VERTEX_ARRAY);
            return;
        }
#endif
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].x);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), &vertices[0].cr);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    // BMP Rasm (Tekstura) yuklash
    GLuint teksturaYuklash(const std::string& faylYoli) {
        std::ifstream fayl(faylYoli, std::ios::binary);
        if (!fayl) return 0;

        unsigned char header[54];
        if (!fayl.read(reinterpret_cast<char*>(header), 54)) return 0;
        if (header[0] != 'B' || header[1] != 'M') return 0;

        int width = *(int*)&header[18];
        int height = *(int*)&header[22];
        
        // Heap-buffer-overflow himoyasi
        if (width <= 0 || height <= 0 || width > 16384 || height > 16384) return 0;

        int imageSize = *(int*)&header[34];
        int minSize = width * height * 3;
        if (imageSize < minSize) imageSize = minSize; // Faqat haqiqiy o'lchamga ruxsat

        std::vector<unsigned char> data(imageSize);
        fayl.read(reinterpret_cast<char*>(data.data()), imageSize);

        // BGR ni RGB ga o'tkazish
        for (int i = 0; i < minSize; i += 3) {
            unsigned char tmp = data[i];
            data[i] = data[i + 2];
            data[i + 2] = tmp;
        }

        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        return tex;
    }

    // Yuklangan Teksturani chizish
    void rasmChizish(Turtburchak t, GLuint tex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
        glColor4ub(255, 255, 255, 255);

        float vertices[] = { t.x, t.y, t.x + t.kenglik, t.y, t.x + t.kenglik, t.y + t.balandlik, t.x, t.y + t.balandlik };
        float texCoords[] = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, vertices);
        glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisable(GL_TEXTURE_2D);
    }

    void matnChizish(float x, float y, const std::string& matn, Rang r) {
        glColor4ub(r.r, r.g, r.b, r.a);
        glRasterPos2f(x, y + 16.0f);
        glPushAttrib(GL_LIST_BIT);
        glListBase(fontBase_ - 32); 
        glCallLists(static_cast<GLsizei>(matn.length()), GL_UNSIGNED_BYTE, matn.c_str());
        glPopAttrib();
    }

private:

    int min(int a, int b) { return a < b ? a : b; }
    int max(int a, int b) { return a > b ? a : b; }

#if defined(UZPP_OS_WINDOWS)
    static LRESULT CALLBACK OynaJarayoni(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        Oyna* oyna = reinterpret_cast<Oyna*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (uMsg == WM_CREATE) {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            oyna = reinterpret_cast<Oyna*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(oyna));
        }

        if (oyna) {
            switch (uMsg) {
                case WM_KEYDOWN:  if (wParam < 256) oyna->klaviatura_.tugmalar[wParam] = true; break;
                case WM_KEYUP:    if (wParam < 256) oyna->klaviatura_.tugmalar[wParam] = false; break;
                case WM_CHAR: 
                    if (wParam >= 32 && wParam <= 126) oyna->klaviatura_.oxirgiBelgi = static_cast<char>(wParam);
                    else if (wParam == 8) oyna->klaviatura_.oxirgiBelgi = '\b'; 
                    break;
                case WM_MOUSEMOVE:
                    oyna->sichqoncha_.x = static_cast<float>(LOWORD(lParam));
                    oyna->sichqoncha_.y = static_cast<float>(HIWORD(lParam));
                    break;
                case WM_LBUTTONDOWN: oyna->sichqoncha_.chapTugmaBosildi = true; break;
                case WM_LBUTTONUP:
                    oyna->sichqoncha_.chapTugmaBosildi = false;
                    oyna->sichqoncha_.chapTugmaQoyibYuborildi = true;
                    break;
                case WM_CLOSE: case WM_DESTROY:
                    oyna->yopildimi_ = true;
                    PostQuitMessage(0);
                    return 0;
            }
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
#endif

    void xabarlarniUqish() {
#if defined(UZPP_OS_WINDOWS)
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) yopildimi_ = true;
            TranslateMessage(&msg); DispatchMessage(&msg);
        }
#elif defined(UZPP_OS_LINUX) || defined(UZPP_OS_UNIX)
        if (!display_) return;
        XEvent xev;
        while (XPending(display_) > 0) {
            XNextEvent(display_, &xev);
            if (xev.type == ClientMessage) yopildimi_ = true;
            else if (xev.type == MotionNotify) {
                sichqoncha_.x = static_cast<float>(xev.xmotion.x);
                sichqoncha_.y = static_cast<float>(xev.xmotion.y);
            } else if (xev.type == ButtonPress) {
                if (xev.xbutton.button == 1) sichqoncha_.chapTugmaBosildi = true;
            } else if (xev.type == ButtonRelease) {
                if (xev.xbutton.button == 1) {
                    sichqoncha_.chapTugmaBosildi = false; sichqoncha_.chapTugmaQoyibYuborildi = true;
                }
            } else if (xev.type == KeyPress) {
                KeySym sym = XLookupKeysym(&xev.xkey, 0);
                if (sym < 256) klaviatura_.tugmalar[sym] = true;
                char szBuf[8];
                int len = XLookupString(&xev.xkey, szBuf, sizeof(szBuf), &sym, nullptr);
                if (len > 0) {
                    if (szBuf[0] == '\b' || szBuf[0] == 127) klaviatura_.oxirgiBelgi = '\b';
                    else if (szBuf[0] >= 32 && szBuf[0] <= 126) klaviatura_.oxirgiBelgi = szBuf[0];
                }
            } else if (xev.type == KeyRelease) {
                KeySym sym = XLookupKeysym(&xev.xkey, 0);
                if (sym < 256) klaviatura_.tugmalar[sym] = false;
            }
        }
#endif
    }

    void yaratish() {
#if defined(UZPP_OS_WINDOWS)
        HINSTANCE hInstance = GetModuleHandle(nullptr);
        WNDCLASS wc = {0};
        wc.lpfnWndProc = OynaJarayoni;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = "UzppOynaSinfiV15";
        RegisterClass(&wc);

        hwnd_ = CreateWindowExA(0, "UzppOynaSinfiV15", sarlavha_.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, kengligi_, balandligi_, nullptr, nullptr, hInstance, this);
        hdc_ = GetDC(hwnd_);

        PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 8, 0, PFD_MAIN_PLANE, 0, 0, 0, 0 };
        SetPixelFormat(hdc_, ChoosePixelFormat(hdc_, &pfd), &pfd);

        hrc_ = wglCreateContext(hdc_);
        wglMakeCurrent(hdc_, hrc_);

        fontBase_ = glGenLists(96);
        HFONT font = CreateFontA(-16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "Arial");
        HGDIOBJ oldFont = SelectObject(hdc_, font);
        wglUseFontBitmaps(hdc_, 32, 96, fontBase_);
        SelectObject(hdc_, oldFont);
        DeleteObject(font);

        uz_glGenBuffers = (UZ_PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
        uz_glBindBuffer = (UZ_PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
        uz_glBufferData = (UZ_PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
        if (uz_glGenBuffers) uz_glGenBuffers(1, &vbo_);

        ShowWindow(hwnd_, SW_SHOW);

#elif defined(UZPP_OS_LINUX) || defined(UZPP_OS_UNIX)
        display_ = XOpenDisplay(nullptr);
        if (!display_) return;

        int attributes[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
        XVisualInfo* vi = glXChooseVisual(display_, 0, attributes);
        if (!vi) return;

        Window root = DefaultRootWindow(display_);
        XSetWindowAttributes swa;
        swa.colormap = XCreateColormap(display_, root, vi->visual, AllocNone);
        swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

        window_ = XCreateWindow(display_, root, 0, 0, kengligi_, balandligi_, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
        XMapWindow(display_, window_);
        XStoreName(display_, window_, sarlavha_.c_str());

        context_ = glXCreateContext(display_, vi, nullptr, GL_TRUE);
        glXMakeCurrent(display_, window_, context_);

        Atom wmDeleteMessage = XInternAtom(display_, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(display_, window_, &wmDeleteMessage, 1);
        
        fontBase_ = glGenLists(96);
        XFontStruct* fontInfo = XLoadQueryFont(display_, "fixed");
        if (fontInfo) {
            glXUseXFont(fontInfo->fid, 32, 96, fontBase_);
            XFreeFont(display_, fontInfo);
        }

        glGenBuffers(1, &vbo_);
#endif

        glViewport(0, 0, kengligi_, balandligi_);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, kengligi_, balandligi_, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_POLYGON_SMOOTH);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    }

    void yopish() {
#if defined(UZPP_OS_WINDOWS)
        if (fontBase_) glDeleteLists(fontBase_, 96);
        // Cleanups...
        if (hrc_) { wglMakeCurrent(nullptr, nullptr); wglDeleteContext(hrc_); }
        if (hdc_) ReleaseDC(hwnd_, hdc_);
        if (hwnd_) DestroyWindow(hwnd_);
#elif defined(UZPP_OS_LINUX) || defined(UZPP_OS_UNIX)
        if (fontBase_) glDeleteLists(fontBase_, 96);
        if (display_) {
            glXMakeCurrent(display_, None, nullptr);
            if (context_) glXDestroyContext(display_, context_);
            if (window_) XDestroyWindow(display_, window_);
            XCloseDisplay(display_);
        }
#endif
    }
};

inline void MatnVidjeti::chizish(Oyna* oyna, Turtburchak hudud) {
    oyna->matnChizish(hudud.x + 15, hudud.y + (hudud.balandlik / 2.0f) - 8.0f, matnFunc_(), {240, 240, 240, 255});
}

inline void TugmaVidjeti::chizish(Oyna* oyna, Turtburchak hudud) {
    oxirgiHudud_ = hudud;
    if (hover_) {
        hoverAnim_ += static_cast<float>(oyna->deltaVaqt() * 10.0);
        if (hoverAnim_ > 1.0f) hoverAnim_ = 1.0f;
    } else {
        hoverAnim_ -= static_cast<float>(oyna->deltaVaqt() * 10.0);
        if (hoverAnim_ < 0.0f) hoverAnim_ = 0.0f;
    }
    
    unsigned char r = static_cast<unsigned char>(70 + (100 - 70) * hoverAnim_);
    unsigned char g = static_cast<unsigned char>(130 + (150 - 130) * hoverAnim_);
    unsigned char b = static_cast<unsigned char>(180 + (200 - 180) * hoverAnim_);
    
    Rang fonRangi = {r, g, b, 255};
    
    // Padding
    hudud.x += 10;
    hudud.y += 10;
    hudud.kenglik -= 20;
    hudud.balandlik -= 20;
    
    oyna->turtburchakChizish(hudud, fonRangi);
    oyna->matnChizish(hudud.x + (hudud.kenglik / 2.0f) - static_cast<float>(matn_.size() * 4), 
                      hudud.y + (hudud.balandlik / 2.0f) - 8.0f, matn_, {255, 255, 255, 255});
}

inline bool TugmaVidjeti::hodisa(SichqonchaHolati sh) {
    hover_ = (sh.x >= oxirgiHudud_.x + 10 && sh.x <= oxirgiHudud_.x + oxirgiHudud_.kenglik - 10 &&
              sh.y >= oxirgiHudud_.y + 10 && sh.y <= oxirgiHudud_.y + oxirgiHudud_.balandlik - 10);
    if (hover_ && sh.chapTugmaQoyibYuborildi) {
        if (onClick_) onClick_();
        return true;
    }
    return false;
}

// Rasm vidjeti uchun chizish amali
inline void RasmVidjeti::chizish(Oyna* oyna, Turtburchak hudud) {
    oyna->rasmChizish(hudud, tekstura_);
}

// === ANIMATSIYA VIDJETLARI IMPLEMENTATSIYASI ===

inline AylanuvchiVidjet::AylanuvchiVidjet(std::shared_ptr<Vidjet> bola, float tezlik)
    : bola_(std::move(bola)), tezlik_(tezlik) {}
    
inline void AylanuvchiVidjet::chizish(Oyna* oyna, Turtburchak hudud) {
    burchak_ += tezlik_ * static_cast<float>(oyna->deltaVaqt());
    if (burchak_ >= 360.0f) burchak_ -= 360.0f;
    else if (burchak_ <= -360.0f) burchak_ += 360.0f;
    
    oyna->saqlashMatritsa();
    oyna->burish(burchak_, hudud.x + hudud.kenglik / 2.0f, hudud.y + hudud.balandlik / 2.0f);
    bola_->chizish(oyna, hudud);
    oyna->tiklashMatritsa();
}
inline bool AylanuvchiVidjet::hodisa(SichqonchaHolati sh) { return bola_->hodisa(sh); }

inline SuzuvchiVidjet::SuzuvchiVidjet(std::shared_ptr<Vidjet> bola, float amplituda, float chastota)
    : bola_(std::move(bola)), amplituda_(amplituda), chastota_(chastota) {}
    
inline void SuzuvchiVidjet::chizish(Oyna* oyna, Turtburchak hudud) {
    float ofsetY = std::sin(static_cast<float>(oyna->umumiyVaqt()) * chastota_ * 3.14159f * 2.0f) * amplituda_;
    
    oyna->saqlashMatritsa();
    oyna->surish(0.0f, ofsetY);
    bola_->chizish(oyna, hudud);
    oyna->tiklashMatritsa();
}
inline bool SuzuvchiVidjet::hodisa(SichqonchaHolati sh) { return bola_->hodisa(sh); }

inline PulsatsiyaVidjeti::PulsatsiyaVidjeti(std::shared_ptr<Vidjet> bola, float tezlik)
    : bola_(std::move(bola)), tezlik_(tezlik) {}
    
inline void PulsatsiyaVidjeti::chizish(Oyna* oyna, Turtburchak hudud) {
    float s = 1.0f + 0.05f * std::sin(static_cast<float>(oyna->umumiyVaqt()) * tezlik_ * 3.14159f * 2.0f);
    
    oyna->saqlashMatritsa();
    oyna->masshtab(s, s, hudud.x + hudud.kenglik / 2.0f, hudud.y + hudud.balandlik / 2.0f);
    bola_->chizish(oyna, hudud);
    oyna->tiklashMatritsa();
}
inline bool PulsatsiyaVidjeti::hodisa(SichqonchaHolati sh) { return bola_->hodisa(sh); }

} // namespace uzpp::Grafika
#endif
