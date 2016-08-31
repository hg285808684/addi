#if !defined(AFX_MAINWINDOWDLG_H__89C0900D_24C6_435C_BC1A_073CB5B95BCE__INCLUDED_)
#define AFX_MAINWINDOWDLG_H__89C0900D_24C6_435C_BC1A_073CB5B95BCE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MainWindowDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMainWindowDlg dialog
#define	 WM_NOTIFY_UPDATE	WM_APP + 1
#define  WM_MH_MESSAGE WM_APP+100
#include "AddinInterface.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <chrono>
#include <iostream>
#include <vector>
#include <list>
#include <unordered_map>
#include <queue>
#include "stdafx.h"
#include "ProtoBufMsgHub.h"
#include <string.h>
//#include "CedarLogging.h"
#include "CedarHelper.h"
#include "Parameter.h"
#include "LogHelper.h"

#include <string.h>

using namespace std;


class CMainWindowDlg : public CDialog
{
// Construction
public:
	void WriteLog(CString strLog);
	CMainWindowDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMainWindowDlg)
	enum { IDD = IDD_DIALOG_MAIN_WINDOW };
	CEdit	m_editMsg;
	int		m_iDirection;
	int		m_iOffSet;
	int		m_iVolume;
	float	m_fPrice;
	int		m_iPriceType;
	BOOL	m_bHedge;
	CString	m_strInstrumentID;
	int		m_iOrderActionID;
	//}}AFX_DATA

	BOOL m_bInit;

	void ReportUpdate(LPARAM l);
	void OrderStatus(BARGAIN_NOTIFY_KSI * pKsi);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainWindowDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL
	
// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMainWindowDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnMove(int x, int y);
	afx_msg void OnPaint();
	virtual void OnCancel();
	afx_msg void OnClose();
	afx_msg void OnButton1();
	afx_msg void OnCheckRport1();
	afx_msg void OnCheckReport2();
	afx_msg void OnButtonLoad();
	afx_msg void OnButtonPing();
	afx_msg void OnCheckStart();
//	afx_msg void OnButton1Gedata();
//	afx_msg void OnButtonGet2();
//	afx_msg void OnButtonOrderin();
//	afx_msg void OnButtonOrderAction();
	//}}AFX_MSG
	afx_msg LRESULT OnNotifyUpdate(WPARAM w,LPARAM l);
	DECLARE_MESSAGE_MAP()


private:
	CString m_strInfo;

	ProtoBufMsgHub msgHub;
	string sendAddress;

	int onMsg(MessageBase msg);
	int sendOrder(OrderRequest &ord);
	int subscribe(MessageBase &msg);
	int cancelOrder(OrderRequest &ord);
	int stringToInt(const string& str);
	/*void setUpdate(REPORT_STRUCT_EX * pData, MarketUpdate mUpdate);*/
	int cancelReport(BARGAIN_NOTIFY_KSI * pKsi, string id);
	int filledReport(BARGAIN_NOTIFY_KSI * pKsi, string id);
	int confirmReport(BARGAIN_NOTIFY_KSI * pKsi, string id);
	int inactiveReport(BARGAIN_NOTIFY_KSI * pKsi, string id);
	void logReport(BARGAIN_NOTIFY_KSI * pData);

	queue<MessageBase> msgTmp;
	
	unordered_map<string, int> idMap;
	unordered_map<int, string> invIdMap;
	unordered_map<string, string> respMap;

public:
//	afx_msg void OnEnChangeEditInfo();
	afx_msg void OnBnClickedButton1Gedata();
	afx_msg LRESULT OnMsgHubMessage(WPARAM wParam, LPARAM lParam);

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINWINDOWDLG_H__89C0900D_24C6_435C_BC1A_073CB5B95BCE__INCLUDED_)
