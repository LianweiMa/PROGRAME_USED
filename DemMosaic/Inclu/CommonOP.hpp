////common_operation.hpp
/********************************************************************
common operation
created:	2015/09/07
author:		mlw
purpose:	This file is for common operation function
*********************************************************************/
#ifndef _WHU_COMMON_OPERATION_MLW_20150907
#define _WHU_COMMON_OPERATION_MLW_20150907
template <typename T1, typename T2>
inline void Memset(T1 *pData, T2 val, int nums){ for (int i = 0; i < nums; i++) *(pData + i) = (T1)val; }
template <typename T>
inline void Memcpy(T *pDestData, T *pSrcData, int nums){ for (int i = 0; i < nums; i++) *(pDestData + i) = *(pSrcData + i); }

/************************************************************************/
/*    NotePrint                                                         */
/*    argc:输入的参数个数                                                */
/*    iArgumentNumbers:必须的参数个数                                    */
/************************************************************************/
bool NotePrint(char**argv, int &argc, int iNeededArgumentNumbers) {
	if (argc != iNeededArgumentNumbers){
		{printf("Wrong argument!\n"); return false; }
	}
	else{
		for (int i = 1; i < argc; i++){
			printf("argv[%d]: %s\n", i, argv[i]);
		}
	}
	return true;
}

/************************************************************************/
/*    Sort                                                              */
/*    pList:系列值                                                      */
/*    pTifFile:值个数                                                   */
/************************************************************************/
template <typename T>
void Sort(T *pList, int size) { T t; for (int m = 0; m < size; m++){ for (int n = m + 1; n < size; n++){ if (pList[m] < pList[n]){ t = pList[n]; pList[n] = pList[m]; pList[m] = t; } } } }
#endif