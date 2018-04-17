
#include "stdafx.h"
#include "SslSocket.h"

#pragma comment(lib, "crypt32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSslSocket

void CSslSocket::SetLastError(DWORD dwErrCode)				//this method and following macro will
{															//provide trace output of error description
    m_dwLastError = dwErrCode;								//with file name and line number where
    ::SetLastError(dwErrCode);								//error occures - it works correctly
}															//only in this order

#ifdef _DEBUG
#define SetLastError(err) DumpErrorCode(err,__LINE__,__FILE__); SetLastError(err)
#endif

IMPLEMENT_DYNAMIC(CSslSocket, CAsyncSocket)

CSslSocket::CSslSocket(BOOL bNoVerify) :
    m_hMyCertStore(NULL),
    m_dwLastError(0),
    m_dwProtocol(0),
    m_bMachineStore(FALSE),
    m_hSecurity(NULL),
    m_bServer(FALSE),
    m_bAuthClient(FALSE),
    m_bConInit(FALSE),
    m_pbReceiveBuf(NULL),
    m_dwReceiveBuf(0),
    m_bAllowPlainText(FALSE),
    m_pbIoBuffer(NULL),
    m_cbIoBuffer(0),
    m_pCertContext(NULL),
    m_bNoVerify(bNoVerify)
{
    ZeroMemory(&m_SchannelCred, sizeof(m_SchannelCred));

    m_hCreds.dwLower = 0;
    m_hCreds.dwUpper = 0;

    m_hContext.dwLower = 0;
    m_hContext.dwUpper = 0;
}

CSslSocket::~CSslSocket()
{
    if ((m_hCreds.dwLower != 0) && (m_hCreds.dwUpper != 0)) {
        m_SecurityFunc.FreeCredentialsHandle(&m_hCreds);
    }
    if ((m_hContext.dwLower != 0) && (m_hContext.dwUpper != 0)) {
        m_SecurityFunc.DeleteSecurityContext(&m_hContext);
    }
    if (m_pCertContext) CertFreeCertificateContext(m_pCertContext);
    if (m_hMyCertStore) CertCloseStore(m_hMyCertStore, 0);
    if (m_hSecurity) FreeLibrary(m_hSecurity);
    if (m_pbReceiveBuf) delete[] m_pbReceiveBuf;
    if (m_pbIoBuffer) delete[] m_pbIoBuffer;
}

// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CSslSocket, CAsyncSocket)
    //{{AFX_MSG_MAP(CSslSocket)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CSslSocket member functions

int CSslSocket::Send(const void* lpBuf, int nBufLen, int nFlags)
{
    int rc = 0;
    SecPkgContext_StreamSizes Sizes;
    SECURITY_STATUS scRet;
    SecBufferDesc   Message;
    SecBuffer       Buffers[4];

    SecBuffer *     pDataBuffer;
    SecBuffer *     pExtraBuffer;

    PBYTE pbIoBuffer = NULL;
    DWORD cbIoBufferLength;
    PBYTE pbMessage;
    DWORD cbMessage;

    DWORD dwAvaLn = 0;
    DWORD dwDataToSend = 0;
    DWORD dwSendInd = 0;
    DWORD dwCurrLn = 0;

    DWORD dwTotSent = 0;

    if (m_bConInit) {

        do {

            scRet = m_SecurityFunc.QueryContextAttributes(&m_hContext, SECPKG_ATTR_STREAM_SIZES, &Sizes);
            if (scRet != SEC_E_OK) {
                SetLastError(scRet);
                break;
            }

            cbIoBufferLength = Sizes.cbHeader +
                Sizes.cbMaximumMessage +
                Sizes.cbTrailer;

            pbIoBuffer = new BYTE[cbIoBufferLength];
            if (pbIoBuffer == NULL) {
                SetLastError(ERROR_OUTOFMEMORY);
                break;
            }

            pbMessage = pbIoBuffer + Sizes.cbHeader;
            dwAvaLn = Sizes.cbMaximumMessage;
            dwDataToSend = ((DWORD)nBufLen);

            do {
                ZeroMemory(pbIoBuffer, cbIoBufferLength);
                dwCurrLn = ((DWORD)nBufLen) - dwSendInd > dwAvaLn ? dwAvaLn : (((DWORD)nBufLen) - dwSendInd);

                CopyMemory(pbMessage, ((BYTE*)lpBuf) + dwSendInd, dwCurrLn);

                DUMP(_T("Plain text to send\n"), pbMessage, dwCurrLn);

                dwSendInd += dwCurrLn;
                dwDataToSend -= dwCurrLn;

                cbMessage = dwCurrLn;

                Buffers[0].pvBuffer = pbIoBuffer;
                Buffers[0].cbBuffer = Sizes.cbHeader;
                Buffers[0].BufferType = SECBUFFER_STREAM_HEADER;

                Buffers[1].pvBuffer = pbMessage;
                Buffers[1].cbBuffer = cbMessage;
                Buffers[1].BufferType = SECBUFFER_DATA;

                Buffers[2].pvBuffer = pbMessage + cbMessage;
                Buffers[2].cbBuffer = Sizes.cbTrailer;
                Buffers[2].BufferType = SECBUFFER_STREAM_TRAILER;

                Buffers[3].BufferType = SECBUFFER_EMPTY;

                Message.ulVersion = SECBUFFER_VERSION;
                Message.cBuffers = 4;
                Message.pBuffers = Buffers;

                scRet = m_SecurityFunc.EncryptMessage(&m_hContext, 0, &Message, 0);

                if (FAILED(scRet)) {
                    SetLastError(scRet);
                    break;
                }

                pDataBuffer = NULL;
                pExtraBuffer = NULL;
                for (int i = 1; i < 4; i++) {
                    if (pDataBuffer == NULL && Buffers[i].BufferType == SECBUFFER_DATA) {
                        pDataBuffer = &Buffers[i];
                    }
                    if (pExtraBuffer == NULL && Buffers[i].BufferType == SECBUFFER_EXTRA) {
                        pExtraBuffer = &Buffers[i];
                    }
                }

                DUMP(_T("Cipher text to send\n"), pbIoBuffer, Buffers[0].cbBuffer + Buffers[1].cbBuffer + Buffers[2].cbBuffer);

                rc = CAsyncSocket::Send(pbIoBuffer, Buffers[0].cbBuffer + Buffers[1].cbBuffer + Buffers[2].cbBuffer, nFlags);

                if ((rc == SOCKET_ERROR) && (WSAGetLastError() == WSAEWOULDBLOCK)) {
                    rc = nBufLen;
                }
                else {
                    if (rc == SOCKET_ERROR) {
                        SetLastError(WSAGetLastError());
                        dwTotSent = rc;
                        break;
                    }
                    else {
                        dwTotSent += rc;
                    }
                }

            } while (dwDataToSend != 0);

        } while (FALSE);

        if (pbIoBuffer) delete[] pbIoBuffer;

    }
    else {
        if (m_bAllowPlainText) {
            rc = CAsyncSocket::Send(lpBuf, nBufLen, nFlags);
            if ((rc == SOCKET_ERROR) && (GetLastError() == WSAEWOULDBLOCK)) {
                rc = nBufLen;
            }
            dwTotSent = rc;
        }
    }

    return dwTotSent > ((DWORD)nBufLen) ? nBufLen : dwTotSent;
}

int CSslSocket::Receive(void* lpBuf, int nBufLen, int nFlags)
{
    int rc = 0;
    SecPkgContext_StreamSizes Sizes;
    SECURITY_STATUS scRet;
    DWORD cbIoBufferLength;
    DWORD cbData;
    SecBufferDesc   Message;
    SecBuffer       Buffers[4];
    SecBuffer *     pDataBuffer;
    SecBuffer *     pExtraBuffer;
    SecBuffer       ExtraBuffer;

    BYTE *pDataBuf = NULL;
    DWORD dwDataLn = 0;
    DWORD dwBufDataLn = 0;
    BOOL bCont = TRUE;

    if (m_bConInit) {
        if (m_dwReceiveBuf) {
            if ((DWORD)nBufLen < m_dwReceiveBuf) {
                rc = nBufLen;
                CopyMemory(lpBuf, m_pbReceiveBuf, rc);
                MoveMemory(m_pbReceiveBuf, m_pbReceiveBuf + rc, m_dwReceiveBuf - rc);
                m_dwReceiveBuf -= rc;
            }
            else {
                rc = m_dwReceiveBuf;
                CopyMemory(lpBuf, m_pbReceiveBuf, rc);
                delete[] m_pbReceiveBuf;
                m_pbReceiveBuf = NULL;
                m_dwReceiveBuf = 0;
            }
        }
        else {

            do {
                scRet = m_SecurityFunc.QueryContextAttributes(&m_hContext, SECPKG_ATTR_STREAM_SIZES, &Sizes);
                if (scRet != SEC_E_OK) {
                    SetLastError(scRet);
                    break;
                }

                cbIoBufferLength = Sizes.cbHeader +
                    Sizes.cbMaximumMessage +
                    Sizes.cbTrailer;

                if (!m_pbIoBuffer) m_pbIoBuffer = new BYTE[cbIoBufferLength];
                pDataBuf = new BYTE[cbIoBufferLength];
                dwBufDataLn = cbIoBufferLength;
                if ((m_pbIoBuffer == NULL) || (pDataBuf == NULL)) {
                    SetLastError(ERROR_OUTOFMEMORY);
                    break;
                }

                do {
                    cbData = CAsyncSocket::Receive(m_pbIoBuffer + m_cbIoBuffer, cbIoBufferLength - m_cbIoBuffer);

                    if (cbData == SOCKET_ERROR) {
                        SetLastError(WSAGetLastError());
                        break;
                    }
                    else if (cbData == 0) {
                        if (m_cbIoBuffer) {
                            TRACE(_T("**** Server unexpectedly disconnected ****\n"));
                            scRet = SEC_E_INTERNAL_ERROR;
                            break;
                        }
                        else {
                            break;
                        }
                    }
                    else {
                        m_cbIoBuffer += cbData;
                    }
                    DUMP(_T("Received cipher text\n"), m_pbIoBuffer, cbData);

                    Buffers[0].pvBuffer = m_pbIoBuffer;
                    Buffers[0].cbBuffer = m_cbIoBuffer;
                    Buffers[0].BufferType = SECBUFFER_DATA;

                    Buffers[1].BufferType = SECBUFFER_EMPTY;
                    Buffers[2].BufferType = SECBUFFER_EMPTY;
                    Buffers[3].BufferType = SECBUFFER_EMPTY;

                    Message.ulVersion = SECBUFFER_VERSION;
                    Message.cBuffers = 4;
                    Message.pBuffers = Buffers;

                    scRet = m_SecurityFunc.DecryptMessage(&m_hContext, &Message, 0, NULL);

                    if (scRet == SEC_E_INCOMPLETE_MESSAGE) {
                        continue;
                    }
                    if (scRet == SEC_I_CONTEXT_EXPIRED) {
                        SetLastError(scRet);
                        break;
                    }
                    if (scRet != SEC_E_OK && scRet != SEC_I_RENEGOTIATE && scRet != SEC_I_CONTEXT_EXPIRED) {
                        SetLastError(scRet);
                        break;
                    }

                    pDataBuffer = NULL;
                    pExtraBuffer = NULL;
                    for (int i = 1; i < 4; i++) {
                        if (pDataBuffer == NULL && Buffers[i].BufferType == SECBUFFER_DATA) {
                            pDataBuffer = &Buffers[i];
                        }
                        if (pExtraBuffer == NULL && Buffers[i].BufferType == SECBUFFER_EXTRA) {
                            pExtraBuffer = &Buffers[i];
                        }
                    }

                    if (pDataBuffer) {
                        DUMP(_T("Plain text received\n"), (BYTE *)(pDataBuffer->pvBuffer), pDataBuffer->cbBuffer);
                        if ((dwDataLn + (pDataBuffer->cbBuffer)) > dwBufDataLn) {
                            BYTE *bNewDataBuf = new BYTE[dwBufDataLn + (pDataBuffer->cbBuffer)];
                            CopyMemory(bNewDataBuf, pDataBuf, dwDataLn);
                            delete[] pDataBuf;
                            pDataBuf = bNewDataBuf;
                            dwBufDataLn = dwBufDataLn + (pDataBuffer->cbBuffer);
                        }
                        CopyMemory(pDataBuf + dwDataLn, pDataBuffer->pvBuffer, pDataBuffer->cbBuffer);
                        dwDataLn += pDataBuffer->cbBuffer;
                    }

                    if (pExtraBuffer) {
                        MoveMemory(m_pbIoBuffer, pExtraBuffer->pvBuffer, pExtraBuffer->cbBuffer);
                        m_cbIoBuffer = pExtraBuffer->cbBuffer;
                        continue;
                    }
                    else {
                        m_cbIoBuffer = 0;
                        bCont = FALSE;
                    }

                    if (scRet == SEC_I_RENEGOTIATE) {

                        scRet = ClientHandshakeLoop(
                            &m_hCreds,
                            &m_hContext,
                            FALSE,
                            &ExtraBuffer);
                        if (scRet != SEC_E_OK) {
                            break;
                        }

                        if (ExtraBuffer.pvBuffer) {
                            MoveMemory(m_pbIoBuffer, ExtraBuffer.pvBuffer, ExtraBuffer.cbBuffer);
                            m_cbIoBuffer = ExtraBuffer.cbBuffer;
                        }

                        if (ExtraBuffer.pvBuffer) delete[] ExtraBuffer.pvBuffer;
                    }
                } while (bCont);

            } while (FALSE);

            if (dwDataLn) {

                if (dwDataLn > (DWORD)nBufLen) {

                    m_dwReceiveBuf = dwDataLn - ((DWORD)(nBufLen));
                    m_pbReceiveBuf = new BYTE[m_dwReceiveBuf];

                    CopyMemory(lpBuf, pDataBuf, nBufLen);
                    rc = nBufLen;

                    CopyMemory(m_pbReceiveBuf, pDataBuf + nBufLen, m_dwReceiveBuf);

                }
                else {
                    CopyMemory(lpBuf, pDataBuf, dwDataLn);
                    rc = dwDataLn;
                }
            }

            if (pDataBuf) delete[] pDataBuf;
        }
    }
    else {
        if (m_bAllowPlainText) rc = CAsyncSocket::Receive(lpBuf, nBufLen, nFlags);
    }

    return rc;
}

BOOL CSslSocket::Create(UINT nSocketPort/* = 0*/, LPCTSTR lpszSocketAddress/* = NULL*/, const TCHAR *szCertName/* = NULL*/, BOOL bMachineStore/* = FALSE*/, DWORD dwProtocol/* = 0*/)
{
    BOOL rc = FALSE;

    rc = CAsyncSocket::Create(nSocketPort, SOCK_STREAM, FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE, lpszSocketAddress);

    if (rc) {
        if (LoadSecurityLibrary()) {
            m_CsCertName = szCertName;
            m_bMachineStore = bMachineStore;
            m_dwProtocol = dwProtocol;
        }
        else {
            rc = FALSE;
        }
    }
    else {
        SetLastError(WSAGetLastError());
    }

    return rc;
}

BOOL CSslSocket::Accept(CSslSocket & rConnectedSocket, SOCKADDR* lpSockAddr/* = NULL*/, int* lpSockAddrLen/* = NULL*/)
{
    BOOL rc = FALSE;

    rc = CAsyncSocket::Accept(rConnectedSocket, lpSockAddr, lpSockAddrLen);

    if (rc) {
        rConnectedSocket.m_bAuthClient = m_bAuthClient;
        rConnectedSocket.m_CsCertName = m_CsCertName;
        rConnectedSocket.m_dwProtocol = m_dwProtocol;
        rConnectedSocket.m_bMachineStore = m_bMachineStore;

        rConnectedSocket.LoadSecurityLibrary();
        rConnectedSocket.ServerCreateCredentials(LPCTSTR(m_CsCertName), &(rConnectedSocket.m_hCreds));
        rc = rConnectedSocket.ServerConnect(lpSockAddr, lpSockAddrLen);
        rConnectedSocket.m_bConInit = rc;
    }

    return rc;
}

void CSslSocket::Close()
{
    if (m_bServer) {
        ServerDisconect(&m_hCreds, &m_hContext);
    }
    else {
        ClientDisconnect(&m_hCreds, &m_hContext);
    }
    CAsyncSocket::Close();
}

BOOL CSslSocket::Connect(LPCTSTR lpszHostAddress, UINT nHostPort)
{
    return CAsyncSocket::Connect(lpszHostAddress, nHostPort);
}

BOOL CSslSocket::Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen)
{
    return CAsyncSocket::Connect(lpSockAddr, nSockAddrLen);
}

BOOL CSslSocket::Listen(int nConnectionBacklog/* = 5*/, BOOL bAuthClient/* = FALSE*/)
{
    m_bAuthClient = bAuthClient;
    m_bServer = TRUE;

    if ((m_hCreds.dwLower == 0) && (m_hCreds.dwUpper == 0)) {
        if (ServerCreateCredentials(LPCTSTR(m_CsCertName), &m_hCreds) == 0) {
            return CAsyncSocket::Listen(nConnectionBacklog);
        }
        else {
            return FALSE;
        }
    }
    else {
        return CAsyncSocket::Listen(nConnectionBacklog);
    }
}

void CSslSocket::OnConnect(int nErrorCode)
{
    if (nErrorCode == 0)
    {
        CString hostname;
        UINT port;
        GetPeerNameEx(hostname, port);
        m_bConInit = ClientConnect(hostname);

        if (m_bConInit)
        {
            CAsyncSocket::OnConnect(nErrorCode);
        }
        else
        {
            Close();
        }
    }
}

DWORD CSslSocket::GetLastError()
{
    return m_dwLastError;
}

BOOL CSslSocket::LoadSecurityLibrary(void)
{
    BOOL rc = FALSE;

    PSecurityFunctionTable  pSecurityFunc;
    INIT_SECURITY_INTERFACE pInitSecurityInterface;

    do {
        m_hSecurity = LoadLibrary(_T("SCHANNEL.DLL"));
        if (m_hSecurity == NULL) {
            SetLastError(::GetLastError());
            break;
        }

#ifdef _UNICODE
        pInitSecurityInterface = (INIT_SECURITY_INTERFACE)GetProcAddress(m_hSecurity, "InitSecurityInterfaceW");
#else
        pInitSecurityInterface = (INIT_SECURITY_INTERFACE)GetProcAddress(m_hSecurity, "InitSecurityInterfaceA");
#endif

        if (pInitSecurityInterface == NULL) {
            SetLastError(::GetLastError());
            break;
        }

        pSecurityFunc = pInitSecurityInterface();

        if (pSecurityFunc == NULL) {
            SetLastError(::GetLastError());
            break;
        }

        CopyMemory(&m_SecurityFunc, pSecurityFunc, sizeof(m_SecurityFunc));

        rc = TRUE;

    } while (FALSE);

    return rc;
}

SECURITY_STATUS CSslSocket::ClientCreateCredentials(const TCHAR *pszUserName, PCredHandle phCreds)
{
    TimeStamp       tsExpiry;
    SECURITY_STATUS Status;
    CERT_RDN cert_rdn;
    CERT_RDN_ATTR cert_rdn_attr;

    do {
        if (m_hMyCertStore == NULL) {
            m_hMyCertStore = CertOpenSystemStore(0, _T("MY"));
            if (!m_hMyCertStore) {
                SetLastError(::GetLastError());
                Status = SEC_E_NO_CREDENTIALS;
                break;
            }
        }

        if (pszUserName && _tcslen(pszUserName) != 0) {

            cert_rdn.cRDNAttr = 1;
            cert_rdn.rgRDNAttr = &cert_rdn_attr;

            cert_rdn_attr.pszObjId = szOID_COMMON_NAME;
            cert_rdn_attr.dwValueType = CERT_RDN_ANY_TYPE;
            cert_rdn_attr.Value.cbData = _tcslen(pszUserName);

#ifdef _UNICODE
            char *pszUn = new char[wcslen(pszUserName) + 1];
            WideCharToMultiByte(CP_ACP, 0, pszUserName, -1, pszUn, wcslen(pszUserName) + 1, NULL, NULL);
            cert_rdn_attr.Value.pbData = (BYTE *)pszUn;
#else
            cert_rdn_attr.Value.pbData = (BYTE *)pszUserName;
#endif			
            m_pCertContext = CertFindCertificateInStore(m_hMyCertStore,
                X509_ASN_ENCODING,
                0,
                CERT_FIND_SUBJECT_ATTR,
                &cert_rdn,
                NULL);

#ifdef _UNICODE
            delete[] pszUn;
#endif
            if (m_pCertContext == NULL) {
                SetLastError(::GetLastError());
                Status = SEC_E_NO_CREDENTIALS;
                break;
            }
        }

        ZeroMemory(&m_SchannelCred, sizeof(m_SchannelCred));

        m_SchannelCred.dwVersion = SCHANNEL_CRED_VERSION;

        if (m_pCertContext) {
            m_SchannelCred.cCreds = 1;
            m_SchannelCred.paCred = &m_pCertContext;
        }

        m_SchannelCred.grbitEnabledProtocols = m_dwProtocol;

        if (m_bNoVerify)
        {
            m_SchannelCred.dwFlags |= SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_MANUAL_CRED_VALIDATION | SCH_CRED_NO_SERVERNAME_CHECK;
        }
        else
        {
            m_SchannelCred.dwFlags |= SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_NO_SYSTEM_MAPPER | SCH_CRED_REVOCATION_CHECK_CHAIN;
        }

        Status = m_SecurityFunc.AcquireCredentialsHandle(
            NULL,
            UNISP_NAME,
            SECPKG_CRED_OUTBOUND,
            NULL,
            &m_SchannelCred,
            NULL,
            NULL,
            phCreds,
            &tsExpiry);

        if (Status != SEC_E_OK) {
            SetLastError(::GetLastError());
            Status = Status;
            break;
        }

    } while (FALSE);

    return Status;
}

SECURITY_STATUS CSslSocket::ServerCreateCredentials(const TCHAR * pszUserName, PCredHandle phCreds)
{
    TimeStamp       tsExpiry;
    SECURITY_STATUS Status;
    CERT_RDN cert_rdn;
    CERT_RDN_ATTR cert_rdn_attr;

    do {

        if (pszUserName == NULL || _tcslen(pszUserName) == 0) {
            SetLastError(ERROR_NO_SUCH_USER);
            Status = SEC_E_NO_CREDENTIALS;
            break;
        }

        if (m_hMyCertStore == NULL) {
            if (m_bMachineStore) {
                m_hMyCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,
                    X509_ASN_ENCODING,
                    0,
                    CERT_SYSTEM_STORE_LOCAL_MACHINE,
                    _T("MY"));
            }
            else {
                m_hMyCertStore = CertOpenSystemStore(0, _T("MY"));
            }

            if (!m_hMyCertStore) {
                SetLastError(::GetLastError());
                Status = SEC_E_NO_CREDENTIALS;
                break;
            }
        }

        cert_rdn.cRDNAttr = 1;
        cert_rdn.rgRDNAttr = &cert_rdn_attr;

        cert_rdn_attr.pszObjId = szOID_COMMON_NAME;
        cert_rdn_attr.dwValueType = CERT_RDN_ANY_TYPE;
        cert_rdn_attr.Value.cbData = _tcslen(pszUserName);

#ifdef _UNICODE
        char *pszUn = new char[wcslen(pszUserName) + 1];
        WideCharToMultiByte(CP_ACP, 0, pszUserName, -1, pszUn, wcslen(pszUserName) + 1, NULL, NULL);
        cert_rdn_attr.Value.pbData = (BYTE *)pszUn;
#else
        cert_rdn_attr.Value.pbData = (BYTE *)pszUserName;
#endif			
        m_pCertContext = CertFindCertificateInStore(m_hMyCertStore,
            X509_ASN_ENCODING,
            0,
            CERT_FIND_SUBJECT_ATTR,
            &cert_rdn,
            NULL);

#ifdef _UNICODE
        delete[] pszUn;
#endif

        if (m_pCertContext == NULL) {
            SetLastError(::GetLastError());
            Status = SEC_E_NO_CREDENTIALS;
            break;
        }

        ZeroMemory(&m_SchannelCred, sizeof(m_SchannelCred));

        m_SchannelCred.dwVersion = SCHANNEL_CRED_VERSION;

        m_SchannelCred.cCreds = 1;
        m_SchannelCred.paCred = &m_pCertContext;
        m_SchannelCred.hRootStore = m_hMyCertStore;
        m_SchannelCred.dwMinimumCipherStrength = 80;
        m_SchannelCred.grbitEnabledProtocols = m_dwProtocol;
        if (m_bNoVerify)
        {
            m_SchannelCred.dwFlags |= SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_MANUAL_CRED_VALIDATION | SCH_CRED_NO_SERVERNAME_CHECK;
        }
        else
        {
            m_SchannelCred.dwFlags |= SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_NO_SYSTEM_MAPPER | SCH_CRED_REVOCATION_CHECK_CHAIN;
        }

        Status = m_SecurityFunc.AcquireCredentialsHandle(
            NULL,
            UNISP_NAME,
            SECPKG_CRED_INBOUND,
            NULL,
            &m_SchannelCred,
            NULL,
            NULL,
            phCreds,
            &tsExpiry);

        if (Status != SEC_E_OK) {
            SetLastError(Status);
            Status = Status;
            break;
        }

    } while (FALSE);

    return Status;
}

BOOL CSslSocket::ClientConnect(const TCHAR *szHostName)
{
    BOOL rc = FALSE;
    SecBuffer  ExtraData;
    SECURITY_STATUS Status;
    PCCERT_CONTEXT pRemoteCertContext = NULL;

    do {
        if (ClientCreateCredentials(LPCTSTR(m_CsCertName), &m_hCreds)) {
            break;
        }

        if (ClientHandshake(&m_hCreds, szHostName, &m_hContext, &ExtraData)) {
            break;
        }

        Status = m_SecurityFunc.QueryContextAttributes(&m_hContext, SECPKG_ATTR_REMOTE_CERT_CONTEXT, (PVOID)&pRemoteCertContext);
        if (Status != SEC_E_OK) {
            SetLastError(Status);
            break;
        }

        if (!m_bNoVerify)
        {
            Status = ClientVerifyCertificate(pRemoteCertContext, szHostName, 0);
            if (Status) {
                SetLastError(Status);
                break;
            }
        }

        CertFreeCertificateContext(pRemoteCertContext);	//pRemoteCertContext

        rc = TRUE;
    } while (FALSE);

    return rc;
}

SECURITY_STATUS CSslSocket::ClientHandshake(PCredHandle phCreds, const TCHAR *pszServerName, CtxtHandle *phContext, SecBuffer *pExtraData)
{
    SecBufferDesc   OutBuffer;
    SecBuffer       OutBuffers[1];
    DWORD           dwSSPIFlags;
    DWORD           dwSSPIOutFlags;
    TimeStamp       tsExpiry;
    SECURITY_STATUS scRet;
    DWORD           cbData;

    dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT |
        ISC_REQ_REPLAY_DETECT |
        ISC_REQ_CONFIDENTIALITY |
        ISC_RET_EXTENDED_ERROR |
        ISC_REQ_ALLOCATE_MEMORY |
        ISC_REQ_STREAM;

    OutBuffers[0].pvBuffer = NULL;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer = 0;

    OutBuffer.cBuffers = 1;
    OutBuffer.pBuffers = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    scRet = m_SecurityFunc.InitializeSecurityContext(
        phCreds,
        NULL,
        (TCHAR*)pszServerName,
        dwSSPIFlags,
        0,
        SECURITY_NATIVE_DREP,
        NULL,
        0,
        phContext,
        &OutBuffer,
        &dwSSPIOutFlags,
        &tsExpiry);

    if (scRet != SEC_I_CONTINUE_NEEDED) {
        SetLastError(scRet);
        return scRet;
    }

    if (OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL) {
        DUMP(_T("CLIENT HELLO\n"), (BYTE*)OutBuffers[0].pvBuffer, OutBuffers[0].cbBuffer);
        m_bAllowPlainText = TRUE;
        cbData = Send(OutBuffers[0].pvBuffer, OutBuffers[0].cbBuffer);
        m_bAllowPlainText = FALSE;

        if (cbData == SOCKET_ERROR || cbData == 0) {
            SetLastError(WSAGetLastError());
            m_SecurityFunc.FreeContextBuffer(OutBuffers[0].pvBuffer);
            m_SecurityFunc.DeleteSecurityContext(phContext);
            return SEC_E_INTERNAL_ERROR;
        }

        m_SecurityFunc.FreeContextBuffer(OutBuffers[0].pvBuffer);
        OutBuffers[0].pvBuffer = NULL;
    }

    SECURITY_STATUS rc = ClientHandshakeLoop(phCreds, phContext, TRUE, pExtraData);

    if (rc == SEC_E_OK && pExtraData->pvBuffer) delete[] pExtraData->pvBuffer;

    return rc;
}

SECURITY_STATUS CSslSocket::ClientHandshakeLoop(PCredHandle phCreds, CtxtHandle *phContext, BOOL fDoInitialRead, SecBuffer *pExtraData)
{
    SecBufferDesc   InBuffer;
    SecBuffer       InBuffers[2];
    SecBufferDesc   OutBuffer;
    SecBuffer       OutBuffers[1];
    DWORD           dwSSPIFlags;
    DWORD           dwSSPIOutFlags;
    TimeStamp       tsExpiry;
    SECURITY_STATUS scRet;
    DWORD           cbData;

    PUCHAR          IoBuffer;
    DWORD           cbIoBuffer;
    BOOL            fDoRead;

    DWORD           dwBIOArgument;

    dwBIOArgument = FALSE;
    AsyncSelect(0);
    IOCtl(FIONBIO, &dwBIOArgument);

    dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT |
        ISC_REQ_REPLAY_DETECT |
        ISC_REQ_CONFIDENTIALITY |
        ISC_RET_EXTENDED_ERROR |
        ISC_REQ_ALLOCATE_MEMORY |
        ISC_REQ_STREAM;

    IoBuffer = new BYTE[IO_BUFFER_SIZE];
    if (IoBuffer == NULL) {
        SetLastError(ERROR_OUTOFMEMORY);
        return SEC_E_INTERNAL_ERROR;
    }
    cbIoBuffer = 0;

    fDoRead = fDoInitialRead;

    scRet = SEC_I_CONTINUE_NEEDED;

    while (scRet == SEC_I_CONTINUE_NEEDED ||
        scRet == SEC_E_INCOMPLETE_MESSAGE ||
        scRet == SEC_I_INCOMPLETE_CREDENTIALS) {

        if (0 == cbIoBuffer || scRet == SEC_E_INCOMPLETE_MESSAGE) {
            if (fDoRead) {
                m_bAllowPlainText = TRUE;
                cbData = CAsyncSocket::Receive((char *)(IoBuffer + cbIoBuffer), IO_BUFFER_SIZE - cbIoBuffer);
                m_bAllowPlainText = FALSE;

                if (cbData == SOCKET_ERROR) {
                    SetLastError(WSAGetLastError());
                    scRet = SEC_E_INTERNAL_ERROR;
                    break;
                }
                else if (cbData == 0) {
                    SetLastError(ERROR_VC_DISCONNECTED);
                    scRet = SEC_E_INTERNAL_ERROR;
                    break;
                }

                cbIoBuffer += cbData;
            }
            else {
                fDoRead = TRUE;
            }
        }

        InBuffers[0].pvBuffer = IoBuffer;
        InBuffers[0].cbBuffer = cbIoBuffer;
        InBuffers[0].BufferType = SECBUFFER_TOKEN;

        InBuffers[1].pvBuffer = NULL;
        InBuffers[1].cbBuffer = 0;
        InBuffers[1].BufferType = SECBUFFER_EMPTY;

        InBuffer.cBuffers = 2;
        InBuffer.pBuffers = InBuffers;
        InBuffer.ulVersion = SECBUFFER_VERSION;

        OutBuffers[0].pvBuffer = NULL;
        OutBuffers[0].BufferType = SECBUFFER_TOKEN;
        OutBuffers[0].cbBuffer = 0;

        OutBuffer.cBuffers = 1;
        OutBuffer.pBuffers = OutBuffers;
        OutBuffer.ulVersion = SECBUFFER_VERSION;

        scRet = m_SecurityFunc.InitializeSecurityContext(phCreds,
            phContext,
            NULL,
            dwSSPIFlags,
            0,
            SECURITY_NATIVE_DREP,
            &InBuffer,
            0,
            NULL,
            &OutBuffer,
            &dwSSPIOutFlags,
            &tsExpiry);

        if (scRet == SEC_E_OK ||
            scRet == SEC_I_CONTINUE_NEEDED ||
            FAILED(scRet) && (dwSSPIOutFlags & ISC_RET_EXTENDED_ERROR)) {

            if (OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL) {
                m_bAllowPlainText = TRUE;
                cbData = Send((const char *)(OutBuffers[0].pvBuffer), OutBuffers[0].cbBuffer);
                m_bAllowPlainText = FALSE;

                if (cbData == SOCKET_ERROR || cbData == 0) {
                    SetLastError(WSAGetLastError());
                    m_SecurityFunc.FreeContextBuffer(OutBuffers[0].pvBuffer);
                    m_SecurityFunc.DeleteSecurityContext(phContext);
                    return SEC_E_INTERNAL_ERROR;
                }

                m_SecurityFunc.FreeContextBuffer(OutBuffers[0].pvBuffer);
                OutBuffers[0].pvBuffer = NULL;
            }
        }

        if (scRet == SEC_E_INCOMPLETE_MESSAGE) {
            continue;
        }

        if (scRet == SEC_E_OK) {

            if (InBuffers[1].BufferType == SECBUFFER_EXTRA) {

                pExtraData->pvBuffer = new BYTE[InBuffers[1].cbBuffer];

                if (pExtraData->pvBuffer == NULL) {
                    SetLastError(ERROR_OUTOFMEMORY);
                    return SEC_E_INTERNAL_ERROR;
                }

                MoveMemory(pExtraData->pvBuffer,
                    IoBuffer + (cbIoBuffer - InBuffers[1].cbBuffer),
                    InBuffers[1].cbBuffer);

                pExtraData->cbBuffer = InBuffers[1].cbBuffer;
                pExtraData->BufferType = SECBUFFER_TOKEN;
            }
            else {
                pExtraData->pvBuffer = NULL;
                pExtraData->cbBuffer = 0;
                pExtraData->BufferType = SECBUFFER_EMPTY;
            }

            break;
        }

        if (FAILED(scRet)) {
            SetLastError(scRet);
            break;
        }

        if (scRet == SEC_I_INCOMPLETE_CREDENTIALS) {
            SetLastError(scRet);
            break;
        }

        if (InBuffers[1].BufferType == SECBUFFER_EXTRA) {
            MoveMemory(IoBuffer,
                IoBuffer + (cbIoBuffer - InBuffers[1].cbBuffer),
                InBuffers[1].cbBuffer);

            cbIoBuffer = InBuffers[1].cbBuffer;
        }
        else {
            cbIoBuffer = 0;
        }
    }

    if (FAILED(scRet)) {
        m_SecurityFunc.DeleteSecurityContext(phContext);
    }

    if (IoBuffer) delete[] IoBuffer;

    dwBIOArgument = TRUE;
    IOCtl(FIONBIO, &dwBIOArgument);
    AsyncSelect(FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE);

    return scRet;
}

DWORD CSslSocket::ClientVerifyCertificate(PCCERT_CONTEXT pServerCert, const TCHAR *pszServerName, DWORD dwCertFlags)
{
    HTTPSPolicyCallbackData		polHttps;
    CERT_CHAIN_POLICY_PARA		PolicyPara;
    CERT_CHAIN_POLICY_STATUS	PolicyStatus;
    CERT_CHAIN_PARA				ChainPara;
    PCCERT_CHAIN_CONTEXT		pChainContext = NULL;

    DWORD   Status;
    PWSTR   pwszServerName = NULL;
#ifndef _UNICODE
    DWORD   cchServerName;
#endif

    do {
        if (pServerCert == NULL) {
            Status = SEC_E_WRONG_PRINCIPAL;
            SetLastError(Status);
            break;
        }
        int iRc = CertVerifyTimeValidity(NULL, pServerCert->pCertInfo);
        if (iRc != 0) {
            Status = SEC_E_CERT_EXPIRED;
            SetLastError(Status);
            break;
        }

#ifndef _UNICODE
        if (pszServerName == NULL || strlen(pszServerName) == 0) {
            return SEC_E_WRONG_PRINCIPAL;
        }

        cchServerName = MultiByteToWideChar(CP_ACP, 0, pszServerName, -1, NULL, 0);
        pwszServerName = new WCHAR[cchServerName];
        if (pwszServerName == NULL) {
            return SEC_E_INSUFFICIENT_MEMORY;
        }
        cchServerName = MultiByteToWideChar(CP_ACP, 0, pszServerName, -1, pwszServerName, cchServerName);
        if (cchServerName == 0) {
            return SEC_E_WRONG_PRINCIPAL;
        }
#else
        pwszServerName = (WCHAR*)pszServerName;
#endif

        ZeroMemory(&ChainPara, sizeof(ChainPara));
        ChainPara.cbSize = sizeof(ChainPara);

        if (!CertGetCertificateChain(
            NULL,
            pServerCert,
            NULL,
            NULL,
            &ChainPara,
            0,
            NULL,
            &pChainContext)) {
            Status = ::GetLastError();
            SetLastError(Status);
            break;
        }

        ZeroMemory(&polHttps, sizeof(HTTPSPolicyCallbackData));
        polHttps.cbStruct = sizeof(HTTPSPolicyCallbackData);
        polHttps.dwAuthType = AUTHTYPE_SERVER;
        polHttps.fdwChecks = dwCertFlags;
        polHttps.pwszServerName = pwszServerName;

        memset(&PolicyPara, 0, sizeof(PolicyPara));
        PolicyPara.cbSize = sizeof(PolicyPara);
        PolicyPara.pvExtraPolicyPara = &polHttps;

        memset(&PolicyStatus, 0, sizeof(PolicyStatus));
        PolicyStatus.cbSize = sizeof(PolicyStatus);

        if (!CertVerifyCertificateChainPolicy(
            CERT_CHAIN_POLICY_SSL,
            pChainContext,
            &PolicyPara,
            &PolicyStatus)) {
            Status = ::GetLastError();
            SetLastError(Status);
            break;
        }

        if (PolicyStatus.dwError) {
            Status = PolicyStatus.dwError;
            SetLastError(Status);
            break;
        }

        PCERT_CONTEXT *pCerts = new PCERT_CONTEXT[pChainContext->cChain];

        for (DWORD i = 0; i < pChainContext->cChain; i++) {
            pCerts[i] = (PCERT_CONTEXT)(pChainContext->rgpChain[i]->rgpElement[0]->pCertContext);
        }

        CERT_REVOCATION_STATUS revStat;
        revStat.cbSize = sizeof(CERT_REVOCATION_STATUS);

        BOOL bRc = CertVerifyRevocation(
            X509_ASN_ENCODING,
            CERT_CONTEXT_REVOCATION_TYPE,
            pChainContext->cChain,
            (void **)pCerts,
            CERT_VERIFY_REV_CHAIN_FLAG,
            NULL,
            &revStat);
        if (!bRc) {
            SetLastError(revStat.dwError);
            break;
        }

        delete[] pCerts;

        Status = SEC_E_OK;

    } while (FALSE);

    if (pChainContext) {
        CertFreeCertificateChain(pChainContext);
    }

#ifndef _UNICODE
    if (pwszServerName) delete[] pwszServerName;
#endif

    return Status;
}

LONG CSslSocket::ClientDisconnect(PCredHandle phCreds, CtxtHandle *phContext)
{
    DWORD           dwType;
    PBYTE           pbMessage;
    DWORD           cbMessage;
    DWORD           cbData;

    SecBufferDesc   OutBuffer;
    SecBuffer       OutBuffers[1];
    DWORD           dwSSPIFlags;
    DWORD           dwSSPIOutFlags;
    TimeStamp       tsExpiry;
    DWORD           Status;

    dwType = SCHANNEL_SHUTDOWN;

    OutBuffers[0].pvBuffer = &dwType;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer = sizeof(dwType);

    OutBuffer.cBuffers = 1;
    OutBuffer.pBuffers = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    do {

        Status = m_SecurityFunc.ApplyControlToken(phContext, &OutBuffer);

        if (FAILED(Status)) {
            SetLastError(Status);
            break;
        }

        dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT |
            ISC_REQ_REPLAY_DETECT |
            ISC_REQ_CONFIDENTIALITY |
            ISC_RET_EXTENDED_ERROR |
            ISC_REQ_ALLOCATE_MEMORY |
            ISC_REQ_STREAM;

        OutBuffers[0].pvBuffer = NULL;
        OutBuffers[0].BufferType = SECBUFFER_TOKEN;
        OutBuffers[0].cbBuffer = 0;

        OutBuffer.cBuffers = 1;
        OutBuffer.pBuffers = OutBuffers;
        OutBuffer.ulVersion = SECBUFFER_VERSION;

        Status = m_SecurityFunc.InitializeSecurityContext(
            phCreds,
            phContext,
            NULL,
            dwSSPIFlags,
            0,
            SECURITY_NATIVE_DREP,
            NULL,
            0,
            phContext,
            &OutBuffer,
            &dwSSPIOutFlags,
            &tsExpiry);

        if (FAILED(Status)) {
            SetLastError(Status);
            break;
        }

        pbMessage = (BYTE *)(OutBuffers[0].pvBuffer);
        cbMessage = OutBuffers[0].cbBuffer;

        if (pbMessage != NULL && cbMessage != 0) {
            m_bAllowPlainText = TRUE; m_bConInit = FALSE;
            cbData = Send(pbMessage, cbMessage);
            m_bAllowPlainText = FALSE;
            if (cbData == SOCKET_ERROR || cbData == 0) {
                Status = WSAGetLastError();
                SetLastError(Status);
                break;
            }

            m_SecurityFunc.FreeContextBuffer(pbMessage);
        }

    } while (FALSE);

    m_SecurityFunc.DeleteSecurityContext(phContext);

    return Status;
}

LONG CSslSocket::ServerDisconect(PCredHandle phCreds, CtxtHandle *phContext)
{
    DWORD           dwType;
    PBYTE           pbMessage;
    DWORD           cbMessage;
    DWORD           cbData;

    SecBufferDesc   OutBuffer;
    SecBuffer       OutBuffers[1];
    DWORD           dwSSPIFlags;
    DWORD           dwSSPIOutFlags;
    TimeStamp       tsExpiry;
    DWORD           Status;

    dwType = SCHANNEL_SHUTDOWN;

    OutBuffers[0].pvBuffer = &dwType;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer = sizeof(dwType);

    OutBuffer.cBuffers = 1;
    OutBuffer.pBuffers = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    do {

        Status = m_SecurityFunc.ApplyControlToken(phContext, &OutBuffer);

        if (FAILED(Status)) {
            SetLastError(Status);
            break;
        }

        dwSSPIFlags = ASC_REQ_SEQUENCE_DETECT |
            ASC_REQ_REPLAY_DETECT |
            ASC_REQ_CONFIDENTIALITY |
            ASC_REQ_EXTENDED_ERROR |
            ASC_REQ_ALLOCATE_MEMORY |
            ASC_REQ_STREAM;

        OutBuffers[0].pvBuffer = NULL;
        OutBuffers[0].BufferType = SECBUFFER_TOKEN;
        OutBuffers[0].cbBuffer = 0;

        OutBuffer.cBuffers = 1;
        OutBuffer.pBuffers = OutBuffers;
        OutBuffer.ulVersion = SECBUFFER_VERSION;

        Status = m_SecurityFunc.AcceptSecurityContext(
            phCreds,
            phContext,
            NULL,
            dwSSPIFlags,
            SECURITY_NATIVE_DREP,
            NULL,
            &OutBuffer,
            &dwSSPIOutFlags,
            &tsExpiry);

        if (FAILED(Status)) {
            SetLastError(Status);
            break;
        }

        pbMessage = (BYTE *)(OutBuffers[0].pvBuffer);
        cbMessage = OutBuffers[0].cbBuffer;

        if (pbMessage != NULL && cbMessage != 0) {
            m_bAllowPlainText = TRUE; m_bConInit = FALSE;
            cbData = Send(pbMessage, cbMessage);
            m_bAllowPlainText = FALSE;
            if (cbData == SOCKET_ERROR || cbData == 0) {
                Status = WSAGetLastError();
                SetLastError(Status);
                break;
            }

            m_SecurityFunc.FreeContextBuffer(pbMessage);
        }

    } while (FALSE);

    m_SecurityFunc.DeleteSecurityContext(phContext);

    return Status;
}

BOOL CSslSocket::ServerConnect(SOCKADDR* lpSockAddr, int* lpSockAddrLen)
{
    BOOL rc = FALSE;
    SECURITY_STATUS scRet;
    PCCERT_CONTEXT pRemoteCertContext = NULL;
    SecPkgContext_StreamSizes Sizes;

    do {

        if (!ServerHandshakeLoop(&m_hContext, &m_hCreds, m_bAuthClient, TRUE, TRUE)) {
            break;
        }

        if (m_bAuthClient) {
            scRet = m_SecurityFunc.QueryContextAttributes(&m_hContext,
                SECPKG_ATTR_REMOTE_CERT_CONTEXT,
                (PVOID)&pRemoteCertContext);

            if (scRet != SEC_E_OK) {
                SetLastError(scRet);
                break;
            }
            else {
                scRet = ServerVerifyCertificate(pRemoteCertContext, 0);
                if (scRet) {
                    SetLastError(scRet);
                    break;
                }
                CertFreeCertificateContext(pRemoteCertContext);
            }
        }

        scRet = m_SecurityFunc.QueryContextAttributes(&m_hContext, SECPKG_ATTR_STREAM_SIZES, &Sizes);

        if (scRet != SEC_E_OK) {
            SetLastError(scRet);
            break;
        }

        rc = TRUE;

    } while (FALSE);

    return rc;
}

BOOL CSslSocket::ServerHandshakeLoop(PCtxtHandle phContext, PCredHandle phCred, BOOL fClientAuth, BOOL fDoInitialRead, BOOL NewContext)
{
    TimeStamp            tsExpiry;
    SECURITY_STATUS      scRet;
    SecBufferDesc        InBuffer;
    SecBufferDesc        OutBuffer;
    SecBuffer            InBuffers[2];
    SecBuffer            OutBuffers[1];
    DWORD                err;

    BOOL                 fDoRead;
    BOOL                 fInitContext = NewContext;

    DWORD                dwSSPIFlags, dwSSPIOutFlags;

    BYTE IoBuffer[IO_BUFFER_SIZE];
    DWORD cbIoBuffer = 0;

    scRet = SEC_E_SECPKG_NOT_FOUND;
    err = 0;

    fDoRead = fDoInitialRead;

    dwSSPIFlags = ASC_REQ_SEQUENCE_DETECT |
        ASC_REQ_REPLAY_DETECT |
        ASC_REQ_CONFIDENTIALITY |
        ASC_REQ_EXTENDED_ERROR |
        ASC_REQ_ALLOCATE_MEMORY |
        ASC_REQ_STREAM;

    if (fClientAuth) {
        dwSSPIFlags |= ASC_REQ_MUTUAL_AUTH;
    }


    OutBuffer.cBuffers = 1;
    OutBuffer.pBuffers = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    scRet = SEC_I_CONTINUE_NEEDED;

    while (scRet == SEC_I_CONTINUE_NEEDED ||
        scRet == SEC_E_INCOMPLETE_MESSAGE ||
        scRet == SEC_I_INCOMPLETE_CREDENTIALS) {

        if (0 == cbIoBuffer || scRet == SEC_E_INCOMPLETE_MESSAGE) {

            if (fDoRead) {
                m_bAllowPlainText = TRUE;
                err = Receive(IoBuffer + cbIoBuffer, IO_BUFFER_SIZE);
                m_bAllowPlainText = FALSE;

                if (err == SOCKET_ERROR || err == 0) {
                    SetLastError(::WSAGetLastError());
                    return FALSE;
                }
                else {
                    cbIoBuffer += err;
                }
            }
            else {
                fDoRead = TRUE;
            }
        }

        InBuffers[0].pvBuffer = IoBuffer;
        InBuffers[0].cbBuffer = cbIoBuffer;
        InBuffers[0].BufferType = SECBUFFER_TOKEN;

        InBuffers[1].pvBuffer = NULL;
        InBuffers[1].cbBuffer = 0;
        InBuffers[1].BufferType = SECBUFFER_EMPTY;

        InBuffer.cBuffers = 2;
        InBuffer.pBuffers = InBuffers;
        InBuffer.ulVersion = SECBUFFER_VERSION;

        OutBuffers[0].pvBuffer = NULL;
        OutBuffers[0].BufferType = SECBUFFER_TOKEN;
        OutBuffers[0].cbBuffer = 0;

        scRet = m_SecurityFunc.AcceptSecurityContext(
            phCred,
            (fInitContext ? NULL : phContext),
            &InBuffer,
            dwSSPIFlags,
            SECURITY_NATIVE_DREP,
            (fInitContext ? phContext : NULL),
            &OutBuffer,
            &dwSSPIOutFlags,
            &tsExpiry);

        fInitContext = FALSE;

        if (scRet == SEC_E_OK ||
            scRet == SEC_I_CONTINUE_NEEDED ||
            (FAILED(scRet) && (0 != (dwSSPIOutFlags & ISC_RET_EXTENDED_ERROR)))) {

            if (OutBuffers[0].cbBuffer != 0 &&
                OutBuffers[0].pvBuffer != NULL) {

                m_bAllowPlainText = TRUE;
                err = Send(OutBuffers[0].pvBuffer, OutBuffers[0].cbBuffer);
                m_bAllowPlainText = FALSE;

                m_SecurityFunc.FreeContextBuffer(OutBuffers[0].pvBuffer);
                OutBuffers[0].pvBuffer = NULL;
            }
        }

        if (scRet == SEC_E_OK) {
            if (InBuffers[1].BufferType == SECBUFFER_EXTRA) {
                memcpy(IoBuffer,
                    (LPBYTE)(IoBuffer + (cbIoBuffer - InBuffers[1].cbBuffer)),
                    InBuffers[1].cbBuffer);
                cbIoBuffer = InBuffers[1].cbBuffer;
            }
            else {
                cbIoBuffer = 0;
            }

            return TRUE;
        }
        else if (FAILED(scRet) && (scRet != SEC_E_INCOMPLETE_MESSAGE)) {
            SetLastError(scRet);
            return FALSE;
        }

        if (scRet != SEC_E_INCOMPLETE_MESSAGE &&
            scRet != SEC_I_INCOMPLETE_CREDENTIALS) {

            if (InBuffers[1].BufferType == SECBUFFER_EXTRA) {
                memcpy(IoBuffer,
                    (LPBYTE)(IoBuffer + (cbIoBuffer - InBuffers[1].cbBuffer)),
                    InBuffers[1].cbBuffer);
                cbIoBuffer = InBuffers[1].cbBuffer;
            }
            else {
                cbIoBuffer = 0;
            }
        }
    }

    return FALSE;
}

DWORD CSslSocket::ServerVerifyCertificate(PCCERT_CONTEXT pServerCert, DWORD dwCertFlags)
{
    HTTPSPolicyCallbackData		polHttps;
    CERT_CHAIN_POLICY_PARA		PolicyPara;
    CERT_CHAIN_POLICY_STATUS	PolicyStatus;
    CERT_CHAIN_PARA				ChainPara;
    PCCERT_CHAIN_CONTEXT		pChainContext = NULL;

    DWORD   Status;

    do {
        if (pServerCert == NULL) {
            Status = SEC_E_WRONG_PRINCIPAL;
            SetLastError(Status);
            break;
        }

        int iRc = CertVerifyTimeValidity(NULL, pServerCert->pCertInfo);
        if (iRc != 0) {
            Status = SEC_E_CERT_EXPIRED;
            SetLastError(Status);
            break;
        }

        ZeroMemory(&ChainPara, sizeof(ChainPara));
        ChainPara.cbSize = sizeof(ChainPara);

        if (!CertGetCertificateChain(
            NULL,
            pServerCert,
            NULL,
            NULL,
            &ChainPara,
            CERT_CHAIN_REVOCATION_CHECK_CHAIN,
            NULL,
            &pChainContext)) {
            Status = ::GetLastError();
            SetLastError(Status);
            break;
        }

        ZeroMemory(&polHttps, sizeof(HTTPSPolicyCallbackData));
        polHttps.cbStruct = sizeof(HTTPSPolicyCallbackData);
        polHttps.dwAuthType = AUTHTYPE_CLIENT;
        polHttps.fdwChecks = dwCertFlags;
        polHttps.pwszServerName = NULL;

        memset(&PolicyPara, 0, sizeof(PolicyPara));
        PolicyPara.cbSize = sizeof(PolicyPara);
        PolicyPara.pvExtraPolicyPara = &polHttps;

        memset(&PolicyStatus, 0, sizeof(PolicyStatus));
        PolicyStatus.cbSize = sizeof(PolicyStatus);

        if (!CertVerifyCertificateChainPolicy(
            CERT_CHAIN_POLICY_SSL,
            pChainContext,
            &PolicyPara,
            &PolicyStatus)) {
            Status = ::GetLastError();
            SetLastError(Status);
            break;
        }

        if (PolicyStatus.dwError) {
            Status = PolicyStatus.dwError;
            SetLastError(Status);
            break;
        }

        PCERT_CONTEXT *pCerts = new PCERT_CONTEXT[pChainContext->cChain];

        for (DWORD i = 0; i < pChainContext->cChain; i++) {
            pCerts[i] = (PCERT_CONTEXT)(pChainContext->rgpChain[i]->rgpElement[0]->pCertContext);
        }

        CERT_REVOCATION_STATUS revStat;
        revStat.cbSize = sizeof(CERT_REVOCATION_STATUS);

        BOOL bRc = CertVerifyRevocation(
            X509_ASN_ENCODING,
            CERT_CONTEXT_REVOCATION_TYPE,
            pChainContext->cChain,
            (void **)pCerts,
            CERT_VERIFY_REV_CHAIN_FLAG,
            NULL,
            &revStat);
        if (!bRc) {
            SetLastError(revStat.dwError);
            break;
        }

        delete[] pCerts;
        Status = SEC_E_OK;

    } while (FALSE);

    if (pChainContext) {
        CertFreeCertificateChain(pChainContext);
    }

    return Status;
}

BOOL GetErrorDescription(DWORD error_code, CString& error_string)
{
    BOOL rc = TRUE;

    error_string.Empty();

    LPVOID message_buffer = NULL;

    ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        error_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
        (LPTSTR)&message_buffer,
        0,
        NULL);

    error_string = (LPCTSTR)message_buffer;

    ::LocalFree(message_buffer);
    message_buffer = NULL;

    if (error_string.GetLength() == 0) {
        CString frm = _T("Unknown error.");
        error_string.Format(frm, error_code);
        rc = FALSE;
    }

    return rc;
}

void DumpErrorCode(DWORD dwErrCode, DWORD dwLine, char *szFileName)
{
#ifdef _UNICODE

    DWORD dwFileName = MultiByteToWideChar(CP_ACP, 0, szFileName, -1, NULL, 0);
    WCHAR *wszFileName = new WCHAR[dwFileName];
    MultiByteToWideChar(CP_ACP, 0, szFileName, -1, wszFileName, dwFileName);

    CString str;
    GetErrorDescription(dwErrCode, str);
    TRACE(_T("%s(%d): 0x%.8x: %s\n"), wszFileName, dwLine, dwErrCode, str);

    delete[] wszFileName;
#else
    CString str;
    GetErrorDescription(dwErrCode, str);
    TRACE(_T("%s(%d): 0x%.8x: %s\n"), szFileName, dwLine, dwErrCode, str);
#endif
}

void UsrDump(TCHAR *msg, BYTE *pData, int len)
{
    try {
        CString trmsg;

        if (pData == NULL) return;

        TRACE(msg);
        TRACE(_T("\nAddress:\t0x%.8x\nOffset\t\tData\n"), pData);

        CString str;

        int c = len / 16;

        int i;
        for (i = 0; i < c; i++) {
            str.Format(_T("0x%.8x     %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x  %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"),
                i * 16,
                pData[0], pData[1], pData[2], pData[3], pData[4], pData[5], pData[6], pData[7],
                pData[8], pData[9], pData[10], pData[11], pData[12], pData[13], pData[14], pData[15],
                GetPrintCharOrDot(pData[0]), GetPrintCharOrDot(pData[1]), GetPrintCharOrDot(pData[2]), GetPrintCharOrDot(pData[3]), GetPrintCharOrDot(pData[4]), GetPrintCharOrDot(pData[5]), GetPrintCharOrDot(pData[6]), GetPrintCharOrDot(pData[7]),
                GetPrintCharOrDot(pData[8]), GetPrintCharOrDot(pData[9]), GetPrintCharOrDot(pData[10]), GetPrintCharOrDot(pData[11]), GetPrintCharOrDot(pData[12]), GetPrintCharOrDot(pData[13]), GetPrintCharOrDot(pData[14]), GetPrintCharOrDot(pData[15]));
            if (len > 16) {
                str += (TCHAR)13;
                str += (TCHAR)10;
            }
            TRACE("%s", str);
            pData += 16;
        }

        c = len % 16;

        CString numbers;
        if (c != 0) {
            str.Format(_T("0x%.8x     "), i * 16);
            numbers += str;
        }

        int j;
        for (j = 0; j < c; j++) {
            str.Format(_T("%.2x "), pData[j]);
            numbers += str;
        }

        CString spaces;
        if (c != 0) {
            int k = c * 3;
            for (j = k; j < 47; j++) {
                spaces += _T(" ");
            }
            spaces += _T("  ");
        }

        CString chars;
        for (j = 0; j < c; j++) {
            str.Format(_T("%c"), GetPrintCharOrDot(pData[j]));
            chars += str;
        }

        if (c != 0) {
            str += numbers + spaces + chars + (TCHAR)13;
            str += (TCHAR)10;
            TRACE("%s", str);
        }

    }
    catch (...) {
        TRACE(_T("(%s)%d: Unknown exception.\n"), __FILE__, __LINE__);
        ::SetLastError((unsigned long)NTE_FAIL);
    }
}

CString GetDumpString(BYTE *pData, int len)
{
    CString dumped;

    try {
        CString str;

        int c = len / 16;

        int i;
        for (i = 0; i < c; i++) {
            str.Format(_T("0x%.8x     %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x  %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"),
                i * 16,
                pData[0], pData[1], pData[2], pData[3], pData[4], pData[5], pData[6], pData[7],
                pData[8], pData[9], pData[10], pData[11], pData[12], pData[13], pData[14], pData[15],
                GetPrintCharOrDot(pData[0]), GetPrintCharOrDot(pData[1]), GetPrintCharOrDot(pData[2]), GetPrintCharOrDot(pData[3]), GetPrintCharOrDot(pData[4]), GetPrintCharOrDot(pData[5]), GetPrintCharOrDot(pData[6]), GetPrintCharOrDot(pData[7]),
                GetPrintCharOrDot(pData[8]), GetPrintCharOrDot(pData[9]), GetPrintCharOrDot(pData[10]), GetPrintCharOrDot(pData[11]), GetPrintCharOrDot(pData[12]), GetPrintCharOrDot(pData[13]), GetPrintCharOrDot(pData[14]), GetPrintCharOrDot(pData[15]));
            if (len > 16) {
                str += (TCHAR)13;
                str += (TCHAR)10;
            }
            dumped += str;
            pData += 16;
        }

        c = len % 16;

        CString numbers;
        if (c != 0) {
            str.Format(_T("0x%.8x     "), i * 16);
            numbers += str;
        }

        int j;
        for (j = 0; j < c; j++) {
            str.Format(_T("%.2x "), pData[j]);
            numbers += str;
        }

        CString spaces;
        if (c != 0) {
            int k = c * 3;
            for (j = k; j < 47; j++) {
                spaces += _T(" ");
            }
            spaces += _T("  ");
        }

        CString chars;
        for (j = 0; j < c; j++) {
            str.Format(_T("%c"), GetPrintCharOrDot(pData[j]));
            chars += str;
        }

        if (c != 0) {
            dumped += numbers + spaces + chars + (TCHAR)13;
            dumped += (TCHAR)10;
        }

    }
    catch (...) {
        TRACE(_T("(%s)%d: Unknown exception.\n"), __FILE__, __LINE__);
        ::SetLastError((unsigned long)NTE_FAIL);
    }

    return dumped;
}
