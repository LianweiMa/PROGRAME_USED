#include <stdio.h>
#include <string.h>
#include "CommonOP.hpp"
#include "CGdalDem.hpp"
#include "TypeDef.h"
float GetMosaicZvalue(double &lfX_i, double &lfY_i, CGdalDem*pDemList_i, int &iDemNum_i)//arithmetic weighted mean
{
	float fZ = NODATA, fSum = 0.0; int iNumZ = 0;
	for (int i = 0; i < iDemNum_i; i++)
	{
		fZ = pDemList_i[i].GetDemZValue(lfX_i, lfY_i);
		if (fZ != NODATA ){
			iNumZ++;
			fSum += fZ;
		}
	}
	if (iNumZ > 0)return fSum / iNumZ;
	else return NODATA;
}
int main(int argc, char**argv)
{
	if (!NotePrint(argv, argc, 2)){ printf("Argument: demlist_file\n"); return false; }
	//read dem file list
	char strInputfile[FILE_PN];strcpy(strInputfile, argv[1]);
	int iDemNum = 0;//nDemNum:the number of dem file
	FILE *fp = fopen(strInputfile, "r"); if (!fp){ printf("Error_FileOpen:%s\n", strInputfile); return false; }
	fscanf(fp, "%d\n", &iDemNum);
	char **pDemfile = new char *[iDemNum];
	for (int i = 0; i < iDemNum; i++)
	{
		pDemfile[i] = new char[CHAR_LEN];
		fscanf(fp, "%s\n", pDemfile[i]);
	}
	fclose(fp);

	//read dem//print dem info
	printf("\n-----Dem info-----\n");
	CGdalDem *pDemList = new CGdalDem[iDemNum]; if (!pDemList)return false;
	double lfXmin = NODATA, lfXmax = NODATA, lfYmin = NODATA, lfYmax = NODATA;
	double lfXgsd = 0.0, lfYgsd = 0.0;
	for (int i = 0; i < iDemNum; i++)
	{
		if (!pDemList[i].LoadFile(pDemfile[i]))
		{
			printf("Error_FileOpen:%s\n", pDemfile[i]);
			return false;
		}
		printf("--Dem %d--:%s\n", i + 1, pDemfile[i]);
		GDALDEMHDR pDemListHdr = pDemList[i].GetDemHeader();
		printf("Xgsd:%lf\tYgsd:%lf\n", pDemListHdr.lfGsdX, pDemListHdr.lfGsdY);
		printf("Colum:%d\tRow:%d\n", pDemListHdr.iCol, pDemListHdr.iRow);
		if (lfXgsd == 0.0)lfXgsd = pDemListHdr.lfGsdX;
		if (lfYgsd == 0.0)lfYgsd = pDemListHdr.lfGsdY;
		if (lfXgsd < pDemListHdr.lfGsdX)lfXgsd = pDemListHdr.lfGsdX;
		if (lfYgsd < pDemListHdr.lfGsdY)lfYgsd = pDemListHdr.lfGsdY;
		double lfZmin = NODATA, lfZmax = NODATA;
		lfZmax = pDemList[i].GetMaxZ(); lfZmin = pDemList[i].GetMinZ();
		printf("Zmin:%lf\tZmax:%lf\n", lfZmin, lfZmax);
		double lfX[4], lfY[4];
		pDemList[i].GetDemRegion(lfX, lfY);
		printf("GeoRange:\n\tLB-[%lf,%lf]\n\tRB-[%lf,%lf]\n\tRT-[%lf,%lf]\n\tLT-[%lf,%lf]\n", lfX[0], lfY[0], lfX[1], lfY[1], lfX[2], lfY[2], lfX[3], lfY[3]);
		if (lfXmin == NODATA) lfXmin = lfX[0];
		if (lfXmax == NODATA) lfXmax = lfX[0];
		if (lfYmin == NODATA) lfYmin = lfY[0];
		if (lfYmax == NODATA) lfYmax = lfY[0];
		for (int i = 0; i < 4; i++)
		{
			if (lfXmin > lfX[i]) lfXmin = lfX[i];
			if (lfXmax < lfX[i]) lfXmax = lfX[i];
			if (lfYmin > lfY[i]) lfYmin = lfY[i];
			if (lfYmax < lfY[i]) lfYmax = lfY[i];
		}
	}

	//Mosaic dem
	printf("\n-----Mosaic dem-----\n");
	int iCols = 0, iRows = 0;
	iCols = int((lfXmax - lfXmin) / lfXgsd) + 1;
	iRows = int((lfYmax - lfYmin) / lfYgsd) + 1;
	printf("Rows:%d\tCols:%d\n", iRows, iCols);

	printf("\n-----Save mosaic Dem-----\n");
	CGdalDem *pMosaicDemFile = new CGdalDem; GDALDEMHDR *pMosaicDemHead = new GDALDEMHDR;
	pMosaicDemHead->lfStartX = lfXmin; pMosaicDemHead->lfStartY = lfYmin;
	pMosaicDemHead->lfGsdX = lfXgsd; pMosaicDemHead->lfGsdY = lfYgsd;
	pMosaicDemHead->iCol = iCols;   pMosaicDemHead->iRow = iRows;
	char strOutputfile[512]; strcpy(strOutputfile, strInputfile);
	char *pS = strrchr(strOutputfile, '/'); if (!pS)pS = strrchr(strOutputfile, '\\');
	sprintf(pS, "%s", "\\Dem_Mosaic.tif");
	if (true != pMosaicDemFile->CreatFile(strOutputfile, pMosaicDemHead))return false;

	//Block processing
	int iStep_Rows = BlOCK_MEMERY / (iCols*sizeof(float));
	int iBlock = iRows / iStep_Rows;
	if (iRows%iStep_Rows)iBlock++;
	float *pMosaicDemData = new float[iStep_Rows*iCols];
	printf("Block processing: %d blocks in total.\n",iBlock);
	for (int k = 0; k < iBlock; k++)
	{
		double x = lfXmin, y = lfYmax - k*(iStep_Rows*lfYgsd);
		int iHeight = iStep_Rows;
		if (iStep_Rows*(k + 1) > iRows)iHeight = iRows - iStep_Rows*k;
		for (int i = 0; i < iHeight; i++)
		{
			x = lfXmin;
			for (int j = 0; j < iCols; j++)
			{
				*(pMosaicDemData + i*iCols + j) = GetMosaicZvalue(x, y, pDemList, iDemNum);
				x += lfXgsd;
			}
			y -= lfYgsd;
		}
		pMosaicDemFile->WriteBlock(pMosaicDemData, 0, k*iStep_Rows, iCols, iHeight);
		printf("%d / %d blocks finished!\n", k + 1, iBlock);
	}
	printf("Saved mosaic Dem file: %s.\n", strOutputfile);

	//delete memory
	delete pMosaicDemFile, pMosaicDemHead;
	delete[]pMosaicDemData;
	delete[]pDemList;
	for (int i = 0; i < iDemNum; i++)
	{
		delete[]pDemfile[i];
	}
	delete[]pDemfile;
	return true;
}