#pragma once

#include "SarPlatformDef.h"

#include <cstddef>


namespace sarSmartar {
    class SarSmart;
    
    // * Android: Note that the constants below must match the java-code constants. 
    const int32_t SAR_OK                        =           0;
    const int32_t SAR_ERROR_EXPIRED_LICENSE     = -2138308609;
    const int32_t SAR_ERROR_UNINITIALIZED       = -2138308608;
#ifdef TARGET_SMARTAR_WINDOWS
#	undef SAR_ERROR_ALREADY_INITIALIZED
#endif
    const int32_t SAR_ERROR_ALREADY_INITIALIZED = -2138308607;
	const int32_t SAR_ERROR_OUT_OF_MEMORY       = -2138308606;
    const int32_t SAR_ERROR_NOT_STOPPED         = -2138308605;
#ifdef TARGET_SMARTAR_WINDOWS
#	undef SAR_ERROR_NOT_EMPTY
#endif
    const int32_t SAR_ERROR_NOT_EMPTY           = -2138308604;
    const int32_t SAR_ERROR_INVALID_VALUE       = -2138308603;
    const int32_t SAR_ERROR_INVALID_POINTER     = -2138308602;
#ifdef TARGET_SMARTAR_WINDOWS
#	undef SAR_ERROR_ALREADY_REGISTERED
#endif
    const int32_t SAR_ERROR_ALREADY_REGISTERED  = -2138308601;
    const int32_t SAR_ERROR_NOT_REGISTERED      = -2138308600;
    const int32_t SAR_ERROR_ALREADY_STARTED     = -2138308599;
    const int32_t SAR_ERROR_NOT_STARTED         = -2138308598;
    const int32_t SAR_ERROR_NOT_REQUIRED        = -2138308597;
    const int32_t SAR_ERROR_VERSION_MISSMATCH   = -2138308596;
    const int32_t SAR_ERROR_NO_DICTIONARY       = -2138308595;
#ifdef TARGET_SMARTAR_WINDOWS
#	undef SAR_ERROR_BUSY
#endif
    const int32_t SAR_ERROR_BUSY                = -2138308594;
    
    // * Android: Note that the constants below must match the java-code constants. 
    enum SarFacing {
        SAR_FACING_BACK,
		SAR_FACING_FRONT,
    };
    
    // * Android: Note that the constants below must match the java-code constants. 
    enum SarRotation {
    	SAR_ROTATION_0   =   0,
		SAR_ROTATION_90  =  90,
		SAR_ROTATION_180 = 180,
		SAR_ROTATION_270 = 270,
    };
    
    // * Android: Note that the constants below must match the java-code constants. 
    enum SarImageFormat {
    	SAR_IMAGE_FORMAT_L8,
		SAR_IMAGE_FORMAT_YCRCB420,
		SAR_IMAGE_FORMAT_YCBCR420,
		SAR_IMAGE_FORMAT_RGBA8888,
		SAR_IMAGE_FORMAT_JPEG,
    };
    
    struct CLASS_DECLSPEC SarVector2
    {
        float x_;
        float y_;

        WINAPI SarVector2()
        : x_(0), y_(0) {}

        WINAPI SarVector2(float x, float y)
        : x_(x), y_(y) {}

        void WINAPI set(float x, float y)
        {
            x_ = x;
            y_ = y;
        }
    };
    
    struct CLASS_DECLSPEC SarVector3
    {
        float x_;
        float y_;
        float z_;

        WINAPI SarVector3()
        : x_(0), y_(0), z_(0) {}

        WINAPI SarVector3(float x, float y, float z)
        : x_(x), y_(y), z_(z) {}

        void WINAPI set(float x, float y, float z)
        {
            x_ = x;
            y_ = y;
            z_ = z;
        }

        SarVector3& WINAPI operator+=(const SarVector3& rhs) {
            x_ += rhs.x_;
            y_ += rhs.y_;
            z_ += rhs.z_;
            return *this;
        }

        SarVector3& WINAPI operator-=(const SarVector3& rhs) {
            x_ -= rhs.x_;
            y_ -= rhs.y_;
            z_ -= rhs.z_;
            return *this;
        }

        SarVector3& WINAPI operator*=(float rhs) {
            x_ *= rhs;
            y_ *= rhs;
            z_ *= rhs;
            return *this;
        }

        SarVector3& WINAPI operator/=(float rhs) {
            x_ /= rhs;
            y_ /= rhs;
            z_ /= rhs;
            return *this;
        }

        SarVector3 WINAPI operator+(const SarVector3& rhs) const {
            return SarVector3(x_ + rhs.x_, y_ + rhs.y_, z_ + rhs.z_);
        }

        SarVector3 WINAPI operator-(const SarVector3& rhs) const {
            return SarVector3(x_ - rhs.x_, y_ - rhs.y_, z_ - rhs.z_);
        }

        SarVector3 WINAPI operator*(float rhs) const {
            return SarVector3(x_ * rhs, y_ * rhs, z_ * rhs);
        }

        SarVector3 WINAPI operator/(float rhs) const {
            return SarVector3(x_ / rhs, y_ / rhs, z_ / rhs);
        }
    };

    struct CLASS_DECLSPEC SarQuaternion
    {
        static const SarQuaternion IDENTITY;

        float w_;
        float x_;
        float y_;
        float z_;

        WINAPI SarQuaternion()
        : w_(1), x_(0), y_(0), z_(0) {}

        WINAPI SarQuaternion(float w, float x, float y, float z)
        : w_(w), x_(x), y_(y), z_(z) {}

        void WINAPI set(float w, float x, float y, float z)
        {
            w_ = w;
            x_ = x;
            y_ = y;
            z_ = z;
        }
        
        SarQuaternion WINAPI operator-() const
        {
            return SarQuaternion(w_, -x_, -y_, -z_);
        }
        
        SarQuaternion WINAPI operator*(const SarQuaternion& rhs) const;
        
        SarQuaternion& WINAPI operator*=(const SarQuaternion& rhs)
        {
            *this = *this * rhs;
            return *this;
        }
    };
    
    struct CLASS_DECLSPEC SarMatrix44
    {
        static const int32_t NUM_VALUES = 16;

        static const SarMatrix44 IDENTITY;

        float values_[NUM_VALUES];

        WINAPI SarMatrix44();

        WINAPI SarMatrix44(const float* values);
        
        SarMatrix44 WINAPI operator*(const SarMatrix44& rhs) const;
        
        SarVector3 WINAPI operator*(const SarVector3& rhs) const;
        
        void WINAPI sarSetRotation(const SarQuaternion& quat);

        void WINAPI sarSetTranslation(float x, float y, float z);

        void WINAPI sarSetScaling(float x, float y, float z);
    };
    
    struct CLASS_DECLSPEC SarTriangle2
    {
        static const int32_t NUM_POINTS = 3;

        SarVector2 points_[NUM_POINTS];
    };
    
    struct CLASS_DECLSPEC SarTriangle3
    {
        static const int32_t NUM_POINTS = 3;

        SarVector3 points_[NUM_POINTS];
    };
    
    struct CLASS_DECLSPEC SarSize
    {
        int32_t width_;
        int32_t height_;

        WINAPI SarSize()
        : width_(0), height_(0) {}

        WINAPI SarSize(int32_t width, int32_t height)
        : width_(width), height_(height) {}
        
        bool WINAPI operator==(const SarSize& rhs) const
        {
            return width_ == rhs.width_ && height_ == rhs.height_;
        }
    };

    struct CLASS_DECLSPEC SarRect
    {
        int32_t left_;
        int32_t top_;
        int32_t right_;
        int32_t bottom_;

        WINAPI SarRect()
        : left_(0), top_(0), right_(0), bottom_(0) {}

        WINAPI SarRect(int32_t left, int32_t top, int32_t right, int32_t bottom)
        : left_(left), top_(top), right_(right), bottom_(bottom) {}
        
        void WINAPI set(int32_t left, int32_t top, int32_t right, int32_t bottom)
        {
            left_ = left;
            top_ = top;
            right_ = right;
            bottom_ = bottom;
        }

        int32_t WINAPI width() const { return right_ - left_; }

        int32_t WINAPI height() const { return bottom_ - top_; }
        
        bool WINAPI operator==(const SarRect& rhs) const
        {
            return left_ == rhs.left_ && top_ == rhs.top_ && right_ == rhs.right_ && bottom_ == rhs.bottom_;
        }
        
        bool WINAPI operator!=(const SarRect& rhs) const
        {
            return !(*this == rhs);
        }
        
        SarRect WINAPI operator/(int rhs) const
        {
            return SarRect(left_ / rhs, top_ / rhs, right_ / rhs, bottom_ / rhs);
        }
    };
    
    
    class SarNonCopyable
    {
    protected:
        SarNonCopyable() { }
    private:
        SarNonCopyable(const SarNonCopyable &);
        SarNonCopyable &operator = (const SarNonCopyable &);
    };
    
    
    class CLASS_DECLSPEC SarImage
    {
    public:
        WINAPI SarImage(SarSmart* smart);
        WINAPI SarImage(const SarImage &other);
        WINAPI ~SarImage();
        SarImage & WINAPI operator = (const SarImage &other);
        
        void WINAPI setData(unsigned char* pixels);
        unsigned char* WINAPI getData();
        void WINAPI setWidth(int32_t width);
        int32_t WINAPI getWidth();
        void WINAPI setHeight(int32_t height);
        int32_t WINAPI getHeight();
        void WINAPI setStride(int32_t stride);
        int32_t WINAPI getStride();
        void WINAPI setImageFormat(SarImageFormat format);
        SarImageFormat WINAPI getImageFormat();

    private:
        struct SarImpl;
        SarImpl *impl_;

        friend class SarImageHolder;
        friend class SarImageQueue;
        friend class SarCameraImageDrawer;
        friend class SarRecognizer;
        
        friend class SarRectDrawer;
        friend class SarCubeDrawer;
        
        friend class SarImageCreator;
    };

    class CLASS_DECLSPEC SarMemoryAllocator
    {
    public:
        virtual WINAPI ~SarMemoryAllocator() {}
        virtual void* WINAPI allocate(size_t size) = 0;
        virtual void WINAPI deallocate(void* ptr) = 0;
    };
    
    class CLASS_DECLSPEC SarStreamIn : SarNonCopyable
    {
    public:
        virtual WINAPI ~SarStreamIn() {}
        virtual size_t WINAPI sarRead(void* buf, size_t size) = 0;
        
    protected:
        struct SarImpl;
        SarImpl *impl_;
        
        friend class SarLearnedImageTarget;
        friend class SarSceneMapTarget;
    };
    
    class CLASS_DECLSPEC SarStreamOut : SarNonCopyable
    {
    public:
        virtual WINAPI ~SarStreamOut() {}
        virtual size_t WINAPI sarWrite(const void* buf, size_t size) = 0;
        
    protected:
        struct SarImpl;
        SarImpl *impl_;
        
        friend class SarRecognizer;
    };
    
    class CLASS_DECLSPEC SarFileStreamIn : public SarStreamIn
    {
    public:
        WINAPI SarFileStreamIn(SarSmart* smart, const char* filePath);
        virtual WINAPI ~SarFileStreamIn();
        
        bool WINAPI sarIsConstructorFailed() const;
        virtual size_t WINAPI sarRead(void* buf, size_t size);
        
    private:
        struct SarImpl;
    };
    
    class CLASS_DECLSPEC SarAssetStreamIn : public SarStreamIn
    {
    public:
        WINAPI SarAssetStreamIn(SarSmart* smart, const char* filePath);
        virtual WINAPI ~SarAssetStreamIn();
        
        bool WINAPI sarIsConstructorFailed() const;
        virtual size_t WINAPI sarRead(void* buf, size_t size);

    private:
        struct SarImpl;
    };
    
    class CLASS_DECLSPEC SarFileStreamOut : public SarStreamOut
    {
    public:
        WINAPI SarFileStreamOut(SarSmart* smart, const char* filePath);
        virtual WINAPI ~SarFileStreamOut();
        
        bool WINAPI sarIsConstructorFailed() const;
        virtual size_t WINAPI sarWrite(const void* buf, size_t size);

    private:
        struct SarImpl;
    };
} // end of namespace smartar
