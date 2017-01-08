/********************************************************************
*
* Copyright (C) 1999-2000 Sven Wiegand
* Copyright (C) 2000-2001 ToolsCenter
*
* This file is free software; you can redistribute it and/or
* modify, but leave the headers intact and do not remove any
* copyrights from the source.
*
* If you have further questions, suggestions or bug fixes, visit
* our homepage
*
*    http://www.ToolsCenter.org
*
********************************************************************/

#include "stdafx.h"
#include "FileVersionInfo.h"

#include <assert.h>

//-------------------------------------------------------------------
// CFileVersionInfo
//-------------------------------------------------------------------

FileVersionInfo::FileVersionInfo()
{
	Reset();
}


FileVersionInfo::~FileVersionInfo()
{}


BOOL FileVersionInfo::GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough/*= FALSE*/)
{
	LPWORD lpwData;
	for (lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData) + unBlockSize; lpwData += 2)
	{
		if (*lpwData == wLangId)
		{
			dwId = *((DWORD*)lpwData);
			return TRUE;
		}
	}

	if (!bPrimaryEnough)
		return FALSE;

	for (lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData) + unBlockSize; lpwData += 2)
	{
		if (((*lpwData) & 0x00FF) == (wLangId & 0x00FF))
		{
			dwId = *((DWORD*)lpwData);
			return TRUE;
		}
	}

	return FALSE;
}


BOOL FileVersionInfo::Create(HMODULE hModule /*= NULL*/)
{
	CString	strPath;

	GetModuleFileName(hModule, strPath.GetBuffer(_MAX_PATH), _MAX_PATH);
	strPath.ReleaseBuffer();
	return Create(strPath);
}


BOOL FileVersionInfo::Create(LPCTSTR lpszFileName)
{
	Reset();

	DWORD	dwHandle;
	DWORD	dwFileVersionInfoSize = GetFileVersionInfoSize((LPTSTR)lpszFileName, &dwHandle);
	if (!dwFileVersionInfoSize)
		return FALSE;

	LPVOID	lpData = (LPVOID)new BYTE[dwFileVersionInfoSize];
	if (!lpData)
		return FALSE;

	try
	{
		if (!GetFileVersionInfo((LPTSTR)lpszFileName, dwHandle, dwFileVersionInfoSize, lpData))
			throw FALSE;

		// catch default information
		LPVOID	lpInfo;
		UINT		unInfoLen;
		if (VerQueryValue(lpData, _T("\\"), &lpInfo, &unInfoLen))
		{
			assert(unInfoLen == sizeof(m_FileInfo));
			if (unInfoLen == sizeof(m_FileInfo))
				memcpy(&m_FileInfo, lpInfo, unInfoLen);
		}

		// find best matching language and codepage
		VerQueryValue(lpData, _T("\\VarFileInfo\\Translation"), &lpInfo, &unInfoLen);

		DWORD	dwLangCode = 0;
		if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, FALSE))
		{
			if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, TRUE))
			{
				if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), dwLangCode, TRUE))
				{
					if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL), dwLangCode, TRUE))
						// use the first one we can get
						dwLangCode = *((DWORD*)lpInfo);
				}
			}
		}


		CString	strSubBlock;
		strSubBlock.Format(_T("\\StringFileInfo\\%04X%04X\\"), dwLangCode & 0x0000FFFF, (dwLangCode & 0xFFFF0000) >> 16);

		// catch string table
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + _T("CompanyName")), &lpInfo, &unInfoLen))
			m_strCompanyName = CString((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + _T("FileDescription")), &lpInfo, &unInfoLen))
			m_strFileDescription = CString((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + _T("FileVersion")), &lpInfo, &unInfoLen))
			m_strFileVersion = CString((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + _T("InternalName")), &lpInfo, &unInfoLen))
			m_strInternalName = CString((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + _T("LegalCopyright")), &lpInfo, &unInfoLen))
			m_strLegalCopyright = CString((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + _T("OriginalFileName")), &lpInfo, &unInfoLen))
			m_strOriginalFileName = CString((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + _T("ProductName")), &lpInfo, &unInfoLen))
			m_strProductName = CString((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + _T("ProductVersion")), &lpInfo, &unInfoLen))
			m_strProductVersion = CString((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + _T("Comments")), &lpInfo, &unInfoLen))
			m_strComments = CString((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + _T("LegalTrademarks")), &lpInfo, &unInfoLen))
			m_strLegalTrademarks = CString((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + _T("PrivateBuild")), &lpInfo, &unInfoLen))
			m_strPrivateBuild = CString((LPCTSTR)lpInfo);
		if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock + _T("SpecialBuild")), &lpInfo, &unInfoLen))
			m_strSpecialBuild = CString((LPCTSTR)lpInfo);

		delete[] lpData;
	}
	catch (BOOL)
	{
		delete[] lpData;
		return FALSE;
	}

	return TRUE;
}


WORD FileVersionInfo::GetFileVersion(int nIndex) const
{
	if (nIndex == 0)
		return (WORD)(m_FileInfo.dwFileVersionLS & 0x0000FFFF);
	else if (nIndex == 1)
		return (WORD)((m_FileInfo.dwFileVersionLS & 0xFFFF0000) >> 16);
	else if (nIndex == 2)
		return (WORD)(m_FileInfo.dwFileVersionMS & 0x0000FFFF);
	else if (nIndex == 3)
		return (WORD)((m_FileInfo.dwFileVersionMS & 0xFFFF0000) >> 16);
	else
		return 0;
}


WORD FileVersionInfo::GetProductVersion(int nIndex) const
{
	if (nIndex == 0)
		return (WORD)(m_FileInfo.dwProductVersionLS & 0x0000FFFF);
	else if (nIndex == 1)
		return (WORD)((m_FileInfo.dwProductVersionLS & 0xFFFF0000) >> 16);
	else if (nIndex == 2)
		return (WORD)(m_FileInfo.dwProductVersionMS & 0x0000FFFF);
	else if (nIndex == 3)
		return (WORD)((m_FileInfo.dwProductVersionMS & 0xFFFF0000) >> 16);
	else
		return 0;
}


DWORD FileVersionInfo::GetFileFlagsMask() const
{
	return m_FileInfo.dwFileFlagsMask;
}


DWORD FileVersionInfo::GetFileFlags() const
{
	return m_FileInfo.dwFileFlags;
}


DWORD FileVersionInfo::GetFileOs() const
{
	return m_FileInfo.dwFileOS;
}


DWORD FileVersionInfo::GetFileType() const
{
	return m_FileInfo.dwFileType;
}


DWORD FileVersionInfo::GetFileSubtype() const
{
	return m_FileInfo.dwFileSubtype;
}


FILETIME FileVersionInfo::GetFileDate() const
{
	FILETIME	ft;
	ft.dwLowDateTime = m_FileInfo.dwFileDateLS;
	ft.dwHighDateTime = m_FileInfo.dwFileDateMS;
	return ft;
}


CString FileVersionInfo::GetCompanyName() const
{
	return m_strCompanyName;
}


CString FileVersionInfo::GetFileDescription() const
{
	return m_strFileDescription;
}


CString FileVersionInfo::GetFileVersion() const
{
	return m_strFileVersion;
}


CString FileVersionInfo::GetInternalName() const
{
	return m_strInternalName;
}


CString FileVersionInfo::GetLegalCopyright() const
{
	return m_strLegalCopyright;
}


CString FileVersionInfo::GetOriginalFileName() const
{
	return m_strOriginalFileName;
}


CString FileVersionInfo::GetProductName() const
{
	return m_strProductName;
}


CString FileVersionInfo::GetProductVersion() const
{
	return m_strProductVersion;
}


CString FileVersionInfo::GetComments() const
{
	return m_strComments;
}


CString FileVersionInfo::GetLegalTrademarks() const
{
	return m_strLegalTrademarks;
}


CString FileVersionInfo::GetPrivateBuild() const
{
	return m_strPrivateBuild;
}


CString FileVersionInfo::GetSpecialBuild() const
{
	return m_strSpecialBuild;
}


void FileVersionInfo::Reset()
{
	ZeroMemory(&m_FileInfo, sizeof(m_FileInfo));
	m_strCompanyName.Empty();
	m_strFileDescription.Empty();
	m_strFileVersion.Empty();
	m_strInternalName.Empty();
	m_strLegalCopyright.Empty();
	m_strOriginalFileName.Empty();
	m_strProductName.Empty();
	m_strProductVersion.Empty();
	m_strComments.Empty();
	m_strLegalTrademarks.Empty();
	m_strPrivateBuild.Empty();
	m_strSpecialBuild.Empty();
}
