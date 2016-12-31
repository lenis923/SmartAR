#pragma once

#include "SarCommon.h"

#if defined (TARGET_SMARTAR_WINDOWS) || defined (TARGET_SMARTAR_LINUX)
#	if defined (TARGET_SMARTAR_WINDOWS)
#		ifndef GL_GLEXT_PROTOTYPES
#			define GL_GLEXT_PROTOTYPES
#		endif
#		include "windows.h"
#		include <GL/gl.h>
#		include "glext.h"
#	else
#		ifndef GL_GLEXT_PROTOTYPES
#			define GL_GLEXT_PROTOTYPES
#		endif
#		include <GL/gl.h>
#		include <GL/glext.h>
#	endif
#else
#	ifdef _IOS_SDK_
#		include <OpenGLES/ES1/gl.h>
#		include <OpenGLES/ES1/glext.h>
#		include <OpenGLES/ES2/gl.h>
#		include <OpenGLES/ES2/glext.h>
#	else
#		include <GLES/gl.h>
#		include <GLES/glext.h>
#		include <GLES2/gl2.h>
#		include <GLES2/gl2ext.h>
#	endif
#endif


#if defined (NDEBUG)
#	define SAR_SMARTAR_LOGW(tag, ...)
#	define SAR_SMARTAR_LOGI(tag, ...)
#	define SAR_SMARTAR_LOGD(tag, ...)
#	define SAR_SMARTAR_LOGV(tag, ...)
#else
#	define SAR_SMARTAR_LOGW(tag, ...) SAR_SMARTAR_LOG_IMPL(SAR_SMARTAR_LOG_WARN, tag, __VA_ARGS__)
#	define SAR_SMARTAR_LOGI(tag, ...) SAR_SMARTAR_LOG_IMPL(SAR_SMARTAR_LOG_INFO, tag, __VA_ARGS__)
#	define SAR_SMARTAR_LOGD(tag, ...) SAR_SMARTAR_LOG_IMPL(SAR_SMARTAR_LOG_DEBUG, tag, __VA_ARGS__)
#	define SAR_SMARTAR_LOGV(tag, ...) SAR_SMARTAR_LOG_IMPL(SAR_SMARTAR_LOG_VERBOSE, tag, __VA_ARGS__)
#endif

#define SAR_SMARTAR_LOGF(tag, ...) SAR_SMARTAR_LOG_IMPL(SAR_SMARTAR_LOG_FATAL, tag, __VA_ARGS__)
#define SAR_SMARTAR_LOGE(tag, ...) SAR_SMARTAR_LOG_IMPL(SAR_SMARTAR_LOG_ERROR, tag, __VA_ARGS__)



namespace sarSmartar {
    struct SarLandmark;
    struct SarNodePoint;
    struct SarInitPoint;
    class SarDrawerImpl;
    
    CLASS_DECLSPEC SarQuaternion WINAPI sarRotationToQuaternion(float angle, float x, float y, float z);
    CLASS_DECLSPEC SarQuaternion WINAPI sarEulerAngleToQuaternion(const SarVector3& src);
    CLASS_DECLSPEC SarVector3 WINAPI sarQuaternionToEulerAngle(const SarQuaternion& src);
    
    CLASS_DECLSPEC int32_t WINAPI sarConvertPose2Matrix(const SarVector3& position, const SarQuaternion& rotation, SarMatrix44* matrix);
    CLASS_DECLSPEC int32_t WINAPI sarConvertImageFormat(const SarImage& fromImage, const SarImage* toImage);
    
    CLASS_DECLSPEC SarRotation WINAPI sarGetDifferenceOfRotation(SarRotation lhs, SarRotation rhs);
    
    CLASS_DECLSPEC SarMatrix44 WINAPI sarTranslateM(const SarMatrix44& matrix, float x, float y, float z);
    CLASS_DECLSPEC SarMatrix44 WINAPI sarScaleM(const SarMatrix44& matrix, float x, float y, float z);
    CLASS_DECLSPEC SarMatrix44 WINAPI sarRotateM(const SarMatrix44& matrix, const SarQuaternion& quat);
    CLASS_DECLSPEC SarMatrix44 WINAPI sarSetPerspectiveM(float fovy, float aspect, float near_, float far_);

    CLASS_DECLSPEC GLuint WINAPI sarCreateShader(GLenum shaderType, const GLchar* src);
    CLASS_DECLSPEC GLuint WINAPI sarCreateProgram(const GLchar* vertexShaderSrc, const GLchar* fragmentShaderSrc);

    CLASS_DECLSPEC bool WINAPI sarIsMultiCore();
    
    // Non blocking queue for video images.
    class CLASS_DECLSPEC SarImageQueue : SarNonCopyable
    {
    public:
        WINAPI SarImageQueue(SarSmart* smart);
        
        WINAPI ~SarImageQueue();
        
        void WINAPI sarClear();
        
        // * This method must be called only on a provider thread.
        // * It is ensured that the returned image will be available until the next invocation of this method or clear().
        SarImage* WINAPI sarNewImage(int32_t sizeInBytes, uint64_t timestamp);
        
        // * This method must be called only on a provider thread.
        void WINAPI sarEnqueue();
        
        // * This method must be called only on a consumer thread.
        // * It is ensured that the returned image will be available until the next invocation of this method or clear().
        SarImage* WINAPI sarRetrieve(uint64_t timestamp = 0);
        
    private:
        struct SarImpl;
        SarImpl *impl_;
    };
    
    class CLASS_DECLSPEC SarCameraImageTransform : SarNonCopyable
    {
    public:
        WINAPI SarCameraImageTransform();
        WINAPI ~SarCameraImageTransform();
        
        void WINAPI sarSetCameraRotation(SarRotation cameraRotation);
        void WINAPI sarSetScreenRotation(SarRotation screenRotation);
        void WINAPI sarSetCameraFacing(SarFacing facing);
        
        SarRotation WINAPI sarGetCameraImageRotation() const;
        bool WINAPI sarGetCameraImageFlipX() const;
        bool WINAPI sarGetCameraImageFlipY() const;
        void WINAPI sarAdjustPose(const SarVector3& fromPosition, const SarQuaternion& fromRotation, SarVector3* toPosition, SarQuaternion* toRotation) const;
        SarMatrix44 WINAPI sarGetAdjustmentMatrix() const;
        
    private:
        struct SarImpl;
        SarImpl *impl_;
    };
    
    class CLASS_DECLSPEC SarRectDrawer : SarNonCopyable
    {
    public:
        enum {
            OPTION_WIRE_FRAME = true,
            OPTION_ADD,
        };

        WINAPI SarRectDrawer();
        WINAPI ~SarRectDrawer();
        
        int32_t WINAPI sarStart();
        int32_t WINAPI sarDraw(const SarMatrix44& pmvMatrix, float width, float height,
            float r, float g, float b, float a, uint32_t option = 0);
        int32_t WINAPI sarStop();

    private:
        struct SarImpl;
        SarImpl *impl_;
    };
    
    class CLASS_DECLSPEC SarCubeDrawer : SarNonCopyable
    {
    public:
        enum {
            OPTION_WIRE_FRAME = true,
            OPTION_ADD,
        };

        WINAPI SarCubeDrawer();
        WINAPI ~SarCubeDrawer();

        int32_t WINAPI sarStart();
        int32_t WINAPI sarDraw(const SarMatrix44& pmvMatrix, float sx, float sy, float sz,
            float r, float g, float b, float a, uint32_t option = 0);
        int32_t WINAPI sarStop();
        
    private:
        struct SarImpl;
        SarImpl *impl_;
    };

    class CLASS_DECLSPEC SarLandmarkDrawer : SarNonCopyable
    {
    public:
        WINAPI SarLandmarkDrawer();
        WINAPI ~SarLandmarkDrawer();
        
        int32_t WINAPI sarStart();
        int32_t WINAPI sarDraw(const SarMatrix44& pmvMatrix, const SarLandmark* landmarks, int32_t numLandmarks);
        int32_t WINAPI sarDraw(const SarMatrix44& pmvMatrix, const SarNodePoint* nodePoints, int32_t numNodePoints);
        int32_t WINAPI sarDraw(const SarMatrix44& imageMatrix, const SarInitPoint* initPoints, int32_t numInitPoints);
        int32_t WINAPI sarStop();

    private:
        struct SarImpl;
        SarImpl *impl_;
    };
} // end of namespace smartar
