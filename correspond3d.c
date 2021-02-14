#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "correspond3d.h"

#define TRUE 1
#define FALSE 0

void corr3d_freeNodes(struct reorder_accnt_divider_node *RADN)
{
   if (RADN)
   free(RADN);
}

void corr3d_createNodes(struct reorder_accnt_divider_node **RADNODE, int nodeCnt)
{
   int i;
   struct reorder_accnt_divider_node *RADN;

   RADN = (struct reorder_accnt_divider_node *)malloc(nodeCnt * sizeof(struct reorder_accnt_divider_node));
   *RADNODE = RADN;

   RADN[0].prior = NULL;
   RADN[nodeCnt-1].next = NULL;
   
   for (i=1; i < nodeCnt; i++)
   RADN[i].prior = &RADN[i-1];
      
   for (i=0; i < nodeCnt-1; i++)
   RADN[i].next = &RADN[i+1];
}

void corr3d_copyData(char axis, float *coors, float *vals, int nodeCnt,
                     char group, struct reorder_accnt_divider_node *RADN)
{
   int i;

   // select axis
   if (axis == AXIS_Y)
   coors = &coors[1];
   
   if (axis == AXIS_Z)
   coors = &coors[2];

   for (i=0; i < nodeCnt; i++)
   {
      RADN[i].loc = coors[i*3]; // coors = {x, y, z, ...}
      RADN[i].value = vals[i];
      RADN[i].groupA_prop = 0;
      RADN[i].groupB_prop = 0;
      RADN[i].group = group;
   }
}

static void moveNodeTo(struct reorder_accnt_divider_node *nodeB, struct reorder_accnt_divider_node *nodeE)
{
   struct reorder_accnt_divider_node *nodeA = nodeB->prior;
   struct reorder_accnt_divider_node *nodeC = nodeB->next;
   struct reorder_accnt_divider_node *nodeD = nodeE->prior;
   
   // mend
   if (nodeA)
   nodeA->next = nodeC;

   if (nodeC)
   nodeC->prior = nodeA;

   // reattach
   nodeB->prior = nodeD;
   nodeB->next = nodeE;

   if (nodeD)
   nodeD->next = nodeB;

   nodeE->prior = nodeB;
}

void corr3d_reorder(struct reorder_accnt_divider_node **RADN)
{
   struct reorder_accnt_divider_node *currNode;
   struct reorder_accnt_divider_node *priorNode;
   struct reorder_accnt_divider_node *moveToNode;
   int move_node;
   
   // reorder nodes
   currNode = *RADN;
   while ((currNode = currNode->next))
   {
      move_node = FALSE;
      
      priorNode = currNode;
      while ((priorNode = priorNode->prior))
      {
         if (currNode->loc >= priorNode->loc)
         break;

         moveToNode = priorNode;
         move_node = TRUE;
      }

      if (move_node)
      moveNodeTo(currNode, moveToNode);
   }

   // find first node
   currNode = *RADN;
   while ((currNode = currNode->prior))
   {
      *RADN = currNode;
   }
}

// get the volume of a sphere segment
static double sphereSegVol(double R, double a, double b)
{
   return M_PI * (R * R * (b - a) - 1.0 / 3 * (b * b * b - a * a * a));
}

// get the volume of a sphere cap
static double sphereCapVol(double R, double a)
{
   double h = R - a;
   return M_PI * h * h / 3 * (3 * R - h);
}

static void accntBackwardsPropagationVal(struct reorder_accnt_divider_node *currNode,
                                         struct reorder_accnt_divider_node *aNode,
                                         struct reorder_accnt_divider_node *bNode)
{
   float R = currNode->value; // sphere radius
   float a = currNode->loc - bNode->loc; // "a" is closer to the center of the sphere than "b"
   float b = currNode->loc - aNode->loc;
   
   if (currNode->group == GROUP_A)
   {
      if (R > b)
      aNode->groupA_prop += sphereSegVol(R, a, b);
      else
      aNode->groupA_prop += sphereCapVol(R, a);
   }
   else if (currNode->group == GROUP_B)
   {
      if (R > b)
      aNode->groupB_prop += sphereSegVol(R, a, b);
      else
      aNode->groupB_prop += sphereCapVol(R, a);
   }
}

static void accntForwardsPropagationVal(struct reorder_accnt_divider_node *currNode,
                                        struct reorder_accnt_divider_node *aNode,
                                        struct reorder_accnt_divider_node *bNode)
{
   float R = currNode->value; // sphere radius
   float a = aNode->loc - currNode->loc; // "a" is closer to the center of the sphere than "b"
   float b = bNode->loc - currNode->loc;
   
   if (currNode->group == GROUP_A)
   {
      if (R > b)
      aNode->groupA_prop += sphereSegVol(R, a, b);
      else
      aNode->groupA_prop += sphereCapVol(R, a);
   }
   else if (currNode->group == GROUP_B)
   {
      if (R > b)
      aNode->groupB_prop += sphereSegVol(R, a, b);
      else
      aNode->groupB_prop += sphereCapVol(R, a);
   }
}

// slices spherical objects with volumes relating to points and values into
// a 3D grid of cuboids with corresponding values (propagation values)
void corr3d_account(struct reorder_accnt_divider_node *RADN)
{
   struct reorder_accnt_divider_node *currNode;
   struct reorder_accnt_divider_node *aNode, *bNode;
   float size;
   float distance;

   // find propagation values moving backwards following the axis
   currNode = RADN;
   while ((currNode = currNode->next))
   {
      distance = 0.0;
      size = currNode->value;
      
      bNode = currNode;
      while ((aNode = bNode->prior))
      {
         if (distance >= size)
         break;

         accntBackwardsPropagationVal(currNode, aNode, bNode);
         
         distance += bNode->loc - aNode->loc;
         bNode = aNode;
      }
   }

   // find propagation values moving forwards following the axis
   currNode = RADN;
   while ((currNode = currNode->next))
   {
      distance = 0.0;
      size = currNode->value;
      
      aNode = currNode;
      while ((bNode = aNode->next))
      {
         if (distance >= size)
         break;

         accntForwardsPropagationVal(currNode, aNode, bNode);
         
         distance += bNode->loc - aNode->loc;
         aNode = bNode;
      }
   }
}

// give an approximate correspondence value from 2 sets of values in a 3D grid of cuboids
float corr3d_correspondenceVal(struct reorder_accnt_divider_node *xRADN,
                               struct reorder_accnt_divider_node *yRADN,
                               struct reorder_accnt_divider_node *zRADN)
{
   float xNum = 0.0, xDen = 0.0; // numerator, denominator of the correspondence value for an axis
   float yNum = 0.0, yDen = 0.0;
   float zNum = 0.0, zDen = 0.0;
   
   while (xRADN)
   {
      xNum += xRADN->groupA_prop + xRADN->groupB_prop - fabs(xRADN->groupA_prop - xRADN->groupB_prop);
      xDen += xRADN->groupA_prop + xRADN->groupB_prop;
      xRADN = xRADN->next;
   }
   
   while (yRADN)
   {
      yNum += yRADN->groupA_prop + yRADN->groupB_prop - fabs(yRADN->groupA_prop - yRADN->groupB_prop);
      yDen += yRADN->groupA_prop + yRADN->groupB_prop;
      yRADN = yRADN->next;
   }
   
   while (zRADN)
   {
      zNum += zRADN->groupA_prop + zRADN->groupB_prop - fabs(zRADN->groupA_prop - zRADN->groupB_prop);
      zDen += zRADN->groupA_prop + zRADN->groupB_prop;
      zRADN = zRADN->next;
   }
   
   return (xNum / xDen) * (yNum / yDen) * (zNum / zDen);
}
