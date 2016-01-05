/* !!! 注意, 为了实现参数配置的兼容(版本的兼容性): 要往 SYS_CONFIG 结构体里面添加成员, 
	一定要修改下面成员:
0. #define		INDEX_PARAM_CONFIG_TOTAL ***
1. static void SysConfigInitMemberAddr()
2. SysConfigpRestoreDefaultParam();
3. static void SysConfigInitStoreHead( PARAM_CONFIG_STORE_HEAD *pStoreHead )
4. int GetParamConfig( int index, char *pBuf, int len, int n)
5. int SetParamConfig( int index, char *pBuf, int len, int n)
6. paramManage.cpp 和 paramManage.h
*/