package com.sony.smartar.sampleapi.full;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Field;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Locale;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL10;

import com.sony.smartar.sampleapi.full.R;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.SystemClock;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.Toast;

public final class MainActivity extends Activity {

    private static final int SMART_INIT_ERROR_DIALOG = 0x0001;
    private static final int SMART_EXPIRED_LICENSE_DIALOG = 0x0002;

    private GLSurfaceView mGLView;
    private SurfaceView mDummyView;
    private int mSampleCore;
    private StatusViewer mStatusViewer;
    private MenuMode mMenuMode;
    private int mSelectedMode;
    private boolean mSampleCoreCreateFailed;
    
    private String[] mStringArrayForAlertDialog;
    
    private final int defaultRecogMode
    	= RecognitionMode.RECOGNITION_MODE_SCENE_MAPPING.ordinal();
    private final int defaultInitMode 
    	= SceneMappingInitMode.SCENE_MAPPING_INIT_MODE_TARGET.ordinal();
    
    private static final String SMARTAR_DIRECTORY = "smartar";
    private static final String CAPTURE_DIRECTORY = "capture";
    private static final String CAPTURE_FILE_NAME_PREFIX = "capture";
    private static final String CAPTURE_FILE_EXTENSION = "jpg";
    private static final String DATA_DIRECTORY = "data";
    private static final String SCENE_MAP_FILE_NAME = "scenemap.dat";
    private static final String TARGET_DIRECTORY = "dict";
    
    public static enum RecognitionMode {
    	RECOGNITION_MODE_TARGET_TRACKING,
        RECOGNITION_MODE_SCENE_MAPPING,
    };
    public static enum SceneMappingInitMode {
        SCENE_MAPPING_INIT_MODE_TARGET,
        SCENE_MAPPING_INIT_MODE_HFG,
        SCENE_MAPPING_INIT_MODE_VFG,
        SCENE_MAPPING_INIT_MODE_SFM,
        SCENE_MAPPING_INIT_MODE_DRY_RUN,
    };
    public static enum SearchPolicy {
        SEARCH_POLICY_FAST,
        SEARCH_POLICY_PRECISIVE,
    };
    public static enum DenseMapMode {
        DENSE_MAP_DISABLE,
        DENSE_MAP_SEMI_DENSE,
    };
    
    public static enum MenuMode{
        Root,
        Recognizer_modes,
        Scene_mappings,
        Target_trackings,
        Camera_settings,
        Miscs,
        Inner_Target_Settings,
        VideoImageSize,
        StillImageSize,
        FpsRange,
        FocusMode,
        FlashMode,
        ExposureMode,
        WhiteBalanceMode,
        SceneMode
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

	SharedPreferences mSharedPreference;
    private static final String PREFERENCE_KEY = "smartar_full";
    private static final String PREFERENCE_KEY_RECOGMODE = "RecogMode";
    private static final String PREFERENCE_KEY_INITMODE = "InitMode";
    private static final String PREFERENCE_KEY_SEARCH_POLICY = "SearchPolicy";
    private static final String PREFERENCE_KEY_DENSE_MAP_MODE = "DenseMapMode";
    private static final String PREFERENCE_KEY_WORKER_THREAD = "WorkerThread";
    private static final String PREFERENCE_KEY_VIDEO_IMAGE_SIZE = "VideoImageSize";
    private static final String PREFERENCE_KEY_STILL_IMAGE_SIZE = "StillImageSize";
    private static final String PREFERENCE_KEY_FPS_RANGE = "FpsRange";
    private static final String PREFERENCE_KEY_FOCUS_MODE = "FocusMode";
    private static final String PREFERENCE_KEY_FLASH_MODE = "FlashMode";
    private static final String PREFERENCE_KEY_EXPOSURE_MODE = "ExposureMode";
    private static final String PREFERENCE_KEY_WHITE_BALANCE_MODE = "BalanceMode";
    private static final String PREFERENCE_KEY_SCENE_MODE = "SceneMode";
    private static final String PREFERENCE_KEY_USE_FRONT_CAMERA = "UseFrontCamera";
    private static final String PREFERENCE_KEY_SENSOR_DEVICE = "SensorDevice";
    private static final String PREFERENCE_KEY_REMOVE_LOST_LANDMARKS = "RemoveLostLandMarks";
    private static final String PREFERENCE_KEY_TRIANGULATE_MASK = "TriangulateMask";
    private static final String PREFERENCE_KEY_SHOW_STATUS = "ShowStatus";
    
    static {
   		System.loadLibrary("smartar");
        System.loadLibrary("sample_api");
    }

    @SuppressWarnings("deprecation")
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        mStatusViewer = new StatusViewer(this);
        
        // Setup GL view
        mGLView = (GLSurfaceView)findViewById(R.id.glview);
        mGLView.setEGLContextClientVersion(2);
        mGLView.setEGLConfigChooser(5, 6, 5, 0, 0, 0);
        mGLView.setEGLConfigChooser(true);//For depth buffer.
        mGLView.setZOrderMediaOverlay(true);
        mGLView.setEGLWindowSurfaceFactory(new GLSurfaceView.EGLWindowSurfaceFactory() {
            @Override
            public EGLSurface createWindowSurface(EGL10 egl, EGLDisplay display
            		, EGLConfig config, Object nativeWindow) {
                return egl.eglCreateWindowSurface(display
                		, config, nativeWindow, null);
            }

            @Override
            public void destroySurface(EGL10 egl, EGLDisplay display
            		, EGLSurface surface) {
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

                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
                	// add navigation bar width
                    DisplayMetrics metrics = new DisplayMetrics();
                    getWindowManager().getDefaultDisplay().getMetrics(metrics);
                    width += (int)(42.0f * metrics.density);
                }
                nativeSurfaceChanged(mSampleCore, width, height);
            }
            
            @Override
            public void onDrawFrame(GL10 gl) {
                if (mSampleCoreCreateFailed) { return; }

                long startTime = SystemClock.uptimeMillis();
                nativeDrawFrame(mSampleCore);
                long endTime = SystemClock.uptimeMillis();
                mStatusViewer.update(endTime - startTime);
            }
        });

        // Setup dummy view.
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.HONEYCOMB) {
            mDummyView = (SurfaceView)findViewById(R.id.dummyview);
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
        
        mSharedPreference = getSharedPreferences(PREFERENCE_KEY, Activity.MODE_PRIVATE);
        
        //Set default recognizer settings.
        int preferenceRecogMode = mSharedPreference.getInt(PREFERENCE_KEY_RECOGMODE, defaultRecogMode);
        int preferenceInitMode = mSharedPreference.getInt(PREFERENCE_KEY_INITMODE, defaultInitMode);
		if(preferenceRecogMode == 0){
			mSelectedMode = 0;
		}else{
			mSelectedMode = preferenceInitMode + 1;
		}
		
        //Create native module.
        mSampleCore = nativeCreate(getApplicationContext(), preferenceRecogMode, preferenceInitMode);
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

        //Set default camera settings.
        nativeInitializeCameraSettings(mSampleCore
        		,mSharedPreference.getInt(PREFERENCE_KEY_VIDEO_IMAGE_SIZE, -1)
        		,mSharedPreference.getInt(PREFERENCE_KEY_STILL_IMAGE_SIZE, -1)
        		,mSharedPreference.getInt(PREFERENCE_KEY_FPS_RANGE, -1)
        		,mSharedPreference.getInt(PREFERENCE_KEY_FOCUS_MODE, -1)
        		,mSharedPreference.getInt(PREFERENCE_KEY_FLASH_MODE, -1)
        		,mSharedPreference.getInt(PREFERENCE_KEY_EXPOSURE_MODE, -1)
        		,mSharedPreference.getInt(PREFERENCE_KEY_WHITE_BALANCE_MODE, -1)
        		,mSharedPreference.getInt(PREFERENCE_KEY_SCENE_MODE, -1)
        		,mSharedPreference.getInt(PREFERENCE_KEY_USE_FRONT_CAMERA, -1)
        		,mSharedPreference.getInt(PREFERENCE_KEY_SENSOR_DEVICE, -1));
        nativeSetSearchPolicy(mSampleCore, mSharedPreference.getInt(PREFERENCE_KEY_SEARCH_POLICY, SearchPolicy.SEARCH_POLICY_PRECISIVE.ordinal()));
        nativeSetDenseMapMode(mSampleCore, mSharedPreference.getInt(PREFERENCE_KEY_DENSE_MAP_MODE, 0));
        boolean putUseWorkerThreadFlag = mSharedPreference.getInt(PREFERENCE_KEY_WORKER_THREAD, 1) == 1 ? true : false;
        nativeSetUseWorkerThreadFlag(mSampleCore, putUseWorkerThreadFlag);
        boolean putRemoveLostLandmarksFlag = mSharedPreference.getInt(PREFERENCE_KEY_REMOVE_LOST_LANDMARKS, 0) == 1 ? true : false;
        nativeSetRemoveLostLandmarksFlag(mSampleCore, putRemoveLostLandmarksFlag);
        boolean putUseTriangulateMasksFlag = mSharedPreference.getInt(PREFERENCE_KEY_TRIANGULATE_MASK, 0) == 1 ? true : false;
        nativeSetUseTriangulateMasksFlag(mSampleCore, putUseTriangulateMasksFlag);
   		int visiblity = mSharedPreference.getBoolean(PREFERENCE_KEY_SHOW_STATUS, true) ? View.VISIBLE: View.INVISIBLE;
   		mStatusViewer.setStatusViewVisiblity(visiblity);
		nativeSetShowLandMarkBarFlag(mSampleCore, mSharedPreference.getBoolean(PREFERENCE_KEY_SHOW_STATUS, true));
        
        Button resetButton = (Button)findViewById(R.id.button_reset);
        resetButton.setOnClickListener(buttonClicked);
        
        Button menuButton = (Button)findViewById(R.id.button_menu);
        menuButton.setOnClickListener(buttonClicked);
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
    
    /*
     * Button touch events.
     */
    private View.OnClickListener buttonClicked = new View.OnClickListener() {
    	public void onClick(View v) {
    		switch(v.getId()){
    			case R.id.button_reset:
    				nativeResetRecognizer(mSampleCore);
    	       		showShortToast("Reset recognizer");
    	       		break;
    			case R.id.button_menu:
    				mMenuMode = MenuMode.Root;
    	        	new AlertDialog.Builder(MainActivity.this) 
    	        	.setTitle("Menu") 
    	        	.setItems(R.array.root_menu, new DialogInterface.OnClickListener() { 
    	        		public void onClick(DialogInterface dialog, int item) {
    	        			handleMenu(item); 
    	        		} 
    	        	}).show(); 
    	       		break;
    	       	default:
    	       		break;
    		}
    	}
    };
	
    /*
     * Show and handle menu.
     */
    private void handleMenu(int item) {
		if(mSampleCore == 0) return;
		switch(mMenuMode){
			case Root:
				handleRootMenu(item);
				break;
			case Recognizer_modes:
				handleSelectModeMenu(item);
				break;
			case Scene_mappings:
				handleSceneMappingMenu(item);
				break;
			case Target_trackings:
				handleTargetTrackingMenu(item);
				break;
			case Camera_settings:
				handleCameraSettingMenu(item);
				break;
			case Miscs:
				handleMiscs(item);
				break;
			case VideoImageSize:
				showShortToast("Cannot change default size.");
				break;
			case StillImageSize:
				nativeSetCameraStillImageSize(mSampleCore, item);
				showShortToast((String) mStringArrayForAlertDialog[item]);
				break;
			case FpsRange:
				nativeSetVideoImageFpsRange(mSampleCore, item);
				showShortToast((String) mStringArrayForAlertDialog[item]);
				break;
			case FlashMode:
				nativeSetFlashMode(mSampleCore, item);
				showShortToast((String) mStringArrayForAlertDialog[item]);
				break;
			case FocusMode:
				nativeSetFocusMode(mSampleCore, item);
				showShortToast((String) mStringArrayForAlertDialog[item]);
				break;
			case ExposureMode:
				nativeSetExposureMode(mSampleCore, item);
				showShortToast((String) mStringArrayForAlertDialog[item]);
				break;
			case WhiteBalanceMode:
				nativeSetWhiteBalanceMode(mSampleCore, item);
				showShortToast((String) mStringArrayForAlertDialog[item]);
				break;
			case SceneMode:
				nativeSetSceneMode(mSampleCore, item);
				showShortToast((String) mStringArrayForAlertDialog[item]);
				break;
			default:
				break;
		}
		return;
	}
    
    private void handleRootMenu(int item) {
    	int itemsId = 0;
    	
		switch(item){
			case 0:
				mMenuMode = MenuMode.Recognizer_modes;
		        new AlertDialog.Builder(this)
	        	.setTitle("Select mode")
	        	.setSingleChoiceItems(R.array.recognizer_mode_menu
	        			, mSelectedMode, new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog, int which) {
						handleMenu(which);
					}
	        	})
	        	.setPositiveButton("Close", null)
	        	.show();
		        return;
			case 1:
				if(nativeIsRecogModeSceneMapping(mSampleCore)){
					mMenuMode = MenuMode.Scene_mappings;
					itemsId = R.array.scene_mapping_menu;
				}else{
					showShortToast("Not scene mapping mode.");
					return;
				}
				break;
			case 2:
				if(!nativeIsRecogModeSceneMapping(mSampleCore)){
					mMenuMode = MenuMode.Target_trackings;
					itemsId = R.array.target_tracking_menu;
				}else{
					showShortToast("Not target tracking mode.");
					return;
				}
		        break;
			case 3:
				mMenuMode = MenuMode.Camera_settings;
				itemsId = R.array.camera_settings_menu;
		        break;
			case 4:
				mMenuMode = MenuMode.Miscs;
				itemsId = R.array.misc_menu;
		        break;
			default:
				break;
		}
		
        new AlertDialog.Builder(this)
    	.setTitle("Select setting")
    	.setItems(itemsId, new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				handleMenu(which);
			}
    	})
    	.setNegativeButton("Cancel", null)
    	.show();
    }
    
    private void handleSceneMappingMenu(int item) {
    	switch(item){
			case 0:
				saveSceneMapToSD();
				break;
			case 1:
				loadSceneMapFromSD();
				break;
			case 2:
				nativeSetFixSceneMapFlag(mSampleCore, true);
				showShortToast("FixSceneMap ON");
				break;
			case 3:
				nativeSetFixSceneMapFlag(mSampleCore, false);
				showShortToast("FixSceneMap OFF");
				break;
			case 4:
				//Change DENSE_MAP_DISABLE to DENSE_MAP_SEMI_DENSE
				//or the other way around.
		        if(nativeIsDenseMapModeSemiDense(mSampleCore)){
					nativeSetDenseMapMode(mSampleCore, DenseMapMode
							.DENSE_MAP_DISABLE.ordinal());
					showShortToast("Set DENSE_MAP_DISABLE");
		        }else{
		    		nativeSetDenseMapMode(mSampleCore, DenseMapMode
		    				.DENSE_MAP_SEMI_DENSE.ordinal());
		    		showShortToast("Set DENSE_MAP_SEMI_DENSE");
		        }
		        break;
			case 5:
		        nativeSetRemoveLostLandmarksFlag(mSampleCore, true);
				showShortToast("Remove Lost Landmarks ON");
				break;
			case 6:
		        nativeSetRemoveLostLandmarksFlag(mSampleCore, false);
				showShortToast("Remove Lost Landmarks OFF");
				break;
			case 7:
				nativeSetUseTriangulateMasksFlag(mSampleCore, true);
				showShortToast("TriangulateMasks ON");
				break;
			case 8:
				nativeSetUseTriangulateMasksFlag(mSampleCore, false);
				showShortToast("TriangulateMasks OFF");
				break;
	        default:
	        	break;
    	}
    }
    
    private void handleTargetTrackingMenu(int item) {
    	switch(item){
			case 0:
				//Change SEARCH_POLICY_PRECISIVE to SEARCH_POLICY_FAST
				//or the other way around.
		        if(nativeIsSearchPolicyPresicive(mSampleCore)){
					nativeSetSearchPolicy(mSampleCore, SearchPolicy
							.SEARCH_POLICY_FAST.ordinal());
					showShortToast("Set SEARCH_POLICY_FAST");
		        }else{
		    		nativeSetSearchPolicy(mSampleCore, SearchPolicy
		    				.SEARCH_POLICY_PRECISIVE.ordinal());
		    		showShortToast("Set SEARCH_POLICY_PRECISIVE");
		        }
		        break;
			case 1:
				nativeSetUseWorkerThreadFlag(mSampleCore, true);
				showShortToast("WorkerThread ON");
				break;
			case 2:
				nativeSetUseWorkerThreadFlag(mSampleCore, false);
				showShortToast("WorkerThread OFF");
				break;
	        default:
	        	break;
    	}
    }
    
    private void handleMiscs(int item) {
    	switch(item){
			case 0:
				showCameraSettingMenu(MenuMode.StillImageSize);				
				break;
			case 1:				
				if(!nativeIsRecogModeSceneMapping(mSampleCore)
					|| nativeGetInitMode(mSampleCore).equals("TARGET")){
					showLoadTargetFromSDMenu();
				}else{
					showShortToast("Not target using mode.");
				}
				break;
			case 2:
				if(!nativeIsRecogModeSceneMapping(mSampleCore)
						|| nativeGetInitMode(mSampleCore).equals("TARGET")){
					showLoadedTargetsMenu();
				}else{
					showShortToast("Not target using mode.");
				}
				break;
			case 3:
				nativeStartSensorDevice(mSampleCore);
				showShortToast("Sensor device ON");
				break;
			case 4:
				nativeStopSensorDevice(mSampleCore);
				showShortToast("Sensor device OFF");
				break;
			case 5:
				nativeForceLocalize(mSampleCore);
				showShortToast("Force localize");
				break;
			case 6:
				boolean isShown = !mStatusViewer.isStatusViewShown();
	       		int visiblity = isShown ? View.VISIBLE : View.INVISIBLE;
	       		mStatusViewer.setStatusViewVisiblity(visiblity);
	       		nativeSetShowLandMarkBarFlag(mSampleCore, isShown);
				break;
			case 7:
				captureImage();
				break;
			case 8:
				savePreferences();
				showShortToast("Save preference");
				break;
			case 9:
				resetAllSettings();
				showShortToast("Reset all settings");
				break;
	        default:
	        	break;
    	}
    }
    
    private void handleCameraSettingMenu(int item) {
    	switch(item){
			case 0:
				showCameraSettingMenu(MenuMode.VideoImageSize);
				break;
			case 1:
				showCameraSettingMenu(MenuMode.FocusMode);
				break;
			case 2:
				showCameraSettingMenu(MenuMode.FlashMode);
				break;
			case 3:
				showCameraSettingMenu(MenuMode.FpsRange);
				break;
			case 4:
				showCameraSettingMenu(MenuMode.SceneMode);
				break;
			case 5:
				showCameraSettingMenu(MenuMode.WhiteBalanceMode);
				break;
			case 6:
				showCameraSettingMenu(MenuMode.ExposureMode);
				break;
			case 7:
		        nativeChangeCamera(mSampleCore, mDummyView);
		        break;
			default:
				break;
    	}
    }
    
    private void handleSelectModeMenu(int item) {
    	mSelectedMode = item;
    	int recogMode = 0;
    	int initMode = 0;
    	String showText = "";
		switch(item){
			case 0:
				showText = "RECOGNITION_MODE_TARGET_TRACKING";
				recogMode = RecognitionMode
						.RECOGNITION_MODE_TARGET_TRACKING.ordinal();
				initMode = SceneMappingInitMode
						.SCENE_MAPPING_INIT_MODE_TARGET.ordinal();
				break;
			case 1:
				showText = "SCENE_MAPPING_INIT_MODE_TARGET";
				recogMode = RecognitionMode
						.RECOGNITION_MODE_SCENE_MAPPING.ordinal();
				initMode = SceneMappingInitMode
						.SCENE_MAPPING_INIT_MODE_TARGET.ordinal();
				break;
			case 2:
				showText = "SCENE_MAPPING_INIT_MODE_HFG";
				recogMode = RecognitionMode
						.RECOGNITION_MODE_SCENE_MAPPING.ordinal();
				initMode = SceneMappingInitMode
						.SCENE_MAPPING_INIT_MODE_HFG.ordinal();
				break;
			case 3:
				showText = "SCENE_MAPPING_INIT_MODE_VFG";
				recogMode = RecognitionMode
						.RECOGNITION_MODE_SCENE_MAPPING.ordinal();
				initMode = SceneMappingInitMode
						.SCENE_MAPPING_INIT_MODE_VFG.ordinal();
				break;
			case 4:
				showText = "SCENE_MAPPING_INIT_MODE_SFM";
				recogMode = RecognitionMode
						.RECOGNITION_MODE_SCENE_MAPPING.ordinal();
				initMode = SceneMappingInitMode
						.SCENE_MAPPING_INIT_MODE_SFM.ordinal();
				break;
			case 5:
				showText = "SCENE_MAPPING_INIT_MODE_DRY_RUN";
				recogMode = RecognitionMode
						.RECOGNITION_MODE_SCENE_MAPPING.ordinal();
				initMode = SceneMappingInitMode
						.SCENE_MAPPING_INIT_MODE_DRY_RUN.ordinal();
				break;
			default:
				break;
		}
		reCreateSampleCore(recogMode, initMode);
		showShortToast(showText);
    }
	
	public void showCameraSettingMenu(MenuMode menuMode){
		CharSequence title = "";
		int checkedItem = 0;
		mMenuMode = menuMode;

		switch(mMenuMode){
			case VideoImageSize:
				mStringArrayForAlertDialog = new String[nativeGetNumOfSupportedVideoImageSize(mSampleCore)];
				nativeGetSupportedVideoImageSize(mSampleCore, mStringArrayForAlertDialog);
				title = "Supported VideoImageSizes";
				checkedItem = nativeGetVideoImageSizeSelected(mSampleCore);
				mStringArrayForAlertDialog[checkedItem] = mStringArrayForAlertDialog[checkedItem] + " : selected";
		        new AlertDialog.Builder(this)
		        	.setTitle(title)
		        	.setItems(mStringArrayForAlertDialog,
		        		new DialogInterface.OnClickListener() {
		        			@Override
		        			public void onClick(DialogInterface dialog, int i) {
		        				handleMenu(i);
		        			}
		        		})
		        	.setPositiveButton("Close", null)
		        	.show();
				return;
			case StillImageSize:
				mStringArrayForAlertDialog = new String[nativeGetNumOfSupportedStillImageSize(mSampleCore)];
				nativeGetSupportedStillImageSize(mSampleCore, mStringArrayForAlertDialog);
				title = "Set StillImageSize";
				checkedItem = nativeGetStillImageSizeSelected(mSampleCore);
				break;
			case FocusMode:
				mStringArrayForAlertDialog = new String[nativeGetNumOfSupportedFocusMode(mSampleCore)];
				nativeGetSupportedFocusMode(mSampleCore, mStringArrayForAlertDialog);
				title = "Set FocusMode";
				checkedItem = nativeGetFocusModeSelected(mSampleCore);
				break;
			case FlashMode:
				mStringArrayForAlertDialog = new String[nativeGetNumOfSupportedFlashMode(mSampleCore)];
				nativeGetSupportedFlashMode(mSampleCore, mStringArrayForAlertDialog);
				title = "Set FlashMode";
				checkedItem = nativeGetFlashModeSelected(mSampleCore);
				break;
			case ExposureMode:
				mStringArrayForAlertDialog = new String[nativeGetNumOfSupportedExposureMode(mSampleCore)];
				nativeGetSupportedExposureMode(mSampleCore, mStringArrayForAlertDialog);
				title = "Set ExposureMode";
				checkedItem = nativeGetExposureModeSelected(mSampleCore);
				break;
			case WhiteBalanceMode:
				mStringArrayForAlertDialog = new String[nativeGetNumOfSupportedWhiteBalanceMode(mSampleCore)];
				nativeGetSupportedWhiteBalanceMode(mSampleCore, mStringArrayForAlertDialog);
				title = "Set WhiteBalanceMode";
				checkedItem = nativeGetWhiteBalanceModeSelected(mSampleCore);
				break;
			case SceneMode:
				mStringArrayForAlertDialog = new String[nativeGetNumOfSupportedSceneMode(mSampleCore)];
				nativeGetSupportedSceneMode(mSampleCore, mStringArrayForAlertDialog);
				title = "Set SceneMode";
				checkedItem = nativeGetSceneModeSelected(mSampleCore);
				break;
			case FpsRange:
				mStringArrayForAlertDialog = new String[nativeGetNumOfSupportedVideoImageFpsRange(mSampleCore)];
				nativeGetSupportedVideoImageFpsRange(mSampleCore, mStringArrayForAlertDialog);
				title = "Set FpsRange";
				checkedItem = nativeGetVideoImageFpsRangeSelected(mSampleCore);
				break;
			default:
				break;
		}
		
        new AlertDialog.Builder(this)
        	.setTitle(title)
        	.setSingleChoiceItems(mStringArrayForAlertDialog, checkedItem,
        		new DialogInterface.OnClickListener() {
        			@Override
        			public void onClick(DialogInterface dialog, int i) {
        				handleMenu(i);
        			}
        		})
        	.setPositiveButton("Close", null)
        	.show();
	}

	/*
	 * File methods.
	 */
	private boolean isExternalStorageMounted(){
		String status = Environment.getExternalStorageState(); 
		
		if (status.equals(Environment.MEDIA_MOUNTED)) {
			return true;
		}else{
			showShortToast("SDcard not mounted.");
			return false;
		}
	}
	
	private boolean createDirectory(File file){
		return file.mkdirs();
	}
	
	//Save scene map to SD card.
	private File mSceneMapPath;
	private void saveSceneMapToSD(){
		if (!isExternalStorageMounted()) return;
		
		File externalStoragePath = Environment.getExternalStorageDirectory();
		File smartarDirectory = new File(externalStoragePath.getAbsolutePath(), SMARTAR_DIRECTORY);
		File scenemapDirectory = new File(smartarDirectory.getAbsolutePath(), DATA_DIRECTORY);
		if(!scenemapDirectory.exists()){
			createDirectory(scenemapDirectory);
		}
		mSceneMapPath = new File(scenemapDirectory.getAbsolutePath(), SCENE_MAP_FILE_NAME);
		nativeSaveSceneMap(mSampleCore, mSceneMapPath.toString());
		
		mediaScan();
		
		showShortToast("Scene map saved.");
	}
	
	//Load scene map from SD card.
	private void loadSceneMapFromSD(){
		if(!checkSceneMapFileExist()) return;
		
		File externalStoragePath = Environment.getExternalStorageDirectory();
		File smartarDirectory = new File(externalStoragePath.getAbsolutePath(), SMARTAR_DIRECTORY);
		File scenemapDirectory = new File(smartarDirectory.getAbsolutePath(), DATA_DIRECTORY);
		
		reCreateSampleCore(RecognitionMode.RECOGNITION_MODE_SCENE_MAPPING.ordinal()
				, SceneMappingInitMode.SCENE_MAPPING_INIT_MODE_TARGET.ordinal());
		
		nativeLoadSceneMap(mSampleCore, new File(scenemapDirectory.getAbsolutePath(), SCENE_MAP_FILE_NAME).toString());
		showShortToast("Scene map loaded.");
	}
	
	private boolean checkSceneMapFileExist(){
		File externalStoragePath = Environment.getExternalStorageDirectory();
		File smartarDirectory = new File(externalStoragePath.getAbsolutePath(), SMARTAR_DIRECTORY);
		File scenemapDirectory = new File(smartarDirectory.getAbsolutePath(), DATA_DIRECTORY);
		
		if(!isExternalStorageMounted()) return false;
		
		if(!new File(scenemapDirectory.getAbsolutePath(), SCENE_MAP_FILE_NAME).exists()){
			showShortToast("No scene map.");
			return false;
		}
		
		return true;
	}
	
	//Load target from SD card.
	private void showLoadTargetFromSDMenu(){
		if (!isExternalStorageMounted()) return;
		
		CharSequence title = "Add files.";
		final ArrayList<CharSequence> targetFiles = new ArrayList<CharSequence>();
		File externalStoragePath = Environment.getExternalStorageDirectory();
		File smartarDirectory = new File(externalStoragePath.getAbsolutePath(), SMARTAR_DIRECTORY);
		final File targetDirectory = new File(smartarDirectory.getAbsolutePath(), TARGET_DIRECTORY);
		
		if(!targetDirectory.exists()){
			showShortToast("There is no \"" + TARGET_DIRECTORY + "\" directory");
			return;
		}
		
		int numOfLoadedTarget = nativeGetNumOfLoadedTarget(mSampleCore);
		mStringArrayForAlertDialog = new String[numOfLoadedTarget];

		nativeGetLoadedTarget(mSampleCore, mStringArrayForAlertDialog, null);
		
		//Make .dic file name ArrayList from SDcard root.
		//If filename already read, ignore the file.
		final File[] files = targetDirectory.listFiles();
		for (int i = 0; i < files.length; i++) {
		    if(files[i].getName().endsWith(".dic")) {
		    	boolean isAlreadyLoaded = false;
		    	for(int j = 0; j < mStringArrayForAlertDialog.length; j++){
		    		if(mStringArrayForAlertDialog[j].equals(files[i].getName())){
		    			isAlreadyLoaded = true;
		    		}
		    	}
		    	if(!isAlreadyLoaded){
		    		targetFiles.add(files[i].getName());
		    	}
		    }
		}
		
        new AlertDialog.Builder(this)
    	.setTitle(title)
    	.setItems((CharSequence[])targetFiles.toArray(new CharSequence[0])
    			, new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog, int which) {
						nativeLoadTarget(mSampleCore
								, new File(targetDirectory, targetFiles.get(which).toString()).getPath()
								, targetFiles.get(which).toString());
						showShortToast(targetFiles.get(which).toString() + " loaded.");
					}
    			})
    	.setPositiveButton("OK", null)
    	.show();
	}
	
	//Set targets to recognizer which already loaded.
	private void showLoadedTargetsMenu() {
		CharSequence title = "Select targets.";
		
		int numOfLoadedTarget = nativeGetNumOfLoadedTarget(mSampleCore);
		mStringArrayForAlertDialog = new String[numOfLoadedTarget];
		final boolean[] usedArray = new boolean[numOfLoadedTarget];
		
		nativeGetLoadedTarget(mSampleCore, mStringArrayForAlertDialog, usedArray);

        new AlertDialog.Builder(this)
        	.setTitle(title)
        	.setMultiChoiceItems(mStringArrayForAlertDialog, usedArray, new DialogInterface.OnMultiChoiceClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which,
						boolean isChecked) {
					usedArray[which] = isChecked;
				}
        	})
        	.setPositiveButton("OK", new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					nativeSetLearnedImageTarget(mSampleCore, usedArray);
				}
        	})
        	.show();
	}
	
	//Capture image to SD card.
    private File mPicturePath;
	private void captureImage(){
		if (!isExternalStorageMounted()) return;
		File externalStoragePath = Environment.getExternalStorageDirectory();
		File smartarDirectory = new File(externalStoragePath.getAbsolutePath(), SMARTAR_DIRECTORY);
		File captureDirectory = new File(smartarDirectory.getAbsolutePath(), CAPTURE_DIRECTORY);
		
		if(!captureDirectory.exists()){
			createDirectory(captureDirectory);
		}
		
		mPicturePath = getUniquePicturePath(captureDirectory, CAPTURE_FILE_NAME_PREFIX, CAPTURE_FILE_EXTENSION);
		nativeCaptureStillImage(mSampleCore, mPicturePath.toString());
	}
	
	//Create unique picture filename based on capture time.
	private File getUniquePicturePath(File captureDirectory, String fileName, String extension){
		int count = 0;
		Date date = new Date();
		SimpleDateFormat simpleDateFormat = new SimpleDateFormat("yyyyMMddHHmmss", Locale.ENGLISH);
		File uniqueFileName = new File(captureDirectory, fileName + simpleDateFormat.format(date) + "." + extension);
		if (uniqueFileName.exists()){
			while(true) {
				count++;
				String newFileName = fileName + simpleDateFormat.format(date)+ "-" + count + "." + extension;
				File newFile = new File(captureDirectory, newFileName);
				if (!newFile.exists()){
					uniqueFileName = newFile;
					break;
				}
			}
		}
        return uniqueFileName;
	}
	
	/*
	 * Misc methods
	 */
	//Save settings to SharedPreference.
    private void savePreferences(){
		SharedPreferences.Editor editor = mSharedPreference.edit();
		int putRecogMode = nativeIsRecogModeSceneMapping(mSampleCore) ? 1 : 0;
		editor.putInt(PREFERENCE_KEY_RECOGMODE, putRecogMode);
		editor.putInt(PREFERENCE_KEY_INITMODE, nativeGetInitModeNum(mSampleCore));
		int putSearchPolicy = nativeIsSearchPolicyPresicive(mSampleCore) ? 1 : 0;
		editor.putInt(PREFERENCE_KEY_SEARCH_POLICY, putSearchPolicy);
		int putDenseMapMode = nativeIsDenseMapModeSemiDense(mSampleCore) ? 1 : 0;
		editor.putInt(PREFERENCE_KEY_DENSE_MAP_MODE, putDenseMapMode);
		int putWorkerThread = nativeIsUseWorkerThread(mSampleCore) ? 1 : 0;
		editor.putInt(PREFERENCE_KEY_WORKER_THREAD, putWorkerThread);
		editor.putInt(PREFERENCE_KEY_VIDEO_IMAGE_SIZE, nativeGetVideoImageSizeSelected(mSampleCore));
		editor.putInt(PREFERENCE_KEY_STILL_IMAGE_SIZE, nativeGetStillImageSizeSelected(mSampleCore));
		editor.putInt(PREFERENCE_KEY_FPS_RANGE, nativeGetVideoImageFpsRangeSelected(mSampleCore));
		editor.putInt(PREFERENCE_KEY_FOCUS_MODE, nativeGetFocusModeSelected(mSampleCore));
		editor.putInt(PREFERENCE_KEY_FLASH_MODE, nativeGetFlashModeSelected(mSampleCore));
		editor.putInt(PREFERENCE_KEY_EXPOSURE_MODE, nativeGetExposureModeSelected(mSampleCore));
		editor.putInt(PREFERENCE_KEY_WHITE_BALANCE_MODE, nativeGetWhiteBalanceModeSelected(mSampleCore));
		editor.putInt(PREFERENCE_KEY_SCENE_MODE, nativeGetSceneModeSelected(mSampleCore));
		int putUseFrontCamera = nativeIsUseFrontCamera(mSampleCore) ? 0 : 1;
		editor.putInt(PREFERENCE_KEY_USE_FRONT_CAMERA, putUseFrontCamera);
		int putUseSensorDevice = nativeIsUseSensorDevice(mSampleCore) ? 1 : 0;
		editor.putInt(PREFERENCE_KEY_SENSOR_DEVICE, putUseSensorDevice);
		int putRemoveLostLandmarks = nativeIsRemoveLostLandmarks(mSampleCore) ? 1 : 0;
		editor.putInt(PREFERENCE_KEY_REMOVE_LOST_LANDMARKS, putRemoveLostLandmarks);
		int putUseTriangulateMasks = nativeIsUseTriangulateMasks(mSampleCore) ? 1 : 0;
		editor.putInt(PREFERENCE_KEY_TRIANGULATE_MASK, putUseTriangulateMasks);
		editor.putBoolean(PREFERENCE_KEY_SHOW_STATUS, mStatusViewer.isStatusViewShown());
		editor.commit();
    }
    
    //Reset all settings.
    private void resetAllSettings(){
		SharedPreferences.Editor editor = mSharedPreference.edit();
		editor.clear().commit();
		reCreateSampleCore(defaultRecogMode, defaultInitMode);
		
		if(defaultRecogMode == 0){
			mSelectedMode = 0;
		}else{
			mSelectedMode = defaultInitMode + 1;
		}
		
   		mStatusViewer.setStatusViewVisiblity(View.VISIBLE);
		nativeSetShowLandMarkBarFlag(mSampleCore, true);
    }
    
	//Create new SampleCore by other recogmode and initmode.
	private void reCreateSampleCore(int recogMode, int initMode){
		SearchPolicy policy = (nativeIsSearchPolicyPresicive(mSampleCore) ? SearchPolicy.SEARCH_POLICY_PRECISIVE : SearchPolicy.SEARCH_POLICY_FAST);
		mGLView.onPause();
		nativePause(mSampleCore);
    	nativeDestroy(mSampleCore);
		mSampleCore = nativeCreate(getApplicationContext(), recogMode, initMode);
		nativeSetSearchPolicy(mSampleCore, policy.ordinal());
		nativeResume(mSampleCore, mDummyView);
		mGLView.onResume();
	}
	
	//Show short toast.
	public void showShortToast(String message){
		Toast.makeText(this, message, Toast.LENGTH_SHORT).show();
	}
	
    //For StatusViewer to check is SampleCore exists.
    public int getSampleCore(){
    	return mSampleCore;
    }
    
	//Callback when capture succeed, or scene map saved, scan captured image file.
    public void mediaScan(){
        String scanFilePath = null;
        String mineType = null;
        if (mPicturePath != null) {
            scanFilePath = mPicturePath.toString();
            mineType = "image/jpeg";
        }
        if (mSceneMapPath != null) {
            scanFilePath = mSceneMapPath.toString();
            mineType = null;
        }

        MediaScannerConnection.scanFile(
                getApplicationContext(),
                new String[] { scanFilePath },
                new String[] { mineType },
                new MediaScannerConnection.OnScanCompletedListener() {
                    @Override
                    public void onScanCompleted(String path, Uri uri) {
                        Log.d("MediaScannerConnection", "Scanned " + path + ":");
                        Log.d("MediaScannerConnection", "-> uri=" + uri);
                        if (mPicturePath != null) { mPicturePath = null; }
                        if (mSceneMapPath != null) { mSceneMapPath = null; }
                    }
                });
    }

    // old API to support GingerBread
    @SuppressWarnings("deprecation")
    private static void surfaceHolderSetTypePushBuffers(SurfaceHolder surfaceHolder) {
        surfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
    }
    
    @SuppressWarnings("deprecation")
    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog dialog = null;

        switch (id) {
        case SMART_EXPIRED_LICENSE_DIALOG:
            dialog = createSmartErrorDialog("SmartAR SDK expired license", "Will close an application.");
            break;
        case SMART_INIT_ERROR_DIALOG:
            dialog = createSmartErrorDialog("Smart initialized error", "Will close an application");
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
    
    native int nativeCameraFrameCount(int sampleCore);
    native int nativeRecogCount(int sampleCore, int index);
    native long nativeRecogTime(int sampleCore, int index);

    native boolean nativeIsRecogModeSceneMapping(int sampleCore);
    native String nativeGetInitMode(int sampleCore);
    native int nativeGetInitModeNum(int sampleCore);
    native String nativeGetState(int sampleCore);
    native boolean nativeIsSearchPolicyPresicive(int sampleCore);
    native boolean nativeIsDenseMapModeSemiDense(int sampleCore);
    native int nativeGetTrackedLandmarkCount(int sampleCore);
    native int nativeGetLostLandmarkCount(int sampleCore);
    native int nativeGetSuspendedLandmarkCount(int sampleCore);
    native int nativeGetMaskedLandmarkCount(int sampleCore);
    native boolean nativeIsUseWorkerThread(int sampleCore);
    native void nativeSetLandMarkBarLocation(int sampleCore, int x, int y);
    
    private native void nativeResetRecognizer(int sampleCore);
    private native void nativeChangeTarget(int sampleCore);
    private native void nativeSetSearchPolicy(int sampleCore, int searchPolicy);
    private native void nativeSetDenseMapMode(int sampleCore, int denseMapMode);
    private native void nativeSaveSceneMap(int sampleCore, String filePath);
    private native void nativeLoadSceneMap(int sampleCore, String filePath);
    private native void nativeSetLearnedImageTarget(int sampleCore, boolean usedArray[]);
    private native void nativeSetUseWorkerThreadFlag(int sampleCore, boolean flag);
    private native void nativeSetUseTriangulateMasksFlag(int sampleCore, boolean flag);
    private native void nativeSetFixSceneMapFlag(int sampleCore, boolean flag);
    private native void nativeForceLocalize(int sampleCore);
    private native void nativeSetRemoveLostLandmarksFlag(int sampleCore, boolean flag);
    
    private native int nativeGetVideoImageSizeSelected(int sampleCore);
    private native int nativeGetStillImageSizeSelected(int sampleCore);
    private native int nativeGetVideoImageFpsRangeSelected(int sampleCore);
    private native int nativeGetFocusModeSelected(int sampleCore);
    private native int nativeGetFlashModeSelected(int sampleCore);
    private native int nativeGetExposureModeSelected(int sampleCore);
    private native int nativeGetWhiteBalanceModeSelected(int sampleCore);
    private native int nativeGetSceneModeSelected(int sampleCore);
    
    private native int nativeGetNumOfSupportedVideoImageSize(int sampleCore);
    private native int nativeGetNumOfSupportedStillImageSize(int sampleCore);
    private native int nativeGetNumOfSupportedVideoImageFpsRange(int sampleCore);
    private native int nativeGetNumOfSupportedFocusMode(int sampleCore);
    private native int nativeGetNumOfSupportedFlashMode(int sampleCore);
    private native int nativeGetNumOfSupportedExposureMode(int sampleCore);
    private native int nativeGetNumOfSupportedWhiteBalanceMode(int sampleCore);
    private native int nativeGetNumOfSupportedSceneMode(int sampleCore);
    
    private native void nativeGetSupportedVideoImageSize(int sampleCore, String sizeArray[]);
    private native void nativeGetSupportedStillImageSize(int sampleCore, String sizeArray[]);
    private native void nativeGetSupportedVideoImageFpsRange(int sampleCore, String sizeArray[]);
    private native void nativeGetSupportedFocusMode(int sampleCore, String sizeArray[]);
    private native void nativeGetSupportedFlashMode(int sampleCore, String sizeArray[]);
    private native void nativeGetSupportedExposureMode(int sampleCore, String sizeArray[]);
    private native void nativeGetSupportedWhiteBalanceMode(int sampleCore, String sizeArray[]);
    private native void nativeGetSupportedSceneMode(int sampleCore, String sizeArray[]);
    
    private native void nativeSetCameraStillImageSize(int sampleCore, int selectNum);
    private native void nativeSetVideoImageFpsRange(int sampleCore, int selectNum);
    private native void nativeSetFocusMode(int sampleCore, int selectNum);
    private native void nativeSetFlashMode(int sampleCore, int selectNum);
    private native void nativeSetExposureMode(int sampleCore, int selectNum);
    private native void nativeSetWhiteBalanceMode(int sampleCore, int selectNum);
    private native void nativeSetSceneMode(int sampleCore, int selectNum);
    
    private native void nativeCaptureStillImage(int sampleCore, String filePath);
    private native int nativeGetNumOfLoadedTarget(int sampleCore);
    private native void nativeGetLoadedTarget(int sampleCore, String stringArray[], boolean usedArray[]);
    private native void nativeLoadTarget(int sampleCore, String filePath, String label);
    private native void nativeChangeCamera(int sampleCore, SurfaceView nativeVideoOutput);
    private native void nativeSetShowLandMarkFlag(int sampleCore, boolean flag);
    private native void nativeSetShowLandMarkBarFlag(int sampleCore, boolean flag);
    private native void nativeStopSensorDevice(int sampleCore);
    private native void nativeStartSensorDevice(int sampleCore);
    private native boolean nativeIsUseFrontCamera(int sampleCore);
    private native boolean nativeIsUseSensorDevice(int sampleCore);
    private native boolean nativeIsFixSceneMap(int sampleCore);
    private native boolean nativeIsRemoveLostLandmarks(int sampleCore);
    private native boolean nativeIsUseTriangulateMasks(int sampleCore);
    
    private native void nativeInitializeCameraSettings(int sampleCore, int videoImageSizeSelected
    		, int stillImageSizeSelected
    		, int videoImageFpsRangeSelected
    		, int focusModeSelected
    		, int flashModeSelected
    		, int exposureModeSelected
    		, int whiteBalanceModeSelected
    		, int sceneModeSelected
    		, int useFrontCameraSelected
    		, int useSensorDevice);

    private native boolean nativeCreateFailed(int sampleCore);

    private native int nativeSmartInitResultCode(int sampleCore);
}
