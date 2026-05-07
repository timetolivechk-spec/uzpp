#pragma once
// status: EXPERIMENTAL — faqat data tuzilmalari (Nuqta, Rang, 3D Model), logika yo'q.
// Data structures only — no logic. See docs/stdlib-status.md.

#include <vector>

namespace uzpp::Grafika {
    struct Nuqta { float x; float y; };
    struct Turtburchak { float x; float y; float kenglik; float balandlik; };
    struct Rang { unsigned char r; unsigned char g; unsigned char b; unsigned char a = 255; };

    // 3D Model ma'lumotlari (.obj)
    struct Model3D {
        std::vector<float> vertices;
        std::vector<float> texCoords;
        std::vector<float> normals;
        int vertexCount = 0;
        unsigned int tekstura = 0;
    };
}
