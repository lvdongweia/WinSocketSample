
// SNBurningDlg.h : 头文件
//

#pragma once


// CSNBurningDlg 对话框
class CSNBurningDlg : public CDialogEx
{
// 构造
public:
	CSNBurningDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SNBURNING_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
