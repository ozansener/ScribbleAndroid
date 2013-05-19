package com.example.ndktest1;

import java.io.IOException;

import android.net.Uri;
import android.os.Bundle;
import android.os.Process;
import android.provider.MediaStore.Images.Media;
import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.ImageView;

public class MainActivity extends Activity {
	
	
	
	private static Bitmap bmp;
	private static boolean firstCall=true;
	DrawView dw;
	Segmenter seg;
	int[] segmentIDs = new int[960*540+1];
	Superpixel[] segmentArray;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, 
                                WindowManager.LayoutParams.FLAG_FULLSCREEN);

		setContentView(R.layout.activity_main);
		//ImageView iv = (ImageView) findViewById(R.id.iv);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}
	public void testt(View vw){
//		EditText e = (EditText) findViewById(R.id.edit_message);
	//	e.setText("A");
		//DrawView dw = (DrawView)findViewById(R.id.drawView1);
		//dw.points.clear();
	}
	public void loadIm(View vw){
		Intent intent = new Intent();
		intent.setType("image/*");
		intent.setAction(Intent.ACTION_GET_CONTENT);
		//intent.addCategory(Intent.CATEGORY_OPENABLE);
		startActivityForResult(Intent.createChooser(intent, "Select Image"), 1);
	}
   @Override
    protected void onActivityResult(int requestCode,int resultCode,Intent data){
    	super.onActivityResult(requestCode, resultCode, data);
    	if ((resultCode==RESULT_OK )&& (requestCode==1)){
    		Uri targetUri =  data.getData();
    		try{
    			bmp = Media.getBitmap(this.getContentResolver(), targetUri);
    			bmp = Bitmap.createScaledBitmap(bmp, 960, 540, true);
    		} catch (IOException e) {
    	        e.printStackTrace();
    	     }
    	}
    	
		dw = new DrawView(this);
		dw.setBackgroundColor(Color.WHITE);
		dw.setBMP(bmp);
		setContentView(dw);
		dw.requestFocus();
    }
	public boolean onOptionsItemSelected(MenuItem item){
		switch(item.getItemId()){
		case	R.id.menu_segment:
			dw.showResult();
			return true;
		case	R.id.menu_settings:
			return true;
		case	R.id.menu_clear:
			Process.killProcess(Process.myPid());
			dw.points.clear();
			setContentView(R.layout.activity_main);
			return true;
		case	R.id.menu_os:
			if(!firstCall)
				if(seg!=null)
					seg.stp();
			firstCall = false;
			seg = new Segmenter();
			seg.setBMP(bmp);
			seg.overSegment();
			seg.getIDs(segmentIDs);
			dw.setOS(segmentIDs);
			dw.setSegmenter(seg);
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}
	}
}
