package com.sony.smartar.sampleapi.simple;

import java.lang.reflect.Field;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;

public final class MainActivity extends Activity {

    private static final int SMART_INIT_ERROR_DIALOG = 0x0001;
    private static final int SMART_EXPIRED_LICENSE_DIALOG = 0x0002;

    private GLSurfaceView mGLView;
    private SurfaceView mDummyView;
    private int mSampleCore;
    private final int mDefaultRecogMode = RecognitionMode.SCENE_MAPPING.ordinal();
    private boolean mSampleCoreCreateFailed;

    static {
   		System.loadLibrary("smartar");
        System.loadLibrary("sample_api");
    }

    public static enum RecognitionMode {
        TARGET_TRACKING,
        SCENE_MAPPING,
    };

    /* Refer to Common.h */
    public enum SmartTypes {
        OK                                     (0),
        ERROR_EXPIRED_LICENSE        (-2138308609),
        ERROR_UNINITIALIZED          (-2138308608),
        ERROR_ALREADY_INITIALIZED    (-2138308607),
        ERROR_OUT_OF_MEMORY          (-2138308606),
        ERROR_NOT_STOPPED            (-2138308605),
        ERROR_NOT_EMPTY              (-2138308604),
        ERROR_INVALID_VALUE          (-2138308603),
        ERROR_INVALID_POINTER        (-2138308602),
        ERROR_ALREADY_REGISTERED     (-2138308601),
        ERROR_NOT_REGISTERED         (-2138308600),
        ERROR_ALREADY_STARTED        (-2138308599),
        ERROR_NOT_STARTED            (-2138308598),
        ERROR_NOT_REQUIRED           (-2138308597),
        ERROR_VERSION_MISSMATCH      (-2138308596),
        ERROR_NO_DICTIONARY          (-2138308595),
        ERROR_BUSY                   (-2138308594);

        public final int TYPE;

        private SmartTypes(final int type) {
            this.TYPE = type;
        }

        public static SmartTypes convertEnumFromInt(int num) {
            SmartTypes[] types = SmartTypes.values();
            for (SmartTypes smartType : types) {
                if (num == smartType.TYPE) { return smartType; }
            }

            return null;
        }
    }

    @SuppressWarnings("deprecation")
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Setup GL view
        mGLView = (GLSurfaceView) findViewById(R.id.glview);
        mGLView.setEGLContextClientVersion(2);
        mGLView.setEGLConfigChooser(5, 6, 5, 0, 0, 0);
        mGLView.setEGLConfigChooser(true);// For depth buffer.
        mGLView.setZOrderMediaOverlay(true);
        mGLView.setEGLWindowSurfaceFactory(new GLSurfaceView.EGLWindowSurfaceFactory() {
            @Override
            public EGLSurface createWindowSurface(EGL10 egl, EGLDisplay display, EGLConfig config, Object nativeWindow) {
                return egl.eglCreateWindowSurface(display, config, nativeWindow, null);
            }

            @Override
            public void destroySurface(EGL10 egl, EGLDisplay display, EGLSurface surface) {
                if (mSampleCoreCreateFailed) { return; }

                nativeDestroySurface(mSampleCore);
                egl.eglDestroySurface(display, surface);
            }
        });
        mGLView.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                if (mSampleCoreCreateFailed) { return; }

                if (mDummyView != null) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            mDummyView.setVisibility(View.VISIBLE);
                        }
                    });
                }

                nativeSurfaceCreated(mSampleCore);
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                if (mSampleCoreCreateFailed) { return; }
                nativeSurfaceChanged(mSampleCore, width, height);
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                if (mSampleCoreCreateFailed) { return; }
                nativeDrawFrame(mSampleCore);
            }
        });

        // Setup dummy view.
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.HONEYCOMB) {
            mDummyView = (SurfaceView) findViewById(R.id.dummyview);
            try {
                Field field = SurfaceView.class.getDeclaredField("mLayout");
                field.setAccessible(true);
                ((WindowManager.LayoutParams) field.get(mDummyView)).alpha = 0f;
            } catch (NoSuchFieldException e) {
                throw new RuntimeException(e);
            } catch (IllegalAccessException e) {
                throw new RuntimeException(e);
            }
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.HONEYCOMB) {
                surfaceHolderSetTypePushBuffers(mDummyView.getHolder());
            }
            mDummyView.getLayoutParams().width = 480;
            mDummyView.getLayoutParams().height = 320;
            mDummyView.setVisibility(View.INVISIBLE);
        } else {
            mDummyView = null;
        }

        // Create native module.
        mSampleCore = nativeCreate(getApplicationContext(), mDefaultRecogMode, 0);
        mSampleCoreCreateFailed = nativeCreateFailed(mSampleCore);

        if (mSampleCoreCreateFailed) {
            final SmartTypes resultCode = SmartTypes.convertEnumFromInt(nativeSmartInitResultCode(mSampleCore));
            switch (resultCode) {
            case ERROR_EXPIRED_LICENSE:
                showDialog(SMART_EXPIRED_LICENSE_DIALOG);
                break;
            default:
                showDialog(SMART_INIT_ERROR_DIALOG);
                break;
            }

            return;
        }

        // Set default camera settings.
        nativeInitializeCameraSettings(mSampleCore, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    }

    @Override
    protected void onDestroy() {
        // Notify event to native module.
        if (!mSampleCoreCreateFailed) {
            nativeDestroy(mSampleCore);
            mSampleCore = 0;
        }

        super.onDestroy();
    }

    @Override
    protected void onResume() {
        super.onResume();

        if (mSampleCoreCreateFailed) { return; }

        // Notify event to native module.
        nativeResume(mSampleCore, mDummyView);

        mGLView.onResume();
    }

    @Override
    protected void onPause() {
        if (!mSampleCoreCreateFailed) {
            mGLView.onPause();

            // Notify event to native module.
            nativePause(mSampleCore);

            if (mDummyView != null) {
                mDummyView.setVisibility(View.INVISIBLE);
            }
        }
        super.onPause();
    }

    @SuppressWarnings("deprecation")
    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog dialog = null;

        switch (id) {
        case SMART_EXPIRED_LICENSE_DIALOG:
            dialog = createSmartErrorDialog("SmartAR SDK expired license.", "Will close an application.");
            break;
        case SMART_INIT_ERROR_DIALOG:
            dialog = createSmartErrorDialog("Smart initialized error.", "Will close an application.");
            break;
        default:
            dialog = super.onCreateDialog(id);
            break;
        }

        if (dialog != null) {
            dialog.setCanceledOnTouchOutside(false);
        }

        return dialog;
    }

    // old API to support GingerBread
    @SuppressWarnings("deprecation")
    private static void surfaceHolderSetTypePushBuffers(SurfaceHolder surfaceHolder) {
        surfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
    }

    private Dialog createSmartErrorDialog(CharSequence title, CharSequence message) {
        return new AlertDialog.Builder(this)
                .setTitle(title)
                .setMessage(message)
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        finish();
                    }
                })
                .create();
    }

    /*
     * Native methods.
     */
    private native int nativeCreate(Context nativeContext, int recogMode, int initMode);

    private native void nativeDestroy(int sampleCore);

    private native void nativeResume(int sampleCore, SurfaceView nativeVideoOutput);

    private native void nativePause(int sampleCore);

    private native void nativeSurfaceCreated(int sampleCore);

    private native void nativeSurfaceChanged(int sampleCore, int width, int height);

    private native void nativeDrawFrame(int sampleCore);

    private native void nativeDestroySurface(int sampleCore);

    private native void nativeInitializeCameraSettings(
            int sampleCore,
            int videoImageSizeSelected,
            int videoImageFpsRangeSelected,
            int focusModeSelected,
            int flashModeSelected,
            int exposureModeSelected,
            int whiteBalanceModeSelected,
            int sceneModeSelected,
            int useFrontCameraSelected,
            int useSensorDevice);

    private native boolean nativeCreateFailed(int sampleCore);

    private native int nativeSmartInitResultCode(int sampleCore);
}
