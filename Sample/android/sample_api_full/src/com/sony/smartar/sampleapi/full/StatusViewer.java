package com.sony.smartar.sampleapi.full;

import com.sony.smartar.sampleapi.full.R;

import android.os.SystemClock;
import android.view.View;
import android.widget.TextView;

public final class StatusViewer {
    private final MainActivity mActivity;
	private final TextView mStatusView;
    
	private static final int UPDATE_INTERVAL = 1000;
	
    private final StringBuilder mStringBuilder = new StringBuilder();
    
    private int mDrawFrameCount = 0;
    private long mDrawFrameTime = 0;
    private long mLastUpdateTime = 0;
    private int mDrawFrameCountWork = 0;
    private long mDrawFrameTimeWork = 0;
    private long mTimeDiffWork = 0;
    
    public StatusViewer(MainActivity activity) {
        mActivity = activity;
		mStatusView = (TextView)mActivity
				.findViewById(R.id.statusView);
    }
    
    public void update(long drawTime) {
        ++mDrawFrameCount;
        mDrawFrameTime += drawTime;
        long newTime = SystemClock.uptimeMillis();
        long timeDiff = newTime - mLastUpdateTime;
        if (timeDiff >= UPDATE_INTERVAL) {
            mDrawFrameCountWork = mDrawFrameCount;
            mDrawFrameTimeWork = mDrawFrameTime;
            mTimeDiffWork = timeDiff;
            mActivity.runOnUiThread(mViewUpdater);
            mDrawFrameCount = 0;
            mDrawFrameTime = 0;
            mLastUpdateTime = newTime;
        }
    }
    
    private final Runnable mViewUpdater = new Runnable() {
        @Override
        public void run() {
        	int sampleCore = mActivity.getSampleCore();
        	if(sampleCore == 0) return;
        	
        	//get framecount
            int cameraFrameCount = mActivity.nativeCameraFrameCount(sampleCore);
            int recogCount0 = mActivity.nativeRecogCount(sampleCore, 0);
            int recogCount1 = mActivity.nativeRecogCount(sampleCore, 1);
            long recogTime0 = mActivity.nativeRecogTime(sampleCore, 0);
            long recogTime1 = mActivity.nativeRecogTime(sampleCore, 1);

        	//Get other settings status.
            boolean isRecogModeSceneMapping = mActivity.nativeIsRecogModeSceneMapping(sampleCore);
            String initMode = mActivity.nativeGetInitMode(sampleCore);
            String state = mActivity.nativeGetState(sampleCore);
            boolean isSearchPolicyPresicive = mActivity.nativeIsSearchPolicyPresicive(sampleCore);
            boolean isDenseMapModeSemiDense = mActivity.nativeIsDenseMapModeSemiDense(sampleCore);
            
        	mStringBuilder.setLength(0);
            mStringBuilder.append("CameraFrame fps:");
            mStringBuilder.append((int)cameraFrameCount * 1000 / mTimeDiffWork);
            mStringBuilder.append(".");
            mStringBuilder.append(getF2((double)cameraFrameCount * 1000 / mTimeDiffWork, 1));
            mStringBuilder.append(getF2((double)cameraFrameCount * 1000 / mTimeDiffWork, 2));
            
            if (recogCount0 != -1) {
                mStringBuilder.append("\n");
                mStringBuilder.append("Recognize0 fps:");
                mStringBuilder.append((int)recogCount0 * 1000 / mTimeDiffWork);
                mStringBuilder.append(".");
                mStringBuilder.append(getF2((double)recogCount0 * 1000 / mTimeDiffWork, 1));
                mStringBuilder.append(getF2((double)recogCount0 * 1000 / mTimeDiffWork, 2));
                mStringBuilder.append(" [");
                if (recogCount0 != 0) {
                    mStringBuilder.append((int)recogTime0 / recogCount0);
                    mStringBuilder.append(".");
                    mStringBuilder.append(getF2((double)recogTime0 / recogCount0, 1));
                    mStringBuilder.append(getF2((double)recogTime0 / recogCount0, 2));
                } else {
                    mStringBuilder.append("-");
                }
                mStringBuilder.append("ms]");
            }
            if (recogCount1 != -1) {
                mStringBuilder.append("\n");
            	mStringBuilder.append("Recognize1 fps:");
                mStringBuilder.append((int)recogCount1 * 1000 / mTimeDiffWork);
                mStringBuilder.append(".");
                mStringBuilder.append(getF2((double)recogCount1 * 1000 / mTimeDiffWork, 1));
                mStringBuilder.append(getF2((double)recogCount1 * 1000 / mTimeDiffWork, 2));
                mStringBuilder.append(" [");
                if (recogCount1 != 0) {
                    mStringBuilder.append((int)recogTime1 / recogCount1);
                    mStringBuilder.append(".");
                    mStringBuilder.append(getF2((double)recogTime1 / recogCount1, 1));
                    mStringBuilder.append(getF2((double)recogTime1 / recogCount1, 2));
                } else {
                    mStringBuilder.append("-");
                }
                mStringBuilder.append("ms]");
            }
            mStringBuilder.append("\n");
            mStringBuilder.append("DrawFrame fps:");
            mStringBuilder.append((int)mDrawFrameCountWork * 1000 / mTimeDiffWork);
            mStringBuilder.append(".");
            mStringBuilder.append(getF2((double)mDrawFrameCountWork * 1000 / mTimeDiffWork , 1));
            mStringBuilder.append(getF2((double)mDrawFrameCountWork * 1000 / mTimeDiffWork , 2));
            mStringBuilder.append(" [");
            if (mDrawFrameCountWork != 0) {
                mStringBuilder.append((int)mDrawFrameTimeWork / mDrawFrameCountWork);
                mStringBuilder.append(".");
                mStringBuilder.append(getF2((double)mDrawFrameTimeWork / mDrawFrameCountWork, 1));
                mStringBuilder.append(getF2((double)mDrawFrameTimeWork / mDrawFrameCountWork, 2));
            } else {
                mStringBuilder.append("-");
            }
            mStringBuilder.append("ms]");
            mStringBuilder.append("\n");
        	
            mStringBuilder.append("RecogMode:");
            mStringBuilder.append(isRecogModeSceneMapping ?
            		"SCENE_MAPPING" :
            		"TARGET_TRACKING");
            mStringBuilder.append("\n");
            
            mStringBuilder.append("State:");
            mStringBuilder.append(state);
            mStringBuilder.append("\n");
            
            mStringBuilder.append("InitMode:");
            mStringBuilder.append(initMode);
            mStringBuilder.append("\n");
            
            mStringBuilder.append("Search policy:");
            mStringBuilder.append(isSearchPolicyPresicive ?
            		"PRECISIVE" :
            		"FAST");
            mStringBuilder.append("\n");
    		
    		mStringBuilder.append("Dense map mode:");
    		mStringBuilder.append(isDenseMapModeSemiDense ?
    				"SEMI_DENSE" :
    				"DISABLE");
    		mStringBuilder.append("\n");
    		
    		mStringBuilder.append("Landmark");
    		mStringBuilder.append("\nTracked:");
    		mStringBuilder.append(String
    				.valueOf(mActivity.nativeGetTrackedLandmarkCount(sampleCore)));
    		mStringBuilder.append(" Lost:");
    		mStringBuilder.append(String
    				.valueOf(mActivity.nativeGetLostLandmarkCount(sampleCore)));
    		mStringBuilder.append(" Suspended:");
    		mStringBuilder.append(String
    				.valueOf(mActivity.nativeGetSuspendedLandmarkCount(sampleCore)));
    		mStringBuilder.append(" Masked:");
    		mStringBuilder.append(String
    				.valueOf(mActivity.nativeGetMaskedLandmarkCount(sampleCore)));
    		
    		mStatusView.setText(mStringBuilder);

            View dummyForLocationView = (View)mActivity.findViewById(R.id.dummyForLocationView);
            mActivity.nativeSetLandMarkBarLocation(sampleCore, dummyForLocationView.getLeft(), dummyForLocationView.getTop());
        }
    };
    
    // Get number(s) after decimal point.
	private int getF2(double num, int digit){
		if(digit == 1){
			return (int)(num * 10) - ((int)num) * 10;
		}else if(digit == 2){
			num = num * 10;
			return (int)(num * 10) - ((int)num) * 10;
		}
		return 0;
	}
    
    public boolean isStatusViewShown(){
    	return mStatusView.isShown();
    }
    
    public void setStatusViewVisiblity(int visiblity){
    	mStatusView.setVisibility(visiblity);
    }
}