package com.example.ndktest1;

import java.util.ArrayList;
import java.util.List;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;

public class DrawView extends View implements OnTouchListener {
    Segmenter seg;
    int[] segmentIDs;
	List<Point> points = new ArrayList<Point>();
    Paint paint = new Paint();
    Paint paint2 = new Paint();
	private static Bitmap bmp;
	private static Bitmap bmpG;
	private static boolean showRes = false;
	boolean osAvailable = false;
	static int lastID = 0;
			
    public DrawView(Context context, AttributeSet attrSet){
        super(context);
        setFocusable(true);
        setFocusableInTouchMode(true);
        this.setOnTouchListener(this);
        paint.setColor(Color.YELLOW);
    }
    public DrawView(Context context) {
        super(context);
        setFocusable(true);
        setFocusableInTouchMode(true);
        this.setOnTouchListener(this);
        paint.setColor(Color.YELLOW);
        paint2.setColor(Color.YELLOW);
        paint2.setAlpha(80);

    }

    public Bitmap toGrayscale(Bitmap bmpOriginal)
    {        
        int width, height;
        height = bmpOriginal.getHeight();
        width = bmpOriginal.getWidth();    

        Bitmap bmpGrayscale = Bitmap.createBitmap(width, height, Bitmap.Config.RGB_565);
        Canvas c = new Canvas(bmpGrayscale);
        Paint paint = new Paint();
        ColorMatrix cm = new ColorMatrix();
        cm.setSaturation(0);
        ColorMatrixColorFilter f = new ColorMatrixColorFilter(cm);
        paint.setColorFilter(f);
        c.drawBitmap(bmpOriginal, 0, 0, paint);
        return bmpGrayscale;
    }
    
    @Override
    public void onDraw(Canvas canvas) {
    	if(showRes)
    	{
    		bmpG.eraseColor(Color.WHITE);
    		lastID = 0;
    	}
   // 	canvas.drawBitmap(bmpG, 0, 0, paint);
  //      for (int i=0;i<points.size()-1;i++) {
     //   	canvas.drawLine(points.get(i).x, points.get(i).y, points.get(i+1).x, points.get(i+1).y, paint);
//            canvas.drawCircle(point.x, point.y, 2, paint);  
    //    }
        if(osAvailable)
        {
        	List<Point> pp = seg.getPoints();
        	//Log.w("Segment","G");
        	Point p;
        	for(int i=lastID;i<pp.size();i++){
        		p=pp.get(i);
        		bmpG.setPixel((int)p.x, (int)p.y, bmp.getPixel((int)p.x, (int)p.y));
        	}
        	lastID = pp.size()>0 ? pp.size()-1:0;
//        	canvas.drawPoints(seg.getPoints(), paint2);
/*        	for(int i=0;i<fgSegments.size();i++)
        		for(int w=(int) segmentArray[fgSegments.get(i)].tl.x;w<segmentArray[fgSegments.get(i)].br.x;w++)
            		for(int h=(int) segmentArray[fgSegments.get(i)].tl.y;h<segmentArray[fgSegments.get(i)].br.y;h++)
            			if(segmentIDs[960*h+w]==fgSegments.get(i))
    	        			canvas.drawCircle(w, h, 1, paint2);*/
        }
    	canvas.drawBitmap(bmpG, 0, 0, paint);
    }

    public boolean onTouch(View view, MotionEvent event) {
        Point point = new Point();
        point.x = event.getX();
        point.y = event.getY();
        points.add(point);
        if(osAvailable){
        	seg.addInteraction(segmentIDs[((int)point.y)*960+(int)point.x]);
        }
        invalidate();
        return true;
    }
    
    public void setBMP(Bitmap b){
    	bmp = b;
    	bmpG = toGrayscale(bmp);
    }
    public void setOS(int[] ids){
    	segmentIDs = ids;
    	osAvailable = true;
    }
    public void setSegmenter(Segmenter _seg){
    	seg = _seg;
    }
    public void showResult(){
    	showRes = true;
    	invalidate();
    }
}