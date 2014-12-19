#include "HelloWorldScene.h"

#include "UIMgr.h"
#include "UIEditor.h"
#include "UITip.h"

USING_NS_CC;

HelloWorld::HelloWorld()
{
}

HelloWorld::~HelloWorld()
{
}


// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    if (!MyLayer::init()) {
        return false;
    }

    this->AddConfigNode("BtnTest");
    return true;
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
