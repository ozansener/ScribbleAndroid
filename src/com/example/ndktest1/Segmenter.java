package com.example.ndktest1;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.util.Log;
import android.view.Window;
import android.view.WindowManager;

public class Segmenter {
	private Lock l=new ReentrantLock();
	private Condition cond=l.newCondition();
	private boolean sFlag=false;
	int lastID;
	List<Integer> interactedSegments;
	List<Integer> fgSegs;

	Superpixel[] segmentArray;
	float[] pointArray;
	List<Point> pointList = new ArrayList<Point>();
	private static Bitmap bmp;
	int[] segmentIDs = new int[960*540+1];
	
	static {
		System.loadLibrary("myndk");
	}
	private static native String NativeF();
	private static native void Init();
	private static native void overSegmentNative(int[] segmentIDs,int [] psValues);
	private static native int[] SegmentNative(int[] interactions,int size);
	private static native void releaseCMem();
	
	public Segmenter(){
		fgSegs = new ArrayList<Integer>();
		interactedSegments = new ArrayList<Integer>();
		lastID = 0;
	}
	public void addInteraction(int id){
		if(!interactedSegments.contains(id))
			interactedSegments.add(id);
	}
	
	
	public void Segment(){
		  new Thread(new Runnable() {
			    public void run() {
					int[] fgVals;
			    	while(true){
			    		if(lastID==interactedSegments.size())
							try {
								Thread.sleep(100);
							} catch (InterruptedException e) {
								// TODO Auto-generated catch block
								e.printStackTrace();
							}
						else{
			    			lastID = interactedSegments.size();
			    			int[] segArray = new int[interactedSegments.size()];
			    			for(int i=0;i<interactedSegments.size();i++)
			    				segArray[i]=interactedSegments.get(i);
			    			fgVals = SegmentNative(segArray,segArray.length);
			    			Log.w("Segment",String.format("S:%d", fgVals.length));
			    			/*
			    			 * Do the segmentation
			    			 * */
			    			l.lock();
			    			try {
				    			while(sFlag=false){				    				
										cond.await();
				    			}
//				    			pointList.clear();
					        	for(int i=0;i<fgVals.length;i++){
					        		if(!fgSegs.contains(fgVals[i])){
					        			for(Point p:segmentArray[fgVals[i]].allPixels)
					        				pointList.add(p);
					        			fgSegs.add(fgVals[i]);
					        		}
					        	}

				    			sFlag = false;
							} catch (InterruptedException e) {
								// TODO Auto-generated catch block
								e.printStackTrace();
							} finally {
					        	l.unlock();
							}

			    		}
			    		
			    	}
			    }
			  }).start();
	}
	
	public List<Point>/*float[]*/ getPoints(){
		l.lock();
		List<Point> pList = new ArrayList<Point>(pointList);
		sFlag = true;
		/*
		int sCur = pointList.size();
		pointArray = new float[sCur*2];
		for(int i=0;i<sCur;i++){
			pointArray[2*i+0]=pointList.get(i).x;
			pointArray[2*i+1]=pointList.get(i).y;
		}*/
		l.unlock();
		//return pointArray;
		return pList;
	}
	
	public void setBMP(Bitmap bmp){
		this.bmp = bmp;
	}
	public void overSegment(){
		int [] pixs=new int[960*540];
		bmp.getPixels(pixs, 0, 960, 0, 0, 960, 540);
   		overSegmentNative(segmentIDs,pixs);
   		
   		Log.w("Over Segment",String.format("Number of Segments:%d", segmentIDs[960*540]));
   		segmentArray = new Superpixel[segmentIDs[960*540]];
   		for(int i=0;i<segmentArray.length;i++){
   			segmentArray[i]=new Superpixel();
   			segmentArray[i].tl.x=960;segmentArray[i].tl.y=540;
   			segmentArray[i].br.x=0;segmentArray[i].br.y=0;
   			segmentArray[i].center = new Point();
   			segmentArray[i].center.x = 0;
   			segmentArray[i].center.y = 0;
   		}
   		for(int w=0;w<960;w++)
   			for(int h=0;h<540;h++){
   				segmentArray[segmentIDs[h*960+w]].allPixels.add(new Point(w,h));
   				segmentArray[segmentIDs[h*960+w]].center.x += w;
   				segmentArray[segmentIDs[h*960+w]].center.y += h;
   			}
  		for(int i=0;i<segmentArray.length;i++){
  			segmentArray[i].center.x = segmentArray[i].center.x / segmentArray[i].allPixels.size(); 
  			segmentArray[i].center.y = segmentArray[i].center.y / segmentArray[i].allPixels.size();
  		}
   		Segment();
	}
	public void getIDs(int[] segIDs){
		System.arraycopy(segmentIDs, 0, segIDs, 0, segmentIDs.length);
	}
	
	public void stp() {
		releaseCMem();
	}

}

