// ConnectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConnectDlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include "Main.h"

// CConnectDlg dialog

IMPLEMENT_DYNAMIC(CConnectDlg, CDialog)

CConnectDlg::CConnectDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_CONNECT, pParent)
{
    m_relayType = theApp.GetProfileInt(REG_SECTION_LOGIN, REG_ENTRY_TYPE, 0);
    m_relayHost = theApp.GetProfileString(REG_SECTION_LOGIN, REG_ENTRY_HOST);
    m_relayPort = theApp.GetProfileInt(REG_SECTION_LOGIN, REG_ENTRY_PORT, 0);
    m_relayPass = theApp.GetProfileString(REG_SECTION_LOGIN, REG_ENTRY_PASS);
}

CConnectDlg::~CConnectDlg()
{
}

void CConnectDlg::OnOK()
{
    UpdateData(TRUE);

    // Ensure that the UI got the necessary input  
    // from the user before closing the dialog. The  
    // default OnOK will close this. 
    if (m_relayHost.IsEmpty())
    {
        AfxMessageBox(_T("Please enter a host"));
        return;
    }
    else if (m_relayPort == 0 || m_relayPort > 65535)
    {
        AfxMessageBox(_T("Please enter a valid port"));
        return;
    }

    CDialog::OnOK(); // This will close the dialog and DoModal will return.
}

void CConnectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_CBIndex(pDX, IDC_CONNTYPE, m_relayType);
    DDX_Text(pDX, IDC_HOST, m_relayHost);
    DDX_Text(pDX, IDC_PORT, m_relayPort);
    DDX_Text(pDX, IDC_PASS, m_relayPass);
}


BEGIN_MESSAGE_MAP(CConnectDlg, CDialog)
END_MESSAGE_MAP()


// CConnectDlg message handlers
