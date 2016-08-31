// MainWindowDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AddinDemo.h"
#include "MainWindowDlg.h"
#include "math.h"
#include "LogInfo.h"
#include "IncludeOnlyInMain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainWindowDlg dialog

CMainWindowDlg::CMainWindowDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMainWindowDlg::IDD, pParent) {
	//{{AFX_DATA_INIT(CMainWindowDlg)
	m_iDirection = 0;
	m_iOffSet = 0;
	m_iVolume = 1;
	m_fPrice = 1;
	m_iPriceType = 0;
	m_bHedge = FALSE;
	m_strInstrumentID = _T("SQCU01");
	m_iOrderActionID = 0;
	//}}AFX_DATA_INIT
	m_bInit = TRUE;
}

void CMainWindowDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMainWindowDlg)
	DDX_Control(pDX, IDC_EDIT_INFO, m_editMsg);
	
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMainWindowDlg, CDialog)
	//{{AFX_MSG_MAP(CMainWindowDlg)
	ON_WM_MOVE()
	ON_WM_PAINT()
	ON_WM_CLOSE()
//	ON_BN_CLICKED(IDC_BUTTON1_GEDATA, OnButton1Gedata)
//	ON_BN_CLICKED(IDC_BUTTON_GET2, OnButtonGet2)
//	ON_BN_CLICKED(IDC_BUTTON_ORDERIN, OnButtonOrderin)
//	ON_BN_CLICKED(IDC_BUTTON_ORDERACTION, OnButtonOrderAction)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_NOTIFY_UPDATE, OnNotifyUpdate)
//	ON_EN_CHANGE(IDC_EDIT_INFO, &CMainWindowDlg::OnEnChangeEditInfo)
	ON_BN_CLICKED(IDC_BUTTON1_GEDATA, &CMainWindowDlg::OnBnClickedButton1Gedata)
	ON_MESSAGE(WM_MH_MESSAGE, OnMsgHubMessage)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainWindowDlg message handlers

BOOL CMainWindowDlg::OnInitDialog() {
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	CDialog::OnInitDialog();
	
	//去除任务栏窗口对应按钮
	ModifyStyleEx (WS_EX_APPWINDOW,WS_EX_TOOLWINDOW );
	
	//记录状态
	WritePrivateProfileInt("AddinDemo","AutoShow",   
		1,"C:\\StockCfg.ini");  

	//注册沪铜行情数据
	/*BOOL tmp = g_pMainFormework->RegReportNotify("10000659",'QQ');
	CString stmp;
	stmp.Format("%s", tmp ? "true" : "false");
	WriteLog(stmp);
	//注册上证指数行情数据
	tmp = g_pMainFormework->RegReportNotify("000001", 'HS');
	stmp.Format("%s", tmp ? "true" : "false");
	WriteLog(stmp);
	//党国不能亏待了你
	tmp = g_pMainFormework->RegReportNotify("RB01", 'SQ');
	stmp.Format("%s", tmp ? "true" : "false");
	WriteLog(stmp);*/
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMainWindowDlg::PostNcDestroy() {
	CDialog::PostNcDestroy();
	g_pdlgMainWindow = NULL;
	delete this;
}

void CMainWindowDlg::OnMove(int x, int y) {
	CDialog::OnMove(x, y);

	//防止调用MoveWindow函数时系统发送此消息
	if(m_bInit)
		return;

	CRect rc;
	GetWindowRect(rc);

	WritePrivateProfileInt("AddinDemo","X",   
		rc.left,"C:\\StockCfg.ini");  
	WritePrivateProfileInt("AddinDemo","Y",   
		rc.top,"C:\\StockCfg.ini");  
}

void CMainWindowDlg::OnPaint() {
	CPaintDC dc(this); // device context for painting
	
	//显示出上证指数的及时报价
	REPORT_STRUCT * pReport = g_pMainFormework->GetReportData("000001",'HS');
	if(pReport) 
	{
		CString strText;
		strText.Format("%s    开盘:%.2f  最高:%.2f  最低:%.2f  最新:%.2f", pReport->m_szName, pReport->m_fOpen, pReport->m_fHigh, pReport->m_fLow, pReport->m_fNewPrice);
		dc.TextOut(20,20,strText);
		//年轻人要多吃点
	}
}

void CMainWindowDlg::OnCancel() {
	WritePrivateProfileInt("AddinDemo","AutoShow",   
		0,"C:\\StockCfg.ini"); 
	
	DestroyWindow();
}

void CMainWindowDlg::OnClose() {
	OnCancel();
}

//接收通知消息
LRESULT CMainWindowDlg::OnNotifyUpdate(WPARAM w,LPARAM l) {
	CString stmp;
	stmp.Format("w:%d", w);
	WriteLog(stmp);
	if(w == 2)
	{
		//注册品种报价变化通知
		ReportUpdate(l);
	}
	else if(w == 3)
	{
		//订单状态变化通知
		BARGAIN_NOTIFY_KSI * pKsi = (BARGAIN_NOTIFY_KSI*)l;
		OrderStatus(pKsi);
	}
	else if(w == 4) {
		BARGAIN_NOTIFY_KSI * pKsi = (BARGAIN_NOTIFY_KSI*)l;
		if(strcmp(pKsi->m_szStatus,"Connected") == 0) {
			//账户已经连接
		}
		else if(strcmp(pKsi->m_szStatus,"Disconnected") == 0) {
			//账户断开连接
		}
	}
	return 0;
}

/*
 订单的变化类型，有如下字符串值：

"Cancelled"  表示订单已经撤销

"Submitted"和"PreSubmitted"  表示订单已经提交，当只成交一部分尚未完全成交时也会出现此事件，此时已成交数量在Filled参数中显示

"Filled"    表示本地订单已经全部成交

"Tradeing"  每笔成交回报，此时Filled是本次成交数量，Remaining将始终为0

"Inactive"  表示本次委托无效，比如价格超过允许范围，委托数量超出范围等等

"PendingSubmit" 本次委托已经发送服务器，但是服务器未发出确认消息（仅IB有效）

"PendingCancel" 本次撤单委托已经发送服务器，但是服务器未发出确认消息（仅IB有效）
*/

void CMainWindowDlg::OrderStatus(BARGAIN_NOTIFY_KSI * pKsi) {
	//AfxMessageBox("status");
	CString strLog;
	strLog.Format("[回报] 挂单%d, 状态 %s", pKsi->m_nOrderID, pKsi->m_szStatus);
	WriteLog(strLog);

	//记录日志到文件
	//LOG_DEBUG_INFO( "c:\\MyData.txt", "[回报] 挂单%d, 状态 %s", pKsi->m_nOrderID, pKsi->m_szStatus);

	if(strcmp(pKsi->m_szStatus,"Submitted") == 0) {
		string id = invIdMap[pKsi->m_nOrderID];
		confirmReport(pKsi, id);
	}
	else if(strcmp(pKsi->m_szStatus ,"Cancelled")==0) {
		string id = invIdMap[pKsi->m_nOrderID];
		idMap.erase(id);
		invIdMap.erase(pKsi->m_nOrderID);
		cancelReport(pKsi, id);
	}
	else if(strcmp(pKsi->m_szStatus,"Filled")==0) {
		string id = invIdMap[pKsi->m_nOrderID];
		idMap.erase(id);
		invIdMap.erase(pKsi->m_nOrderID);
		filledReport(pKsi, id);
	}
	else if(strcmp(pKsi->m_szStatus,"Inactive")==0) {
		string id = invIdMap[pKsi->m_nOrderID];
		idMap.erase(id);
		invIdMap.erase(pKsi->m_nOrderID);
		inactiveReport(pKsi,id);
	}
}


void CMainWindowDlg::logReport(BARGAIN_NOTIFY_KSI * pData) {
	LOG(INFO) << "**********************log begins:******************************";
	LOG(INFO) << pData->m_nOrderID;
	LOG(INFO) << pData->m_szStatus;
	LOG(INFO) << pData->m_nFilled;
	LOG(INFO) << pData->m_nRemaining;
	LOG(INFO) << pData->m_fPrice;
	LOG(INFO) << pData->m_szCode;
	LOG(INFO) << pData->m_szMarket;
	LOG(INFO) << pData->m_nKaiping;
	LOG(INFO) << pData->m_nType;
	LOG(INFO) << pData->m_nAspect;
	LOG(INFO) << pData->m_szAccount;
	LOG(INFO) << pData->m_nAccountType;
	LOG(INFO) << pData->m_fAvgerPrice;
	LOG(INFO) << "***********************log ends:*******************************";
}


//only ID has been reported here
int CMainWindowDlg::inactiveReport(BARGAIN_NOTIFY_KSI * pKsi, string id) {
	ResponseMessage rmsg;
	//rmsg.set_code();
	rmsg.set_ref_id(id);
	rmsg.set_type(TYPE_ERROR);
	rmsg.set_error_code(BAD_ORDER_ERROR);
	msgHub.pushMsg(respMap[id], ProtoBufHelper::wrapMsg(TYPE_RESPONSE_MSG, rmsg));

	LogHelper::logObject(rmsg);
	LOG(INFO) << "resp to:" <<respMap[id];
	respMap.erase(id);
	return 0;
}

int CMainWindowDlg::filledReport(BARGAIN_NOTIFY_KSI * pKsi, string id) {
	ResponseMessage rmsg;
	rmsg.set_ref_id(id);
	rmsg.set_type(TYPE_TRADE);
	rmsg.set_error_code(DEAL);
	msgHub.pushMsg(respMap[id], ProtoBufHelper::wrapMsg(TYPE_RESPONSE_MSG, rmsg));

	LogHelper::logObject(rmsg);
	LOG(INFO) << "resp to:" << respMap[id];
	respMap.erase(id);
	return 0;
}

int CMainWindowDlg::confirmReport(BARGAIN_NOTIFY_KSI * pKsi, string id) {
	ResponseMessage rmsg;
	rmsg.set_ref_id(id);
	rmsg.set_type(TYPE_NEW_ORDER_CONFIRM);
	rmsg.set_error_code(NONE_ERROR);
	msgHub.pushMsg(respMap[id], ProtoBufHelper::wrapMsg(TYPE_RESPONSE_MSG, rmsg));

	LogHelper::logObject(rmsg);
	LOG(INFO) << "resp to:" << respMap[id];
	//respMap.erase(id);
	return 0;
}

int CMainWindowDlg::cancelReport(BARGAIN_NOTIFY_KSI * pKsi, string id) {
	ResponseMessage rmsg;
	rmsg.set_ref_id(id);
	rmsg.set_type(TYPE_CANCEL_ORDER_CONFIRM);
	rmsg.set_error_code(CANCELLED);
	msgHub.pushMsg(respMap[id], ProtoBufHelper::wrapMsg(TYPE_RESPONSE_MSG, rmsg));

	LogHelper::logObject(rmsg); 
	LOG(INFO) << "resp to:" << respMap[id];
	respMap.erase(id);
	return 0;
}

/*void CMainWindowDlg::setUpdate(REPORT_STRUCT_EX * pData, MarketUpdate mUpdate) {

	CString stmp;
	time_t timetmp = (time_t)pData->m_time;
	tm *ttmp;
	ttmp = localtime(&timetmp);
	stmp.Format("%d:%d:%d", ttmp->tm_hour, ttmp->tm_min, ttmp->tm_sec);

	mUpdate.set_code(pData->m_szLabel);
	mUpdate.set_symbol(pData->m_szName);
	//mUpdate.set_exchange(pData->m_szName);
	mUpdate.set_pre_close_price(pData->m_fLastClose);
	mUpdate.set_open_price(pData->m_fOpen);
	mUpdate.set_last_price(pData->m_fNewPrice);
	mUpdate.set_highest_price(pData->m_fHigh);
	mUpdate.set_lowest_price(pData->m_fLow);
	//mUpdate.set_num_trades()
	mUpdate.set_exchange_timestamp(stmp);

	mUpdate.add_bid_price(pData->m_fBuyPrice[0]);
	mUpdate.add_bid_price(pData->m_fBuyPrice[1]);
	mUpdate.add_bid_price(pData->m_fBuyPrice[2]);
	mUpdate.add_bid_price(pData->m_fBuyPrice4);
	mUpdate.add_bid_price(pData->m_fBuyPrice5);
	mUpdate.add_bid_price(pData->m_fBuyPrice6);
	mUpdate.add_bid_price(pData->m_fBuyPrice7);
	mUpdate.add_bid_price(pData->m_fBuyPrice8);
	mUpdate.add_bid_price(pData->m_fBuyPrice9);
	mUpdate.add_bid_price(pData->m_fBuyPrice10);

	mUpdate.add_ask_price(pData->m_fSellPrice[0]);
	mUpdate.add_ask_price(pData->m_fSellPrice[1]);
	mUpdate.add_ask_price(pData->m_fSellPrice[2]);
	mUpdate.add_ask_price(pData->m_fSellPrice4);
	mUpdate.add_ask_price(pData->m_fSellPrice5);
	mUpdate.add_ask_price(pData->m_fSellPrice6);
	mUpdate.add_ask_price(pData->m_fSellPrice7);
	mUpdate.add_ask_price(pData->m_fSellPrice8);
	mUpdate.add_ask_price(pData->m_fSellPrice9);
	mUpdate.add_ask_price(pData->m_fSellPrice10);

	mUpdate.add_bid_volume(pData->m_fBuyVolume[0]);
	mUpdate.add_bid_volume(pData->m_fBuyVolume[1]);
	mUpdate.add_bid_volume(pData->m_fBuyVolume[2]);
	mUpdate.add_bid_volume(pData->m_fBuyVolume4);
	mUpdate.add_bid_volume(pData->m_fBuyVolume5);
	mUpdate.add_bid_volume(pData->m_fBuyVolume6);
	mUpdate.add_bid_volume(pData->m_fBuyVolume7);
	mUpdate.add_bid_volume(pData->m_fBuyVolume8);
	mUpdate.add_bid_volume(pData->m_fBuyVolume9);
	mUpdate.add_bid_volume(pData->m_fBuyVolume10);

	mUpdate.add_ask_volume(pData->m_fSellVolume[0]);
	mUpdate.add_ask_volume(pData->m_fSellVolume[1]);
	mUpdate.add_ask_volume(pData->m_fSellVolume[2]);
	mUpdate.add_ask_volume(pData->m_fSellVolume4);
	mUpdate.add_ask_volume(pData->m_fSellVolume5);
	mUpdate.add_ask_volume(pData->m_fSellVolume6);
	mUpdate.add_ask_volume(pData->m_fSellVolume7);
	mUpdate.add_ask_volume(pData->m_fSellVolume8);
	mUpdate.add_ask_volume(pData->m_fSellVolume9);
	mUpdate.add_ask_volume(pData->m_fSellVolume10);
}*/

void CMainWindowDlg::ReportUpdate(LPARAM l) {
	REPORT_STRUCT_EX * pData = (REPORT_STRUCT_EX*)l;

	MarketUpdate mUpdate;
	//setUpdate(pData, mUpdate);

	CString stmp;
	time_t timetmp = (time_t)pData->m_time;
	tm *ttmp;
	ttmp = localtime(&timetmp);
	stmp.Format("%d:%d:%d", ttmp->tm_hour, ttmp->tm_min, ttmp->tm_sec);

	mUpdate.set_code(pData->m_szLabel);
	//mUpdate.set_symbol(pData->m_szName);
	//mUpdate.set_exchange(pData->m_szName);
	mUpdate.set_pre_close_price(pData->m_fLastClose);
	mUpdate.set_open_price(pData->m_fOpen);
	mUpdate.set_last_price(pData->m_fNewPrice);
	mUpdate.set_highest_price(pData->m_fHigh);
	mUpdate.set_lowest_price(pData->m_fLow);
	//mUpdate.set_num_trades()
	mUpdate.set_exchange_timestamp(stmp);

	mUpdate.add_bid_price(pData->m_fBuyPrice[0]);
	mUpdate.add_bid_price(pData->m_fBuyPrice[1]);
	mUpdate.add_bid_price(pData->m_fBuyPrice[2]);
	mUpdate.add_bid_price(pData->m_fBuyPrice4);
	mUpdate.add_bid_price(pData->m_fBuyPrice5);
	mUpdate.add_bid_price(pData->m_fBuyPrice6);
	mUpdate.add_bid_price(pData->m_fBuyPrice7);
	mUpdate.add_bid_price(pData->m_fBuyPrice8);
	mUpdate.add_bid_price(pData->m_fBuyPrice9);
	mUpdate.add_bid_price(pData->m_fBuyPrice10);

	mUpdate.add_ask_price(pData->m_fSellPrice[0]);
	mUpdate.add_ask_price(pData->m_fSellPrice[1]);
	mUpdate.add_ask_price(pData->m_fSellPrice[2]);
	mUpdate.add_ask_price(pData->m_fSellPrice4);
	mUpdate.add_ask_price(pData->m_fSellPrice5);
	mUpdate.add_ask_price(pData->m_fSellPrice6);
	mUpdate.add_ask_price(pData->m_fSellPrice7);
	mUpdate.add_ask_price(pData->m_fSellPrice8);
	mUpdate.add_ask_price(pData->m_fSellPrice9);
	mUpdate.add_ask_price(pData->m_fSellPrice10);

	mUpdate.add_bid_volume(pData->m_fBuyVolume[0]);
	mUpdate.add_bid_volume(pData->m_fBuyVolume[1]);
	mUpdate.add_bid_volume(pData->m_fBuyVolume[2]);
	mUpdate.add_bid_volume(pData->m_fBuyVolume4);
	mUpdate.add_bid_volume(pData->m_fBuyVolume5);
	mUpdate.add_bid_volume(pData->m_fBuyVolume6);
	mUpdate.add_bid_volume(pData->m_fBuyVolume7);
	mUpdate.add_bid_volume(pData->m_fBuyVolume8);
	mUpdate.add_bid_volume(pData->m_fBuyVolume9);
	mUpdate.add_bid_volume(pData->m_fBuyVolume10);

	mUpdate.add_ask_volume(pData->m_fSellVolume[0]);
	mUpdate.add_ask_volume(pData->m_fSellVolume[1]);
	mUpdate.add_ask_volume(pData->m_fSellVolume[2]);
	mUpdate.add_ask_volume(pData->m_fSellVolume4);
	mUpdate.add_ask_volume(pData->m_fSellVolume5);
	mUpdate.add_ask_volume(pData->m_fSellVolume6);
	mUpdate.add_ask_volume(pData->m_fSellVolume7);
	mUpdate.add_ask_volume(pData->m_fSellVolume8);
	mUpdate.add_ask_volume(pData->m_fSellVolume9);
	mUpdate.add_ask_volume(pData->m_fSellVolume10);


	LOG(INFO) << mUpdate.DebugString();
	LOG(INFO) << mUpdate.code();
	LOG(INFO) << mUpdate.last_price();
	LOG(INFO) << pData->m_szLabel;
	string chan = pData->m_szLabel;
	chan = "QQ" + chan;
	msgHub.boardcastMsg(chan, ProtoBufHelper::wrapMsg(TYPE_MARKETUPDATE, mUpdate));
}

int CMainWindowDlg::sendOrder(OrderRequest &ord) {
	CString tmp;
	ResponseMessage rmsg;

	//WriteLog(ord.DebugString().c_str());
	string market = "QQ";

	m_iOffSet = 0;
	m_iVolume = ord.trade_quantity();
	m_fPrice = ord.limit_price();
	if (ord.buy_sell() == LONG_BUY) {
		m_iDirection = 0;
	}
	else if (ord.buy_sell() == SHORT_SELL) {
		m_iDirection = 1;
	}
	if (ord.open_close() == OPEN_POSITION) {
		m_iOffSet = 0;
	}
	else if (ord.open_close() == CLOSE_POSITION) {
		m_iOffSet = 1;
	}
	m_iPriceType = 0;
	m_bHedge = FALSE;
	m_strInstrumentID = _T("SQCU01");
	m_iOrderActionID = 0;
	//}}AFX_DATA_INIT
	m_bInit = TRUE;

	CString lpszLabel = ord.code().c_str();

	//long orderID = g_pMainFormework->PlaceOrder(nType, fLmtPrice, fStopLmtPrice, nVol, nAspact, lpszLabel, wMarket, bMustOK, lpszAccount, nKaiPing, nTouBao, bOrderQueue);
	//tmp.Format("position:%d, open-close:%d",ord.open_close().);
	long orderID = g_pMainFormework->PlaceOrder(m_iPriceType, m_fPrice, 0, m_iVolume, m_iDirection, lpszLabel, 20817, 0, "", m_iOffSet, m_bHedge, 0);
	if (orderID == -1) {
		WriteLog("下单失败!");
		CString bug;
		bug.Format("1: %d, 2: %f, 3: %d, 4: %d, 5:%s, 6:%d, 7:%d", m_iPriceType, m_fPrice, m_iVolume, m_iDirection, lpszLabel, m_iOffSet, m_bHedge);
		//WriteLog(bug);

		rmsg.set_ref_id(ord.id());
		rmsg.set_type(TYPE_ERROR);
		rmsg.set_error_code(SEND_ERROR);
		msgHub.pushMsg(ord.response_address(), ProtoBufHelper::wrapMsg(TYPE_RESPONSE_MSG, rmsg));
		return 1;
	}
	else {
		idMap.insert(pair<string, int>(ord.id() ,orderID));
		invIdMap.insert(pair<int, string>(orderID, ord.id()));
		respMap.insert(pair<string, string>(ord.id(),ord.response_address()));

		CString re;
		re.Format("order sent: %d", orderID);
		WriteLog(re);
		re.Format("map %s to %d", ord.id().c_str(),orderID);
		WriteLog(re);

		/*rmsg.set_ref_id(ord.id());
		rmsg.set_type(TYPE_NEW_ORDER_CONFIRM);
		rmsg.set_error_code(NONE_ERROR);
		msgHub.pushMsg(ord.response_address(), ProtoBufHelper::wrapMsg(TYPE_RESPONSE_MSG, rmsg));*/
	}
	return 0;
}

int CMainWindowDlg::subscribe(MessageBase &msg) {
	DataRequest req = ProtoBufHelper::unwrapMsg<DataRequest>(msg);
	BOOL result = g_pMainFormework->RegReportNotify(const_cast<char*>(req.code().c_str()),'QQ');
	if (result == FALSE) {
		WriteLog("subscribe failed");
	}
	WriteLog("subscribe");

	MarketUpdate mUpdate;
	//setUpdate(pData, mUpdate);
	/*while (1) {
		string chan = "10000555";
		chan = "QQ" + chan;
		mUpdate.set_code("this is a test!");
		msgHub.boardcastMsg(chan, ProtoBufHelper::wrapMsg(TYPE_MARKETUPDATE, mUpdate));
	}*/
	return 0;
}

int CMainWindowDlg::stringToInt(const string& str) {
	istringstream iss(str);
	int num;
	iss >> num;
	return num;
}

int CMainWindowDlg::cancelOrder(OrderRequest &ord) {
	LONG_PTR cancel_id = idMap[ord.cancel_order_id()];
	CString tmp;
	tmp.Format("cancel:%d,%s", cancel_id, ord.cancel_order_id().c_str());
	WriteLog(tmp);
	g_pMainFormework->OrderCancel(cancel_id, 0);
	return 0;
}

afx_msg LRESULT CMainWindowDlg::OnMsgHubMessage(WPARAM wParam, LPARAM lParam) {
	while (!msgTmp.empty()) {
		MessageBase msg = msgTmp.front();
		if (msg.type() == TYPE_ORDER_REQUEST) {
			OrderRequest ord = ProtoBufHelper::unwrapMsg<OrderRequest>(msg);
			//WriteLog(ord.DebugString().c_str());
			if (ord.type() == TYPE_LIMIT_ORDER_REQUEST) {
				sendOrder(ord);
			}
			else if (ord.type() == TYPE_CANCEL_ORDER_REQUEST) {
				WriteLog(ord.DebugString().c_str());
				cancelOrder(ord);
			}
		}
		else if (msg.type() == TYPE_DATAREQUEST) {
			//subscribe(msg);
			DataRequest req = ProtoBufHelper::unwrapMsg<DataRequest>(msg);
			REPORT_STRUCT* tmp = g_pMainFormework->GetReportData(const_cast<char*>(req.code().c_str()), 'QQ');
			ReportUpdate((LPARAM) tmp);
		}
		msgTmp.pop();
	}
	return 0;
}

int CMainWindowDlg::onMsg(MessageBase msg) {
	msgTmp.push(msg);
	//msgTmp.CopyFrom(msg);
	//WriteLog(msgTmp.DebugString().c_str());
	PostMessage(WM_MH_MESSAGE);
	//WriteLog("msg get");
	return 0;
}

void CMainWindowDlg::WriteLog(CString strLog) {
	SYSTEMTIME stime;
	COleDateTime timeNow = COleDateTime::GetCurrentTime();
	GetLocalTime(&stime);
	//m_strInfo += timeNow.Format("%Y.%m.%d %H:%M:%S") + " ";
	CString tmp;
	tmp.Format("%d-%d-%d:%d", stime.wHour,stime.wMinute, stime.wSecond, stime.wMilliseconds);
	m_strInfo += tmp;
	m_strInfo += strLog;
	m_strInfo += "\r\n";
	
	m_editMsg.SetWindowText(m_strInfo);
	m_editMsg.SetSel(m_strInfo.GetLength(),m_strInfo.GetLength());
}

void CMainWindowDlg::OnBnClickedButton1Gedata() {
	std::string configPath = "C:\\Weisoft Stock(x86)\\AddinDemo.json";
	CedarJsonConfig::getInstance().loadConfigFile(configPath);
	CedarLogger::init();

	ProtoBufHelper::setupProtoBufMsgHub(msgHub);
	msgHub.registerCallback(std::bind(&CMainWindowDlg::onMsg, this, std::placeholders::_1));

	AfxMessageBox("init finish");
}