#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include "avecis.h"
#include "drawPtElement.h"
#include "ucrot.h"
#include "correspond3d.h"

#define HOSTNAME "127.0.0.1"
#define PORT 27015
#define win_width 640
#define win_height 360
#define display_unit_size (win_height / 6.5)


float cube_static[] = {-1.0, 1.0, 1.0, -1.0, 1.0,-1.0,  1.0, 1.0, 1.0,  1.0, 1.0,-1.0,
                        1.0,-1.0, 1.0,  1.0,-1.0,-1.0, -1.0,-1.0, 1.0, -1.0,-1.0,-1.0,
                        1.0,-1.0, 1.0, -1.0,-1.0, 1.0,  1.0, 1.0, 1.0, -1.0, 1.0, 1.0,
                        1.0, 1.0,-1.0, -1.0, 1.0,-1.0,  1.0,-1.0,-1.0, -1.0,-1.0,-1.0,
                        1.0, 1.0,-1.0,  1.0,-1.0,-1.0,  1.0, 1.0, 1.0,  1.0,-1.0, 1.0,
                       -1.0, 1.0, 1.0, -1.0,-1.0, 1.0, -1.0, 1.0,-1.0, -1.0,-1.0,-1.0};

#define boundsSz (sizeof(cube_static) / sizeof(float))
float bounds[boundsSz];
float global_UC[6];

struct ptElemSphere ptES = {0};

#define pt_cnt 5
#define radius_divider_pts (pt_cnt * 2)
#define pt_cntA (pt_cnt + radius_divider_pts)

float ptsA_static[pt_cntA][3];
float ptsA[pt_cntA][3];
float ptValsA[pt_cntA];

#define pt_cntB (pt_cnt + radius_divider_pts)

float ptsB_static[pt_cntB][3];
float ptsB[pt_cntB][3];
float ptValsB[pt_cntB];

float ptsB_UC[6];
float ucBuff_UC[6];

struct reorder_accnt_divider_node *xRADN = {0};
struct reorder_accnt_divider_node *yRADN = {0};
struct reorder_accnt_divider_node *zRADN = {0};


void rot(float *verts, int cnt, float *UC)
{
   int i;
   
   for (i=0; i < cnt; i+=3)
   ucRotVertex(&UC[0], &verts[i+0], &verts[i+1], &verts[i+2]);
}

void scale(float *verts, int cnt, float x, float y, float z)
{
   int i;
   
   for (i=0; i < cnt; i+=3)
   {
      verts[i+0] *= x;
      verts[i+1] *= y;
      verts[i+2] *= z;
   }
}

void move(float *verts, int cnt, float x, float y, float z)
{
   int i;
   
   for (i=0; i < cnt; i+=3)
   {
      verts[i+0] += x;
      verts[i+1] += y;
      verts[i+2] += z;
   }
}

void copy(float *vertsA, float *vertsB, int cnt)
{
   int i;
   
   for (i=0; i < cnt; i++)
   vertsB[i] = vertsA[i];
}


void drawScreen()
{
   int i;
   struct timeval tval;
   static unsigned long long msec = 0, msec_prior = 0;
   
   char black[3] = {0x0, 0x0, 0x0};
   char white[3] = {0xFF, 0xFF, 0xFF};
   
   // draw screen at time intervals
   gettimeofday(&tval, NULL);
   msec = tval.tv_sec*1000LL + tval.tv_usec/1000;
   
   if (msec - msec_prior >= 50)
   {
      setColor(white, 3);
      clearScreen();
      
      // draw bounds
      setColor(black, 3);
      copy(&cube_static[0], &bounds[0], boundsSz);
      scale(&bounds[0], boundsSz, display_unit_size*2, display_unit_size*2, display_unit_size*2);
      rot(&bounds[0], boundsSz, &global_UC[0]);
      drawLine(&bounds[0], boundsSz);
      
      // draw points
      copy((float *)&ptsA_static, (float *)&ptsA, pt_cntA*3);
      scale((float *)&ptsA, pt_cntA*3, display_unit_size, display_unit_size, display_unit_size);
      rot((float *)&ptsA, pt_cntA*3, &global_UC[0]);
      
      copy((float *)&ptsB_static, (float *)&ptsB, pt_cntB*3);
      scale((float *)&ptsB, pt_cntB*3, display_unit_size, display_unit_size, display_unit_size);
      
      memcpy(ucBuff_UC, ptsB_UC, sizeof(ptsB_UC));
      ucRotUc(&global_UC[0], &ucBuff_UC[0]);
      rot((float *)&ptsB, pt_cntB*3, &ucBuff_UC[0]);
      
      for (i=0; i < pt_cntA; i++)
      drawPtElementSphere(&ptES, ptsA[i][0], ptsA[i][1], ptsA[i][2], ptValsA[i]);

      for (i=0; i < pt_cntB; i++)
      drawPtElementSphere(&ptES, ptsB[i][0], ptsB[i][1], ptsB[i][2], ptValsB[i]);
      
      showContent();
      msec_prior = msec;
   }
}


void eventCallback(int evType, int keyCode, int xM, int yM)
{
   static int lMouseDwn = FALSE;
   static int rMouseDwn = FALSE;
   static int mMouseDwn = FALSE;
   static float xMouse = 0, yMouse = 0;
   static float xMousePrior = 0, yMousePrior = 0;
   static int updateGraphics = FALSE;
   
   // store mouse events
   if (evType == MOUSE_MOVE)
   {
      xMouse = xM - win_width/2.0;
      yMouse = -yM + win_height/2.0;
   }
   
   if (evType == L_MOUSE_DOWN)
   {
      lMouseDwn = TRUE;
      updateGraphics = TRUE;
   }
   
   if (evType == L_MOUSE_UP)
   {
      lMouseDwn = FALSE;
      updateGraphics = TRUE;
   }

   if (evType == R_MOUSE_DOWN)
   {
      rMouseDwn = TRUE;
      updateGraphics = TRUE;
   }
   
   if (evType == R_MOUSE_UP)
   {
      rMouseDwn = FALSE;
      updateGraphics = TRUE;
   }

   if (evType == M_MOUSE_DOWN)
   {
      mMouseDwn = TRUE;
      updateGraphics = TRUE;
   }
   
   if (evType == M_MOUSE_UP)
   {
      mMouseDwn = FALSE;
      updateGraphics = TRUE;
   }

   // press 'R' to refresh graphics
   if (keyCode == KC_R)
   updateGraphics = TRUE;
   
   
   if ((rMouseDwn) || (mMouseDwn))
   {
      trackBallRotUc(win_height/2, xMousePrior, yMousePrior, xMouse, yMouse, &global_UC[0]);
      drawScreen();
   }
   
   if (lMouseDwn)
   {
      ucRotUc(&global_UC[0], &ptsB_UC[0]);
      trackBallRotUc(win_height/2, xMousePrior, yMousePrior, xMouse, yMouse, &ptsB_UC[0]);
      ucRotUcUNDO(&global_UC[0], &ptsB_UC[0]);
      drawScreen();
   }
   
   if ((evType == L_MOUSE_UP) || (evType == R_MOUSE_UP) || (evType == M_MOUSE_UP))
   {
      copy((float *)&ptsA_static, (float *)&ptsA, pt_cntA*3);

      copy((float *)&ptsB_static, (float *)&ptsB, pt_cntB*3);
      rot((float *)&ptsB, pt_cntB*3, &ptsB_UC[0]);
      
      corr3d_createNodes(&xRADN, pt_cntA+pt_cntB);
      corr3d_createNodes(&yRADN, pt_cntA+pt_cntB);
      corr3d_createNodes(&zRADN, pt_cntA+pt_cntB);
      
      corr3d_copyData(AXIS_X, (float *)ptsA, ptValsA, pt_cntA, GROUP_A, xRADN);
      corr3d_copyData(AXIS_Y, (float *)ptsA, ptValsA, pt_cntA, GROUP_A, yRADN);
      corr3d_copyData(AXIS_Z, (float *)ptsA, ptValsA, pt_cntA, GROUP_A, zRADN);
      
      corr3d_copyData(AXIS_X, (float *)ptsB, ptValsB, pt_cntB, GROUP_B, &xRADN[pt_cntA]);
      corr3d_copyData(AXIS_Y, (float *)ptsB, ptValsB, pt_cntB, GROUP_B, &yRADN[pt_cntA]);
      corr3d_copyData(AXIS_Z, (float *)ptsB, ptValsB, pt_cntB, GROUP_B, &zRADN[pt_cntA]);
      
      corr3d_reorder(&xRADN);
      corr3d_reorder(&yRADN);
      corr3d_reorder(&zRADN);
      
      corr3d_account(xRADN);
      corr3d_account(yRADN);
      corr3d_account(zRADN);
      
      printSB("Correspondence Value: %f", corr3d_correspondenceVal(xRADN, yRADN, zRADN));

      corr3d_freeNodes(xRADN);
      corr3d_freeNodes(yRADN);
      corr3d_freeNodes(zRADN);
   }
   
   if (updateGraphics)
   {
      drawScreen();
      updateGraphics = FALSE;
   }
   
   
   if (evType == MOUSE_MOVE)
   {
      xMousePrior = xMouse;
      yMousePrior = yMouse;
   }
   
   if (keyCode == KC_ESC)
   unblockAvecisDisconnect();
}

int main(int argc, char **argv) 
{
   int i, j;
   
   setbuf(stdout, NULL);

   getPtElementSphere(&ptES, 24, 24, display_unit_size);
   getUc(0, 0, 0, &global_UC[0]);
   getUc(0, 0, 0, &ptsB_UC[0]);
   
   unsigned int seed = time(0);
   printf("seed: %u\n\n", seed);
   srand(seed);
   
   // create random points and values
   for (i=0; i < pt_cntA; i+=3)
   {
      ptValsA[i+1] = (double)rand() / RAND_MAX * 1.0;
      printf("%i) val:%f ", i/3, ptValsA[i+1]);
      ptValsA[i+0] = 0.0;
      ptValsA[i+2] = 0.0;

      for (j=0; j < 3; j++)
      {
         ptsA_static[i+1][j] = ((double)rand() / RAND_MAX * 2.0 - 1.0) * 1.0 / sqrt(3); // 1/sqrt(3) == max cube coor. within sphere
         ptsA_static[i+0][j] = ptsA_static[i+1][j] - ptValsA[i+1];
         ptsA_static[i+2][j] = ptsA_static[i+1][j] + ptValsA[i+1];
      }

      printf("x:%f y:%f z:%f\n", ptsA_static[i+0][0], ptsA_static[i+0][1], ptsA_static[i+0][2]);
      printf("\t\tx:%f y:%f z:%f null value \"radius point\"\n", ptsA_static[i+1][0], ptsA_static[i+1][1], ptsA_static[i+1][2]);
      printf("\t\tx:%f y:%f z:%f null value \"radius point\"\n", ptsA_static[i+2][0], ptsA_static[i+2][1], ptsA_static[i+2][2]);
   }
   
   for (i=0; i < pt_cntA; i++)
   for (j=0; j < 3; j++)
   ptsB_static[i][j] = ptsA_static[i][j];
   
   for (i=0; i < pt_cntA; i++)
   ptValsB[i] = ptValsA[i];

   
   if (avecisConnect(HOSTNAME, PORT))
   {
      freePtElementSphere(&ptES);
      return 1;
   }
   
   // initialize graphics variables
   viewStart(250);
   viewEnd(-250);
   setPerspective(375);
   orthographicMode(FALSE);
   fogMode(TRUE);
   fogStart(350);
   fogEnd(-120);
   antialiasingMode(TRUE);
   
   printSB("Correspondence Value: 1.0  Use mouse to view and change orientation.");
   
   setColor("\xFF\xFF\xFF", 3);
   fogColor();
   
   drawScreen();
   
   blockAvecisDisconnect();
   
   freePtElementSphere(&ptES);
   return 0;
}
