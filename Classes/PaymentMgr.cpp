//
//  PaymentMgr.cpp
//
//  Created by Ruoqian, Chen on 2014/12/20
//

#include "StdAfx.h"

#include "PaymentInterface.h"
#include "PaymentMgr.h"

namespace
{
static PaymentMgr* s_pInstance = NULL;
}

PaymentMgr * PaymentMgr::GetInstance( void )
{
	if (NULL == s_pInstance) {
		s_pInstance = new PaymentMgr;
	}

	return s_pInstance;
}

void PaymentMgr::Release( void )
{
	CC_SAFE_DELETE(s_pInstance);
}

PaymentMgr::PaymentMgr()
{
	m_nVerifyMode = PAY_VERIFY_CLIENT;
}

void PaymentMgr::ReqItemInfo( void )
{
	if (!m_vecItem.empty()) {
		return;
	}

	const char *pszConfig = NULL;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	pszConfig = "GooglePlayIAB";
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	pszConfig = "AppleIAP";
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	pszConfig = "WinSim";
#endif

	if (NULL == pszConfig) {
		return;
	}

	auto infoCfg = CIniMgr::GetInstance()->GetSectionInfo("ini/payment.ini", pszConfig);
	std::vector<std::string> vecItemTypeId;
	for (auto& rInfo : infoCfg) {
		vecItemTypeId.push_back(rInfo.second);
	}

	PaymentInterface::ReqItemInfo(vecItemTypeId);
}

void PaymentMgr::ClearItemInfo( void )
{
	m_vecItem.clear();
}

void PaymentMgr::AddItemInfo( const PAY_ITEMINFO& info )
{
	m_vecItem.push_back(info);
}

const std::vector<PAY_ITEMINFO>& PaymentMgr::GetItemInfo( void ) const
{
	return m_vecItem;
}

bool PaymentMgr::TestVerifyMode( int nMode )
{
	return (nMode & m_nVerifyMode) ? true : false;
}

void PaymentMgr::OnRestore( const char *pszItemKey, const char *pszItemInfo, const char *pszVerifyInfo )
{
	CCLog("PaymentMgr::OnRestore [%s] [%s]", pszItemKey, pszVerifyInfo);

	if (this->TestVerifyMode(PAY_VERIFY_SERVER)) {
		this->PayServerVerify(pszItemKey, pszVerifyInfo);
	} else {
		this->PayEnd(pszItemKey);
	}
}

void PaymentMgr::PayStart( const char *pszItemTypeId )
{
	CCLog("PaymentMgr::PayStart [%s]", pszItemTypeId);

	PaymentInterface::PayStart(pszItemTypeId);
}

void PaymentMgr::OnPurchased( const char *pszItemKey, const char *pszItemInfo, const char *pszVerifyInfo )
{
	CCLog("PaymentMgr::OnPurchased [%s] [%s]", pszItemKey, pszVerifyInfo);

	if (this->TestVerifyMode(PAY_VERIFY_SERVER)) {
		this->PayServerVerify(pszItemKey, pszVerifyInfo);
	} else {
		this->PayEnd(pszItemKey);
	}
}

void PaymentMgr::OnFailed( const char* pszItemKey, const char *pszInfo )
{
	CCLog("PaymentMgr::OnFailed [%s] [%s]", pszItemKey, pszInfo);

	this->PayEnd(pszItemKey);
}

void PaymentMgr::PayServerVerify( const char* pszItemKey, const char *pszVerifyInfo )
{
	CCLog("PaymentMgr::PayServerVerify [%s] [%s]", pszItemKey, pszVerifyInfo);

	// TODO: 
	//   Send msg to game server to verify, and call OnServerVerifyResult when recv response.
	//   Since this is a SAMPLE, we just call OnServerVerifyResult here
	this->OnServerVerifyResult(pszItemKey, true);
}

void PaymentMgr::OnServerVerifyResult( const char *pszItemKey, bool bVerifyFin )
{
	CCLog("PaymentMgr::OnServerVerifyResult [%s] [%s]", pszItemKey, bVerifyFin ? "true" : "false");

	if (bVerifyFin) {
		this->PayEnd(pszItemKey);
	}
}

void PaymentMgr::PayEnd( const char *pszItemKey )
{
	CCLog("PaymentMgr::PayEnd [%s]", pszItemKey);

	// if it a non-consumable like item and ONLY save owned status in google IAB, DONOT call it
	PaymentInterface::PayEnd(pszItemKey);
}
