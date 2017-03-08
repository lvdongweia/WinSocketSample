
// SNBurningDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SNBurning.h"
#include "SNBurningDlg.h"
#include "afxdialogex.h"
#include "LibSNLocal.h"
#include "LibSNRemote.h"
#include "PTBase.h"
#include "snRemote.h"
#include <windows.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSNBurningDlg �Ի���

void CSNBurningDlg::getErrorString(int err_code, char *err_str)
{
	switch (err_code) {
	case NO_PT_ERROR:
		sprintf(err_str, "%s", "No Error");
		break;
	case OPEN_DEVICE_FAILED:
		sprintf(err_str, "%s", "Open Device Failed");
		break;
	case INIT_CAN_FAILED:
		sprintf(err_str, "%s", "Init Can Failed");
		break;
	case START_CAN_FAILED:
		sprintf(err_str, "%s", "Start Can Failed");
		break;
	case CLOSE_DEVICE_FAILED:
		sprintf(err_str, "%s", "Close Device Failed");
		break;
	case INVALID_PARAMETER:
		sprintf(err_str, "%s", "Invalid parameter");
		break;
	case DEVICE_NOT_OPEN:
		sprintf(err_str, "%s", "Device not found");
		break;
	case SEND_ERROR:
		sprintf(err_str, "%s", "Send error");
		break;
	case PACKET_IS_INCOMPLETE:
		sprintf(err_str, "%s", "Packet is incomplete");
		break;
	case RECEIVE_ERROR:
		sprintf(err_str, "%s", "Receive error");
		break;
	case RECEIVE_PACKET_FAILED:
		sprintf(err_str, "%s", "Receive packet failed");
		break;
	case RECEIVE_PACKET_TIMEOUT:
		sprintf(err_str, "%s", "Receive packet timeout");
		break;
	case RECEIVE_PACKET_CANCELED:
		sprintf(err_str, "%s", "Receive packet canceled");
		break;
	case NO_CAN_OBJ:
		sprintf(err_str, "%s", "No can object");
		break;
	case SWITCH_MODE_FAILED:
		sprintf(err_str, "%s", "Switch mode Failed");
		break;
	case SAVE_SN_FAILED:
		sprintf(err_str, "%s", "Save SN Failed");
		break;
	case REQUEST_SOCKET_FAILED:
		sprintf(err_str, "%s", "Request socket Failed");
		break;
	case CONNECT_SOCKET_FAILED:
		sprintf(err_str, "%s", "Connect socket Failed");
		break;
	case SEND_DATA_FAILED:
		sprintf(err_str, "%s", "Send data Failed");
		break;
	case RECEIVE_DATA_FAILED:
		sprintf(err_str, "%s", "Receive data Failed");
		break;
	case INVALID_MESSAGE_TYPE:
		sprintf(err_str, "%s", "Invalid message type");
		break;
	case GENERATE_SN_FAILED:
		sprintf(err_str, "%s", "Generate SN Failed");
		break;
	case READ_SERVER_FAILED:
		sprintf(err_str, "%s", "Read Server Failed");
		break;
	case READ_USERID_FAILED:
		sprintf(err_str, "%s", "Read Userid Failed");
		break;
	case REMOTE_HOST_NOT_FOUND:
		sprintf(err_str, "%s", "Remote host not found");
		break;
	case NO_DATA_RECORD_FOUND:
		sprintf(err_str, "%s", "No data record found");
		break;
	case GET_HOST_BY_NAME_FAILED:
		sprintf(err_str, "%s", "Get host by name Failed");
		break;
	case NETBIOS_ADDRESS_RETURN:
		sprintf(err_str, "%s", "NetBios address return");
		break;
	default:
		sprintf(err_str, "%s", "Invalid Error Code");
		break;
	}
}


CSNBurningDlg::CSNBurningDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSNBurningDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSNBurningDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSNBurningDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_GBS, &CSNBurningDlg::OnBnClickedGbs)
	ON_BN_CLICKED(IDC_RADIO_AUTO, &CSNBurningDlg::OnBnClickedRadioAuto)
	ON_BN_CLICKED(IDC_RADIO_Manual, &CSNBurningDlg::OnBnClickedRadioManual)
	ON_BN_CLICKED(IDC_BUTTON_READ_SN, &CSNBurningDlg::OnBnClickedButtonReadSn)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_GENERATE, &CSNBurningDlg::OnBnClickedButtonAutoGenerate)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_RSN, &CSNBurningDlg::OnBnClickedButtonClearRsn)
	ON_BN_CLICKED(IDC_BUTTON_WRITE_SN, &CSNBurningDlg::OnBnClickedButtonWriteSn)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_WSN, &CSNBurningDlg::OnBnClickedButtonClearWsn)
END_MESSAGE_MAP()


// CSNBurningDlg ��Ϣ�������

BOOL CSNBurningDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	((CButton *)GetDlgItem(IDC_RADIO_AUTO))->SetCheck(TRUE);
	(CEdit*) GetDlgItem(IDC_EDIT_READ_SN)->EnableWindow(FALSE);
	(CEdit*) GetDlgItem(IDC_EDIT_WRITE_SN)->EnableWindow(FALSE);
	((CButton *)GetDlgItem(IDC_BUTTON_AUTO_GENERATE))->EnableWindow(TRUE);

	//Font
	CFont f_Font;
	f_Font.CreateFont(0,0,0,0,0,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_SWISS,L"Arial");
	CEdit *r_Edit=(CEdit *)GetDlgItem(IDC_EDIT_READ_SN);
	r_Edit->SetFont(&f_Font,FALSE);
	CEdit *w_Edit=(CEdit *)GetDlgItem(IDC_EDIT_WRITE_SN);
	w_Edit->SetFont(&f_Font,FALSE);

	//Language String
	CString strTemp; 
    strTemp.LoadString(IDS_SN_TITLE);    //LoadString��ȡһ��ID�Ŷ�����ַ�����Դ     
	SetWindowText(strTemp); 
 
    strTemp.LoadString(IDS_INPUT_METHOD); 
    SetDlgItemText(IDC_GBIM,strTemp); 
 
    strTemp.LoadString(IDS_AUTO); 
    SetDlgItemText(IDC_RADIO_AUTO,strTemp); 

    strTemp.LoadString(IDS_MANUAL); 
    SetDlgItemText(IDC_RADIO_Manual,strTemp); 

    strTemp.LoadString(IDS_STATISTICS); 
    SetDlgItemText(IDC_GBS,strTemp); 

	strTemp.LoadString(IDS_SUCCESS); 
    SetDlgItemText(IDC_STATIC_SUCCESS,strTemp); 

	strTemp.LoadString(IDS_READ_SN); 
    SetDlgItemText(IDC_BUTTON_READ_SN,strTemp); 

	strTemp.LoadString(IDS_WRITE_SN); 
    SetDlgItemText(IDC_BUTTON_WRITE_SN,strTemp); 

	strTemp.LoadString(IDS_CLEAR_SN); 
    SetDlgItemText(IDC_BUTTON_CLEAR_WSN,strTemp); 
    SetDlgItemText(IDC_BUTTON_CLEAR_RSN,strTemp); 

	strTemp.LoadString(IDS_AUTO_GENERATE); 
    SetDlgItemText(IDC_BUTTON_AUTO_GENERATE,strTemp); 

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CSNBurningDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CSNBurningDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CSNBurningDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSNBurningDlg::OnBnClickedGbs()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}


void CSNBurningDlg::OnBnClickedRadioAuto()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CButton * radio_auto = ((CButton *)GetDlgItem(IDC_RADIO_AUTO));

	if (radio_auto->GetCheck())
	{
		(CEdit*) GetDlgItem(IDC_EDIT_WRITE_SN)->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_BUTTON_AUTO_GENERATE))->EnableWindow(TRUE);
	}
	else
	{
		(CEdit*) GetDlgItem(IDC_EDIT_WRITE_SN)->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_BUTTON_AUTO_GENERATE))->EnableWindow(FALSE);
	}

}


void CSNBurningDlg::OnBnClickedRadioManual()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CButton * radio_manual = ((CButton *)GetDlgItem(IDC_RADIO_Manual));

	if (radio_manual->GetCheck())
	{
		(CEdit*) GetDlgItem(IDC_EDIT_WRITE_SN)->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_BUTTON_AUTO_GENERATE))->EnableWindow(FALSE);
	}
	else
	{
		(CEdit*) GetDlgItem(IDC_EDIT_WRITE_SN)->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_BUTTON_AUTO_GENERATE))->EnableWindow(TRUE);
	}
}


void CSNBurningDlg::OnBnClickedButtonReadSn()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int ret = 0;
	CString Str;

	/* 1. Read the SN */
	char sn[SN_LEN + 1] = {0};
	ret = readSN(NULL, NULL, sn);
	if (ret < 0)
	{
		goto out;
	}

	/* 2. Show the SN*/
	Str = (CString) sn;
	CEdit * read_sn = (CEdit*) GetDlgItem(IDC_EDIT_READ_SN);
	read_sn->SetWindowText(Str);

out:
	if (ret < 0)
	{
		char err_str[1024] = {0};
		getErrorString(ret, err_str);
		CString cstr_err(err_str);
		CString retstr;
		retstr.Format(L"Read SN Error:%s", cstr_err);
		MessageBox(retstr);
	}
}


void CSNBurningDlg::OnBnClickedButtonAutoGenerate()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString Str;
	/* 1. Get CPUID */
	char cpuid[CPUID_LEN] = {0};
	int ret = readCPUID(NULL, NULL, cpuid);
	if (ret < 0)
		goto out;

	/* 2. Generate SN via remote server */			
	char sn[SN_LEN + 1] = {0};
	ret = generateSNRemote(cpuid, sn);	
	if (ret < 0)
		goto out;

	Str = (CString) sn;
	CEdit * write_sn = (CEdit*) GetDlgItem(IDC_EDIT_WRITE_SN);
	write_sn->SetWindowText(Str);

	bool is_valid = isvalidSNRemote(cpuid, sn);
out:
	if (ret < 0)
	{
		char err_str[1024] = {0};
		getErrorString(ret, err_str);
		CString cstr_err(err_str);
		CString retstr;
		retstr.Format(L"Generate SN Error:%s", cstr_err);
		MessageBox(retstr);
	}
}


void CSNBurningDlg::OnBnClickedButtonClearRsn()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString Str("");
	CEdit * read_sn = (CEdit*) GetDlgItem(IDC_EDIT_READ_SN);
	read_sn->SetWindowText(Str);
}


void CSNBurningDlg::OnBnClickedButtonWriteSn()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int ret = 0;
	CString CStr_sn, CStr_count;
	GetDlgItem(IDC_EDIT_WRITE_SN)->GetWindowTextW(CStr_sn);

	if (CStr_sn != L"")
	{
		/* 1. Convert the String into char array */
		char sn[SN_LEN + 1] = {0};
		LPCTSTR p =(LPCTSTR)CStr_sn;
		int len = CStr_sn.GetLength() < SN_LEN ? CStr_sn.GetLength() : SN_LEN;
		for ( int i = 0; i < len; i++ )
		{
			sn[i] = (char) p[i];
		}

		/* 2. Write the SN */
		ret = writeSN(NULL, NULL, sn);
		if (ret < 0)
			goto out;

		/* 3. Update the count to write SN successfully */
		GetDlgItem(IDC_STATIC_NUM)->GetWindowTextW(CStr_count);
		LPCTSTR q =(LPCTSTR)CStr_count;
		char str_count[100] = {0};
		int str_count_len = CStr_count.GetLength() < 99 ? CStr_count.GetLength() : 99;
		for ( int i = 0; i < str_count_len; i++ )
		{
			str_count[i] = (char) q[i];
		}
		int count = atoi(str_count);
		count++;
		itoa(count, str_count, 10);

		CStr_count = (CString) str_count;
		GetDlgItem(IDC_STATIC_NUM)->SetWindowText(CStr_count);

		/*4. Clear the SN to write to avoid the SN be write into multi-board*/
		//this->TBWSN->Text = L"";
		CStr_sn = (CString) "";
		CEdit * write_sn = (CEdit*) GetDlgItem(IDC_EDIT_WRITE_SN);
		write_sn->SetWindowText(CStr_sn);
	}
out:
	if (ret < 0)
	{
		char err_str[1024] = {0};
		getErrorString(ret, err_str);
		CString cstr_err(err_str);
		CString retstr;
		retstr.Format(L"Write SN Error:%s", cstr_err);
		MessageBox(retstr);
	}
}


void CSNBurningDlg::OnBnClickedButtonClearWsn()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString Str("");
	CEdit * read_sn = (CEdit*) GetDlgItem(IDC_EDIT_WRITE_SN);
	read_sn->SetWindowText(Str);
}
