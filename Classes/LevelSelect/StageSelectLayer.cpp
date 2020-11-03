//
//  StageSelectLayer.m
//  JarHead
//
//  Created by admin on 6/26/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#include "StageSelectLayer.h"
#include "Constants.h"
#include "ccframework/ResourceManager.h"
#include "ccframework/GrowButton.h"
#include "LevelSelect/LevelSelectLayer.h"
#include "AppDelegate.h"
#include "ModalAlert/ModalAlert.h"

USING_NS_CC_EXT;

#define STAGE_STEP iDevPixelX(107)

CCScene* StageSelectLayer::scene()
{
	// 'scene' is an autorelease object.
	CCScene *scene = CCScene::create();
	
	// 'layer' is an autorelease object.
	StageSelectLayer *layer = StageSelectLayer::create();
	
	// add layer as a child to scene
	scene->addChild(layer);
	
	// return the scene
	return scene;
}

bool StageSelectLayer::init()
{
	if ( ScaleLayer::init() )
	{
		m_app->playBGM(B_TITLE);
		createBackground();
		createInfo();
        createStage();
    }
	return true;
}

StageSelectLayer::~StageSelectLayer()
{
    removeAllChildrenWithCleanup(true);
	CCTextureCache::sharedTextureCache()->removeUnusedTextures();
}

void StageSelectLayer::createBackground()
{
	CCSprite *sprite = m_resMgr->getSpriteWithName("common_bg1");
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

	GrowButton *btnBack = GrowButton::buttonWithSpriteFileName("options/back_btn1", "options/back_btn2", this, cccontrol_selector(StageSelectLayer::onBack));
	btnBack->setPosition(ccp(iDevPixelX(128), iDevPixelY(85)));
	addChild(btnBack);

	GrowButton* btn = GrowButton::buttonWithSpriteFileName("mainmenu/btnClose1","mainmenu/btnClose2",this,cccontrol_selector(StageSelectLayer::onClose));
	btn->setPosition( ccp(iDevPixelX(896), SCREEN_HEIGHT-iDevPixelY(672)) );
	addChild(btn);
}

void StageSelectLayer::onClose( CCObject* sender, CCControlEvent event )
{
	m_app->playEff(E_CLICK);

	CCDirector::sharedDirector()->end();
	//ModalAlert::Confirm("Do you want to quit?", this, menu_selector(StageSelectLayer::onQuitOK),NULL);
}

void StageSelectLayer::onQuitOK( CCObject* sender )
{
	CCDirector::sharedDirector()->end();
}

void StageSelectLayer::createInfo()
{
	CCSprite*	sprite;
	sprite = m_resMgr->getSpriteWithName("levelselect/levelinfo");
	sprite->setPosition(ccp(SCREEN_WIDTH/2, iDevPixelY(120)));
	addChild(sprite);

	CCString* str = CCString::createWithFormat( "Mission %d", g_nCurMission+1 );
	CCSize	size = sprite->getContentSize();
	m_lblLevel = CCLabelBMFont::create(str->getCString(), "font/MainMenuFont@2x.fnt");
	m_lblLevel->setPosition( ccp(size.width/2, size.height/2) );
	sprite->addChild(m_lblLevel);

	m_lblLevel->setScaleX( iDevPixelX(1.0f) );
	m_lblLevel->setScaleY( iDevPixelY(1.0f) );
}

void StageSelectLayer::createStage()
{
	m_stageLayer = (ScaleLayer*)ScaleLayer::create();    
    
    int nStage = g_nAvailableLevel[g_nCurMission];
	
    for (int i = 0; i < PER_LEVEL_COUNT; i++)
	{
		char*	szNorm;
		char*	szHigh;

		if ( i <= nStage )
		{
			szNorm = "levelselect/stage_mark_d";
			szHigh = "levelselect/stage_mark_f";
		}
		else
		{
			szNorm = "levelselect/stage_mark_lock";
			szHigh = "levelselect/stage_mark_lock";
		}

		GrowButton* btn = GrowButton::buttonWithSpriteFileName(szNorm, szHigh, this, cccontrol_selector(StageSelectLayer::onStage), i);
        
        int nStep = i / PER_LEVEL_COUNT;

        float x = i % 8 - 3.5;
        float y = (i%PER_LEVEL_COUNT) / 8 - 2;

		float xPos = SCREEN_WIDTH / 2 + SCREEN_WIDTH * nStep + 1.1 * x * STAGE_STEP;
		float yPos = SCREEN_HEIGHT / 2 + iDevPixelY(40) - y * STAGE_STEP;
        
        btn->setPosition(ccp(xPos,yPos));
        m_stageLayer->addChild(btn);

		if ( i > nStage )
			btn->setEnabled(false);
		
		CCString*		strNumber = CCString::createWithFormat("%d", i+1);
		CCLabelAtlas*	label = CCLabelAtlas::create(strNumber->getCString(), "font/number.png", (40), (49), '0');
		label->setScale( iDevSize(1) );
        label->setAnchorPoint(ccp(0.5f, 0.6f));
        label->setPosition(ccp(xPos,yPos));
        m_stageLayer->addChild(label);
    }
    addChild(m_stageLayer, 0, 0);
}

void StageSelectLayer::onBack( CCObject* sender, CCControlEvent event )
{
	m_app->playEff(E_CLICK);
	CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.7f, LevelSelectLayer::scene()));
}

void StageSelectLayer::onStage( CCObject* sender, CCControlEvent event )
{
    GrowButton* button = (GrowButton*)sender;
    int nStage = button->getTag();
    
    if (nStage > g_nAvailableLevel[g_nCurMission])
        return;

    g_nCurLevel = g_nCurMission * PER_LEVEL_COUNT + nStage;
    
	g_bTutorial = false;
	m_app->changeWindow(WND_GAME);
}
