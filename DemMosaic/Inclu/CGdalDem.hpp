#ifndef _GDAL_DIGITAL_ELEVATION_MODELS_OPERATION_20150605
#define _GDAL_DIGITAL_ELEVATION_MODELS_OPERATION_20150605
#pragma warning(disable:4251)
#include "TypeDef.h"
#include "GDAL_1.10.1/gdal_priv.h"
#define SORTF( pList,size ) { float t; for ( int m=0;m<size;m++ ){ for ( int n=m+1;n<size;n++ ){ if (pList[m]<pList[n]){ t=pList[n]; pList[n]=pList[m];pList[m]=t; } } } }

#ifndef _GDAL_DEM_HEADER
#define _GDAL_DEM_HEADER
typedef struct tagGDALDEMHEADER{
	double	lfStartX, lfStartY;
	double	lfGsdX, lfGsdY;
	int		iCol, iRow;
	char strProj[CHAR_LEN];
}GDALDEMHDR, *LPGDALDEMHDR;
#endif

class CGdalDem{
public:
	CGdalDem(){
		CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO"); GDALAllRegister();
		m_pDataSet = NULL;
		m_MeanZ = m_MaxZ = m_MinZ = NODATA;
	};
	~CGdalDem(){
		GDALClose(m_pDataSet);
	};
public:
	enum OPENFLAGS { eModeRead = 0x0000, eModeCreate = 0x0001 };
public:
	//file open
	bool LoadFile(char* strFileName_i, GDALDEMHDR *pDemHeader_o = NULL){
		m_pDataSet = (GDALDataset*)GDALOpen(strFileName_i, GA_ReadOnly);
		m_DemHeader = GetDemHeader();
		if (NULL != pDemHeader_o)memcpy(pDemHeader_o, &m_DemHeader, sizeof(GDALDEMHDR));

		if (NULL == m_pDataSet){ printf("ERROR_FILE_OPEN: %s\n", strFileName_i); return false; }
		return true;
	}
	//file creat
	bool CreatFile(char* strFileName_i, GDALDEMHDR *pDemHeader_i){
		SetDemHdr(pDemHeader_i);
		GDALDriver *pDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
		if (!pDriver) { printf("ERROR_GDAL_DRIVER:%s\n", "GDALDriver *pDriver = GetGDALDriverManager()->GetDriverByName(""GTiff"")"); return false; }
		m_pDataSet = pDriver->Create(strFileName_i, pDemHeader_i->iCol, pDemHeader_i->iRow, 1, GDT_Float32, NULL);
		if (NULL == m_pDataSet){ printf("ERROR_FILE_CREATE: %s\n", strFileName_i); return false; }

		double lfGeoTransform[6] = { pDemHeader_i->lfStartX, pDemHeader_i->lfGsdX, 
			0, pDemHeader_i->lfStartY + pDemHeader_i->lfGsdY*pDemHeader_i->iRow, 
			0, -pDemHeader_i->lfGsdY };
		m_pDataSet->SetGeoTransform(lfGeoTransform);
		//if (pDemHeader_i->strProj[0]!='\0')m_pDataSet->SetProjection(pDemHeader_i->strProj);
		return true;
	}
	void CloseFile(){ m_pDataSet->FlushCache(); GDALClose(m_pDataSet); }
public:
	//read block
	bool ReadBlock(float *pBuf, int iStartCol, int iStartiRow, int iBlockWidth, int iBlockHeight){
		m_pDataSet->RasterIO(GF_Read, iStartCol, iStartiRow, iBlockWidth, iBlockHeight, pBuf, iBlockWidth, iBlockHeight, GDT_Float32, 1, 0, 0, 0, 0);
		return true;
	}
	//write block
	bool WriteBlock(float *pBuf, int iStartCol, int iStartiRow, int iBlockWidth, int iBlockHeight){
		m_pDataSet->RasterIO(GF_Write, iStartCol, iStartiRow, iBlockWidth, iBlockHeight, pBuf, iBlockWidth, iBlockHeight, GDT_Float32, 1, 0, 0, 0, 0);
		m_pDataSet->FlushCache();
		return true;
	}
	//release memory
	void ReleaseBlockMemory(float *pBuf){ delete[]pBuf; }

	inline void MedianFilter(){
		float *pZ, grid[32], *pZN0 = new float[m_DemHeader.iCol*m_DemHeader.iRow];
		ReadBlock(pZN0, 0, 0, m_DemHeader.iCol, m_DemHeader.iRow);
		int r, c, i;
		for (r = 2; r < m_DemHeader.iRow - 2; r++){
			for (c = 2; c < m_DemHeader.iCol - 2; c++){
				pZ = pZN0 + r*m_DemHeader.iCol + c; if (*pZ == NODATA) continue;

				pZ = pZN0 + (r - 2)*m_DemHeader.iCol + c - 2;
				grid[0] = *pZ++; grid[1] = *pZ++; grid[2] = *pZ++; grid[3] = *pZ++; grid[4] = *pZ++;
				pZ = pZN0 + (r - 1)*m_DemHeader.iCol + c - 2;
				grid[5] = *pZ++; grid[6] = *pZ++; grid[7] = *pZ++; grid[8] = *pZ++; grid[9] = *pZ++;
				pZ = pZN0 + (r)*m_DemHeader.iCol + c - 2;
				grid[10] = *pZ++; grid[11] = *pZ++; grid[12] = *pZ++; grid[13] = *pZ++; grid[14] = *pZ++;
				pZ = pZN0 + (r + 1)*m_DemHeader.iCol + c - 2;
				grid[15] = *pZ++; grid[16] = *pZ++; grid[17] = *pZ++; grid[18] = *pZ++; grid[19] = *pZ++;
				pZ = pZN0 + (r + 2)*m_DemHeader.iCol + c - 2;
				grid[20] = *pZ++; grid[21] = *pZ++; grid[22] = *pZ++; grid[23] = *pZ++; grid[24] = *pZ++;

				SORTF(grid, 25); for (i = 0; i < 25; i++){ if (grid[i] == NODATA) break; }
				WriteBlock(grid + i / 2, c, r, 1, 1);
			}
		}
		delete []pZN0;
	};

	//set info
	inline	bool	SetDemHdr(GDALDEMHDR*pDemHeader_i){ m_DemHeader = *pDemHeader_i; return true; }
	inline bool		SetGeoTransform(double*lfGeoTransform){ m_pDataSet->SetGeoTransform(lfGeoTransform); return true; }
	inline	bool	SetDemProj(const char*strProjection){ m_pDataSet->SetProjection(strProjection); return true; }
	inline	bool	SetDemNoDataValue(double lfNewNoDataValue){ m_pDataSet->GetRasterBand(1)->SetNoDataValue(lfNewNoDataValue); return true; }
	//get info
	inline	GDALDEMHDR&	GetDemHeader()	{
		//m_DemHeader
		m_DemHeader.iCol = m_pDataSet->GetRasterXSize();
		m_DemHeader.iRow = m_pDataSet->GetRasterYSize();
		double lfGeoTransform[6] = { 0.0 };
		m_pDataSet->GetGeoTransform(lfGeoTransform);
		m_DemHeader.lfGsdX = lfGeoTransform[1];
		m_DemHeader.lfGsdY = -lfGeoTransform[5];
		m_DemHeader.lfStartX = lfGeoTransform[0];
		m_DemHeader.lfStartY = lfGeoTransform[3] - m_DemHeader.lfGsdY * m_DemHeader.iRow;
		strcpy(m_DemHeader.strProj, m_pDataSet->GetProjectionRef());
		return m_DemHeader;
	}
	inline int GetCols(){ return m_DemHeader.iCol; }
	inline int GetRows(){ return m_DemHeader.iRow; }
	inline const char *GetDemProj(){ return m_pDataSet->GetProjectionRef(); }
	inline  float   GetMeanZ(){ if (m_MeanZ == NODATA)GetDemStats(); return m_MeanZ; }
	inline  float   GetMinZ(){ if (m_MinZ == NODATA)GetDemStats(); return m_MinZ; }
	inline  float   GetMaxZ(){ if (m_MaxZ == NODATA)GetDemStats(); return m_MaxZ; }
	inline	bool	GetDemStats(){
		//meanZ,minZ,maxZ
		float meanZ = 0.0f; int numZ = 0;
		for (int i = 0; i < m_DemHeader.iRow; i += BLOCK_HEIGHT_MLW)
		{
			for (int j = 0; j < m_DemHeader.iCol; j += BLOCK_WIDTH_MLW)
			{
				int Block_Height = 0, Block_Width = 0;
				if (i + BLOCK_HEIGHT_MLW <= m_DemHeader.iRow)
				{
					if (j + BLOCK_WIDTH_MLW <= m_DemHeader.iCol)
					{
						Block_Height = BLOCK_HEIGHT_MLW;
						Block_Width = BLOCK_WIDTH_MLW;
					}
					else
					{
						Block_Height = BLOCK_HEIGHT_MLW;
						Block_Width = (m_DemHeader.iCol - j);
					}
				}
				else
				{
					if (j + BLOCK_WIDTH_MLW <= m_DemHeader.iCol)
					{
						Block_Height = (m_DemHeader.iRow - i);
						Block_Width = BLOCK_WIDTH_MLW;
					}
					else
					{
						Block_Height = (m_DemHeader.iRow - i);
						Block_Width = (m_DemHeader.iCol - j);
					}
				}
				float *pData = new float[Block_Height*Block_Width];
				memset(pData, 0, sizeof(float)*Block_Height*Block_Width);
				ReadBlock(pData, j, i, Block_Width, Block_Height);
				for (int i_Block = 0; i_Block < Block_Height; i_Block++)
				{
					for (int j_Block = 0; j_Block < Block_Width; j_Block++)
					{
						if (*(pData + i_Block*Block_Width + j_Block) <= NODATA)continue;
						if (m_MinZ == NODATA)m_MinZ = *(pData + i_Block*Block_Width + j_Block);
						if (m_MaxZ == NODATA)m_MaxZ = *(pData + i_Block*Block_Width + j_Block);
						if (m_MinZ>*(pData + i_Block*Block_Width + j_Block))m_MinZ = *(pData + i_Block*Block_Width + j_Block);
						if (m_MaxZ < *(pData + i_Block*Block_Width + j_Block))m_MaxZ = *(pData + i_Block*Block_Width + j_Block);
						if (*(pData + i_Block*Block_Width + j_Block) != NODATA){ meanZ += *(pData + i_Block*Block_Width + j_Block); numZ++; }
					}
				}
				delete[]pData;
			}
		}
		if (numZ)m_MeanZ = meanZ / numZ;
		else m_MeanZ = NODATA;

		return true;
	}
	inline  bool    GetDemRegion(double x[], double y[]){//LB-RB-RT-LT
		GDALDEMHDR demHdr = m_DemHeader;
		double tempX = 0 * demHdr.lfGsdX, tempY = 0 * demHdr.lfGsdY;
		x[0] = tempX + demHdr.lfStartX;  y[0] = tempY + demHdr.lfStartY;//LB

		tempX = (demHdr.iCol - 1)*demHdr.lfGsdX; tempY = 0 * demHdr.lfGsdY;
		x[1] = tempX + demHdr.lfStartX;  y[1] = tempY + demHdr.lfStartY;//RB

		tempX = (demHdr.iCol - 1)*demHdr.lfGsdX; tempY = (demHdr.iRow - 1)*demHdr.lfGsdY;
		x[2] = tempX + demHdr.lfStartX;  y[2] = tempY + demHdr.lfStartY;//RT

		tempX = 0 * demHdr.lfGsdX; tempY = (demHdr.iRow - 1)*demHdr.lfGsdY;
		x[3] = tempX + demHdr.lfStartX;  y[3] = tempY + demHdr.lfStartY;//LT
		return true;
	};
	inline	float	GetDemZValue(double x, double y, bool bLi = true){
		double lfGeoTransform[6] = { 0.0 };
		m_pDataSet->GetGeoTransform(lfGeoTransform);
		double dx = (x - lfGeoTransform[0]);
		double dy = (lfGeoTransform[3] - y);

		x = dx / m_DemHeader.lfGsdX;
		y = dy / m_DemHeader.lfGsdY;

		int lbGridx = int(x);	dx = (x - lbGridx);
		int lbGridy = int(y);	dy = (y - lbGridy);

		if (lbGridx < 0 || lbGridx >= m_DemHeader.iCol - 1 ||
			lbGridy < 0 || lbGridy >= m_DemHeader.iRow - 1) return(NODATA);

		int lbOffset = lbGridy * m_DemHeader.iCol + lbGridx;
		int ltOffset = lbOffset + m_DemHeader.iCol;

		float z00, z01, z10, z11;
		m_pDataSet->RasterIO(GF_Read, lbGridx, lbGridy, 1, 1, &z00, 1, 1, GDT_Float32, 1, 0, 0, 0, 0);
		m_pDataSet->RasterIO(GF_Read, lbGridx + 1, lbGridy, 1, 1, &z01, 1, 1, GDT_Float32, 1, 0, 0, 0, 0);
		m_pDataSet->RasterIO(GF_Read, lbGridx, lbGridy + 1, 1, 1, &z10, 1, 1, GDT_Float32, 1, 0, 0, 0, 0);
		m_pDataSet->RasterIO(GF_Read, lbGridx + 1, lbGridy + 1, 1, 1, &z11, 1, 1, GDT_Float32, 1, 0, 0, 0, 0);

		if (z00 == NODATA || z01 == NODATA ||
			z10 == NODATA || z11 == NODATA){
			if (z00 != NODATA) return (float)z00; if (z01 != NODATA) return (float)z01;
			if (z10 != NODATA) return (float)z10; if (z11 != NODATA) return (float)z11;
			return NODATA;
		}

		z00 += (float)dx*(z01 - z00);	/* px0 */
		z10 += (float)dx*(z11 - z10);	/* px1 */
		if (z00 == z10) return z00;
		return float(z00 + dy*(z10 - z00));
	};
	inline	float	GetDemZValue(int iCol, int iRow){
		float z = NODATA;
		if (iCol<0 || iRow<0 || iCol>GetDemHeader().iCol - 1 || iRow>GetDemHeader().iRow - 1) return NODATA;
		m_pDataSet->RasterIO(GF_Read, iCol, iRow, 1, 1, &z, 1, 1, GDT_Float32, 1, 0, 0, 0, 0);
		return z;
	};
	inline	double	GetDemNoDataValue(){ return m_pDataSet->GetRasterBand(1)->GetNoDataValue(); }
	inline void GetXYZValue(int &iColID_i, int &iRowID_i, double&lfX_o, double&lfY_o, float&fZ_o){
		lfX_o = m_DemHeader.lfStartX + m_DemHeader.lfGsdX*iColID_i;
		lfY_o = m_DemHeader.lfStartY + m_DemHeader.lfGsdY*(m_DemHeader.iRow - iRowID_i);
		fZ_o = GetDemZValue(iColID_i, iRowID_i);
	}
	inline	char**	GetFileName(){
		return m_pDataSet->GetFileList();
	}
private:
	GDALDataset *m_pDataSet;
	GDALDEMHDR m_DemHeader;
	float m_MeanZ, m_MinZ, m_MaxZ;
};
#endif