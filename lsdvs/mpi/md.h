#ifndef __MD_H__
#define __MD_H__

#define MAX_MACROCELL_NUM		1620     // = (720*576)/(16*16) = D1的像素点/一个宏块的像素点
#define MOVE_FRAME_INTERVAL		5

/* 视频移动设置 */
typedef struct _MdArea_
{
	ushort	area[12];
} MD_AREA_T;

typedef struct _Md_Attr_
{
	unsigned int    macroThreshold;     /* 宏块阈值 */
	unsigned int 	macroRatio;         /* 区域中报警宏块比例 */
	unsigned char	frameCount;         // 累计有多少帧检测到移动侦测
	unsigned char	wBlock;             // 宽度总共有多少个模块(16 个像素一个模块)
	unsigned char	hBlock;             // 高度总共有多少个模块
	unsigned char	mask[MAX_MACROCELL_NUM];   /* 单个移动侦测区域对应的宏块号 */
	unsigned int	sadSum[MAX_MACROCELL_NUM];
} MD_ATTR_T;

void StartMdThread();
void StopMdThread();
void MdSendMsgChangeParam();

#endif 

