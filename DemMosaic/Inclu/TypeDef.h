//about file name length
#define FILE_PN	512
#define CHAR_LEN	512
#define MAX_WKT_LENGTH_MLW	512

//Block processing
#define BLOCK_HEIGHT_MLW	1024
#define BLOCK_WIDTH_MLW		1024
#define	BlOCK_MEMERY	10*1024*1024//Units:byte
//LAS
#define PT_NUM		1000000
//Dem
#define NODATA	-32767
#define MARK_VALUE_MLW	32767
#define SRTM_AP	120//Absolute precision
#define SRTM_RP 10//Relative precision
//image
#define BG_COLOUR	0
//Data type
typedef  unsigned char BYTE;
//typedef	unsigned short	WORD;
//typedef unsigned int	DWORD;
typedef const char *	LPCSTR;
//bool
//typedef bool BOOL;
//#define FALSE	0
typedef unsigned char MSKMETHOD;
typedef unsigned char PREMETHOD;
//geo
#define  PI 3.141592653
#define  R 6371393
#define L_UNIT 111000
#define SEMIMAJOR_WGS84	        6378137.0	
#define SEMIMINOR_WGS84		    6356752.31424517929
#define R2D                57.295779513082320876798154814105e0
#define D2R                0.017453292519943295769236907684886e0
//中国大陆经纬度范围
#define MIN_LOGITUDE 72
#define MAX_LOGITUDE 135
#define  MIN_LATITUDE 17
#define  MAX_LATITUDE 52
//log
#define ERROR_FILE_OPEN "ERROE_FILE_OPEN"
#define ERROR_FILE_READ "ERROR_FILE_READ"
#define ERROR_FILE_WRITE "ERROR_FILE_WRITE"
#define ERROR_GET_DRIVER "ERROR_GET_DRIVER"
#define ERROE_FILE_NO_EXIST "ERROE_FILE_NO_EXIST"