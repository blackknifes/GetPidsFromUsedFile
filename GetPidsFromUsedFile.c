#include "GetPidsFromUsedFile.h"

#include <RestartManager.h>

typedef DWORD
(WINAPI* PFN_RmStartSession)(
	_Out_ DWORD* pSessionHandle,
	_Reserved_ DWORD dwSessionFlags,
	_Out_writes_(CCH_RM_SESSION_KEY + 1) WCHAR strSessionKey[]);


typedef DWORD
(WINAPI* PFN_RmRegisterResources)(
	_In_ DWORD dwSessionHandle,
	_In_ UINT nFiles,
	_In_reads_opt_(nFiles) LPCWSTR rgsFileNames[],
	_In_ UINT nApplications,
	_In_reads_opt_(nApplications) RM_UNIQUE_PROCESS rgApplications[],
	_In_ UINT nServices,
	_In_reads_opt_(nServices) LPCWSTR rgsServiceNames[]);

typedef DWORD
(WINAPI* PFN_RmGetList)(
	_In_ DWORD dwSessionHandle,
	_Out_ UINT* pnProcInfoNeeded,
	_Inout_ UINT* pnProcInfo,
	_Inout_updates_opt_(*pnProcInfo) RM_PROCESS_INFO rgAffectedApps[],
	_Out_ LPDWORD lpdwRebootReasons);

typedef DWORD(WINAPI* PFN_RmEndSession)(_In_ DWORD dwSessionHandle);

int GetPidsFromUsedFile(PCWSTR path, DWORD** pids)
{
	*pids = NULL;
	HMODULE hModule = LoadLibraryW(L"Rstrtmgr.dll");
	if (!hModule)
		return E_GET_PIDS_FROM_FILE_CANNOT_LOAD_RSTRMGR_DLL;
	PFN_RmStartSession RmStartSession = (PFN_RmStartSession)GetProcAddress(hModule, "RmStartSession");
	PFN_RmRegisterResources RmRegisterResources = (PFN_RmRegisterResources)GetProcAddress(hModule, "RmRegisterResources");
	PFN_RmGetList RmGetList = (PFN_RmGetList)GetProcAddress(hModule, "RmGetList");
	PFN_RmEndSession RmEndSession = (PFN_RmEndSession)GetProcAddress(hModule, "RmEndSession");

	if(!RmStartSession || !RmRegisterResources || !RmGetList || !RmEndSession)
	{
		FreeLibrary(hModule);
		return E_GET_PIDS_FROM_FILE_CANNOT_FIND_INTERFACE_FROM_RSTRMGR_DLL;
	}

	DWORD dwSession = 0;
	WCHAR szSessionKey[CCH_RM_SESSION_KEY + 1] = { 0 };
	DWORD dwError = RmStartSession(&dwSession, 0, szSessionKey);
	if(dwError != dwError)
	{
		SetLastError(dwError);
		FreeLibrary(hModule);
		return E_GET_PIDS_FROM_FILE_WIN32_ERROR;
	}

	dwError = RmRegisterResources(dwSession, 1, &path,
		0, NULL, 0, NULL);
	if (dwError != dwError)
	{
		SetLastError(dwError);
		FreeLibrary(hModule);
		return E_GET_PIDS_FROM_FILE_WIN32_ERROR;
	}

	DWORD dwReason = 0;
	UINT nProcInfoNeeded = 0;
	UINT nProcInfo = 0;
	do
	{
		nProcInfo = nProcInfo == 0 ? 8 : nProcInfo << 1;
		RM_PROCESS_INFO* rgpi = (RM_PROCESS_INFO*)malloc(nProcInfo * sizeof(RM_PROCESS_INFO));
		dwError = RmGetList(dwSession, &nProcInfoNeeded,
			&nProcInfo, rgpi, &dwReason);
		if (dwError == ERROR_SUCCESS)
		{
			if (nProcInfo != 0)
			{
				*pids = (DWORD*)malloc(sizeof(DWORD) * nProcInfo);
				for (UINT i = 0; i < nProcInfo; i++)
					(*pids)[i] = rgpi[i].Process.dwProcessId;
			}
		}
		free(rgpi);
	} while (dwError == ERROR_MORE_DATA);
	RmEndSession(dwSession);
	FreeLibrary(hModule);

	if (dwError != ERROR_SUCCESS)
	{
		SetLastError(dwError);
		return E_GET_PIDS_FROM_FILE_WIN32_ERROR;
	}
	return nProcInfo;
}

void FreePidsFromUsedFile(DWORD* pids)
{
	if (pids)
		free(pids);
}
