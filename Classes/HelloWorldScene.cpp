#include "HelloWorldScene.h"

#include "UIMgr.h"
#include "UIEditor.h"
#include "UITip.h"
#include "PaymentMgr.h"
#include "BaseFunc.h"

USING_NS_CC;

HelloWorld::HelloWorld()
{
	PaymentMgr::GetInstance()->AddObserver(this);
}

HelloWorld::~HelloWorld()
{
	PaymentMgr::GetInstance()->DelObserver(this);
}


// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    if (!MyLayer::init()) {
        return false;
    }

    m_pQuerySkuDetail = this->AddConfigMenuItem("BtnQueryItem", menu_selector(HelloWorld::OnBtnQueryItem));
    return true;
}

void HelloWorld::OnBtnQueryItem(CCObject* pObj)
{
	m_pQuerySkuDetail->setEnabled(false);
	PaymentMgr::GetInstance()->ReqItemInfo();
}

void HelloWorld::OnNotify( int nEvent, int nParam )
{
	switch (nEvent)
	{
	case PAY_EVENT_QUERY_SKU_FIN:
		{
			auto& vecItemInfo = PaymentMgr::GetInstance()->GetItemInfo();
			const int SPACE_Y = 100;
			CCPoint ptOffset;
			for (auto& rItem : vecItemInfo) {
				auto* pNode = this->AddConfigMenuItem("BtnPay", menu_selector(HelloWorld::OnBtnPay));
				if (NULL == pNode) {
					continue;
				}

				std::string strName = "@";
				const char* STATIC_TEST_PREFIX = "android.test.";
				auto pPos = strstr(rItem.m_strTypeId.c_str(), STATIC_TEST_PREFIX);
				if (pPos) {
					strName += rItem.m_strTypeId;
					ReplaceStdString(strName, STATIC_TEST_PREFIX, "");
				} else {
					strName += rItem.m_strName;	
				}

				strName += " -l";
				UIMgr::GetInstance()->AddConfigStateChild(pNode, strName.c_str());

				std::string strPrice = "@";
				strPrice += rItem.m_strPrice;
				strPrice += " -r";
				UIMgr::GetInstance()->AddConfigStateChild(pNode, strPrice.c_str());

				auto* pStrSku = CCString::create(rItem.m_strTypeId);
				pNode->setUserObject(pStrSku);

				UIMgr::GetInstance()->NodePositionMove(pNode, ptOffset);
				ptOffset.y -= SPACE_Y;
			}
		}
		break;
	default:
		break;
	}
}

void HelloWorld::OnBtnPay( CCObject* pObj )
{
	if (NULL == pObj) {
		return;
	}

	auto pNode = dynamic_cast<CCNode*>(pObj);
	if (NULL == pNode) {
		return;
	}

	auto pUserObject = pNode->getUserObject();
	if (NULL == pUserObject) {
		return;
	}

	auto pStrTypeId = dynamic_cast<CCString*>(pUserObject);
	if (NULL == pStrTypeId) {
		return;
	}

	PaymentMgr::GetInstance()->PayStart(pStrTypeId->getCString());
}

void HelloWorld::menuCloseCallback(CCObject* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT) || (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
	CCMessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
#else
    CCDirector::sharedDirector()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
#endif
}

bool HelloWorldScene::OnInit( void )
{
	if (!MyScene::OnInit()) {
		return false;
	}

	auto* pLayer = HelloWorld::create();
	this->addChild(pLayer);
	return true;
}
