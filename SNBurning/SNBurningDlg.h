
// SNBurningDlg.h : ͷ�ļ�
//

#pragma once


// CSNBurningDlg �Ի���
class CSNBurningDlg : public CDialogEx
{
// ����
public:
	CSNBurningDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SNBURNING_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedGbs();
	afx_msg void OnBnClickedRadioAuto();
	afx_msg void OnBnClickedRadioManual();
	afx_msg void OnBnClickedButtonReadSn();
	afx_msg void OnBnClickedButtonAutoGenerate();
	afx_msg void OnBnClickedButtonClearRsn();
	afx_msg void OnBnClickedButtonWriteSn();
	afx_msg void OnBnClickedButtonClearWsn();
	void getErrorString(int err_code, char *err_str);
};
