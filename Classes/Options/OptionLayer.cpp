

#include "OptionLayer.h"
#include "AppDelegate.h"
#include "Constants.h"
#include "ModalAlert/ModalAlert.h"
#include "ccframework/ResourceManager.h"
#include "MainMenu/TitleLayer.h"

USING_NS_CC_EXT;

CCScene* OptionLayer::scene()
{
	// 'scene' is an autorelease object
	CCScene *scene = CCScene::create();

	// 'layer' is an autorelease object
	OptionLayer *layer = OptionLayer::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

bool OptionLayer::init()
{
	// always call "super" init
	// Apple recommends to re-assign "self" with the "super" return value
	if ( ScaleLayer::init() )
	{
		m_app->playBGM(B_TITLE);

		createBackground();
		createButtons();
	}
	return true;
}

OptionLayer::~OptionLayer()
{
	unscheduleAllSelectors();
	removeAllChildrenWithCleanup(true);
	CCTextureCache::sharedTextureCache()->removeUnusedTextures();
}

void OptionLayer::createBackground()
{
	CCSprite *sprite = m_resMgr->getSpriteWithName("common_bg1");
	sprite->setPosition(ccp(SCREEN_WIDTH/2, SCREEN_HEIGHT/2));
	addChild(sprite, -100);

	sprite = m_resMgr->getSpriteWithName("mainmenu/flag_bar");
	sprite->setPosition(ccp(iDevPixelX(149), iDevPixelY(384)));
	addChild(sprite, -100);

	sprite = m_resMgr->getSpriteWithName("mainmenu/title_hero");
	sprite->setPosition(ccp(SCREEN_WIDTH/2, SCREEN_HEIGHT/2));
	addChild(sprite, -100);

	sprite = m_resMgr->getSpriteWithName("common_sun");
	sprite->setPosition( ccp(iDevPixelX(747), iDevPixelY(700)) );
	addChild(sprite, -100);
	CCRepeatForever *repAct = CCRepeatForever::create( CCRotateBy::create(0.1f, 2.0f) );
	sprite->runAction(repAct);

	sprite = m_resMgr->getSpriteWithName("common_bg2");
	sprite->setPosition(ccp(SCREEN_WIDTH/2, SCREEN_HEIGHT/2));
	addChild(sprite, -100);

	sprite = m_resMgr->getSpriteWithName("common_bg3");
	sprite->setPosition(ccp(SCREEN_WIDTH/2, SCREEN_HEIGHT/2));
	addChild(sprite, -100);
}

void OptionLayer::createButtons()
{
	int selIndex;

	CCMenuItemSprite* itemBgOff = CCMenuItemSprite::create(
		m_resMgr->getSpriteWithName("options/btnBg2"),
		m_resMgr->getSpriteWithName("options/btnBg2"),
		m_resMgr->getSpriteWithName("options/btnBg2"));
	CCMenuItemSprite* itemBgOn  = CCMenuItemSprite::create(
		m_resMgr->getSpriteWithName("options/btnBg1"),
		m_resMgr->getSpriteWithName("options/btnBg1"),
		m_resMgr->getSpriteWithName("options/btnBg1"));
	CCMenuItemToggle *item1 = CCMenuItemToggle::createWithTarget(
		this,
		menu_selector(OptionLayer::onBGM),
		itemBgOff,
		itemBgOn,
		NULL);
	if ( m_app->getBGMEnable() )
		selIndex = 1;
	else
		selIndex = 0;

	item1->setSelectedIndex(selIndex);
	CCMenu *menu1 = CCMenu::create(item1, NULL);
	menu1->setAnchorPoint(CCPointZero);
	menu1->setPosition(ccp(iDevPixelX(209), SCREEN_HEIGHT-iDevPixelY(154)));
	addChild(menu1);

	CCMenuItemSprite* itemEffOff = CCMenuItemSprite::create(
		m_resMgr->getSpriteWithName("options/btnEffect2"),
		m_resMgr->getSpriteWithName("options/btnEffect2"),
		m_resMgr->getSpriteWithName("options/btnEffect2"));
	CCMenuItemSprite* itemEffOn  = CCMenuItemSprite::create(
		m_resMgr->getSpriteWithName("options/btnEffect1"),
		m_resMgr->getSpriteWithName("options/btnEffect1"),
		m_resMgr->getSpriteWithName("options/btnEffect1"));
	CCMenuItemToggle *item2 = CCMenuItemToggle::createWithTarget(
		this,
		menu_selector(OptionLayer::onEffect),
		itemEffOff,
		itemEffOn,
		NULL);

	if ( m_app->getEffEnable() )
		selIndex = 1;
	else
		selIndex = 0;

	item2->setSelectedIndex(selIndex);
	CCMenu *menu2 = CCMenu::create(item2, NULL);
	menu2->setAnchorPoint(CCPointZero);
	menu2->setPosition(ccp(iDevPixelX(171), SCREEN_HEIGHT-iDevPixelY(256)));
	addChild(menu2);

	GrowButton *btn;
	btn = GrowButton::buttonWithSpriteFileName("options/btnResetgame1","options/btnResetgame2" ,this,cccontrol_selector(OptionLayer::onResetGame));
	btn->setPosition(iDevPixelX(139), SCREEN_HEIGHT-iDevPixelY(361));
	addChild(btn);

	btn = GrowButton::buttonWithSpriteFileName("options/btnBack1","options/btnBack2",this,cccontrol_selector(OptionLayer::onBack));
	btn->setPosition(ccp(iDevPixelX(151), SCREEN_HEIGHT-iDevPixelY(491)));
	addChild(btn);

	btn = GrowButton::buttonWithSpriteFileName("mainmenu/btnClose1","mainmenu/btnClose2",this,cccontrol_selector(OptionLayer::onClose));
	btn->setPosition( ccp(iDevPixelX(896), SCREEN_HEIGHT-iDevPixelY(672)) );
	addChild(btn);
}

void OptionLayer::onClose( CCObject* sender, CCControlEvent event )
{
	m_app->playEff(E_CLICK);

	CCDirector::sharedDirector()->end();
	//ModalAlert::Confirm("Do you want to quit?", this, menu_selector(OptionLayer::onQuitOK),NULL);
}

void OptionLayer::onQuitOK( CCObject* sender )
{
	CCDirector::sharedDirector()->end();
}

void OptionLayer::onBGM( CCObject* sender )
{
	m_app->playEff(E_CLICK);
	CCMenuItemToggle *mm = (CCMenuItemToggle*)sender;
	if (mm->getSelectedIndex())
	{
		m_app->setBGMEnable(true);
	}
	else
	{
		m_app->setBGMEnable(false);
	}
	m_app->saveSetting();
}

void OptionLayer::onEffect( CCObject* sender )
{
	m_app->playEff(E_CLICK);

	CCMenuItemToggle *mm = (CCMenuItemToggle*)sender;
	if (mm->getSelectedIndex())
	{
		m_app->setEffEnable(true);
	}
	else
	{
		m_app->setEffEnable(false);
	}
	m_app->saveSetting();
}

void OptionLayer::onResetGame( CCObject* sender, CCControlEvent event )
{
    m_app->playEff(E_CLICK);

	ModalAlert::Confirm("Do you want to really reset game?", this, menu_selector(OptionLayer::onResetOK),NULL);
}

void OptionLayer::onResetOK( CCObject* sender )
{
	g_nCurLevel = 0;
	g_nGameCoins = 0;
	
	g_nHeroUnlocked[0] = true;
	g_nHeroUnlocked[1] = false;
	g_nHeroUnlocked[2] = false;

	g_nMissionsUnlocked[0] = true;
	g_nMissionsUnlocked[1] = false;
	g_nMissionsUnlocked[2] = false;
	g_nMissionsUnlocked[3] = false;
	g_nMissionsUnlocked[4] = false;
	g_nSolutionsNumber = 3;
	g_nSkipsNumber = 3;

	for ( int i = 0; i < LEVEL_COUNT; i++ )
		g_nBestScore[i] = 0;
	for ( int i = 0; i < STAGE_COUNT; i++ )
		g_nAvailableLevel[i] = -1;
	g_nAvailableLevel[0] = 0;

	m_app->saveSetting();
}

void OptionLayer::onBack( CCObject* sender, CCControlEvent event )
{
	m_app->playEff(E_CLICK);
	CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.7f, TitleLayer::scene()));
}
