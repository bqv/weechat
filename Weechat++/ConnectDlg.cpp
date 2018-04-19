// ConnectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConnectDlg.h"
#include "afxdialogex.h"
#include "resource.h"


// CConnectDlg dialog

IMPLEMENT_DYNAMIC(CConnectDlg, CDialog)

CConnectDlg::CConnectDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_CONNECT, pParent)
{
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
    DDX_Text(pDX, IDC_HOST, m_relayHost);
    DDX_Text(pDX, IDC_PORT, m_relayPort);
    DDX_Text(pDX, IDC_PASS, m_relayPass);
}


BEGIN_MESSAGE_MAP(CConnectDlg, CDialog)
END_MESSAGE_MAP()


// CConnectDlg message handlers
