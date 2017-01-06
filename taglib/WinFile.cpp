#include "stdafx.h"
#include "WinFile.h"


CWinFile::CWinFile()
{
	m_bEnableCache = FALSE;
	m_dwWrCachePtr = 0;
	m_pucWrCache = NULL;
	m_dwWrCacheSize = CWINFILE_DEFAULTCACHESIZE;
	m_dwRdCachePtr = 0;
	m_dwRdCacheBytes = 0;
	m_pucRdCache = NULL;
	m_dwRdCacheSize = CWINFILE_DEFAULTCACHESIZE;

	m_hFile = INVALID_HANDLE_VALUE;

	DefaultOpenParams();
}

CWinFile::~CWinFile()
{
	Close();
}

void CWinFile::Close()
{
	// Call to delete cache buffers
	FlushCache();

	// Close the file if open
	if (!IsOpen()) return;
	FlushFileBuffers(m_hFile);
	CloseHandle(m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;
}

BOOL CWinFile::Write(LPVOID pBuf, DWORD dwLen, LPDWORD pdwWritten, BOOL bBypassCache)
{
	DWORD written;

	// Sanity checks
	if (!IsOpen() || pBuf == NULL) return FALSE;

	// Zero length means NULL teminated string
	if (dwLen == 0)
	{
		// Count characters
		while (((LPBYTE)pBuf)[dwLen] != 0x0) dwLen++;

	} // end if
	if (dwLen == 0) return FALSE;

	// Attempt to use the cache
	if (!bBypassCache && m_bEnableCache)
		if (WriteCache(pBuf, dwLen, pdwWritten)) return TRUE;

	// Attempt to read from the file
	if (!WriteFile(m_hFile, pBuf, dwLen, &written, NULL))
		return FALSE;

	// Let the user know how many bytes went out if interested
	if (pdwWritten != 0x0) *pdwWritten = written;
	return TRUE;
}

BOOL CWinFile::Read(LPVOID pBuf, DWORD dwLen, LPDWORD pdwRead, BOOL bBypassCache)
{
	DWORD	read = 0;

	// Sanity checks
	if (!IsOpen() || pBuf == NULL || dwLen == 0) return FALSE;

	// Attempt to use the cache
	if (!bBypassCache && m_bEnableCache)
		if (ReadCache(pBuf, dwLen, pdwRead)) return TRUE;

	if (!ReadFile(m_hFile, pBuf, dwLen, &read, NULL))
		return FALSE;

	// Let the user know how many bytes were read if interested
	if (pdwRead != NULL) *pdwRead = read;

	return (read != 0);
}

BOOL CWinFile::Open( LPCTSTR pFilename, DWORD access )
{
	// Close any open file
	Close();

	if (access != 0xffffffff) m_ofp.dwDesiredAccess = access;

	// Set the users filename if needed
	if (pFilename != NULL) SetPath(pFilename);

	// Make sure we have a filename
	if (!IsPath()) return FALSE;

	// Attempt to open a new file
	m_hFile = CreateFile(m_szPath,
		m_ofp.dwDesiredAccess,
		m_ofp.dwShareMode,
		m_ofp.lpSecurityAttributes,
		m_ofp.dwCreationDisposition,
		m_ofp.dwFlagsAndAttributes,
		m_ofp.hTemplateFile);
	return IsOpen();
}

LONGLONG CWinFile::GetPtrPos()
{
	if (!IsOpen()) return 0;

	// Get the current file pointer position
	LONG lHi = 0;
	DWORD pos = SetFilePointer(m_hFile, 0, &lHi, FILE_CURRENT);

	// Was there an error?
	if (pos == 0xffffffff) return 0;

	return pos;
}

void CWinFile::DefaultOpenParams()
{
	m_ofp.dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
	m_ofp.dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
	m_ofp.lpSecurityAttributes = NULL;
	m_ofp.dwCreationDisposition = OPEN_ALWAYS;
	m_ofp.dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
	m_ofp.hTemplateFile = NULL;
}

void CWinFile::FlushReadCache()
{
	if (m_pucRdCache == NULL) return;
	delete[] m_pucRdCache;
	m_pucRdCache = NULL;
	m_dwRdCachePtr = 0;
	m_dwRdCacheBytes = 0;
}

void CWinFile::FlushWriteCache()
{
	if (m_pucWrCache == NULL) return;

	// Flush the data
	WriteCache(NULL, 0, NULL);

	delete[] m_pucWrCache;
	m_pucWrCache = NULL;
	m_dwWrCachePtr = 0;
}

BOOL CWinFile::WriteCache(LPVOID pBuffer, DWORD dwLength, LPDWORD pdwWritten)
{
	if (!IsOpen()) return FALSE;
	if (!m_bEnableCache) return FALSE;

	if (m_pucWrCache == NULL)
	{
		m_dwWrCachePtr = 0;
		m_pucWrCache = new BYTE[m_dwWrCacheSize];
		if (m_pucWrCache == NULL) return FALSE;
	} // end if

	// Is there still room in the cache
	if (pBuffer != NULL && (m_dwWrCachePtr + dwLength) < m_dwWrCacheSize)
	{
		if (dwLength == 1) m_pucWrCache[m_dwWrCachePtr++] = *((BYTE*)pBuffer);
		else
		{
			memcpy((LPVOID)&m_pucWrCache[m_dwWrCachePtr], (LPVOID)pBuffer, dwLength);
			m_dwWrCachePtr += dwLength;
		} // end else
	} // end if
	else
	{
		DWORD dwBytesWritten = 0;

		// Write the cache to the disk
		if (!Write(m_pucWrCache, m_dwWrCachePtr, &dwBytesWritten, TRUE) ||
			dwBytesWritten != m_dwWrCachePtr) return FALSE;
		m_dwWrCachePtr = 0;

		if (pBuffer != NULL)
		{
			// Save the rest of the data in the cache
			if (dwLength == 1) m_pucWrCache[m_dwWrCachePtr++] = *((BYTE*)pBuffer);
			else
			{
				memcpy((LPVOID)m_pucWrCache, (LPVOID)pBuffer, dwLength);
				m_dwWrCachePtr += dwLength;
			} // end else
		} // end if
	} // end else

	// Tell the user how it went if their interested
	if (pdwWritten != NULL) *pdwWritten = dwLength;

	return TRUE;
}

BOOL CWinFile::ReadCache(LPVOID pBuffer, DWORD dwLength, LPDWORD pdwRead)
{
	if (!IsOpen()) return FALSE;
	if (!m_bEnableCache) return FALSE;

	if (m_pucRdCache == NULL)
	{
		m_dwRdCachePtr = 0;
		m_dwRdCacheBytes = 0;
		m_pucRdCache = new BYTE[m_dwRdCacheSize];
		if (m_pucRdCache == NULL) return FALSE;
	} // end if

	// Are there enough bytes left in the cache
	if ((m_dwRdCacheBytes - m_dwRdCachePtr) >= dwLength)
	{
		if (dwLength == 1) *((BYTE*)pBuffer) = m_pucRdCache[m_dwRdCachePtr++];
		else
		{
			memcpy((LPVOID)pBuffer, (LPVOID)&m_pucRdCache[m_dwRdCachePtr], dwLength);
			m_dwRdCachePtr += dwLength;
		} // end else
	} // end if
	else
	{
		DWORD dwBytesRead = 0;

		// Adjust CacheSize
		m_dwRdCacheBytes -= m_dwRdCachePtr;

		// Move bytes back in the buffer
		if (m_dwRdCacheBytes)
			memmove((LPVOID)m_pucRdCache,
			&m_pucRdCache[m_dwRdCachePtr],
			m_dwRdCacheBytes);
		m_dwRdCachePtr = 0;

		// Read some more bytes
		if (!Read(&m_pucRdCache[m_dwRdCacheBytes],
			m_dwRdCacheSize - m_dwRdCacheBytes,
			&dwBytesRead,
			TRUE)) return FALSE;

		// Add the bytes we read
		m_dwRdCacheBytes += dwBytesRead;

		// Let's try it again
		if (m_dwRdCacheBytes > dwLength)
		{
			if (dwLength == 1) *((BYTE*)pBuffer) = m_pucRdCache[m_dwRdCachePtr++];
			else
			{
				memcpy((LPVOID)pBuffer, (LPVOID)m_pucRdCache, dwLength);
				m_dwRdCachePtr += dwLength;
			} // end else
		} // end if
		else return FALSE;

	} // end else

	return TRUE;

}

BOOL CWinFile::GetSpecialFolder(int nID, LPTSTR buf)
{
	// Sanity check
	if (buf == NULL) return FALSE;

	if (nID == CSIDL_USER_NONE)
	{
		*buf = 0x0;
		return TRUE;
	} // end if

	// If they want the system directory
	if (nID == CSIDL_USER_SYSTEM)
		return (::GetSystemDirectory(buf, CWF_STRSIZE) != 0x0);

	// If they want the Windows directory
	else if (nID == CSIDL_USER_WINDOWS)
		return (::GetWindowsDirectory(buf, CWF_STRSIZE) != 0x0);

	// If they want the Temprorary directory
	else if (nID == CSIDL_USER_TEMP)
		return (::GetTempPath(CWF_STRSIZE, buf) != 0x0);

	// If they want the Current directory
	else if (nID == CSIDL_USER_CURRENT)
		return (::GetCurrentDirectory(CWF_STRSIZE, buf) != 0x0);

	// If they want the Current directory
	else if (nID == CSIDL_USER_DEFDRIVE)
	{
		if (::GetWindowsDirectory(buf, CWF_STRSIZE) == 0x0)
			return FALSE;
		buf[3] = 0x0;
		return TRUE;
	} // end else if

	// Check for invalid
	else if (nID == -1) return FALSE;

	// Ask windows what they want
	else
	{
		LPMALLOC		pMalloc;
		LPITEMIDLIST	pidl;

		if (SHGetSpecialFolderLocation(NULL, nID, &pidl) != NOERROR)
			return FALSE;

		// Get the path name
		BOOL ret = SHGetPathFromIDList(pidl, buf);

		// Free the memory
		if (SHGetMalloc(&pMalloc) == NOERROR)
			pMalloc->Free(pidl);

		return ret;

	} // end else

	// !!??
	return FALSE;
}

BOOL CWinFile::CopyFile(HANDLE hFile)
{
	// Do we have a file open
	if (!IsOpen()) return FALSE;

	// copy to the current file location
	return CopyFile(hFile, m_hFile);
}

BOOL CWinFile::CopyFile(HANDLE hSource, HANDLE hTarget)
{
	if (hSource == INVALID_HANDLE_VALUE ||
		hTarget == INVALID_HANDLE_VALUE) return FALSE;

	BYTE	buf[BUFSIZE];
	ZeroMemory(buf, BUFSIZE);
	DWORD	bytes = 0;

	// For the entire file
	while (ReadFile(hSource, buf, BUFSIZE, &bytes, NULL) && bytes)
	{
		// Write the bytes to the end of the file
		if (!WriteFile(hTarget, buf, bytes, &bytes, NULL))
			return FALSE;
	} // end while

	return TRUE;
}

BOOL CWinFile::OpenNew(LPCTSTR pFilename /*= NULL*/, DWORD access /*= 0xffffffff*/)
{
	DefaultOpenParams();
	m_ofp.dwCreationDisposition = CREATE_ALWAYS;

	return Open(pFilename, access);
}
