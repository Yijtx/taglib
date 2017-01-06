#pragma once

#define CWF_STRSIZE		1024
#if( CWF_STRSIZE < MAX_PATH )
#error CWF_STRSIZE is too small!
#endif

typedef struct tagOPENFILEPARAMS
{
	/// Desired file access
	DWORD					dwDesiredAccess;

	/// File share mode
	DWORD					dwShareMode;

	/// Security attributes
	LPSECURITY_ATTRIBUTES	lpSecurityAttributes;

	/// Create disposition, over-write, etc...
	DWORD					dwCreationDisposition;

	/// File attribute flags
	DWORD					dwFlagsAndAttributes;

	/// Handle to template file
	HANDLE					hTemplateFile;

} OPENFILEPARAMS; // typedef struct
typedef OPENFILEPARAMS* LPOPENFILEPARAMS;


// For getting special folders
#define CSIDL_USER_NONE					0xff00
#define CSIDL_USER_SYSTEM				0xff01
#define CSIDL_USER_WINDOWS				0xff02
#define CSIDL_USER_TEMP					0xff03
#define CSIDL_USER_CURRENT				0xff04
#define CSIDL_USER_DEFDRIVE				0xff05

#define CSIDL_FLAG_VIRTUAL				0x00000001

#define CWINFILE_DEFAULTCACHESIZE		1024

class CWinFile
{
public:
	CWinFile();

	CWinFile(HANDLE hFile) { CWinFile();}


	~CWinFile();


	void Close();

	/* 
	 * Attaches to an existing file handle
	 * Parameter: HANDLE hFile
	 */
	inline void Attach(HANDLE hFile)
	{
		Close(); m_hFile = hFile;
	}

	/* 
	 * Detaches from file handle without closing the handle.
	 */
	inline void Detach() { 
		m_hFile = INVALID_HANDLE_VALUE; 
	}

	/* 
	 * Returns non-zero if the file is open
	 */
	inline BOOL IsOpen() { 
		return (m_hFile != INVALID_HANDLE_VALUE); 
	}

	/* 
	 * Returns the curent file handle
	 */
	inline HANDLE GetHandle() { 
		return m_hFile; 
	}
	/* 
	 * Returns the curent file handle
	 */
	operator HANDLE() { return GetHandle(); }

	/* 
	 * Returns the size of the current file in bytes
	 */
	DWORD Size()
	{
		DWORD size = GetFileSize(m_hFile, NULL);
		if (size == INVALID_FILE_SIZE) return 0; else return size;
	}

	/* 
	 * Gets the current 32-bit file pointer position
	 * Parameter: LPDWORD ptr
	 */
	BOOL GetPtrPos(LPDWORD ptr);

	/* 
	 * Gets the current 64-bit file pointer position
	 * Parameter: LONGLONG * ptr
	 */
	BOOL GetPtrPos(LONGLONG *ptr);

	/* 
	 * Returns the current 64-bit file pointer position
	 */
	LONGLONG GetPtrPos();

	/* 
	 * Sets the 64-bit file pointer position
	 * Parameter: LONGLONG offset
	 * Parameter: DWORD ref
	 */
	inline BOOL SetPtrPos(LONGLONG offset, DWORD ref)
	{
		if (!IsOpen()) return FALSE;
		LONG lHi = (DWORD)((offset >> 32) & 0xffffffff);
		return (SetFilePointer(m_hFile, (LONG)(offset & 0xffffffff), &lHi, ref) != 0xffffffff || GetLastError() == NO_ERROR);
	}

	/* 
	 * Sets the 64-bit file pointer position relative to the beginning of the file
	 * Parameter: LONGLONG offset
	 */
	inline BOOL SetPtrPosBegin(LONGLONG offset = 0)
	{
		if (!IsOpen()) return FALSE;
		LONG lHi = (DWORD)((offset >> 32) & 0xffffffff);
		BOOL ret = (SetFilePointer(m_hFile, (LONG)(offset & 0xffffffff), &lHi, FILE_BEGIN) != 0xffffffff || GetLastError() == NO_ERROR);
		return ret;
	}

	/* 
	 * Sets the 64-bit file pointer position relative to the end of the file
	 * Parameter: LONGLONG offset
	 */
	inline BOOL SetPtrPosEnd(LONGLONG offset = 0)
	{
		if (!IsOpen()) return FALSE;
		LONG lHi = (DWORD)((offset >> 32) & 0xffffffff);
		BOOL ret = (SetFilePointer(m_hFile, (LONG)(offset & 0xffffffff), &lHi, FILE_END) != 0xffffffff || GetLastError() == NO_ERROR);
		return ret;
	}

	/* 
	 * Sets the 64-bit file pointer position relative to the current file pointer
	 * Parameter: LONGLONG offset
	 */
	inline BOOL SetPtrPosCur(LONGLONG offset = 0)
	{
		if (!IsOpen()) return FALSE;
		LONG lHi = (DWORD)((offset >> 32) & 0xffffffff);
		BOOL ret = (SetFilePointer(m_hFile, (LONG)(offset & 0xffffffff), &lHi, FILE_CURRENT) != 0xffffffff || GetLastError() == NO_ERROR);
		return ret;
	}

	/* 
	 * Returns non-zero if there is a non-zero length file path
	 */
	inline BOOL IsPath() { return (m_szPath[0] != 0x0); }

	/* 
	 * Sets the current file path
	 * Parameter: LPCTSTR ptr
	 */
	inline BOOL SetPath(LPCTSTR ptr)
	{
		if (IsOpen()) return FALSE;
		if (ptr == NULL) { m_szPath[0] = 0x0; return TRUE; }
		_tcscpy_s(m_szPath, ptr);
		return TRUE;
	}

	/* 
	 * Retrieves the current file path string
	 * Parameter: LPTSTR ptr
	 */
	inline BOOL GetPath(LPTSTR ptr)
	{
		if (!IsPath()) return FALSE;
		if (ptr != NULL) { _tcscpy(ptr,m_szPath); return TRUE; }
		else return FALSE;
	}

	/* 
	 * Returns a pointer to the current file path
	 */
	inline LPCTSTR GetPath() { return m_szPath; }

	/* 
	 * Writes a DWORD value into the file
	 * Parameter: DWORD dw
	 */
	inline BOOL SaveDWORD(DWORD dw)
	{
		return Write((LPVOID)&dw, sizeof(DWORD));
	}

	// Reads a DWORD value from the current file

	inline BOOL RestoreDWORD(LPDWORD pdw)
	{
		return Read((LPVOID)pdw, sizeof(DWORD));
	}

	/* 
	 * Sets the size of the file cache
	 * Parameter: DWORD read
	 * Parameter: DWORD write
	 */
	inline void SetCacheSize(DWORD read, DWORD write)
	{
		FlushCache(); 
		m_dwRdCacheSize = read; 
		m_dwWrCacheSize = write;
	}

	/* 
	 * Flushes the file cache
	 */
	inline void FlushCache() { FlushReadCache(); FlushWriteCache(); }

	/* 
	 * Enables software file cache
	 * Parameter: BOOL enable
	 */
	inline void EnableCache(BOOL enable)
	{
		FlushCache(); m_bEnableCache = enable;
	}

	/* 
	 * Flushes the Windows file buffers
	 */
	inline void Flush() { FlushFileBuffers(m_hFile); }

public:
	/* 
	 * Deletes the specified directory
	 * Parameter: LPCTSTR pDir - Directory to delete
	 * Parameter: BOOL bEmptyOnly - Non-zero to delete only the contents of the specified directory.
	 * Parameter: BOOL bSubDirectories - Non-zero to delete the sub all sub directories.
	 * Parameter: BOOL bSubDirectoriesAndFilesOnly - Deletes only sub directories.  That is, it does not delete files in the specified directories.
	 */
	static BOOL DeleteDirectory(LPCTSTR pDir, BOOL bEmptyOnly = FALSE, BOOL bSubDirectories = TRUE, BOOL bSubDirectoriesAndFilesOnly = FALSE);

	/* 
	 * Creates the specified directory
	 * Parameter: LPCTSTR pDir
	 */
	static BOOL CreateDirectory(LPCTSTR pDir);

	static BOOL GetSaveFileName(LPSTR pFileName, LPCTSTR pTitle = NULL,
		LPCTSTR pFilter = NULL, LPCTSTR pDefExt = NULL,
		HWND hOwner = NULL, BOOL bPreview = FALSE);

	/* 
	 * Opens a temporary file
	 */
	inline BOOL OpenTemp();

	/* 
	 * Deletes the specified filename
	 * Parameter: LPCTSTR pFileName
	 */
	static BOOL Delete(LPCTSTR pFileName)
	{
		SetFileAttributes(pFileName, FILE_ATTRIBUTE_NORMAL); return DeleteFile(pFileName);
	}

	/* 
	 * Creates a temporary filename
	 * Parameter: LPSTR pFileName
	 */
	static BOOL CreateTemp(LPSTR pFileName);

	/* 
	 * Returns non-zero if the filename contains the relative path specifiers.
	 * Parameter: LPCTSTR pFileName
	 */
	static BOOL IsRelative(LPCTSTR pFileName);

	/* 
	 * Builds a path using the forward slash '/'
	 * Parameter: LPSTR pFullPath
	 * Parameter: LPCTSTR pPath
	 * Parameter: LPCTSTR pFileName
	 */
	static LPCTSTR WebBuildPath(LPSTR pFullPath, LPCTSTR pPath, LPCTSTR pFileName);

	/* 
	 * Builds a file path
	 * Parameter: LPSTR pFullPath
	 * Parameter: LPCTSTR pPath
	 * Parameter: LPCTSTR pFileName
	 * Parameter: char sep
	 */
	static LPCTSTR BuildPath(LPSTR pFullPath, LPCTSTR pPath, LPCTSTR pFileName, TCHAR sep = '\\');

	/* 
	 * Extracts the path from the specified filename
	 * Parameter: LPCTSTR pFileName
	 * Parameter: LPSTR pPath
	 */
	static BOOL GetPathFromFileName(LPCTSTR pFileName, LPSTR pPath);

	/* 
	 *  Extracts the filename from a specified file path
	 * Parameter: LPCTSTR pPath
	 * Parameter: LPSTR pFileName
	 * Parameter: BOOL bNoExt
	 */
	static void GetFileNameFromPath(LPCTSTR pPath, LPSTR pFileName, BOOL bNoExt = FALSE);

	/* 
	 * Copies data from the specified file
	 * Parameter: HANDLE hFile
	 * Parameter: DWORD dwCopy
	 * Parameter: LPDWORD pdwCopied
	 */
	BOOL Copy(HANDLE hFile, DWORD dwCopy, LPDWORD pdwCopied = NULL);

	/* 
	 * Copies data from one file to another
	 * Parameter: HANDLE hSource
	 * Parameter: HANDLE hTarget
	 * Parameter: DWORD dwCopy
	 * Parameter: LPDWORD pdwCopied
	 */
	static BOOL Copy(HANDLE hSource, HANDLE hTarget, DWORD dwCopy, LPDWORD pdwCopied = NULL);

	/* 
	 * Copies the entire contents of the specified file handle
	 * Parameter: HANDLE hFile
	 */
	BOOL CopyFile(HANDLE hFile);

	/* 
	 * Copies the entire contents of one file into another 
	 * Parameter: HANDLE hSource
	 * Parameter: HANDLE hTarget
	 */
	static BOOL CopyFile(HANDLE hSource, HANDLE hTarget);

	/* 
	 * Reads file data into a buffer pointer
	 * Parameter: LPVOID pStruct
	 * Parameter: DWORD dwLen
	 */
	BOOL RestoreStruct(LPVOID pStruct, DWORD dwLen = 0);

	/* 
	 * Writes data from buffer into file
	 * Parameter: LPVOID pStruct
	 * Parameter: DWORD dwLen
	 */
	BOOL SaveStruct(LPVOID pStruct, DWORD dwLen = 0);

	/* 
	 * Writes data into file
	 * Parameter: LPVOID pBuf
	 * Parameter: DWORD dwLen
	 * Parameter: LPDWORD pdwWritten
	 * Parameter: BOOL bBypassCache
	 */
	virtual BOOL Write(LPVOID pBuf, DWORD dwLen = 0, LPDWORD pdwWritten = NULL, BOOL bBypassCache = FALSE);
	
	/* 
	 * Reads data from file
	 * Parameter: LPVOID pBuf
	 * Parameter: DWORD dwLen
	 * Parameter: LPDWORD pdwRead
	 * Parameter: BOOL bBypassCache
	 */
	virtual BOOL Read(LPVOID pBuf, DWORD dwLen, LPDWORD pdwRead = NULL, BOOL bBypassCache = FALSE);

	/* 
	 * Opens the specified file
	 * Parameter: LPCTSTR pFilename
	 * Parameter: DWORD access
	 */
	virtual BOOL Open(LPCTSTR pFilename = NULL, DWORD access = 0xffffffff);

	/* 
	 * Opens an existing file.  Fails if file does not exist.
	 * Parameter: LPCTSTR pFilename
	 * Parameter: DWORD access
	 */
	BOOL OpenExisting(LPCTSTR pFilename = NULL, DWORD access = 0xffffffff);

	/* 
	 * Opens a new file.  If file exists it is truncated.
	 * Parameter: LPCTSTR pFilename
	 * Parameter: DWORD access
	 */
	BOOL OpenNew(LPCTSTR pFilename = NULL, DWORD access = 0xffffffff);

	/* 
	 * Opens new or existing file.
	 * Parameter: LPCTSTR pFilename
	 * Parameter: DWORD access
	 */
	BOOL OpenAlways(LPCTSTR pFilename = NULL, DWORD access = 0xffffffff);

	/* 
	 * Sets default file open parameters
	 */
	void DefaultOpenParams();

	/// File open parameters
	OPENFILEPARAMS		m_ofp;

	/// File security attributes
	SECURITY_ATTRIBUTES	m_saSecurityAttributes;

public:

	/*
	 * Attempts to delete temporary directories and files
	 * Parameter: HWND hWnd
	 * Parameter: DWORD dwAttempts
	 */
	static BOOL VerboseCleanup(HWND hWnd = NULL, DWORD dwAttempts = 5);

	/* 
	 * Creates the named temporary directory
	 * Parameter: LPSTR pDir
	 */
	static BOOL CreateTempDir(LPSTR pDir);

	/* 
	 * Returns non-zero if a custom temporary path is specified
	 */
	static BOOL IsTempPath() { return (*m_szTempPath != 0x0); }

	/* 
	 * Deletes temporary files from custom temporary path
	 */
	static BOOL CleanTempPath();

	/* 
	 * Deletes the custom temporary path and all contained files
	 */
	static BOOL DeleteTempPath();

	/* 
	 * Sets the custom temporary path
	 * Parameter: LPCTSTR pPath
	 */
	static BOOL SetTempPath(LPCTSTR pPath = NULL);

	/* 
	 * Returns a pointer to the custom temporary path
	 */
	static LPCTSTR GetTempPath() { return m_szTempPath; }
public:

	/* 
	 * FlushWriteCache
	 */
	void FlushWriteCache();

	/* 
	 * Flushes the software read cache
	 */
	void FlushReadCache();

	/* 
	 * Reads data from the cache
	 * Parameter: LPVOID pBuffer
	 * Parameter: DWORD dwLength
	 * Parameter: LPDWORD pdwRead
	 */
	BOOL ReadCache(LPVOID pBuffer, DWORD dwLength, LPDWORD pdwRead);

	/* 
	 * Writes data to cache
	 * Parameter: LPVOID pBuffer
	 * Parameter: DWORD dwLength
	 * Parameter: LPDWORD pdwWritten
	 */
	BOOL WriteCache(LPVOID pBuffer, DWORD dwLength, LPDWORD pdwWritten);

	/* 
	 * Returns non-zero if the specified file or directory exists
	 * Parameter: LPCTSTR pFilename
	 */
	static BOOL DoesExist(LPCTSTR pFilename);

	/* 
	 * Returns a pointer to the file extension in pFile
	 * Parameter: LPCTSTR pFile
	 */
	static LPCTSTR GetExtension(LPCTSTR pFile);

	/* 
	 * Displays the Windows Folder picker
	 * Parameter: HWND hWnd
	 * Parameter: LPCTSTR pTitle
	 * Parameter: LPSTR pDir
	 * Parameter: LPCTSTR pInit
	 */
	static BOOL GetBrowseDir(HWND hWnd, LPCTSTR pTitle, LPSTR pDir, LPCTSTR pInit = NULL);

	/* 
	 * Callback procedure used by GetBrowseDir() to set the initial folder
	 * Parameter: HWND hwnd
	 * Parameter: UINT uMsg
	 * Parameter: LPARAM lParam
	 * Parameter: LPARAM lpData
	 */
	static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

	/// Returns the path to special windows folders
	/**
	\param [in] nID		-	Special folder ID
	\param [out] buf	-	Receives the path to the specified folder

	Codes with possible example folders...

	<b>Extra folders</b>

	-	CSIDL_USER_SYSTEM	-	System 32 folder
	-	C:\\Windows\\System32

	-	CSIDL_USER_WINDOWS	-	Windows folder
	-	C:\\Windows

	-	CSIDL_USER_TEMP		-	Windows temporary directory
	-	C:\\Temp

	-	CSIDL_USER_CURRENT	-	Current folder

	-	CSIDL_USER_DEFDRIVE	-	Drive that windows is installed on.
	-	C:\\

	<b>Default folders</b>

	-	CSIDL_ALTSTARTUP				-	File system directory that corresponds to the
	user's nonlocalized Startup program group.

	-	CSIDL_APPDATA					-	File system directory that serves as a common
	repository for application-specific data.

	-	CSIDL_BITBUCKET					-	File system directory containing file objects
	in the user's Recycle Bin. The location of this
	directory is not in the registry; it is marked
	with the hidden and system attributes to prevent
	the user from moving or deleting it.

	-	CSIDL_COMMON_ALTSTARTUP			-	File system directory that corresponds to the
	nonlocalized Startup program group for all users.

	-	CSIDL_COMMON_DESKTOPDIRECTORY	-	File system directory that contains files and
	folders that appear on the desktop for all users.

	-	CSIDL_COMMON_FAVORITES			-	File system directory that serves as a common
	repository for all users' favorite items.

	-	CSIDL_COMMON_PROGRAMS			-	File system directory that contains the
	directories for the common program groups that
	appear on the Start menu for all users.

	-	CSIDL_COMMON_STARTMENU			-	File system directory that contains the programs
	and folders that appear on the Start menu for
	all users.

	-	CSIDL_COMMON_STARTUP			-	File system directory that contains the programs
	that appear in the Startup folder for all users.

	-	CSIDL_CONTROLS					-	Virtual folder containing icons for the Control
	Panel applications.

	-	CSIDL_COOKIES					-	File system directory that serves as a common
	repository for Internet cookies.

	-	CSIDL_DESKTOP					-	Windows Desktop—virtual folder at the root of
	the namespace.

	-	CSIDL_DESKTOPDIRECTORY			-	File system directory used to physically store
	file objects on the desktop (not to be confused
	with the desktop folder itself).

	-	CSIDL_DRIVES					-	My Computer—virtual folder containing everything
	on the local computer: storage devices, printers,
	and Control Panel. The folder may also contain
	mapped network drives.

	-	CSIDL_FAVORITES					-	File system directory that serves as a common
	repository for the user's favorite items.

	-	CSIDL_FONTS						-	Virtual folder containing fonts.

	-	CSIDL_HISTORY					-	File system directory that serves as a common
	repository for Internet history items.

	-	CSIDL_INTERNET					-	Virtual folder representing the Internet.

	-	CSIDL_INTERNET_CACHE			-	File system directory that serves as a common
	repository for temporary Internet files.

	-	CSIDL_NETHOOD					-	File system directory containing objects that
	appear in the network neighborhood.

	-	CSIDL_NETWORK					-	Network Neighborhood Folder—virtual folder
	representing the top level of the network
	hierarchy.

	-	CSIDL_PERSONAL					-	File system directory that serves as a common
	repository for documents.

	-	CSIDL_PRINTERS					-	Virtual folder containing installed printers.

	-	CSIDL_PRINTHOOD					-	File system directory that serves as a common
	repository for printer links.

	-	CSIDL_PROGRAMS					-	File system directory that contains the user's
	program groups (which are also file system
	directories).

	-	CSIDL_RECENT					-	File system directory that contains the user's
	most recently used documents.

	-	CSIDL_SENDTO					-	File system directory that contains Send To
	menu items.

	-	CSIDL_STARTMENU					-	File system directory containing Start menu
	items.

	-	CSIDL_STARTUP					-	File system directory that corresponds to the
	user's Startup program group. The system starts
	these programs whenever any user logs onto
	Windows NT or starts Windows 95.

	-	CSIDL_TEMPLATES					-	File system directory that serves as a common
	repository for document templates.

	\return Non-zero if valid folder is returned.

	\see
	*/
	static BOOL GetSpecialFolder(int nID, LPTSTR buf);

	/* 
	 * Displays the last Windows error returned by ::GetLastError()
	 * Parameter: LPCTSTR pTitle
	 * Parameter: LPCTSTR pAdd
	 * Parameter: UINT type
	 */
	static int ShowLastError(LPCTSTR pTitle, LPCTSTR pAdd = NULL, UINT type = MB_OK | MB_ICONEXCLAMATION);

	/* 
	 * Returns a human readable string describing a Windows error code
	 * Parameter: DWORD err
	 * Parameter: LPSTR pMsg
	 * Parameter: LPCTSTR pTemplate
	 */
	static BOOL GetSystemErrorMsg(DWORD err, LPSTR pMsg, LPCTSTR pTemplate = TEXT("Error Code: %lu\r\n%s"));
	
	/* 
	 * Returns the total disk space on the specified drive
	 * Parameter: LPCTSTR pDrive
	 * Parameter: double * total
	 * Parameter: double * free
	 */
	static BOOL GetDiskSpace(LPCTSTR pDrive, double *total, double *free);

	/* 
	 * Creates a shortened string representing the specified quantity
	 * Parameter: LPSTR pString
	 * Parameter: double size
	 */
	static BOOL CreateSizeString(LPSTR pString, double size);
private:
	/// Handle to encapsulated file
	HANDLE				m_hFile;
	
	/// Full file path
	TCHAR				m_szPath[CWF_STRSIZE];

	/// Path to temporary folder
	static TCHAR		m_szTempPath[CWF_STRSIZE];

	// Non-zero to enable file cache
	BOOL				m_bEnableCache;

	/// Write cache position pointer
	DWORD				m_dwWrCachePtr;

	/// Pointer to write cache buffer
	LPBYTE				m_pucWrCache;

	/// Size of buffer in m_pucWrCache
	DWORD				m_dwWrCacheSize;

	/// Read cache position pointer
	DWORD				m_dwRdCachePtr;

	/// Number of bytes in the read caceh
	DWORD				m_dwRdCacheBytes;

	/// Pointer to read cache buffer
	LPBYTE				m_pucRdCache;

	/// Size of buffer in m_pucRdCache
	DWORD				m_dwRdCacheSize;

public:

	/* 
	 * Generates a repeatable but indecipherable file name from pFilename
	 * Parameter: LPCTSTR pFilename
	 * Parameter: LPCTSTR pDir
	 * Parameter: LPSTR pCacheFilename
	 */
	static BOOL GetCacheFileName(LPCTSTR pFilename, LPCTSTR pDir, LPSTR pCacheFilename);

	/* 
	 * Attempts to match filename against multiple patterns
	 * Parameter: LPCTSTR pFile
	 * Parameter: LPCTSTR pPattern
	 */
	static BOOL MatchPattern(LPCTSTR pFile, LPCTSTR pPattern);

	/*
	* Attempts to match filename against multiple patterns
	* Parameter: LPCTSTR pFile
	* Parameter: LPCTSTR pPattern
	* Parameter: TCHAR cSep = ';'
	* Parameter: BOOL bAll = FALSE
	*/
	static BOOL MatchPatterns(LPCTSTR pFile, LPCTSTR pPattern, TCHAR cSep = ';', BOOL bAll = FALSE);

	/* 
	 * Returns non-zero if the character is a valid file path character
	 * Parameter: TCHAR ch
	 */
	static BOOL IsValidPathChar(TCHAR ch);

	/* 
	 * Returns non-zero if the character is a valid filename character
	 * Parameter: BYTE ch
	 */
	static BOOL IsValidFileChar(TCHAR ch);

	/* 
	 * GetFileCount
	 * Parameter: LPCTSTR pPath
	 * Parameter: LPCTSTR pMask
	 */
	static DWORD GetFileCount(LPCTSTR pPath, LPCTSTR pMask = TEXT("*.*"));

	/* 
	 * Copies the files in one folder to another
	 * Parameter: LPCTSTR pSrc
	 * Parameter: LPCTSTR pDst
	 * Parameter: BOOL bSubFolders
	 */
	static BOOL CopyFolder(LPCTSTR pSrc, LPCTSTR pDst, BOOL bSubFolders = TRUE);

	/* 
	 * Returns non-zero if the named path is a file.
	 * Parameter: LPCTSTR pFile
	 */
	static BOOL IsFile(LPCTSTR pFile);

	/* 
	 * Compares the named files extension
	 * Parameter: LPCTSTR pFile
	 * Parameter: LPCTSTR pExt
	 * Parameter: DWORD len
	 */
	static BOOL CmpExt(LPCTSTR pFile, LPCTSTR pExt, DWORD len = 3);

	/* 
	 * Returns the specified path element
	 * Parameter: LPSTR pElement
	 * Parameter: LPCTSTR pPath
	 * Parameter: DWORD dwElement
	 * Parameter: LPDWORD pLeft
	 * Parameter: BOOL bReverse
	 */
	static LPCTSTR GetPathElement(LPSTR pElement, LPCTSTR pPath, DWORD dwElement, LPDWORD pLeft = NULL, BOOL bReverse = FALSE);

	/* 
	 * Quotes a given string escaping as needed
	 * Parameter: LPSTR dst
	 * Parameter: LPCTSTR src
	 * Parameter: char ch
	 * Parameter: char esc
	 */
	static BOOL Quote(LPSTR dst, LPCTSTR src = NULL, char ch = '\"', char esc = '\\');

	/* 
	 * Gets the folder path for the current application
	 * Parameter: LPSTR buf
	 * Parameter: DWORD size
	 */
	static LPCTSTR GetExePath(LPSTR buf, DWORD size = CWF_STRSIZE);

	/* 
	 * Removes the specified path, can delay until reboot if in use
	 * Parameter: LPCTSTR pPath
	 * Parameter: BOOL bReboot
	 * Parameter: BOOL bEmptyOnly
	 */
	static BOOL DeletePath(LPCTSTR pPath, BOOL bReboot, BOOL bEmptyOnly);

	/* 
	 * Gets the human readable name of a Windows special folder
	 * Parameter: int nID
	 * Parameter: LPSTR pName
	 * Parameter: LPDWORD pdwFlags
	 * Parameter: DWORD dwIndex
	 * Parameter: int * pnID
	 */
	static BOOL GetSpecialFolderName(int nID, LPSTR pName, LPDWORD pdwFlags = NULL, DWORD dwIndex = 0, int *pnID = NULL);
};

