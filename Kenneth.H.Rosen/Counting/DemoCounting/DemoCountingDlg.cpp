
// DemoCountingDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "DemoCounting.h"
#include "DemoCountingDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <windows.h>
#include <gdiplus.h>
#include <string>

using namespace Gdiplus;

#pragma comment(lib, "gdiplus.lib")
static GdiplusStartupInput gdiplusStartupInput;
static ULONG_PTR gdiplusToken;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDemoCountingDlg dialog



CDemoCountingDlg::CDemoCountingDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DEMOCOUNTING_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDemoCountingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDemoCountingDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CDemoCountingDlg message handlers

BOOL CDemoCountingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDemoCountingDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.


void CDemoCountingDlg::OnClose() {
	GdiplusShutdown(gdiplusToken);
	CDialogEx::OnClose();
}
void CDemoCountingDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect rect;
	GetClientRect(&rect);
	HDC hdc = dc.GetSafeHdc();
	Gdiplus::Graphics graphics(hdc);
	// Create a font and brush to draw the text
	//::Font font(L"Arial", 24);          // Font name "Arial" and size 24
	//CFont font;
	Gdiplus::Font font(L"Arial", 24);
	//CClientDC dcd(this);
	//VERIFY(font.CreatePointFont(120, _T("Arial"), &dcd));
	SolidBrush brush(Color(255, 0, 0, 0)); // Black color brush
	if (IsIconic())
	{
		//CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		//CRect rect;
		//GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
	HBRUSH hBrush;
	hBrush = CreateSolidBrush(RGB(0, 200, 0));
	int padding = 30;
	int x0 = 100, y0 = 0;
	int height_revert = rect.bottom;
	height_revert -= 40;
	int thick = 20;
	int t = 0;
	wchar_t textx[10];
	graphics.SetSmoothingMode(SmoothingModeAntiAlias);
	for (int i = 0; i < 10; ++i) {
		t = i;
		dc.SelectStockObject(BLACK_PEN);
		//dc.Rectangle(CRect(x0, height_revert - 0, x0+ thick, height_revert - (y0 + i * 20)));
		FillRect(hdc, CRect(x0, height_revert - 0, x0 + thick, height_revert - (y0 + t * 40) - 20), hBrush);
		_snwprintf(textx, 10, _T("%d"), i);
		graphics.DrawString(textx, -1, &font, Gdiplus::PointF(x0 - 3, height_revert - 0), &brush);
		x0 += thick;
		x0 += padding;
	}
	//std::wstring fontNameText = L"Font: Arial";
	
	//graphics.DrawString(fontNameText.c_str(), -1, &font, PointF(50.0f, 50.0f), &brush);
	
	//graphics.DrawString()
	//graphics.DrawString()
	//font.DeleteObject();
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDemoCountingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

