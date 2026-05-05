#pragma once

namespace uzpp::Grafika {
    struct KlaviaturaHolati {
        bool tugmalar[256] = {false};
        char oxirgiBelgi = 0;
    };

    struct SichqonchaHolati {
        float x = 0.0f;
        float y = 0.0f;
        bool chapTugmaBosildi = false;
        bool chapTugmaQoyibYuborildi = false;
    };
}
