#pragma once
#include "painting.h"

int checkObjInPict(int* picture,int* object,int pictSize,int objSize,double match,int* answerArr);

void readFromFile(const char *fileName,Paint** pictures,Paint** objects,double* match,int* pictNum,int*objNum);

void writeToFile(const char* fileName,int picId,int objId,int row,int col,int found);

void packPicture(Paint* picture,char** buffer,int* position,int bufferSize);


