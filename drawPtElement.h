struct ptElemSphere
{
   float *staticMeshData;
   float *meshData;
   int meshDataSz;
   float *shadeData;
   int shadeDataSz;
};

void freePtElementSphere(struct ptElemSphere *);
void drawPtElementSphere(struct ptElemSphere *, float, float, float, float);
void getPtElementSphere(struct ptElemSphere *, int, int, float);
