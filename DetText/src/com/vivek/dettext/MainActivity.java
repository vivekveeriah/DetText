package com.vivek.dettext;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Size;

import android.net.NetworkInfo.DetailedState;
import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;
import android.view.Window;
import android.view.WindowManager;

public class MainActivity extends Activity implements CvCameraViewListener2 {
	
	private Mat viewFrame;
	private Mat outputFrame;
	private Mat mRGBAinnerFrame;
	
	private Size viewFrameSize;
	
	public native void detectText(long inputFrame, long outputFrame);
	
	private CameraBridgeViewBase mOpencvCameraView;
		private BaseLoaderCallback mLoaderCallBack = new BaseLoaderCallback(this) 
	{
		@Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    System.out.println("OpenCV Loaded Successfully");
                    System.loadLibrary("DetText");
                    mOpencvCameraView.enableView();
                } break;
                default:
                {
                    super.onManagerConnected(status);
                } break;
            }
        }
	};
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_main);
        
        mOpencvCameraView = (CameraBridgeViewBase) findViewById(R.id.textlocalization_surface_view);
        mOpencvCameraView.setCvCameraViewListener(this);
    }
    
    @Override
    public void onPause()
    {
        super.onPause();
        if (mOpencvCameraView != null)
        	mOpencvCameraView.disableView();
    }

    @Override
    public void onResume()
    {
        super.onResume();
        OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_6, this, mLoaderCallBack);
    }

    public void onDestroy() {
        super.onDestroy();
        if (mOpencvCameraView != null)
        	mOpencvCameraView.disableView();
    }
 
    private void createAuxillaryMats()
    {
    	if(viewFrame.empty())
			return;
		viewFrameSize = viewFrame.size();
		
		int rows = (int) viewFrame.height();
		int cols = (int) viewFrame.width();
		
		int left = cols / 8;
		int top = rows / 8;
		
		int width = cols * 3 / 4;
		int height = rows * 3 / 4;
		
		if(mRGBAinnerFrame == null)
		{
			mRGBAinnerFrame = viewFrame.submat(top, top + height, left, left + width);
			outputFrame = new Mat(mRGBAinnerFrame.size(), CvType.CV_8UC1);
		}
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

	@Override
	public void onCameraViewStarted(int width, int height) {
		viewFrame = new Mat(width, height, CvType.CV_8UC1);		
	}

	@Override
	public void onCameraViewStopped() {
		viewFrame.release();
	}

	@Override
	public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
		viewFrame = inputFrame.gray();
		if(mRGBAinnerFrame == null)
			createAuxillaryMats();
		detectText(mRGBAinnerFrame.getNativeObjAddr(), outputFrame.getNativeObjAddr());
		return viewFrame;
	}
    
}
