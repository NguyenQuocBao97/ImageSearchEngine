// 1553001_1553003_DoAn.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ImageSearchEngine.h"

#define MAX_LOADSTRING 100

HINSTANCE hInst;
HWND hwndDlg;
HWND _progressBar = {};
HANDLE startThread;
bool clear = false;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hWnd, hCombo;
TCHAR tempPath[MAX_PATH];
BOOL                InitInstance(HINSTANCE, int);
INT_PTR CALLBACK    dlgProc(HWND, UINT, WPARAM, LPARAM);
string simagePath, sinputPath, soutputPath;
Mat image;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY1553001_1553003_DOAN, szWindowClass, MAX_LOADSTRING);
   // MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY1553001_1553003_DOAN));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
  
   DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, dlgProc);
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

INT_PTR CALLBACK dlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
	{
		hwndDlg = hDlg;
		hCombo = GetDlgItem(hDlg, IDC_COMBO1);

		for (int i = 1; i <= 25; ++i) {
			WCHAR buff[10];
			wsprintf(buff, L"%d", i);
			ComboBox_AddString(hCombo, buff);

		}
		
		ComboBox_SetCurSel(hCombo, 10);
		break;
	}
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			PostQuitMessage(0);
            return (INT_PTR)TRUE;
		case IDC_BUTTON1:
			OnBrowseCopyDestinate(hDlg, L"Choose Folder", IDC_EDIT1);
			break;
		case IDC_BUTTON2:
			OnBrowseCopyDestinate(hDlg, L"Choose Folder", IDC_EDIT2);
			break; 
		case IDC_BUTTON3:
		{
			simagePath = "";
			OnOpenCopyDestination(hDlg, L"Choose Image", IDC_EDIT3);
			GetDlgItemText(hDlg, IDC_EDIT3, tempPath, MAX_PATH);
			for (auto i : tempPath) {
				simagePath.push_back(i);
			}
			string2String(simagePath);
			image = imread(simagePath, CV_LOAD_IMAGE_COLOR);
			Mat image_bmp;
			image.convertTo(image_bmp, CV_8UC3);
			imwrite("png2bmp.bmp", image_bmp);
			HBITMAP hBitmap = reinterpret_cast<HBITMAP>(LoadImage(
				NULL, L"png2bmp.bmp", IMAGE_BITMAP, 175, 200, LR_LOADFROMFILE));
			SendDlgItemMessage(hDlg, IDC_IMAGE, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(hBitmap));
			break;
		}
		case IDC_START:
		{
			if (GetWindowTextLength(GetDlgItem(hDlg, IDC_EDIT1)) == 0 || 
				GetWindowTextLength(GetDlgItem(hDlg, IDC_EDIT2)) == 0 || 
				GetWindowTextLength(GetDlgItem(hDlg, IDC_EDIT3)) == 0) {
				MessageBox(hWnd, L"Edit boxes are not fully filled\nPlease try again", L"Error!!!", MB_OK);
				break;
			}
			sinputPath = soutputPath = "";
			EnableWindow(hDlg, false);
			int confirm = MessageBox(hDlg, L"Do you want to erase old image file in output folder", L"", MB_YESNO);
			if (confirm == IDYES)
				clear = true;
			else
				clear = false;
			GetDlgItemText(hDlg, IDC_EDIT1, tempPath, MAX_PATH);
			for (auto i : tempPath) {
				sinputPath.push_back(i);
			}
			GetDlgItemText(hDlg, IDC_EDIT2, tempPath, MAX_PATH);
			for (auto i : tempPath) {
				soutputPath.push_back(i);
			}

			ShowWindow(GetDlgItem(hDlg, IDC_STATE), SW_SHOW);
			UpdateWindow(hDlg);
			disableItem(hDlg);

			vector<Mat> result;
			_searchImage(image, sinputPath, result, ComboBox_GetCurSel(hCombo) + 1);
			
			SetDlgItemText(hDlg, IDC_STATE, L"Done!");
			namedWindow("Input Image");
			namedWindow("Query Image");

			Mat showImg;
			makeCanvas(result, 700, showImg);

			imshow("Input Image", image);
			imshow("Query Image", showImg);
			disableItem(hDlg, true);
			save2File(result, soutputPath);
			ShowWindow(GetDlgItem(hDlg, IDC_STATIC7), SW_SHOW);
			ShowWindow(GetDlgItem(hDlg, IDC_STATIC8), SW_SHOW);
			ShowWindow(GetDlgItem(hDlg, IDC_BUTTON4), SW_SHOW);
			EnableWindow(hDlg, true);
			break;
		}
		case IDC_BUTTON4:
			ShellExecute(NULL, L"open", tempPath, NULL, NULL, SW_SHOW);
			break;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


