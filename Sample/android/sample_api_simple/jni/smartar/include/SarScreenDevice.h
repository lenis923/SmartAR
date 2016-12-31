#pragma once

#include "SarCommon.h"


namespace sarSmartar {
    class SarSmart;
    
    class CLASS_DECLSPEC SarScreenDevice : SarNonCopyable
    {
    public:
        WINAPI SarScreenDevice(SarSmart* smart);
        WINAPI ~SarScreenDevice();
        bool WINAPI sarIsConstructorFailed() const;

        int32_t WINAPI sarGetRotation(SarRotation* rotation) const;
        
    private:
        struct SarImpl;
        SarImpl* impl_;
    };
} // end of namespace smartar
