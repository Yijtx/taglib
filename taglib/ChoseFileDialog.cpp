#include "stdafx.h"
#include "ChoseFileDialog.h"


BOOL ChooseDirectory(HWND parent, PCTSTR tips, PTCH dirPath)
{
	LPITEMIDLIST pidl = NULL;
	SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pidl);//Ä¬ÈÏÎ»ÖÃ
	BROWSEINFO bi = { parent, pidl, dirPath, tips, BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE, FileDailogBrowseCallbackProc, (LPARAM)dirPath, };
	LPITEMIDLIST idlist = SHBrowseForFolder(&bi);
	if (idlist && SHGetPathFromIDList(idlist, dirPath))
	{
		IMalloc *m;
		SHGetMalloc(&m);
		m->Free(idlist);
		return true;
	}
	return false;
}

static int CALLBACK WINAPI FileDailogBrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch (uMsg)
	{
		case BFFM_INITIALIZED:
			if (lpData && ((char *)lpData)[0])
			{
				SendMessage(hwnd, BFFM_SETSELECTION, 1, lpData);
			}
			break;
		case BFFM_SELCHANGED:
			LOG(INFO) << "BFFM_SELCHANGED";
			break;
	}
	return 0;
}