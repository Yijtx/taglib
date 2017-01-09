#pragma once

#include <atldef.h>
#include <atlsimpcoll.h>

inline BOOL IsTypeStartEnable(DWORD nType)
{
	return (nType == SERVICE_AUTO_START) || (nType == SERVICE_BOOT_START) || (nType == SERVICE_SYSTEM_START);
}

class CServHandle {

public:
	CServHandle() :m_handle(NULL)
	{
	}
	CServHandle(SC_HANDLE handle) :m_handle(handle)
	{
	}
	virtual ~CServHandle()
	{
		if (m_handle != NULL)
		{
			::CloseServiceHandle(m_handle);
			m_handle = NULL;
		}
	}
public:
	BOOL IsValid()
	{
		return m_handle != NULL;
	}

	SC_HANDLE operator=(SC_HANDLE handle)
	{
		m_handle = handle;
		return m_handle;
	}

	operator SC_HANDLE()
	{
		return m_handle;
	}
protected:
	SC_HANDLE m_handle;
};

class CServiceOper{

public:
	CServiceOper(){}
	~CServiceOper(){}
public:
	BOOL ControlRun(BOOL bEnable, LPCTSTR lpServName)
	{
		return EnableService(lpServName, bEnable);
	}

	BOOL DeleteService(LPCTSTR lpServName)
	{
		CServHandle cServMan = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (!cServMan.IsValid())
			return FALSE;

		CServHandle hService = ::OpenService(cServMan, lpServName, GENERIC_ALL);
		if (!hService.IsValid())
			return FALSE;

		return ::DeleteService(hService);
	}
protected:
	BOOL EnableService(LPCTSTR lpServName, BOOL bEnable)
	{
		CServHandle cServMan = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (!cServMan.IsValid())
			return FALSE;

		CServHandle hService = ::OpenService(cServMan, lpServName, GENERIC_WRITE);
		if (!hService.IsValid())
			return FALSE;

		return ::ChangeServiceConfig(hService,
			SERVICE_NO_CHANGE,
			bEnable ? SERVICE_AUTO_START : SERVICE_DEMAND_START,
			SERVICE_NO_CHANGE,
			NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	}
};

class CServiceEnumer{

public:

	struct SERVICE_CONFIG
	{
		CString					strName;
		CString					strServDLL;
		CString					strDesc;
		LPQUERY_SERVICE_CONFIG	lpqscfg;
	};

	CServiceEnumer()
	{
	}

	virtual ~CServiceEnumer()
	{
		for (int i = 0; i < m_servicebuffer.GetSize(); i++)
		{
			delete m_servicebuffer[i].lpqscfg;
		}
	}

	BOOL DoEnum()
	{
		DWORD ServicesCount = 0;
		DWORD size = 0;
		SC_HANDLE hHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);

		if (hHandle == NULL)
			return FALSE;

		if (!EnumServicesStatus(hHandle, SERVICE_WIN32, SERVICE_STATE_ALL,
			NULL, 0, &size, &ServicesCount, NULL))
		{
			if (GetLastError() == ERROR_MORE_DATA)
			{
				LPENUM_SERVICE_STATUS lpinfo = new ENUM_SERVICE_STATUS[size];
				if (EnumServicesStatus(hHandle,
					SERVICE_WIN32,
					SERVICE_STATE_ALL,
					(LPENUM_SERVICE_STATUS)lpinfo,
					size, &size,
					&ServicesCount, NULL))
				{
					for (DWORD i = 0; i < ServicesCount; i++)
					{
						SC_HANDLE hService = ::OpenService(hHandle, lpinfo[i].lpServiceName, SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS);
						if (hService != NULL)
						{
							DWORD nNeedSize = 0;
							::QueryServiceConfig(hService, NULL, 0, &nNeedSize);

							if (nNeedSize > 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
							{
								SERVICE_CONFIG			svcfg;
								LPQUERY_SERVICE_CONFIG	qscfg = (LPQUERY_SERVICE_CONFIG)new char[nNeedSize];

								if (::QueryServiceConfig(hService, qscfg, nNeedSize, &nNeedSize))
								{
									svcfg.lpqscfg = qscfg;
									svcfg.strName = lpinfo[i].lpServiceName;
									GetServiceDLL(svcfg.strName, svcfg.strServDLL);

									if (TRUE)
									{
										DWORD	nSize = 0;
										::QueryServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, NULL, 0, &nSize);
										if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
										{
											nSize++;
											LPSERVICE_DESCRIPTION buffer = (LPSERVICE_DESCRIPTION)new BYTE[nSize];
											memset(buffer, 0, nSize);
											if (::QueryServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, (LPBYTE)buffer, nSize, &nSize))
											{
												svcfg.strDesc = buffer->lpDescription;
											}
											delete buffer;
										}

									}
									m_servicebuffer.Add(svcfg);
								}
							}
						}
						::CloseServiceHandle(hService);
					}
				}
				delete lpinfo;
			}
		}
		CloseServiceHandle(hHandle);
		return (m_servicebuffer.GetSize() > 0);
	}

	size_t	GetRunSrvCount()
	{
		int nResult = 0;
		DWORD ServicesCount = 0;
		DWORD size = 0;
		SC_HANDLE hHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);

		if (hHandle == NULL)
			return FALSE;

		if (!EnumServicesStatus(hHandle, SERVICE_WIN32, SERVICE_STATE_ALL,
			NULL, 0, &size, &ServicesCount, NULL))
		{
			if (GetLastError() == ERROR_MORE_DATA)
			{
				LPENUM_SERVICE_STATUS lpinfo = new ENUM_SERVICE_STATUS[size];
				if (EnumServicesStatus(hHandle,
					SERVICE_WIN32,
					SERVICE_STATE_ALL,
					(LPENUM_SERVICE_STATUS)lpinfo,
					size, &size,
					&ServicesCount, NULL))
				{
					for (DWORD i = 0; i < ServicesCount; i++)
					{
						SC_HANDLE hService = ::OpenService(hHandle, lpinfo[i].lpServiceName, SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS);
						if (hService != NULL)
						{
							DWORD nNeedSize = 0;
							::QueryServiceConfig(hService, NULL, 0, &nNeedSize);

							if (nNeedSize > 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
							{
								LPQUERY_SERVICE_CONFIG	qscfg = (LPQUERY_SERVICE_CONFIG)new char[nNeedSize];

								if (::QueryServiceConfig(hService, qscfg, nNeedSize, &nNeedSize))
								{
									if (IsTypeStartEnable(qscfg->dwStartType))
										nResult++;
								}
							}
						}
						::CloseServiceHandle(hService);
					}
				}
				delete lpinfo;
			}
		}
		CloseServiceHandle(hHandle);
		return nResult;
	}

	size_t GetCount()
	{
		return m_servicebuffer.GetSize();
	}
	SERVICE_CONFIG& operator[](int iIndex)
	{
		ATLASSERT(iIndex < m_servicebuffer.GetSize());
		return m_servicebuffer[iIndex];
	}

	SERVICE_CONFIG& GetItem(int iIndex)
	{
		ATLASSERT(iIndex < m_servicebuffer.GetSize());
		return m_servicebuffer[iIndex];
	}

protected:
	BOOL GetServiceDLL(LPCTSTR lpServiceName, CString& strDLL)
	{
		CString		strFormat;
#define KEY_SYS_SERVICE _T("SYSTEM\\CurrentControlSet\\Services\\%s\\Parameters")
		strFormat.Format(KEY_SYS_SERVICE, lpServiceName);
		return (SHGetStringValue(HKEY_LOCAL_MACHINE, strFormat, _T("ServiceDll"), strDLL) == ERROR_SUCCESS);
	}

	DWORD SHGetStringValue(HKEY hRoot, LPCTSTR lpSubKey, LPCTSTR lpValue, CString& strData)
	{
		DWORD	nRet = ERROR_SUCCESS;
		DWORD	nSize = MAX_PATH;
		TCHAR*	buffer = new TCHAR[nSize];

		do
		{
			DWORD	nType = REG_SZ;
			memset(buffer, 0, sizeof(MAX_PATH)*sizeof(TCHAR));
			nRet = SHGetValue(hRoot, lpSubKey, lpValue, &nType, buffer, &nSize);

			if (nRet == ERROR_SUCCESS)
			{
				if (nType == REG_SZ || nType == REG_EXPAND_SZ)
					strData = buffer;
				else
					nRet = ERROR_BAD_FORMAT;
				delete buffer;
				break;
			}
			else if (nRet == ERROR_MORE_DATA)
			{
				delete buffer;
				nSize += MAX_PATH;
				buffer = new TCHAR[nSize];
			}
			else
			{
				delete buffer;
				break;
			}
		} while (TRUE);

		return nRet;
	}
protected:
	CSimpleArray<SERVICE_CONFIG>	m_servicebuffer;
};