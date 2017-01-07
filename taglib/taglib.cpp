// taglib.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "taglib.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
HWND g_hwnd;
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

//
HWND GetHWND();
VOID Win32FuncLearn();
VOID ReadPrgProperty(HWND hWnd);
VOID WritePrgProperty();
VOID OpenPath(LPCTSTR path);

VOID testWinFile();
VOID testCopyFile();

//stlsoft
VOID testStlSoft();


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_TAGLIB, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TAGLIB));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TAGLIB));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_TAGLIB);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable 

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_HSCROLL,
	   CW_USEDEFAULT, CW_USEDEFAULT, 500, 300, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   
   g_hwnd = hWnd;

   //Win32FuncLearn();
      //read prev properties

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   //testWinFile();
   //testCopyFile();
   testStlSoft();

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		ReadPrgProperty(hWnd);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		WritePrgProperty();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

HWND GetHWND()
{
	assert(g_hwnd);
	return g_hwnd;
}

VOID Win32FuncLearn()
{
	DWORD retval = 0;
	CString filePath = "/select, D:\\cpp\\win32\\taglib\\Debug\\taglib.exe";

	ShellExecute(NULL, _T("open"), _T("explorer.exe"), LPCTSTR(filePath), NULL, SW_SHOWNORMAL);
}

VOID WritePrgProperty()
{
	WINDOWPLACEMENT wp;
	GetWindowPlacement(GetHWND(), &wp);
	WritePrivateProfileStruct(L"Main", L"WP", &wp, sizeof(wp), _T("D:\\work\\cpp\\taglib\\Debug\\taglib.ini"));
}

VOID ReadPrgProperty(HWND hWnd)
{
	WINDOWPLACEMENT wp;
	GetPrivateProfileStruct(L"Main", L"WP", &wp, sizeof(wp), _T("D:\\work\\cpp\\taglib\\Debug\\taglib.ini"));
	SetWindowPlacement(hWnd, &wp);
}

VOID testWinFile()
{
	CWinFile winFile;
	winFile.Open(TEXT("D:\\work\\cpp\\taglib\\taglib\\test.txt"));

	LONGLONG pos = winFile.GetPtrPos();
	TCHAR data[] = TEXT("My name is Yijtx\r\n");
	TCHAR chData[] = TEXT("我是Yijtx");
	DWORD writeen;
	WORD a = 0xFEFF;
	winFile.Write(&a, sizeof(a), &writeen);//在写入前插入0xFEFF，让系统认为是Unicode编码。
	winFile.Write(data, lstrlen(data)*sizeof(TCHAR), &writeen);
	winFile.Write(chData, lstrlen(chData)*sizeof(TCHAR), &writeen);
	pos = winFile.GetPtrPos();
	winFile.Close();

	TCHAR userHome[MAX_PATH];
	ZeroMemory(userHome, _tcslen(userHome));
	winFile.GetSpecialFolder(CSIDL_USER_WINDOWS, userHome);
}

VOID testCopyFile()
{
	CWinFile srcFile, dstFile;
	LONGLONG pos = 0;
	srcFile.Open(TEXT("D:\\work\\cpp\\taglib\\taglib\\test.txt"));
	TCHAR buf[BUFSIZE];
	ZeroMemory(buf, BUFSIZE);

	srcFile.Read(buf, 1024);
	pos = srcFile.GetPtrPos();
	srcFile.SetPtrPosBegin();
	pos = srcFile.GetPtrPos();

	dstFile.OpenNew(TEXT("D:\\work\\cpp\\taglib\\taglib\\test_d.txt"));
	//OpenPath(dstFile.GetPath());
	dstFile.CopyFile(srcFile.GetHandle());
	pos = dstFile.GetPtrPos();
	printf("%d", pos);
}

VOID OpenPath(LPCTSTR path)
{
	TCHAR selecPath[MAX_PATH] = TEXT("/select, ");
	_tcscat(selecPath, path);
	ShellExecute(NULL, _T("open"), _T("explorer.exe"), selecPath, NULL, SW_SHOWNORMAL);
}

VOID testStlSoft()
{
	using vole::object;
	using vole::collection;
	using vole::of_type;


	// Q238393: Get CLSID for Word.Application ...
	// VOLE: Don't need to do that
	// Q238393: Start Word and get IDispatch...

	// VOLE: Just create in a vole::object. No need to get raw IDispatch pointer.
	//       Note that we must explicitly specify CLSCTX_LOCAL_SERVER, just as
	//       in Q238393
	object  word = object::create("Word.Application", CLSCTX_LOCAL_SERVER, vole::coercion_level::naturalPromotion);
	try
	{
		// Q238393: Make Word visible
		word.put_property(L"Visible", true);

		// Q238393: Get Documents collection
		// VOLE: Rather than dealing with raw pointers, we just return an instance
		//       of vole::collection.
		collection  documents = word.get_property(of_type<collection>(), L"Documents");

		// Q238393: Call Documents.Open() to open C:\Doc1.doc

		// VOLE: Invoke the method, returning an instance of vole::object that holds
		//       the document object.
		object      document = documents.invoke_method(of_type<object>(), L"Open", L"D:\\Doc1.doc");
		try
		{
			// Q238393: Get BuiltinDocumentProperties collection
			collection  builtInProps = document.get_property(of_type<collection>(), L"BuiltinDocumentProperties");
#ifdef _DEBUG
			int     numProps1 = builtInProps.get_Count();
			double  numProps2 = builtInProps.get_property(of_type<double>(), L"Count");

			STLSOFT_SUPPRESS_UNUSED(numProps1);
			STLSOFT_SUPPRESS_UNUSED(numProps2);
#endif /* _DEBUG */


			// Q238393: Get "Subject" from BuiltInDocumentProperties.Item("Subject")
			object      propSubject = builtInProps.get_property(of_type<object>(), L"Item", L"Subject");

			// Q238393: Get the Value of the Subject property and display it
			std::string subject = propSubject.get_property(of_type<std::string>(), L"Value");
			std::cout << "Subject property: \"" << subject << '"' << std::endl;

			// Q238393: Set the Value of the Subject DocumentProperty
			propSubject.put_property(L"Value", "This is my subject");

			// Q238393: Get CustomDocumentProperties collection
			collection  customProps = document.get_property(of_type<collection>(), L"CustomDocumentProperties");
			
			// Q238393: Add a new property named "CurrentYear"
			customProps.invoke_method_v(L"Add", L"CurrentYear", false, 1, 1999);

			// Q238393: Get the custom property "CurrentYear" and delete it
			object      propCurrYear = customProps.get_property(of_type<object>(), L"Item", L"CurrentYear");

			propCurrYear.invoke_method_v(L"Delete");

			// Q238393: Close the document without saving changes and quit Word
			document.invoke_method_v(L"Close", false);

			word.invoke_method_v(L"Quit");
		}
		catch (comstl::com_exception&)
		{
			try
			{
				document.invoke_method_v(L"Close", false);
			}
			catch (...)
			{
			}

			throw;
		}
	}
	catch (comstl::com_exception&)
	{
		word.invoke_method_v(L"Quit");

		throw;
	}
}