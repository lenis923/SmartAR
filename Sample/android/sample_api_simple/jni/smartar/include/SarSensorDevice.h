#pragma once

#include "SarCommon.h"
#include "SarPlatformDef.h"


namespace sarSmartar {
    class SarSmart;

    class CLASS_DECLSPEC SarSensorState : SarNonCopyable
    {
    public:
        WINAPI SarSensorState();
        WINAPI SarSensorState(const SarSensorState &other);
        WINAPI ~SarSensorState();
        SarSensorState & WINAPI operator = (const SarSensorState &other);
        
    private:
        struct SarImpl;
        SarImpl *impl_;
        
        friend struct SarSensorListenerProxy;
        friend class SarRecognizer;
    };
    
    class CLASS_DECLSPEC SarSensorDeviceInfo : SarNonCopyable
    {
    public:
        WINAPI SarSensorDeviceInfo();
        WINAPI ~SarSensorDeviceInfo();

        WINAPI SarSensorDeviceInfo(const SarSensorDeviceInfo& rhs);
        SarSensorDeviceInfo& WINAPI operator=(const SarSensorDeviceInfo& rhs);

    private:
        struct SarImpl;
        SarImpl* impl_;
        
        friend class SarSensorDevice;
        friend class SarRecognizer;
    };
    
    class CLASS_DECLSPEC SarSensorListener
    {
    public:
        virtual WINAPI ~SarSensorListener() {}
        virtual void WINAPI sarOnSensor(const SarSensorState& state) = 0;
    };
    
    class CLASS_DECLSPEC SarSensorDevice : SarNonCopyable
    {
    public:
        WINAPI SarSensorDevice(SarSmart* smart, void* nativeDevice = NULL);
        WINAPI ~SarSensorDevice();
        bool WINAPI sarIsConstructorFailed() const;
        
        // setting
        int32_t WINAPI sarSetSensorListener(SarSensorListener* listener);
        int32_t WINAPI sarSetOwningNativeDevice(bool isOwning);
        
        // get info
        int32_t WINAPI sarGetDeviceInfo(SarSensorDeviceInfo* info) const;
        int32_t WINAPI sarGetNativeDevice(void** nativeDevice) const;

        // start and stop
        int32_t WINAPI sarStart();
        int32_t WINAPI sarStop();

    private:
        struct SarImpl;
        SarImpl* impl_;
    };
} // end of namespace smartar
