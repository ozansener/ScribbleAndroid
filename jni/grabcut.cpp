#include "grabcut.h"
#include <math.h>
#include <algorithm>
using namespace std;
GrabCut::GrabCut()
{
    fgGMM.initializeAndClear(3,5);
    bgGMM.initializeAndClear(3,5);
    foregroundGraph.clear();
    backgroundGraph.clear();
    firstRun = true;
    isROC = true;
}

void GrabCut::add2FG(int toAdd){

    if(isROC)
    {
        bBoxXmin = width;
        bBoxXmax = 0;
        bBoxYmin = height;
        bBoxYmax = 0;
        isROC = false;
    }
    if((*regionNode)[toAdd].minX<bBoxXmin)
        bBoxXmin=(*regionNode)[toAdd].minX;
    if((*regionNode)[toAdd].maxX>bBoxXmax)
        bBoxXmax=(*regionNode)[toAdd].maxX;
    if((*regionNode)[toAdd].minY<bBoxYmin)
        bBoxYmin=(*regionNode)[toAdd].minY;
    if((*regionNode)[toAdd].maxY>bBoxYmax)
        bBoxYmax=(*regionNode)[toAdd].maxY;

    if(toAdd>segmentNumber)
        return;
    for(int i=0;i<((int)foregroundGraph.size());i++)
        if(foregroundGraph[i]==toAdd)
            return;
    foregroundGraph.push_back(toAdd);

    int size= (*regionNode)[toAdd].segmentSize;
    int cnt=0;
    float* inVec = new float[size*3];
    unsigned int c;
    for(int w=(*regionNode)[toAdd].minX;w<(*regionNode)[toAdd].maxX;w=w+4)
       for(int h=(*regionNode)[toAdd].minY;h<(*regionNode)[toAdd].maxY;h=h+4)
       {
            if(segmentIDs[h*width +w]==toAdd)
            {
    			c = imBuff[h*width+w];
                inVec[cnt*3+0]=(c >> 16) & 0xff;
                inVec[cnt*3+1]=(c >> 8) & 0xff;
                inVec[cnt*3+2]=(c) & 0xff;
                cnt++;
            }
       }
    fgGMM.insertData(inVec,cnt);
    fgGMM.iterateGMM(3);
    delete [] inVec;
}
void GrabCut::add2BG(int toAdd){
    backgroundGraph.push_back(toAdd);

    for(int i=0;i<((int)backgroundGraph.size())-1;i++)
        if(backgroundGraph[i]==toAdd)
            return;

    int size=(*regionNode)[toAdd].segmentSize;
    int cnt=0;
    float* inVec = new float[size*3];
    unsigned int c;
    for(int w=(*regionNode)[toAdd].minX;w<(*regionNode)[toAdd].maxX;w=w+4)
       for(int h=(*regionNode)[toAdd].minY;h<(*regionNode)[toAdd].maxY;h=h+4)
       {
            if(segmentIDs[h*width+w]==toAdd)
            {
    			c = imBuff[h*width+w];
                inVec[cnt*3+0]=(c >> 16) & 0xff;
                inVec[cnt*3+1]=(c >> 8) & 0xff;
                inVec[cnt*3+2]=(c) & 0xff;
                cnt++;
            }
       }
    bgGMM.insertData(inVec,cnt);
    bgGMM.iterateGMM(3);
    delete [] inVec;
}
void GrabCut::setSegmentNumber(int number){
    segmentNumber = number;
}
void GrabCut::setRegionNode(RegionNode** rN){
    regionNode = rN;
}
int GrabCut::getResult(int segmentID){
    if((*regionNode)[segmentID].groupID==1)
        return 0;
    else
        return 1;
}


void GrabCut::setImage(unsigned int* inpBuf){
	imBuff = inpBuf;
}

void GrabCut::reset(){
    bgGMM.clear();
    fgGMM.clear();
    foregroundGraph.clear();
    backgroundGraph.clear();
}

void GrabCut::setSegmentID(int *s){
    segmentIDs = s;
}


void GrabCut::runResidualGrabCutFirstRun(){
    bgVec = new float[3*width*height];
    gr = new FloatGraph(segmentNumber,segmentNumber*10);
    for(int i=0;i<segmentNumber;i++)
        gr->add_node();
    curGMM = new EM;
    oldGMM = new EM;

    curGMM->EnergyBG = new float[segmentNumber];
    curGMM->EnergyFG = new float[segmentNumber];

    oldGMM->EnergyFG = new float[segmentNumber];
    oldGMM->EnergyBG = new float[segmentNumber];

    oldGMM->RegionNumPcsdPixel = new int[segmentNumber];
    curGMM->RegionNumPcsdPixel = new int[segmentNumber];

    oldIDs = new int[segmentNumber];

    for(int i=0;i<segmentNumber;i++)
     {
        curGMM->EnergyBG[i]=0;
        curGMM->EnergyFG[i]=0;
        curGMM->RegionNumPcsdPixel[i]=0;

        oldGMM->EnergyBG[i]=0;
        oldGMM->EnergyFG[i]=0;
        oldGMM->RegionNumPcsdPixel[i]=0;

        oldIDs[i]=0;
    }

    for(int i=0;i<segmentNumber;i++)
        (*regionNode)[i].groupID = 3;

    for(int x=max(0,bBoxXmin-20);x<min(width,bBoxXmax+20);x++)
        for(int y=max(0,bBoxYmin-20);y<min(height,bBoxYmax+20);y++)
            (*regionNode)[segmentIDs[y*width+x]].groupID=2;

    int nID;
    for(int i=0;i<foregroundGraph.size();i++)
    {
        (*regionNode)[foregroundGraph[i]].groupID = 1;
        for(int j=0;j<(*regionNode)[foregroundGraph[i]].neighborNum;j++)
        {
            nID = (*regionNode)[foregroundGraph[i]].neighborID[j] ;
            (*regionNode)[nID].groupID=0;
        }
    }

    for(int i=0;i<foregroundGraph.size();i++)
        (*regionNode)[foregroundGraph[i]].groupID = 1;

    unsigned int c;
    int rId;
    float inve[3];
    for(int w=0;w<width;w=w+6)
    {
        for(int h=0;h<height;h=h+6)
        {
        	c = imBuff[h*width+w];
            inve[0]=(c >> 16) & 0xff;
            inve[1]=(c >> 8) & 0xff;
            inve[2]=(c) & 0xff;
            rId = segmentIDs[w+h*width];
            curGMM->EnergyFG[rId]+=fgGMM.getLikelihood(inve);
            curGMM-> RegionNumPcsdPixel[rId]++;
        }
    }

    for(int i=0;i<segmentNumber;i++)
     {
         if(curGMM->RegionNumPcsdPixel[i]==0)
         {
             curGMM->EnergyFG[i]=-1;
         }
         else
         {
             curGMM->EnergyFG[i]=curGMM->EnergyFG[i]/((float)curGMM->RegionNumPcsdPixel[i]);
         }
     }

    int bCount =0;
    for(int w=0;w<width;w=w+16)
        for(int h=0;h<height;h=h+16){
            if((*regionNode)[ segmentIDs[h*width + w]].groupID==3){
            	c = imBuff[h*width+w];
                bgVec[bCount*3+0]= (c >> 16) & 0xff;
                bgVec[bCount*3+1]= (c >> 8) & 0xff;
                bgVec[bCount*3+2]= (c) & 0xff;
                bCount++;
            }
        }

    for(int w=0;w<width;w=w+8)
        for(int h=0;h<height;h=h+8){
            if((*regionNode)[ segmentIDs[h*width + w]].groupID==2){
            	c = imBuff[h*width+w];
                bgVec[bCount*3+0]= (c >> 16) & 0xff;
                bgVec[bCount*3+1]= (c >> 8) & 0xff;
                bgVec[bCount*3+2]= (c) & 0xff;
                bCount++;
            }
        }
    bgGMM.clear();
    bgGMM.insertData(bgVec,bCount);
    bgGMM.iterateGMM(15);
    for(int i=0;i<segmentNumber;i++)
     {
         curGMM->RegionNumPcsdPixel[i]=0;
     }
    for(int w=0;w<width;w=w+6)
        for(int h=0;h<height;h=h+6){
        	c = imBuff[h*width+w];
            inve[0]=(c >> 16) & 0xff;
            inve[1]=(c >> 8) & 0xff;
            inve[2]=(c) & 0xff;
            rId = segmentIDs[w+h*width];
            curGMM->EnergyBG[rId]+=bgGMM.getLikelihood(inve);
            curGMM->RegionNumPcsdPixel[rId]++;
        }

    for(int i=0;i<segmentNumber;i++)
     {
         if(curGMM->RegionNumPcsdPixel[i]==0)
             curGMM->EnergyBG[i]=0;
         else
             curGMM->EnergyBG[i]=curGMM->EnergyBG[i]/((float)curGMM->RegionNumPcsdPixel[i]);
     }
    for(int i=0;i<segmentNumber;i++)
        if(curGMM->EnergyFG[i]==-1)
            curGMM->EnergyFG[i]=0;
    int betaNum=0;
    float meane1=0;
    float incR;
    betaMean = 0;
    for(int i=0;i<segmentNumber;i++)
     {
         meane1+=curGMM->EnergyBG[i];
         meane1+=curGMM->EnergyFG[i];

         for(int j = 0;j<(*regionNode)[i].neighborNum;j++)
         {

             incR = ((*regionNode)[i].meanR - (*regionNode)[(*regionNode)[i].neighborID[j]].meanR)*((*regionNode)[i].meanR - (*regionNode)[(*regionNode)[i].neighborID[j]].meanR);
             incR += ((*regionNode)[i].meanG - (*regionNode)[(*regionNode)[i].neighborID[j]].meanG)*((*regionNode)[i].meanG - (*regionNode)[(*regionNode)[i].neighborID[j]].meanG);
             incR += ((*regionNode)[i].meanB - (*regionNode)[(*regionNode)[i].neighborID[j]].meanB)*((*regionNode)[i].meanB - (*regionNode)[(*regionNode)[i].neighborID[j]].meanB);

             betaMean += incR;
             betaNum++;
         }
     }
    betaMean = betaMean/betaNum;
    meane1=meane1/(2*segmentNumber);
    float lambda = 1;//If too low let changes on t-weights

    float e1const = 1 *(1/meane1);
    float e2const = 0.6; //Less than 2

    float beta = 1/betaMean;
    float capp;
    float capp2;
    for(int i=0;i<segmentNumber;i++)
    {
          if((*regionNode)[i].groupID==1)
          {
             gr->add_tweights(i,0,100);//0.5 * 10 + 1
             oldIDs[i]=1;
          }
          else if((*regionNode)[i].groupID==3)
          {
             gr->add_tweights(i,100,0);
             oldIDs[i]=3;
          }
          else
              gr->add_tweights(i,e1const*curGMM->EnergyBG[i],e1const*curGMM->EnergyFG[i]);


          for(int j=0;j<(*regionNode)[i].neighborNum;j++)
          {
              capp = ((*regionNode)[i].meanR - (*regionNode)[(*regionNode)[i].neighborID[j]].meanR)*((*regionNode)[i].meanR - (*regionNode)[(*regionNode)[i].neighborID[j]].meanR);
              capp += ((*regionNode)[i].meanG - (*regionNode)[(*regionNode)[i].neighborID[j]].meanG)*((*regionNode)[i].meanG - (*regionNode)[(*regionNode)[i].neighborID[j]].meanG);
              capp += ((*regionNode)[i].meanB - (*regionNode)[(*regionNode)[i].neighborID[j]].meanB)*((*regionNode)[i].meanB - (*regionNode)[(*regionNode)[i].neighborID[j]].meanB);

              capp2 = capp;
              capp = exp((-1)*capp*beta*lambda);
              gr->add_edge(i,(*regionNode)[i].neighborID[j],e2const*capp,e2const*capp);

          }

    }
    gr->maxflow();
    for(int i=0;i<segmentNumber;i++)
        if(gr->what_segment(i)==FloatGraph::SOURCE)
            (*regionNode)[i].groupID = 2;
        else
            (*regionNode)[i].groupID = 1;

}


void GrabCut::runResidualGrabCutNoBG(){

    if(firstRun)
    {
        runResidualGrabCutFirstRun();
        firstRun=false;
        return;
    }

    float olde1const = e1const;
    EM* dummy;
    dummy = curGMM;
    curGMM = oldGMM;
    oldGMM = dummy;
    for(int i=0;i<segmentNumber;i++)
    {
        curGMM->RegionNumPcsdPixel[i]=0;
        oldGMM->RegionNumPcsdPixel[i]=0;
    }

    unsigned int c;
    for(int i=0;i<segmentNumber;i++)
            (*regionNode)[i].groupID = 3;

        for(int x=max(0,bBoxXmin-20);x<min(width,bBoxXmax+20);x++)
            for(int y=max(0,bBoxYmin-20);y<min(height,bBoxYmax+20);y++)
                (*regionNode)[segmentIDs[y*width+x]].groupID=2;

        int nID;
        for(int i=0;i<foregroundGraph.size();i++)
        {
            (*regionNode)[foregroundGraph[i]].groupID = 1;
            for(int j=0;j<(*regionNode)[foregroundGraph[i]].neighborNum;j++)
            {
                nID = (*regionNode)[foregroundGraph[i]].neighborID[j] ;
                (*regionNode)[nID].groupID=0;
            }
        }

        for(int i=0;i<foregroundGraph.size();i++)
            (*regionNode)[foregroundGraph[i]].groupID = 1;


        int rId;
        float inve[3];
        for(int i=0;i<segmentNumber;i++){
            inve[0]= (*regionNode)[i].meanR;
            inve[1]= (*regionNode)[i].meanG;
            inve[2]= (*regionNode)[i].meanB;
            curGMM->EnergyFG[i]=fgGMM.getLikelihood(inve);
        }

        int bCount =0;
        for(int i=0;i<segmentNumber;i++){
        	if(((*regionNode)[i].groupID==3) || ((*regionNode)[i].groupID==2))
                bgVec[bCount*3+0]= (*regionNode)[i].meanR;
                bgVec[bCount*3+1]= (*regionNode)[i].meanG;
                bgVec[bCount*3+2]= (*regionNode)[i].meanB;
                bCount++;
                }

        bgGMM.clear();
        bgGMM.insertData(bgVec,bCount);
        bgGMM.initLastMean();
        bgGMM.iterateGMM(5);

        for(int i=0;i<segmentNumber;i++){
            inve[0]= (*regionNode)[i].meanR;
            inve[1]= (*regionNode)[i].meanG;
            inve[2]= (*regionNode)[i].meanB;
            curGMM->EnergyBG[i]=bgGMM.getLikelihood(inve);
        }

            float meane1=0;
            float incR;
            for(int i=0;i<segmentNumber;i++)
             {
                 meane1+=curGMM->EnergyBG[i];
                 meane1+=curGMM->EnergyFG[i];

             }
            meane1=meane1/(2*segmentNumber);
            float lambda = 1;//If too low let changes on t-weights

            e1const = 1 *(1/meane1);
            float e2const = 0.6; //Less than 2

            float beta = 1/betaMean;
            float capp;
            float capp2;
            for(int i=0;i<segmentNumber;i++)
            {
                  if((*regionNode)[i].groupID==1)
                  {
                      if(oldIDs[i]==1)
                      {
                          continue;
                      }
                      else
                      {
                          if(oldIDs[i]==0)
                              gr->set_trcap(i,gr->get_trcap(i)+(0-100-olde1const*oldGMM->EnergyBG[i]+olde1const*oldGMM->EnergyFG[i]));
                          else if(oldIDs[i]==3)
                              gr->set_trcap(i,gr->get_trcap(i)+(-200));
                          oldIDs[i]=1;
                          gr->mark_node(i);
                      }
                  }
                  else if((*regionNode)[i].groupID==3)
                  {
                      if(oldIDs[i]==3)
                      {
                          continue;
                      }
                      else
                      {
                          if(oldIDs[i]==0)
                              gr->set_trcap(i,gr->get_trcap(i)+(100-olde1const*oldGMM->EnergyBG[i]+olde1const*oldGMM->EnergyFG[i]));
                          else if(oldIDs[i]==1)
                              gr->set_trcap(i,gr->get_trcap(i)+(200));
                          oldIDs[i]=3;
                          gr->mark_node(i);
                      }
                  }
                  else
                  {
                      if(oldIDs[i]==1)
                      {
                          gr->set_trcap(i,gr->get_trcap(i)+(e1const*curGMM->EnergyBG[i]-e1const*curGMM->EnergyFG[i]+100));
                      }
                      else if(oldIDs[i]==0)
                      {
                          gr->set_trcap(i,gr->get_trcap(i)+(e1const*curGMM->EnergyBG[i]-e1const*curGMM->EnergyFG[i]-olde1const*oldGMM->EnergyBG[i]+olde1const*oldGMM->EnergyFG[i]));
                      }
                      else{
                          gr->set_trcap(i,gr->get_trcap(i)+(e1const*curGMM->EnergyBG[i]-e1const*curGMM->EnergyFG[i]-100));
                      }
                      oldIDs[i]=0;
                      gr->mark_node(i);
                  }
            }
            gr->maxflow();
            for(int i=0;i<segmentNumber;i++)
                if(gr->what_segment(i)==FloatGraph::SOURCE)
                    (*regionNode)[i].groupID = 2;
                else
                    (*regionNode)[i].groupID = 1;

}

void GrabCut::delSegs(){
	reset();
}

GrabCut::~Grabcut(){
	reset();
	if(bgVec)
		delete [] bgVec;
	if(gr)
		delete gr;
	if(curGMM->EnergyBG)
		delete [] curGMM->EnergyBG;
	if(curGMM->EnergyFG)
		delete [] curGMM->EnergyFG;
	if(oldGMM->EnergyFG)
		delete [] oldGMM->EnergyFG;
	if(oldGMM->EnergyBG)
		delete [] oldGMM->EnergyBG;
	if(oldGMM->RegionNumPcsdPixel)
		delete [] oldGMM->RegionNumPcsdPixel;
	if(curGMM->RegionNumPcsdPixel)
		delete [] curGMM->RegionNumPcsdPixel;
	if(oldIDs)
		delete [] oldIDs;
	delete curGMM;
	delete oldGMM;
}


