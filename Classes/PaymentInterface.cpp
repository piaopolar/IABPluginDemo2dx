//
//  PaymentInterface.cpp
//
//  Created by Ruoqian, Chen on 2014/12/20
//

#include "StdAfx.h"

#include "PaymentInterface.h"
#include "PaymentMgr.h"
#include "UITip.h"
#include "spine/Json.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "platform/android/jni/JniHelper.h"
#include <jni.h>
#include <android/log.h>
const char* GP_IAB_JavaClassName = "org/PayPlugin/GooglePlayIABPlugin";

void GooglePayInAppBilling_OnReceiveItemInfo(const char *pszInfo);

extern "C"
{
	void Java_org_PayPlugin_GooglePlayIABPlugin_nativeOnReceiveItemInfo(JNIEnv* jEnv, jobject jObj, jstring params)
	{
		const char *pszParams = params ? jEnv->GetStringUTFChars(params, NULL) : NULL;
		if (pszParams) {
			GooglePayInAppBilling_OnReceiveItemInfo(pszParams);
			jEnv->ReleaseStringUTFChars(params, pszParams);
		}
	}

	void Java_org_PayPlugin_GooglePlayIABPlugin_nativeOnPurchased(JNIEnv* jEnv, jobject jObj, jstring jstrJsonPurchaseInfo, jstring jstrSignature)
	{
		const char *pszJsonPurchaseInfo = jstrJsonPurchaseInfo ? jEnv->GetStringUTFChars(jstrJsonPurchaseInfo, NULL) : NULL;
		const char *pszSignature = jstrSignature ? jEnv->GetStringUTFChars(jstrSignature, NULL) : NULL;

		auto* pJsonValue = Json_create(pszJsonPurchaseInfo);
		if (pJsonValue) {
			std::string strItemTypeId = Json_getString(pJsonValue, "productId", "");
			PaymentMgr::GetInstance()->OnPurchased(strItemTypeId.c_str(), pszJsonPurchaseInfo, pszSignature);
			Json_dispose(pJsonValue);
		}

		if (pszJsonPurchaseInfo) {
			jEnv->ReleaseStringUTFChars(jstrJsonPurchaseInfo, pszJsonPurchaseInfo);
		}

		if (pszSignature) {
			jEnv->ReleaseStringUTFChars(jstrSignature, pszSignature);
		}
	}

	void Java_org_PayPlugin_GooglePlayIABPlugin_nativeOnFailed(JNIEnv* jEnv, jobject jObj, jstring jItemKey, jstring jInfo)
	{
		const char *pszItemKey = jItemKey ? jEnv->GetStringUTFChars(jItemKey, NULL) : NULL;
		const char *pszInfo = jInfo ? jEnv->GetStringUTFChars(jInfo, NULL) : NULL;

		PaymentMgr::GetInstance()->OnFailed(pszItemKey, pszInfo);

		if (pszItemKey) {
			jEnv->ReleaseStringUTFChars(jItemKey, pszItemKey);
		}

		if (pszInfo) {
			jEnv->ReleaseStringUTFChars(jInfo, pszInfo);
		}
	}

	void Java_org_PayPlugin_GooglePlayIABPlugin_nativeOnRestore(JNIEnv* jEnv, jobject jObj, jstring jstrJsonPurchaseInfo, jstring jstrSignature)
	{
		const char *pszJsonPurchaseInfo = jstrJsonPurchaseInfo ? jEnv->GetStringUTFChars(jstrJsonPurchaseInfo, NULL) : NULL;
		const char *pszSignature = jstrSignature ? jEnv->GetStringUTFChars(jstrSignature, NULL) : NULL;

		auto* pJsonValue = Json_create(pszJsonPurchaseInfo);
		if (pJsonValue) {
			std::string strItemTypeId = Json_getString(pJsonValue, "productId", "");
			PaymentMgr::GetInstance()->OnRestore(strItemTypeId.c_str(), pszJsonPurchaseInfo, pszSignature);
			Json_dispose(pJsonValue);
		}

		if (pszJsonPurchaseInfo) {
			jEnv->ReleaseStringUTFChars(jstrJsonPurchaseInfo, pszJsonPurchaseInfo);
		}

		if (pszSignature) {
			jEnv->ReleaseStringUTFChars(jstrSignature, pszSignature);
		}
	}
};
#endif

void GooglePayInAppBilling_OnReceiveItemInfo( const char *pszInfo )
{
	auto* pJsonValue = Json_create(pszInfo);
	if (NULL == pJsonValue) {
		return;
	}

	PaymentMgr::GetInstance()->ClearItemInfo();

	int nSize = Json_getSize(pJsonValue);
	for (int i = 0; i < nSize; ++i) {
		PAY_ITEMINFO info;

		auto* pJsonEach = Json_getItemAt(pJsonValue, i);
		info.m_strTypeId = Json_getString(pJsonEach, "productId", "");
		info.m_strName = Json_getString(pJsonEach, "title", "");

		auto pos = info.m_strName.find_last_of(' ');
		if (std::string::npos != pos) {
			info.m_strName = info.m_strName.substr(0, pos);
		}

		info.m_strPrice = Json_getString(pJsonEach, "price", "");		
		info.m_strKey = info.m_strTypeId;
		PaymentMgr::GetInstance()->AddItemInfo(info);
	}

	Json_dispose(pJsonValue);
	PaymentMgr::GetInstance()->NotifyObserver(PAY_EVENT_QUERY_SKU_FIN);
}

void PaymentInterface::ReqItemInfo( const std::vector<std::string>& vecItemTypeId )
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	std::string strItemTypeIdSet;
	for (auto &rStrTypeInfo : vecItemTypeId) {
		strItemTypeIdSet += rStrTypeInfo;
		strItemTypeIdSet += " ";
	}

	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, GP_IAB_JavaClassName, "ReqItemInfo", "(Ljava/lang/String;)V")) {
		jstring jItemTypeIdSet = t.env->NewStringUTF(strItemTypeIdSet.c_str());
		t.env->CallStaticVoidMethod(t.classID, t.methodID, jItemTypeIdSet);
		t.env->DeleteLocalRef(jItemTypeIdSet);
		t.env->DeleteLocalRef(t.classID);
	}
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	const char *pszSimInfo = CIniMgr::GetInstance()->GetValue("ini/payment.ini", "GooglePlayWinSim", "QueryResponse", "");
	GooglePayInAppBilling_OnReceiveItemInfo(pszSimInfo);
#endif
}

void PaymentInterface::PayStart( const char *pszItemTypeId )
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, GP_IAB_JavaClassName, "PayStart", "(Ljava/lang/String;)V")) {
		jstring jItemTypeId = t.env->NewStringUTF(pszItemTypeId);
		t.env->CallStaticVoidMethod(t.classID, t.methodID, jItemTypeId);
		t.env->DeleteLocalRef(jItemTypeId);
		t.env->DeleteLocalRef(t.classID);
	}
#endif
}

void PaymentInterface::PayEnd( const char *pszItemKey )
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	cocos2d::JniMethodInfo t;
	if (cocos2d::JniHelper::getStaticMethodInfo(t, GP_IAB_JavaClassName, "PayEnd", "(Ljava/lang/String;)V")) {
		jstring jItemKey = t.env->NewStringUTF(pszItemKey);
		t.env->CallStaticVoidMethod(t.classID, t.methodID, jItemKey);
		t.env->DeleteLocalRef(jItemKey);
		t.env->DeleteLocalRef(t.classID);
	}
#endif
}
