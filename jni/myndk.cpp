#include <android/log.h>

#include <jni.h>
#include "SLIC.h"
#include "StructDefs.h"

#include <string.h>
#include<stdio.h>
#include<iostream>
#include <vector>
#include "grabcut.h"
using namespace std;
static bool firstCall = true;
int* segmentID;
int numlabels;
RegionNode* regionNode;
unsigned int* ubuff;
int b;
GrabCut* gCut;
static int lastFG=0;

extern "C" {

static void createGraphStructure(int width,int height,jint* _psValues);

void Java_com_example_ndktest13D_Segmenter_releaseCMem(JNIEnv* env,jobject thiz){
	b=0;
	if(gCut)
		delete gCut;
	if(regionNode)
		delete [] regionNode;
	if(ubuff)
	delete [] ubuff;
}

void Java_com_example_ndktest13D_MainActivity_Init(JNIEnv* env,jobject thiz){
	b=0;
}

jstring Java_com_example_ndktest13D_MainActivity_NativeF(JNIEnv* env, jobject javaThis) {
		char testStr[1024];
		static int a=0;
		sprintf(testStr,"Val= %d %d",a++,b++);
      	return env->NewStringUTF(testStr);
	}
JNIEXPORT jintArray JNICALL Java_com_example_ndktest13D_Segmenter_SegmentNative(JNIEnv* env, jobject thiz,jintArray jintIDs,jint sizee){
	vector<int> aa;
    	jint*  _jinteract = env->GetIntArrayElements(jintIDs, 0);
	jintArray result ;
	for(int i=lastFG;i<sizee;i++){
		gCut->add2FG(_jinteract[i]);
	//	aa.push_back(_jinteract[i]);
	}
	lastFG = sizee;
	gCut->runResidualGrabCutNoBG();
	for(int i=0;i<numlabels;i++)
		if(regionNode[i].groupID==1)
			aa.push_back(i);
	/*
	*	Real Segmentation Part
	*	Is Here
	*/
	result = env->NewIntArray(aa.size());
	if (result == NULL) {
	     return NULL; /* out of memory error thrown */
	}
//	__android_log_print(ANDROID_LOG_INFO, "SegmentNative", "Number of FGs = %d", aa.size());

	jint* fill=new jint[aa.size()];
	 for (int i = 0; i < aa.size(); i++) {
	     fill[i] = aa[i]; 
	 }
	 env->SetIntArrayRegion(result, 0, aa.size(), fill);
	 delete [] fill;
	 return result;
    	
}


JNIEXPORT void JNICALL Java_com_example_ndktest13D_Segmenter_overSegmentNative(JNIEnv* env, jobject thiz,jintArray jsegmentID,jintArray psValues)
{
    	jint*  _jsegmentID = env->GetIntArrayElements(jsegmentID, 0);
    	jint*  _psValues = env->GetIntArrayElements(psValues, 0);

	if(firstCall)
	{
		segmentID = (int*) calloc(960*540,sizeof(int));
		firstCall = false;
	}
	ubuff = new unsigned int[960*540];
	int r,g,b,c;
	for(int i=0;i<540;i++)
		for(int j=0;j<960;j++)
		{
			c = _psValues[i*960+j];
    	         	r = (c >> 16) & 0xff;     //bitwise shifting
                 	g = (c >> 8) & 0xff;
                 	b = (c) & 0xff;

			ubuff[i*960+j]= (r<<16) + (g<<8) + (b);
		}

	int blockSize=16;
	numlabels = 0;
	int m_spcount = 1000 ;
	int m_compactness = 20;
	SLIC slic;
	slic.DoSuperpixelSegmentation_ForGivenK(ubuff, 960, 540, segmentID, numlabels, m_spcount, m_compactness);
	//Create graph structure
	createGraphStructure(960,540,_psValues);
	gCut = new GrabCut();
    gCut->setSegmentNumber(numlabels);
    gCut->setImage(ubuff);
    gCut->setRegionNode(&regionNode);
    gCut->setSegmentID(segmentID);
    gCut->width = 960;
    gCut->height = 540;

	for(int i=0;i<960*540;i++)
		_jsegmentID[i]=segmentID[i];
	_jsegmentID[960*540]=numlabels;
	env->ReleaseIntArrayElements(jsegmentID, _jsegmentID, 0);
	env->ReleaseIntArrayElements(psValues, _psValues, 0);

}


static void createGraphStructure(int width,int height,jint* _psValues){
    static int fCnt = 0;
    regionNode = new RegionNode[numlabels];
    for(int i=0;i<numlabels;i++){
        regionNode[i].neighborNum=0;
        regionNode[i].groupID = 0;
        regionNode[i].maxX = 0;
        regionNode[i].maxY = 0;
        regionNode[i].minX = width;
        regionNode[i].minY = height;
        regionNode[i].segmentSize = 0;
        regionNode[i].meanB=0;
        regionNode[i].meanG=0;
        regionNode[i].meanR=0;
        for(int j=0;j<10;j++)
        {
            regionNode[i].boundSize[j]=0;
            regionNode[i].neighborDistance[j]=0;
        }
    }
    int* numberOfNeighBoors = new int[numlabels*numlabels];
    memset(numberOfNeighBoors,0,numlabels*numlabels*sizeof(int));
    float * neighbDist = new float[numlabels*numlabels];
    memset(neighbDist,0,sizeof(float)*numlabels*numlabels);

    for(int w=0;w<width-1;w++)
        for(int h=0;h<height-1;h++)
        {
            numberOfNeighBoors[segmentID[h*width+w]*numlabels+segmentID[h*width+w+1]]++;
            numberOfNeighBoors[segmentID[h*width+w]*numlabels+segmentID[(h+1)*width+w]]++;
        }

	int r,g,b,c;
    for(int w=0;w<width;w++)
        for(int h=0;h<height;h++)
        {
            regionNode[segmentID[h*width+w]].segmentSize++;
		c = _psValues[h*width+w];
	 	r = (c >> 16) & 0xff;     //bitwise shifting
	 	g = (c >> 8) & 0xff;
	 	b = (c) & 0xff;
            regionNode[segmentID[h*width+w]].meanB+=b;
            regionNode[segmentID[h*width+w]].meanG+=g;
            regionNode[segmentID[h*width+w]].meanR+=r;


            if(w>regionNode[segmentID[h*width+w]].maxX)
                    regionNode[segmentID[h*width+w]].maxX = w;
            if(h>regionNode[segmentID[h*width+w]].maxY)
                    regionNode[segmentID[h*width+w]].maxY = h;
            if(w<regionNode[segmentID[h*width+w]].minX)
                regionNode[segmentID[h*width+w]].minX = w;
            if(h<regionNode[segmentID[h*width+w]].minY)
                regionNode[segmentID[h*width+w]].minY = h;
        }
    for(int i=0;i<numlabels;i++)
        for(int j=0;j<numlabels;j++)
        {
            if(i==j)
                continue;
            if((numberOfNeighBoors[i*numlabels+j]!=0)||(numberOfNeighBoors[j*numlabels+i]!=0))
            {
                if(regionNode[j].neighborNum<10)
                {
                    regionNode[j].neighborID[regionNode[j].neighborNum]=i;
                    regionNode[j].neighborNum++;
                }
            }
        }


    for(int i=0;i<numlabels;i++)
    {
        regionNode[i].meanB = regionNode[i].meanB/regionNode[i].segmentSize;
        regionNode[i].meanR = regionNode[i].meanR/regionNode[i].segmentSize;
        regionNode[i].meanG = regionNode[i].meanG/regionNode[i].segmentSize;
    }

    delete [] neighbDist;
    delete [] numberOfNeighBoors;
}
}
