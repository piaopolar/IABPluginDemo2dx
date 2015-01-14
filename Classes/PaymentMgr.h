//
//  PaymentMgr.h
//
//  Created by Ruoqian, Chen on 2014/12/20
//

#ifndef  __PAYMENT_MGR_H__
#define  __PAYMENT_MGR_H__

#include <vector>
#include "ObserverPattern.h"

enum
{
	PAY_EVENT_QUERY_SKU_FIN,
};

enum
{
	PAY_VERIFY_NONE = 0,
	PAY_VERIFY_CLIENT = 1,
	PAY_VERIFY_SERVER = 1 << 1,
	PAY_VERIFY_BOTH = PAY_VERIFY_CLIENT | PAY_VERIFY_SERVER,
};

struct PAY_ITEMINFO
{
	std::string m_strKey;
	std::string m_strTypeId;
	std::string m_strName;
	std::string m_strDesc;
	std::string m_strPrice;
};

class PaymentMgr : public Subject
{
public:
	static PaymentMgr *GetInstance(void);
	static void Release(void);

	void ReqItemInfo(void);	
	const std::map<std::string, PAY_ITEMINFO>& GetItemInfo(void) const;
	const PAY_ITEMINFO* GetItemInfo(const char *pszItemTypeId) const;
	void ClearItemInfo(void);
	void AddItemInfo(const PAY_ITEMINFO& info);

	bool TestVerifyMode(int nMode);
	void PayStart(const char *pszItemTypeId);

	void OnPurchased(const char *pszItemKey, const char *pszItemInfo, const char *pszVerifyInfo);
	void OnFailed(const char* pszItemKey, const char *pszInfo);
	void OnRestore(const char* pszItemKey, const char *pszItemInfo, const char *pszVerifyInfo);
	void OnServerVerifyResult(const char *pszItemKey, bool bVerifyFin);
private:
	PaymentMgr();
	void PayServerVerify(const char* pszItemKey, const char *pszVerifyInfo);
	void PayEnd(const char *pszItemKey);

	int m_nVerifyMode;
	std::map<std::string, PAY_ITEMINFO> m_mapItem;
};
#endif  //__INTERFACE_FACEBOOK_H__
