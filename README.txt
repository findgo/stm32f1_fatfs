此为STM32F103模板
实现串口FIFO 的DEBUG功能
其中COM0做为调试口
实现DEBUG功能控制功能

移植FATFS框架（fatfs版本0.12c）， 供给外部接口
为其它工程做准备 ，做到模块化


FATFS移植记录
源代码下载地址：
http://elm-chan.org/fsw/ff/00index_e.html

本移植记录对应的版本为 R0.12C

1. 直接下载 ff12c.zip 文件，然后解压，将其复制到自己的工程中。
		将其存放在： lib\FatFS\src

2. 在工程中添加2个源代码文件：
	ff.c
	diskio.c
	cc936.c

4. 主要就修改 diskio.c 文件，实现SD卡驱动

5. 为了支持长文件名，必须包含 cc936.c 文件，其中 static const WCHAR uni2oem[] 常量数组
	是一个GB2312码和UNICODE代码的转换表，会占用程序空间约170K字节。
	
==========================================
【ffconf.h 修改记录】
（1）#define _CODE_PAGE	932		/* 这是日文编码 */
		修改为：
	#define _CODE_PAGE	936		/* 这是简体中文编码 */

（2）#define	_FS_LOCK	0	修改 3，  可以同时打开3个文件，0表示只能打开1个。

（3）#define	_USE_LFN	0		/* 0 to 3 */
	修改为
	#define	_USE_LFN	1   使能长文件名，分配静态内存用于存储长文件名
	1: Enable LFN with static working buffer on the BSS. Always NOT reentrant.

（4）在末尾增加 #include "diskio.h"。 主要目的是声明多个磁盘系统的ID（FS_SD和FS_NAND)


【diskio.c 修改记录】
1. 修改 disk_initialize, 添加SD卡初始化
2. 修改 disk_status, 返回SD卡状态
3. 修改 disk_read，实现读SD卡扇区
4. 修改 disk_write，实现写SD卡扇区
5. 修改 disk_ioctl， 重要实现，主要是获取SD容量，扇区等信息
6. 添加函数 ： DWORD get_fattime (void); 放在diskio.c下，返回0

【diskio.h 修改记录】
1. 文件末尾增加物理磁盘代码
	/* Definitions of physical drive number for each media */
	#define FS_SD		0
	#define FS_NAND		1
	
SD的API函数实现
#define MMC_disk_status()     SD_status()
#define MMC_disk_initialize() SD_Init()
#define MMC_disk_read(buff, sector, count) SD_ReadMultiBlock(sector,buff, count)
#define MMC_disk_write(buff, sector, count) SD_WriteMultiBlock(sector,buff, count)
#define MMC_disk_ioctl(cmd,buff) SD_Ioctl(cmd,buff)


【长文件名问题】
1. 部分文件的属性中只有短文件名 FileInf.fname ，长文件名域 FileInf.lfname 为空
2. ff_mkdir 创建目录时，无论文件名长短，均可以正确读取到长文件名FileInf.lfname
3. f_open 创建文件时，如果文件名长度大于 8.3格式名，则会正确填充FileInf.lfname

【SD_WriteMultiBlocks()函数问题】
1. 在 disk_write 中，如果待写入的 block大于1，可以选择 SD_WriteMultiBlocks（）函数加速
	但是最后一个形参需要是 count + 1 才能正确写入，否则最后1个扇区的数据将不正确。
【原因待查】	
	/* 此处存在疑问： 扇区个数如果写 count ，将导致最后1个block无法写入 */
	Status = SD_WriteMultiBlocks((uint8_t *)buff, sector << 9 ,SECTOR_SIZE, count + 1);
