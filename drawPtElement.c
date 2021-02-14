#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "avecis.h"
#include "drawPtElement.h"


static void copy(float *vertsA, float *vertsB, int cnt)
{
   int i;
   
   for (i=0; i < cnt; i++)
   {
      vertsB[i] = vertsA[i];
   }
}

static void scale(float *verts, int cnt, float x, float y, float z)
{
   int i;
   
   for (i=0; i < cnt; i+=3)
   {
      verts[i+0] *= x;
      verts[i+1] *= y;
      verts[i+2] *= z;
   }
}

static void move(float *verts, int cnt, float x, float y, float z)
{
   int i;
   
   for (i=0; i < cnt; i+=3)
   {
      verts[i+0] += x;
      verts[i+1] += y;
      verts[i+2] += z;
   }
}


void freePtElementSphere(struct ptElemSphere *ptES)
{
   if (ptES->staticMeshData)
   free(ptES->staticMeshData);
   
   if (ptES->meshData)
   free(ptES->meshData);
   
   if (ptES->shadeData)
   free(ptES->shadeData);
}

void drawPtElementSphere(struct ptElemSphere *ptES, float x, float y, float z, float intensity)
{
   int i, j;
   unsigned char staticColor[3] = {0xFF, 0xFF, 0x0};
   unsigned char color[3];
   
   copy(&ptES->staticMeshData[0], &ptES->meshData[0], ptES->meshDataSz);
   scale(&ptES->meshData[0], ptES->meshDataSz, intensity, intensity, intensity);
   move(&ptES->meshData[0], ptES->meshDataSz, x, y, z);
   
   staticColor[0] = (unsigned char)((float)staticColor[0] * (1.0-intensity));
   staticColor[1] = (unsigned char)((float)staticColor[1] * intensity);
   
   for (i=0; i < ptES->shadeDataSz; i++)
   {
      for (j=0; j < 3; j++)
      color[j] = 0xFF - (unsigned char)((float)staticColor[j] * ptES->shadeData[i]);
      
      setColor((char *)color, 3);
      drawLine(&ptES->meshData[i*(6*ptES->shadeDataSz*4)], 6*ptES->shadeDataSz*4);
   }
}

void getPtElementSphere(struct ptElemSphere *ptES, int ringCnt, int ringPtCnt, float resize)
{
   int i, j, k;
   float height, size;
   float xPt, yPt, zPt;
   
   ptES->meshDataSz = (ringCnt * ringPtCnt * 2) * 6;
   ptES->staticMeshData = (float *)malloc(ptES->meshDataSz * sizeof(float));
   ptES->meshData = (float *)malloc(ptES->meshDataSz * sizeof(float));
   ptES->shadeDataSz = ringCnt / 2;
   ptES->shadeData = (float *)malloc(ptES->shadeDataSz * sizeof(float));
   
   // calculate mesh data
   k = 0;
   for (j=1; j <= ringCnt/2; j++)
   {
      height = cos(j * (M_PI / (ringCnt+1)));
      size = sin(j * (M_PI / (ringCnt+1)));
      
      xPt = cos(0 * (M_PI / ringPtCnt)) * size;
      yPt = sin(0 * (M_PI / ringPtCnt)) * size;
      zPt = height;
      
      for (i=1; i <= ringPtCnt*2; i++)
      {
         ptES->staticMeshData[k+0] = xPt;
         ptES->staticMeshData[k+1] = yPt;
         ptES->staticMeshData[k+2] = zPt;
         k += 3;
         
         xPt = cos(i * (M_PI / ringPtCnt)) * size;
         yPt = sin(i * (M_PI / ringPtCnt)) * size;
         zPt = height;
         
         ptES->staticMeshData[k+0] = xPt;
         ptES->staticMeshData[k+1] = yPt;
         ptES->staticMeshData[k+2] = zPt;
         k += 3;
      }
   }
   
   // calculate shade data
   for (i=0; i < ptES->shadeDataSz; i++)
   ptES->shadeData[i] = cos(i * (M_PI / (ringCnt-1)));
   
   // resize mesh
   scale(&ptES->staticMeshData[0], ptES->meshDataSz, resize, resize, resize);
}
