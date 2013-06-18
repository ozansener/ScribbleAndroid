package com.example.ndktest13D;

import java.util.List;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.os.Bundle;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.htc.view.DisplaySetting;

public class Draw3DView extends SurfaceView implements SurfaceHolder.Callback {
    private Canvas mcanvas;
    private Bitmap mbitmap;
    private Surface msurface;
    private SurfaceHolder holder;
	private int width;
	private int height;
    boolean s3DAvailable = true;
    private static Bitmap bmpOriginal;
    private static Bitmap bmpBG;
    private static Bitmap bmpSideBySide;	
    private List<Integer> depth;
    private static int isF = 0;
    public Draw3DView(Context context) {
        super(context);

      //  mbitmap = BitmapFactory.decodeResource(getResources(), R.drawable.test);
//        bitmap2D = BitmapUtils.create2DBitmap(bitmap);

        holder = this.getHolder();
        holder.setFormat(PixelFormat.RGBA_8888);
        holder.addCallback(this);

        msurface = holder.getSurface();
        
        

    }

   
	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		
	      boolean formatResult = true;
	      this.width  = width;
	      this.height = height;
	      if(isF==1){
	    	  processDepth();
	    	  isF++;
	      }
		try {    
	        formatResult = DisplaySetting.setStereoscopic3DFormat(msurface,
	                  DisplaySetting.STEREOSCOPIC_3D_FORMAT_SIDE_BY_SIDE);
	      } catch (NoClassDefFoundError e) {
	        android.util.Log.i("S3D", "class not found - S3D display not available");
	        s3DAvailable = false;
	      }

	      android.util.Log.i("S3D", "return value:" + formatResult);
	      
	      if (!formatResult) {
	        android.util.Log.i("S3D", "S3D format not supported");
	        return;
	      }

	      mcanvas = holder.lockCanvas();
	      if (!s3DAvailable) {
	        // if DisplaySetting not found, create anaglyph version of 3D image
	        mbitmap = BitmapUtils.createAnaglyphBitmap(mbitmap);
	      }
	      mcanvas.drawBitmap(mbitmap, null, new Rect(0,0,width,height), null);
	      holder.unlockCanvasAndPost(mcanvas);
	    } 
	@Override
	public void surfaceCreated(SurfaceHolder arg0) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder arg0) {
		// TODO Auto-generated method stub
        try {
            DisplaySetting.setStereoscopic3DFormat(msurface,
                    DisplaySetting.STEREOSCOPIC_3D_FORMAT_OFF);
        } catch (NoClassDefFoundError e) {
        }		
	} 
	boolean isValid(int x,int y,int width,int height){
		return (x>0)&(x<width)&(y>0)&(y<height);
	}
	public void setImage(Bitmap b,List<Integer> depth){
			this.depth = depth;
			this.bmpOriginal = b;
			bmpBG = b.copy(Bitmap.Config.ARGB_8888, true);
			isF = 1;
	}
	
	private void processDepth(){
		boolean fll = false;
		int a=0;
		for(int x=1;x<width-1;x++)
			for(int y=1;y<height-1;y++){
				if((depth.get(y*width+x)==1)||(depth.get((y-1)*width+x)==1)||(depth.get(y*width+x-1)==1)||(depth.get(y*width+x+1)==1)||(depth.get((y+1)*width+x)==1)){
					fll = true;
					bmpBG.setPixel(x, y, Color.BLACK);
					a=1;
					while(fll){
						if(isValid(y,x-a-3,width,height))
							if(depth.get(y*width+x-a)==0){
								bmpBG.setPixel(x, y, bmpBG.getPixel(x-a-3, y));
								fll=false;
								break;
							}
						if(isValid(y,x+a+3,width,height))
							if(depth.get(y*width+x+a)==0){
								bmpBG.setPixel(x, y, bmpBG.getPixel(x+a+3, y));
								fll=false;
								break;							
							}
						a=a+6;
						if(a>800)
							break;
					}
				}
			}
	
		bmpSideBySide = this.bmpOriginal.copy(Bitmap.Config.ARGB_8888, true);
		int bb = 12;

		for(int x=0;x<(int)Math.floor(width/2);x++){
			for(int y=0;y<height;y++){
				if(depth.get(y*width+x*2)==0)
					bmpSideBySide.setPixel(x+(int)(width/2), y, bmpBG.getPixel(2*x, y));
				else
					bmpSideBySide.setPixel(x+(int)(width/2), y, bmpOriginal.getPixel(2*x, y));
				if(isValid(2*(x-bb), y, width, height)){
						if(depth.get(y*width+2*(x-bb))==1)
							bmpSideBySide.setPixel(x/*+(int)(width/2)*/, y, bmpOriginal.getPixel(2*(x-bb), y));					
						else
							bmpSideBySide.setPixel(x/*+(int)(width/2)*/, y, bmpBG.getPixel(2*x, y));
				}else{
							bmpSideBySide.setPixel(x/*+(int)(width/2)*/, y, bmpOriginal.getPixel(2*x, y));					
				}
			}	
		}	
			
		mbitmap = bmpSideBySide;
	}
}
