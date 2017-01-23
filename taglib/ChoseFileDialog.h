#pragma once

BOOL ChooseDirectory(HWND parent, PCTSTR tips, PTCH dirPath);

static int CALLBACK WINAPI FileDailogBrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
