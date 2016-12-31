#pragma once

#include "SarCommon.h"


namespace sarSmartar {
    class SarSmart;

    class CLASS_DECLSPEC SarCameraImageDrawer : SarNonCopyable
    {
    public:
        WINAPI SarCameraImageDrawer(SarSmart* smart);
        WINAPI ~SarCameraImageDrawer();
        bool WINAPI sarIsConstructorFailed() const;
        
        int32_t WINAPI sarSetDrawRange(float x1, float y1, float x2, float y2);
        int32_t WINAPI sarSetRotation(SarRotation rotation);
        int32_t WINAPI sarSetFlipX(bool flipX);
        int32_t WINAPI sarSetFlipY(bool flipY);
        
        int32_t WINAPI sarStart();
        int32_t WINAPI sarStop();
        int32_t WINAPI sarDraw(const SarImage& image);
        int32_t WINAPI sarDraw(const SarImage& image, const SarRect& rect);

    private:
        struct SarImpl;
        SarImpl* impl_;
    };
} // end of namespace smartar
