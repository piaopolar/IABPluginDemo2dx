//
//  PaymentInterface.h
//
//  Created by Ruoqian, Chen on 2014/12/20
//


#ifndef  __PAYMENT_INTERFACE_H__
#define  __PAYMENT_INTERFACE_H__

#include <string>
#include <vector>

class PaymentInterface
{
public:
	static void ReqItemInfo(const std::vector<std::string>& vecItemTypeId);

	static void PayStart(const char *pszItemTypeId, const char *pszExtraVerifyInfo);
	static void PayEnd(const char *pszItemKey);
};
#endif  //__INTERFACE_FACEBOOK_H__
