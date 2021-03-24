#ifndef __GETPIDSSFROMUSEDFILE_H__
#define __GETPIDSSFROMUSEDFILE_H__
#include <Windows.h>

#define E_GET_PIDS_FROM_FILE_CANNOT_LOAD_RSTRMGR_DLL -1
#define E_GET_PIDS_FROM_FILE_CANNOT_FIND_INTERFACE_FROM_RSTRMGR_DLL -2
#define E_GET_PIDS_FROM_FILE_WIN32_ERROR -3

#ifdef __cplusplus
extern "C" {
#endif

	//获取文件占用的进程id，如果返回 < 0则表示发回错误，否则代表占用进程数
	int GetPidsFromUsedFile(PCWSTR path, DWORD** pids);
	void FreePidsFromUsedFile(DWORD* pids);

#ifdef __cplusplus
};
#endif
#endif
