#ifndef __TEST_H__
#define __TEST_H__

// Õâ¸öºêÀïÃæ°üº¬µÄ¶«Î÷Ã»ÓÐ±àÒë½ø¹¤³Ì,
// °ÑÒ»Ð©½á¹¹Ìå·ÅÔÚÕâÀï½ö½öÎªÁË·½±ã²é¿´
#ifdef __OSCARDEBUG__
struct statfs {
                 long    f_type;     /* type of filesystem (see below) */
                 long    f_bsize;    /* optimal transfer block size */
                 long    f_blocks;   /* total data blocks in file system */
                 long    f_bfree;    /* free blocks in fs */
                 long    f_bavail;   /* free blocks avail to non-superuser */
                 long    f_files;    /* total file nodes in file system */
                 long    f_ffree;    /* free file nodes in fs */
                 fsid_t  f_fsid;     /* file system id */
                 long    f_namelen;  /* maximum length of filenames */
              };

struct __dirstream {
    int fd;            /* File descriptor.  */
    char * data;        /* Directory block.  */
    size_t allocation;        /* Space allocated for the block.  */
    size_t size;        /* Total valid data in the block.  */
    size_t offset;        /* Current offset into the block.  */
    off_t filepos;        /* Position of next entry to read.  */
    pthread_mutex_t lock;    /* Mutex lock for this structure.  */
};

typedef struct __dirstream DIR;


struct stat {
	unsigned long  st_dev;
	unsigned long  st_ino;
	unsigned short st_mode;
	unsigned short st_nlink;
	unsigned short st_uid;
	unsigned short st_gid;
	unsigned long  st_rdev;
	unsigned long  st_size;
	unsigned long  st_blksize;
	unsigned long  st_blocks;
	unsigned long  st_atime;
	unsigned long  st_atime_nsec;
	unsigned long  st_mtime;
	unsigned long  st_mtime_nsec;
	unsigned long  st_ctime;
	unsigned long  st_ctime_nsec;
	unsigned long  __unused4;
	unsigned long  __unused5;
};

struct dirent
  {
#ifndef __USE_FILE_OFFSET64
    __ino_t d_ino;
    __off_t d_off;
#else
    __ino64_t d_ino;
    __off64_t d_off;
#endif
    unsigned short int d_reclen;
    unsigned char d_type;
    char d_name[256];        /* We must not include limits.h! */
  };


#endif

// ²âÊÔ·þÎñ
void TestServiceStart();   
void TestServiceStop();

// epoll 测试线程
void StartTestEpollwThread();
void StopTestEpollwThread();

// mutexw 测试线程
void StartTestMutexwThread();
void StopTestMutexwThread();

// 侧试定时器
void TestTimerAddTestTimerToTimer();

// 测试哈希表
void *HashTestGetVal( char *key );

// xml
void TestXmlMakeTermRegisterPack();
void TestXmlMakeTermRegisterParse();
void TestXmlMakeTermRegisterParseLoadHead();

// test netStm.cpp
void TestStreamGet();
void TestStreamSend();

// test rand
void TestRandAb();

// test procon
void TestProcon();
void TestPcp();

// test rtc
void TestRtcGetTime();

//test enc set param
void TestEncSetResolution();

// test proconJpg
void TestProconJpg();
void TestFtpJpgParam();

// test proconH264Read
void TestProconH264Read();

void TestMympi();

void TestSnap();
void TestSnapSearch();
void FiRecTestInquireRecord();

void TestIpcHttpSnap();

void TestReadMtd1();


void TestTw2867();

#endif //__TEST_H__

