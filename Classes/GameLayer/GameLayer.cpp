//
//  GameLayer.m
//  OutZone_iphone
//
//  Created by admin on 1/19/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#include "GameLayer.h"
#include "Constants.h"
#include "AppDelegate.h"
#include "ccframework/ResourceManager.h"
#include "gamelayer/bullet/Bullet.h"
#include "PauseLayer.h"
#include "GameOverLayer.h"
#include "GameSuccessLayer.h"
#include "LevelHelper/Nodes/LHSettings.h"
#include "HelpLabel.h"

USING_NS_CC_EXT;

#define GAME_PROCESSING 0
#define GAME_FAILED     1
#define GAME_SUCCEESS   2

#define ENEGY_SUB_STEP  25
#define GAME_RESULT_TAG 0x123

enum Z_VALUE {
	Z_BG = -100,
	Z_SUN,
	Z_CLOUD,

	Z_ENEMY = 0,
	Z_HERO,
	Z_BOMB_EFF,
	Z_DIRECTION,

	Z_INFO = 500,
	Z_GUNBAR,
	Z_BTN,

	Z_RESULT,
	Z_DIALOG,
};

// #define	Z_BG		(-1)
// #define	Z_CLOUD		(-1)
// #define	Z_SUN		(-0.5)
// #define	Z_ENEMY		(0)
// #define	Z_HERO		(1)
// #define	Z_GUNBAR	(2)
// #define	Z_DIRECTION	(3)
// #define	Z_RESULT	(4)
// #define	Z_BTN		(10)
// #define	Z_INFO		(500)
// #define	Z_DIALOG	(1000)
// #define	Z_BOMB_EFF	(1000)

const float32 FIXED_TIMESTEP = 1.0f / 60.0f;
const float32 MINIMUM_TIMESTEP = 1.0f / 600.0f;
const int32 VELOCITY_ITERATIONS = 8;
const int32 POSITION_ITERATIONS = 8;
const int32 MAXIMUM_NUMBER_OF_STEPS = 25;

//Screen Size 1024*768 !!!
const CCPoint sunPos[5] = {
	CCPoint(iDevPixelX(128), iDevPixelY(600)),
	CCPoint(iDevPixelX(512), iDevPixelY(576)),
	CCPoint(iDevPixelX(512), iDevPixelY(312)),
	CCPoint(iDevPixelX(853), iDevPixelY(576)),
	CCPoint(iDevPixelX(512), iDevPixelY(312))
};

CCScene* GameLayer::scene()
{
	CCScene *scene = CCScene::create();
	CCNode *layer = GameLayer::create();
	//LoadScene *layer = [LoadScene node]; 
	scene->addChild(layer);

	return scene;
}

bool GameLayer::init() {
	if( ScaleLayer::init() ){		
        initVariables();
        initGameInfo();
        initButtons();
        initWorld();
        createListener();
        loadLevel();
		initHelpLabels();
		schedule(schedule_selector(GameLayer::onTime),1.0f / 20.0f);
		return true;
    }
	return false;
}

void GameLayer::initVariables() {
    setTouchEnabled(true);
    m_level = NULL;
    g_bGameOver = false;
    g_bGamePause = false;
    g_bGameSuccess = false;
    
    m_bShootBullet = false;
    m_fDirectionAngle = UNDEFINED_VALUE;
    m_nDirectionSubWidth = 0;
    g_arrBulletsCount = 0;
    m_nTutorialIndex = 0;
    m_nCode = 1;
    for(int i = 0; i < 6; i++){
        m_nBulletInfo[i] = g_nBulletInfo[g_nCurLevel][i];
    }
    for(int i = 0; i < 6; i++){
        if(m_nBulletInfo[i] > 0){
            g_nHeroWeaponType = HERO_HAND_GRENADE_BULLET + i;
            break;
        }
    }
    m_nTick = 0;
	m_nBodyCount = 0;
	m_nGameState = GAME_PROCESSING;
	m_bHeroFriendDestroyed = false;
	bDown = false;
	m_BulletArray = new CCArray();
	m_MemBodyDict = new CCDictionary();
	nGameScore = 0;

	g_nSolutionsNumber = 10;
}

void GameLayer::initButtons()
{
	CCPoint	pt = ccp(SCREEN_WIDTH - iDevPixelX(139), SCREEN_HEIGHT - iDevPixelY(48));
	SEL_CCControlHandler selector = cccontrol_selector(GameLayer::onPause);

	CCScale9Sprite *spriteNorm = CCScale9Sprite::create( "ipad_res/game/btnPause1-ipad.png" );
	CCScale9Sprite *spriteHigh = CCScale9Sprite::create( "ipad_res/game/btnPause2-ipad.png" );

	CCControlButton *button = CCControlButton::create(spriteNorm);
	button->setBackgroundSpriteForState(spriteHigh, CCControlStateHighlighted);
	button->setAdjustBackgroundImage(false);
	button->setPosition( pt );
	button->addTargetWithActionForControlEvents(this, selector, CCControlEventTouchUpInside);

	addChild(button, Z_BTN);

	menuPause = button;
}

void GameLayer::onPause(CCObject* sender, CCControlEvent event)
{
    if(m_nGameState > 0)
        return;
    if(g_bGamePause)
        return;
    g_bGamePause = true;
    m_app->playEff(E_CLICK);
	PauseLayer* pauseLayer = PauseLayer::create();
	pauseLayer->setInfo(this,nGameScore);
	addChild(pauseLayer, Z_DIALOG, 0);
}

void GameLayer::initGameInfo() {
	CCSprite *sprGameBG = m_resMgr->getSpriteWithName(CCString::createWithFormat("game/background/bg%d", g_nCurMission+1)->getCString());
    addChild(sprGameBG, Z_BG);
    sprGameBG->setPosition(ccp(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2));
	sprGameBG->setOpacity(220);

	//Sun Process
	CCSprite *sunSp = m_resMgr->getSpriteWithName(CCString::createWithFormat("game/background/sun")->getCString());
	int id = g_nCurLevel / PER_LEVEL_COUNT;
	sunSp->setPosition( ccp(sunPos[id].x, sunPos[id].y) );
	addChild(sunSp, Z_SUN);
	CCRepeatForever *repAct = CCRepeatForever::create( CCRotateBy::create(0.1f, 2) );
	sunSp->runAction(repAct);
	//////////////////////////////////////////////////////////////////////////
    
	m_lblLevel = CCLabelBMFont::create("0","font/MainMenuFont@2x.fnt");
    m_lblLevel->setColor( ccc3(230, 255, 235) );
    m_lblLevel->setPosition(ccp(iDevPixelX(491), SCREEN_HEIGHT-iDevPixelY(30)));
	m_lblLevel->setScale( iDevSize(1) );
    addChild(m_lblLevel, Z_INFO);    
    
	//m_lblGameScore = CCLabelTTF::create("0","font/ARIALNB.TTF", 38);//,,CCSizeMake(1.5, -1.5),0.5f);
	//m_lblGameScore->setColor( ccc3(30, 50, 70) );
	//m_lblGameScore.shadowColor = ccc4(255, 0, 0, 0);
	m_lblGameScore = CCLabelBMFont::create("0","font/MainMenuFont@2x.fnt");
	m_lblGameScore->setColor( ccc3(247,166,0) );
    m_lblGameScore->setPosition(ccp(iDevPixelX(708), SCREEN_HEIGHT-iDevPixelY(30)));
	m_lblGameScore->setScale( iDevSize(1) );
    addChild(m_lblGameScore, Z_INFO);
    
    initGunBar();
    
    m_sprDirection1 = m_resMgr->getSpriteWithName("game/direction1");
    m_sprDirection1->setAnchorPoint(ccp(0, 0.5f));
    addChild(m_sprDirection1, Z_DIRECTION);
    
    m_sprDirection2 = m_resMgr->getSpriteWithName("game/direction2");
    m_sprDirection2->setAnchorPoint(ccp(0, 0.5f));
    addChild(m_sprDirection2, Z_DIRECTION);
    
    m_nDirectionWidth = m_sprDirection2->getContentSize().width;
	m_nDirectionSubWidth = m_nDirectionWidth/2;
    
    m_sprGameResultTxt = m_resMgr->getSpriteWithName("game/result1_txt");
    m_sprGameResultTxt->setPosition(ccp(-1000, -1000));
    addChild(m_sprGameResultTxt, Z_RESULT);
    
	m_bmbManager = new BombEffectManager();
    addChild(m_bmbManager, Z_BOMB_EFF);
    
    m_app->playBGM(B_GAME);

	//////////////////////////////////////////////////////////////////////////
	m_tapColor = CCLayerColor::create( ccc4(0,0,0,155),3000,3000 );
	m_tapColor->setVisible(g_bTutorial);
	addChild(m_tapColor,Z_DIALOG);
	m_tapStartLabel = CCLabelTTF::create( "Tap to Start","Arial",iDevSize(32) );
	m_tapStartLabel->setVisible( g_bTutorial );
	addChild(m_tapStartLabel,Z_DIALOG);
	m_tapStartLabel->setPosition( ccp(SCREEN_WIDTH/2,SCREEN_HEIGHT/2) );

	if ( g_TestTutorial )
		schedule(schedule_selector(GameLayer::hideTapStart),1.0f);
	//////////////////////////////////////////////////////////////////////////

	if ( g_bTutorial )
	{
		CCString* str = CCString::createWithFormat("tutorials/%d_%d.tut",g_nCurMission+1,g_nCurLevel % PER_LEVEL_COUNT+1);
		m_TutotialRead.readFromFile(str->getCString());
	}
    
    hideDirections();
    initClouds();

	preloadImages();
}

void GameLayer::initClouds() {
    for(int i = 0; i < 5; i++){
		m_sprCloud[i] = m_resMgr->getSpriteWithName(CCString::createWithFormat("game/clouds/cloud%d", i + 1)->getCString());
		m_sprCloud[i]->setOpacity(200);
        addChild(m_sprCloud[i], Z_CLOUD);
        m_sprCloud[i]->setPosition(ccp(iDevPixelX(-40), 0));
        //[m_sprDuck[i]->setPosition(ccp(100, 100)];        
    }
	composeClouds();
}

void GameLayer::composeClouds() {
    for(int i = 0; i < 5; i++){
        int nY = (rand() % 10) * iDevPixelY(21) + iDevPixelY(384);
        m_sprCloud[i]->setPosition(ccp(171*i, nY));
//        m_sprCloud[i].scale = ((arc4random() % 4) + 1) * 0.25f;
        m_fCloudSpeed[i] = ((rand() % 7) + 1) * iDevPixelX(1.0f);
    }
}

void GameLayer::hideDirections() {
    m_sprDirection1->setPosition(ccp(-100, -100));
    m_sprDirection2->setPosition(ccp(-100, -100));
}

void GameLayer::initHelpLabels_1_1(int z)
{
	HelpLabel* label;

	label = HelpLabel::create();
	label->setLabel("Tap here:", iDevSize(40));
	label->setPosition(iDevPixelX(600), iDevPixelY(550));
//	label->setPosition(450, 413);
	addChild(label, z);

	CCSprite* spArrow1 = ResourceManager::sharedResourceManager()->getSpriteWithName("game/background/helparrow");
	spArrow1->setPosition( ccp(iDevPixelX(600), iDevPixelY(480)) );
//	spArrow1->setPosition( ccp(450, 360) );
	spArrow1->setRotation(-90.0f);
	spArrow1->setScale(0.7f);
	addChild(spArrow1, z);

	label = HelpLabel::create();
	label->setLabel("Press To\n Fire!", iDevSize(33));
	label->setPosition(iDevPixelX(391), iDevPixelY(140));
	addChild(label, z);

	CCSprite* spArrow2 = ResourceManager::sharedResourceManager()->getSpriteWithName("game/background/helparrow");
	spArrow2->setPosition( ccp(iDevPixelX(240), iDevPixelY(140)) );
	addChild(spArrow2, z);
}

void GameLayer::initHelpLabels_1_2(int z)
{
	HelpLabel* label;

	label = HelpLabel::create();
	label->setLabel("Shoot both cats to clear the level", iDevSize(40));
	label->setPosition(iDevPixelX(512), iDevPixelY(651));
	addChild(label, z);

	CCSprite* spArrow1 = ResourceManager::sharedResourceManager()->getSpriteWithName("game/background/helparrow");
	spArrow1->setPosition( ccp(iDevPixelX(351), iDevPixelY(571)) );
	spArrow1->setRotation(-45);
	//CCArray* enemys = m_level->spritesWithTag(ENEMY_TAG);
	//((Enemy*)enemys->objectAtIndex(0))->m_helpArrow = spArrow1;
	addChild(spArrow1, z);

	CCSprite* spArrow2 = ResourceManager::sharedResourceManager()->getSpriteWithName("game/background/helparrow");
	spArrow2->setPosition( ccp(iDevPixelX(700), iDevPixelY(571)) );
	spArrow2->setRotation(-135);
	//((Enemy*)enemys->objectAtIndex(1))->m_helpArrow = spArrow2;
	addChild(spArrow2, z);
}

void GameLayer::initHelpLabels_1_5(int z)
{
	HelpLabel* label;

	label = HelpLabel::create();
	label->setLabel("Glass breaks with one shot", iDevSize(40));
	label->setPosition(iDevPixelX(650), iDevPixelY(600));
	addChild(label, z);

	CCSprite* spArrow1 = ResourceManager::sharedResourceManager()->getSpriteWithName("game/background/helparrow");
	spArrow1->setPosition( ccp(iDevPixelX(651), iDevPixelY(500)) );
	spArrow1->setRotation(-45);spArrow1->setScale(1.2f);
	addChild(spArrow1, z);
}

void GameLayer::initHelpLabels_1_19(int z)
{
	HelpLabel* label;

	label = HelpLabel::create();
	label->setLabel("You have unlocked a\nnew weapon: The Uzi", iDevSize(33));
	label->setPosition(iDevPixelX(300), iDevPixelY(700));
	addChild(label, z);
}

void GameLayer::initHelpLabels_1_20(int z)
{
	HelpLabel* label;

	label = HelpLabel::create();
	label->setLabel("While shooting...", iDevSize(40));
	label->setPosition(iDevPixelX(200), iDevPixelY(700));
	addChild(label, z);

	label = HelpLabel::create();
	label->setLabel("...drag your finger", iDevSize(33));
	label->setPosition(iDevPixelX(451), iDevPixelY(100));
	addChild(label, z);
}

void GameLayer::initHelpLabels_3_3(int z)
{
	HelpLabel* label;

	label = HelpLabel::create();
	label->setLabel("This bounces!", iDevSize(40));
	label->setPosition(iDevPixelX(380), iDevPixelY(100));
	addChild(label, z);

	CCSprite* spArrow1 = ResourceManager::sharedResourceManager()->getSpriteWithName("game/background/helparrow");
	spArrow1->setPosition( ccp(iDevPixelX(540),iDevPixelY(131)) );
	spArrow1->setRotation(135);
	addChild(spArrow1, z);
}

void GameLayer::initHelpLabels_3_8(int z)
{
	HelpLabel* label;

	label = HelpLabel::create();
	label->setLabel("You have unlocked a\nnew weapon: The\nBow!", iDevSize(40));
	label->setPosition(iDevPixelX(250),iDevPixelY(650));
	addChild(label, z);
}

void GameLayer::initHelpLabels_3_9(int z)
{
	HelpLabel* label;

	label = HelpLabel::create();
	label->setLabel("Control the strength\nof your throw by\nadjusting the\npower bar", iDevSize(33));
	label->setPosition(iDevPixelX(250),iDevPixelY(650));
	addChild(label, z);
}

void GameLayer::initHelpLabels_3_12(int z)
{
	HelpLabel* label;

	label = HelpLabel::create();
	label->setLabel("Explosions affect\nthe Pigs through\nthe walls", iDevSize(33));
	label->setPosition(iDevPixelX(130),iDevPixelY(680));
	addChild(label, z);
}

void GameLayer::initHelpLabels_4_7(int z)
{
	HelpLabel* label;

	label = HelpLabel::create();
	label->setLabel("Barrels are highly volatile!\nUse them in your advantage", iDevSize(33));
	label->setPosition(iDevPixelX(240),iDevPixelY(700));
	addChild(label, z);
}

void GameLayer::initHelpLabels()
{
	int HELP_ZVALUE = m_heroManager->getZOrder() - 0.1;

	switch (g_nCurLevel)
	{
	case 0:		initHelpLabels_1_1(HELP_ZVALUE);	break;
	case 1:		initHelpLabels_1_2(HELP_ZVALUE);	break;
	case 4:		initHelpLabels_1_5(HELP_ZVALUE);	break;
	case 18:	initHelpLabels_1_19(HELP_ZVALUE);	break;
	case 19:	initHelpLabels_1_20(HELP_ZVALUE);	break;
	case 42:	initHelpLabels_3_3(HELP_ZVALUE);	break;
	case 47:	initHelpLabels_3_8(HELP_ZVALUE);	break;
	case 48:	initHelpLabels_3_9(HELP_ZVALUE);	break;
	case 51:	initHelpLabels_3_12(HELP_ZVALUE);	break;
	case 66:	initHelpLabels_4_7(HELP_ZVALUE);	break;
	}
}

void GameLayer::initGunBar() {    
	m_btnGuns[0] = CCMenuItemImage::create(
		m_resMgr->makeFileName("game/gun_bar/gun1_f","png"),
		m_resMgr->makeFileName("game/gun_bar/gun1_d","png"),
		this,
		menu_selector(GameLayer::onGun1));    

	m_btnGuns[1] = CCMenuItemImage::create(
		m_resMgr->makeFileName("game/gun_bar/gun2_f","png"),
		m_resMgr->makeFileName("game/gun_bar/gun2_d","png"),
		this,
		menu_selector(GameLayer::onGun2));  

	m_btnGuns[2] = CCMenuItemImage::create(
		m_resMgr->makeFileName("game/gun_bar/gun3_f","png"),
		m_resMgr->makeFileName("game/gun_bar/gun3_d","png"),
		this,
		menu_selector(GameLayer::onGun3));  

	m_btnGuns[3] = CCMenuItemImage::create(
		m_resMgr->makeFileName("game/gun_bar/gun4_f","png"),
		m_resMgr->makeFileName("game/gun_bar/gun4_d","png"),
		this,
		menu_selector(GameLayer::onGun4)); 

	m_btnGuns[4] = CCMenuItemImage::create(
		m_resMgr->makeFileName("game/gun_bar/gun5_f","png"),
		m_resMgr->makeFileName("game/gun_bar/gun5_d","png"),
		this,
		menu_selector(GameLayer::onGun5)); 

	m_btnGuns[5] = CCMenuItemImage::create(
		m_resMgr->makeFileName("game/gun_bar/gun6_f","png"),
		m_resMgr->makeFileName("game/gun_bar/gun6_d","png"),
		this,
		menu_selector(GameLayer::onGun6)); 

	m_btnGuns[6] = CCMenuItemImage::create(
		m_resMgr->makeFileName("game/gun_bar/gun7_f","png"),
		m_resMgr->makeFileName("game/gun_bar/gun7_d","png"),
		this,
		menu_selector(GameLayer::onGun7)); 
    
	for ( int i = 0 ; i < 7; i++ )
	{
		m_menuGuns[i] = CCMenu::createWithItem(m_btnGuns[i]);
		m_menuGuns[i]->setAnchorPoint(ccp(0,0));
		addChild(m_menuGuns[i], Z_GUNBAR);
	}
	
	m_btnGuns[g_nHeroWeaponType - HERO_HAND_GRENADE_BULLET]->selected();    
    
    int nVisibleIndex = 0;
    for(int i = 0; i < 6; i++){        
		m_lblBulletCount[i] = CCLabelTTF::create("00","font/Markerfelt.ttc", iDevSize(32));
		//m_lblBulletCount[i] = CCLabelBMFont::create("00","font/PopupScreenTitleFont.fnt");
        m_lblBulletCount[i]->setColor(ccBLACK);
        addChild(m_lblBulletCount[i], Z_GUNBAR);
        
        if(m_nBulletInfo[i] <= 0){
            m_lblBulletCount[i]->setVisible(false);
            m_btnGuns[i]->setVisible(false);
        }else{
            m_btnGuns[i]->setVisible(true);
            m_lblBulletCount[i]->setVisible(true);
            m_menuGuns[i]->setPosition(ccp(iDevPixelX(80), iDevPixelY(156 + nVisibleIndex * 171)));
            m_lblBulletCount[i]->setPosition(ccp(iDevPixelX(80), iDevPixelY(124 + nVisibleIndex * 171))); 
            nVisibleIndex ++;
        }
    }

	m_menuGuns[6]->setPosition( m_menuGuns[5]->getPosition() );
	m_btnGuns[6]->setVisible( false );

	animateFocusNode(m_btnGuns[g_nHeroWeaponType - HERO_HAND_GRENADE_BULLET],1);
	CCPoint pos = m_lblBulletCount[g_nHeroWeaponType - HERO_HAND_GRENADE_BULLET]->getPosition();
	pos.y -= iDevPixelY(7);
	m_lblBulletCount[g_nHeroWeaponType - HERO_HAND_GRENADE_BULLET]->setPosition(pos); 

    displayBulletCount();
}

void GameLayer::enableButtons(bool bEnable)
{
	for ( int i = 0; i < 6; i++ )
		m_btnGuns[i]->setEnabled(bEnable);
	menuPause->setEnabled(bEnable);
}

void GameLayer::displayBulletCount() {
    for(int i = 0; i < 6; i++)
		m_lblBulletCount[i]->setString(CCString::createWithFormat("%d", m_nBulletInfo[i])->getCString());
}

void GameLayer::unselectAllGunMenus() {
    for(int i = 0; i < 6; i++)
        m_btnGuns[i]->unselected();
}

void GameLayer::onGun1( CCObject*sender ) {
    if(g_bGamePause || g_bGameOver || g_bGameSuccess )
        return;
	if ( g_bTutorial && sender ) return;//Can't use Button in Tutorial Type
	unselectAllGunMenus();
	if ( g_nHeroWeaponType != HERO_HAND_GRENADE_BULLET )
	{
		replaceWeaponBoard(HERO_HAND_GRENADE_BULLET);
	}	    
	else
	{
		createBullets(m_CurPoi);

		createScore(m_menuGuns[0]->getPosition(),1000);
	}
	m_heroManager->replaceWeapon();

	m_TutotialWrite.addTutorial(m_nTick,WEAPON,CCPoint(0,0),CCPoint(0,0),g_nHeroWeaponType);
}

void GameLayer::onGun2( CCObject*sender ) {
    if(g_bGamePause || g_bGameOver || g_bGameSuccess)
        return;
	if ( g_bTutorial && sender ) return;//Can't use Button in Tutorial Type
    unselectAllGunMenus();
	if ( g_nHeroWeaponType != HERO_KNIFE_BULLET )
	{
		replaceWeaponBoard(HERO_KNIFE_BULLET);
	}	    
	else
	{
		createBullets(m_CurPoi);

		createScore(m_menuGuns[1]->getPosition(),1000);
	}
	m_heroManager->replaceWeapon();

	m_TutotialWrite.addTutorial(m_nTick,WEAPON,CCPoint(0,0),CCPoint(0,0),g_nHeroWeaponType);
}

void GameLayer::onGun3( CCObject*sender ) {
    if(g_bGamePause || g_bGameOver || g_bGameSuccess )
        return;
	if ( g_bTutorial && sender ) return;//Can't use Button in Tutorial Type
    unselectAllGunMenus();
    //m_btnGuns[2]->selected();
	if ( g_nHeroWeaponType != HERO_PISTOL_BULLET )
	{
		replaceWeaponBoard(HERO_PISTOL_BULLET);
	}	    
	else
	{
		createBullets(m_CurPoi);

		createScore(m_menuGuns[2]->getPosition(),1000);
	}
	m_heroManager->replaceWeapon();

	m_TutotialWrite.addTutorial(m_nTick,WEAPON,CCPoint(0,0),CCPoint(0,0),g_nHeroWeaponType);
}

void GameLayer::onGun4( CCObject*sender ) {
    if(g_bGamePause || g_bGameOver || g_bGameSuccess )
        return;
	if ( g_bTutorial && sender ) return;//Can't use Button in Tutorial Type
    unselectAllGunMenus();
    //m_btnGuns[3]->selected();
	if ( g_nHeroWeaponType != HERO_UZI_BULLET )
	{
		replaceWeaponBoard(HERO_UZI_BULLET);
	}	
	else
	{
		createBullets(m_CurPoi);

		createScore(m_menuGuns[3]->getPosition(),1000);
	}
	m_heroManager->replaceWeapon();

	m_TutotialWrite.addTutorial(m_nTick,WEAPON,CCPoint(0,0),CCPoint(0,0),g_nHeroWeaponType);
}

void GameLayer::onGun5( CCObject*sender ) {
    if(g_bGamePause || g_bGameOver || g_bGameSuccess )
        return;
	if ( g_bTutorial && sender ) return;//Can't use Button in Tutorial Type
    unselectAllGunMenus();
    //m_btnGuns[4]->selected();
    
	if ( g_nHeroWeaponType != HERO_FIRE_ARROW_BULLET )
	{
		replaceWeaponBoard(HERO_FIRE_ARROW_BULLET);
	}
	else
	{
		createBullets(m_CurPoi);

		createScore(m_menuGuns[4]->getPosition(),1000);
	}
	
	m_heroManager->replaceWeapon();

	m_TutotialWrite.addTutorial(m_nTick,WEAPON,CCPoint(0,0),CCPoint(0,0),g_nHeroWeaponType);
}

void GameLayer::onGun6( CCObject*sender ) {
    if(g_bGamePause || g_bGameOver || g_bGameSuccess )
        return;
	if ( g_bTutorial && sender ) return;//Can't use Button in Tutorial Type
    unselectAllGunMenus();
	//m_btnGuns[4]->selected();

	if ( g_nHeroWeaponType != HERO_BOMB_BULLET )
	{
		replaceWeaponBoard(HERO_BOMB_BULLET);
	}
	else
	{
		createBullets(m_CurPoi);

		createScore(m_menuGuns[5]->getPosition(),1000);
		m_btnGuns[6]->setVisible(true);
		animateFocusNode(m_btnGuns[6],1);
		m_btnGuns[5]->setVisible(false);
		m_lblBulletCount[5]->setVisible(false);
	}
	m_heroManager->replaceWeapon();

	m_TutotialWrite.addTutorial(m_nTick,WEAPON,CCPoint(0,0),CCPoint(0,0),g_nHeroWeaponType);
}

void GameLayer::onGun7( CCObject*sender ) {
	if(g_bGamePause || g_bGameOver || g_bGameSuccess )
		return;
	if ( g_bTutorial && sender ) return;//Can't use Button in Tutorial Type
	unselectAllGunMenus();
	//m_btnGuns[4]->selected();

	if ( g_nHeroWeaponType != HERO_BOMB_BULLET )
	{
		replaceWeaponBoard( HERO_BOMB_BULLET );
	}
	else
	{
		m_btnGuns[5]->setVisible(true);
		m_lblBulletCount[5]->setVisible(true);
		animateFocusNode(m_btnGuns[5],1);
		m_btnGuns[6]->setVisible(false);

		if ( m_heroManager->m_BombBullet )
		{
			m_heroManager->m_BombBullet->setTag( HERO_BOMB_BULLET );
			((LHSprite*)m_heroManager->m_BombBullet)->bomb(m_level);
			m_heroManager->m_BombBullet = NULL;
		}
	}
	m_heroManager->replaceWeapon();

	m_TutotialWrite.addTutorial(m_nTick,WEAPON,CCPoint(0,0),CCPoint(0,0),g_nHeroWeaponType+1);
}

void GameLayer::replaceWeaponBoard(int newWeaponID)
{
	m_app->playEff(E_REPLACE_WEAPON);
	if ( m_btnGuns[6]->isVisible() )
		animateFocusLoseNode(m_btnGuns[g_nHeroWeaponType - HERO_HAND_GRENADE_BULLET + 1],1);
	animateFocusLoseNode(m_btnGuns[g_nHeroWeaponType - HERO_HAND_GRENADE_BULLET],1);

	m_lblBulletCount[g_nHeroWeaponType - HERO_HAND_GRENADE_BULLET]->setPosition(
		ccp(m_lblBulletCount[g_nHeroWeaponType - HERO_HAND_GRENADE_BULLET]->getPosition().x, 
		m_lblBulletCount[g_nHeroWeaponType - HERO_HAND_GRENADE_BULLET]->getPosition().y + iDevPixelY(7))); 
	g_nHeroWeaponType = newWeaponID;
	animateFocusNode(m_btnGuns[g_nHeroWeaponType - HERO_HAND_GRENADE_BULLET],1);
	m_lblBulletCount[g_nHeroWeaponType - HERO_HAND_GRENADE_BULLET]->setPosition(
		ccp(m_lblBulletCount[g_nHeroWeaponType - HERO_HAND_GRENADE_BULLET]->getPosition().x,
		m_lblBulletCount[g_nHeroWeaponType - HERO_HAND_GRENADE_BULLET]->getPosition().y - iDevPixelY(7))); 
}

void GameLayer::initWorld() {
    // Define the gravity vector.
    b2Vec2 gravity;
    gravity.Set(0.0f, -7.5f);
    
    // Construct a world object, which will hold and simulate the rigid bodies.
    m_world = new b2World(gravity);
    
    m_world->SetContinuousPhysics(true);    
    
	schedule(schedule_selector(GameLayer::tick));
}

void GameLayer::createListener()
{
	m_pContactListener = new MyListener();
	m_world->SetContactListener(m_pContactListener);
}

void GameLayer::loadLevel() {
    if(m_level != NULL){
        m_level->release();
        m_level = NULL;
    }
	m_level = new LevelHelperLoader();
	//CCMessageBox("aa","aaa");
	m_level->initWithContentOfFile(CCString::createWithFormat("levels/level%d_%d", g_nCurMission+1, g_nCurLevel % PER_LEVEL_COUNT + 1));
    
	//notification have to be added before creating the objects
    //if you dont want notifications - it is better to remove this lines
	m_level->registerNotifierOnAllPathEndPoints(this, NULL);//GameLayer::spriteMoveOnPathEnded);//:pathUniqueName:)];
    m_level->registerNotifierOnAllAnimationEnds(this, NULL);//callfunc_selector(GameLayer::spriteAnimHasEnded));//:animationName:)];
    m_level->enableNotifOnLoopForeverAnimations();
    
    
    //creating the objects
    m_level->addObjectsToWorld(m_world ,this);
    
    if( m_level->hasPhysicBoundaries() )
        m_level->createPhysicBoundaries(m_world);
    
    if( !m_level->isGravityZero())
        m_level->createGravity(m_world);
    
    initHeroManager();
    initEnemyManager();

#ifdef DEBUG_BOX2D
	// Debug Draw functions
	m_debugDraw = new GLESDebugDraw( LHSettings::sharedInstance()->lhPtmRatio );
	m_world->SetDebugDraw(m_debugDraw);

	uint32 flags = 0;
	flags += b2Draw::e_shapeBit;
	flags += b2Draw::e_jointBit;
	m_debugDraw->SetFlags(flags);
#endif

    
	m_CurPoi = ccp(SCREEN_WIDTH/2,m_heroManager->getHeroPos().y);
	((CCNode*)m_level->spritesWithTag(CURSOR_TAG)->objectAtIndex(0))->setPosition( m_CurPoi );
    ((CCNode*)m_level->spritesWithTag(CURSOR_TAG)->objectAtIndex(0))->setScale( (1.3f) );
}

void GameLayer::initHeroManager() {
	m_heroManager = new HeroManager();
    m_heroManager->initWithLevelHelper(m_level);
    addChild(m_heroManager, Z_HERO);
}

void GameLayer::initEnemyManager() {
	m_enemyManager = new EnemyManager();
    m_enemyManager->initWithLevelHelper(m_level);
    addChild(m_enemyManager, Z_ENEMY);
}

void GameLayer::onTime(float t){
	m_fDirectionAngle = m_heroManager->getBulletAngleByHeroPos(m_CurPoi);
    flyClouds();
    if(g_bGamePause || g_bGameOver || g_bGameSuccess)
        return;
    processTutorial();

// 	if ( !m_tapStartLabel->isVisible() )
// 		m_nTick += t;

    if(int(m_nTick) % 1 == 0)
        checkGameResult();  
    drawLife();
    processSubLife();
    processDirection();
	processEnemyAfraid();
    drawCurLevelNumber(); 
    displayGameScore();
}

void GameLayer::preloadImages()
{
	//Image PreLoad
	const char *imgPath;
	int i;
	for ( i = 1; i <= 6; i++ )
	{
		imgPath = m_resMgr->makeFileName(CCString::createWithFormat("game/bullets/bullet%d", i)->getCString(),"png");
		CCTextureCache::sharedTextureCache()->addImage( imgPath );
	}

	for ( i = 1; i <= 4; i++ )
	{
		imgPath = CCString::createWithFormat("ipad_res/game/enemy_die_effect/item%d.png", i)->getCString();
		CCTextureCache::sharedTextureCache()->addImage( imgPath );
	}

	for ( i = 1; i <= 9; i++ )
	{
		imgPath = m_resMgr->makeFileName(CCString::createWithFormat("game/bomb_effect/ba%d", i)->getCString(),"png");
		CCTextureCache::sharedTextureCache()->addImage( imgPath );
	}

	for ( i = 1; i <= 6; i++ )
	{
		imgPath = m_resMgr->makeFileName(CCString::createWithFormat("game/bomb_effect/bomb_effect%d", i)->getCString(),"png");
		CCTextureCache::sharedTextureCache()->addImage( imgPath );
	}

	for ( i = 1; i <= 5; i++ )
	{
		imgPath = m_resMgr->makeFileName(CCString::createWithFormat("game/ice_break_effect/ice_break%d", i)->getCString(),"png");
		CCTextureCache::sharedTextureCache()->addImage( imgPath );
	}

	imgPath = m_resMgr->makeFileName(CCString::createWithFormat("game/bullets/thunder")->getCString(),"png");
	CCTextureCache::sharedTextureCache()->addImage( imgPath );
	//////////////////////////////////////////////////////////////////////////
}

void GameLayer::processTutorial() {
	if(!g_bTutorial/* || m_nTick == 0*/)
        return;

    if(m_TutotialRead.m_tutArray->count() == 0) {
        //g_bTutorial = false;
    } else {
		TUTORIAL_TYPE mainType = ((TUTORIAL*)m_TutotialRead.m_tutArray->objectAtIndex(0))->m_TutorialType;
		for ( unsigned int i = 0; i < m_TutotialRead.m_tutArray->count() - 1; i++ )
		{
			TUTORIAL *tut1 = (TUTORIAL*)m_TutotialRead.m_tutArray->objectAtIndex(i);
			TUTORIAL *tut2 = (TUTORIAL*)m_TutotialRead.m_tutArray->objectAtIndex(i+1);
			if ( tut1->m_TutorialType != TOUCHMOVE )
				break;
				
			if ( tut1->m_Time <= m_nTick && tut2->m_Time <= m_nTick )
			{
				if ( tut1->m_PrevTouchPos.x > 0 && tut1->m_PrevTouchPos.y > 0 )
					tut2->m_PrevTouchPos = tut1->m_PrevTouchPos;

				m_TutotialRead.m_tutArray->removeObjectAtIndex(0);
				i--;
			}
		}
		TUTORIAL *tut = (TUTORIAL*)m_TutotialRead.m_tutArray->objectAtIndex(0);
		if ( m_nTick >= tut->m_Time )
		{
			CCTouch touch;CCSet set;CCEvent e;
			tut->m_PrevTouchPos = CCDirector::sharedDirector()->convertToGL(tut->m_PrevTouchPos);
			tut->m_TouchPos = CCDirector::sharedDirector()->convertToGL(tut->m_TouchPos);

			if ( tut->m_TutorialType == TOUCHBEGAN )
			{
				touch.setTouchInfo(0,tut->m_TouchPos.x,tut->m_TouchPos.y);
				set.addObject(&touch);
				ccTouchesBegan(&set,&e);
			}
			if ( tut->m_TutorialType == TOUCHMOVE )
			{
				touch.setTouchInfo(0,tut->m_PrevTouchPos.x,tut->m_PrevTouchPos.y);
				touch.setTouchInfo(0,tut->m_TouchPos.x,tut->m_TouchPos.y);
				set.addObject(&touch);
				ccTouchesMoved(&set,&e);
			}
			if ( tut->m_TutorialType == TOUCHENDED )
			{
				touch.setTouchInfo(0,tut->m_TouchPos.x,tut->m_TouchPos.y);
				set.addObject(&touch);
				ccTouchesEnded(&set,&e);
			}
			if ( tut->m_TutorialType == WEAPON )
			{
				switch ( tut->m_WeaponType ) {
					case HERO_HAND_GRENADE_BULLET:			onGun1(NULL);	break;
					case HERO_KNIFE_BULLET:			onGun2(NULL);	break;
					case HERO_PISTOL_BULLET:		onGun3(NULL);	break;
					case HERO_UZI_BULLET:			onGun4(NULL);	break;
					case HERO_FIRE_ARROW_BULLET:	onGun5(NULL);	break;
					case HERO_BOMB_BULLET:			onGun6(NULL);	break;
					case HERO_BOMB_CONTROLER:			onGun7(NULL);	break;
					default:						break;
				}
			}
			if ( tut->m_TutorialType == BODYDELETE )
			{
				//Iterate over the bodies in the physics world
				for (b2Body* b = m_world->GetBodyList(); b; b = b->GetNext())
				{
					if (b->GetUserData() != NULL) 
					{
						//Synchronize the AtlasSprites position and rotation with the corresponding body
						CCSprite *myActor = (CCSprite*)b->GetUserData();

						if ( myActor != 0 )
						{
							//�������--Solution����
							int *sprInd = (int*)myActor->getUserData();
							if ( sprInd && *sprInd == tut->m_WeaponType )
							{
								int tag = myActor->getTag();
								if ( tag == HERO_FIRE_ARROW_BULLET || 
									tag == HERO_BOMB_BULLET || 
									tag == HERO_HAND_GRENADE_BULLET ||
									isBarrel((LHSprite*)myActor))
								{
									((LHSprite*)myActor)->bomb(m_level);
								}
								else
								{
									myActor->setTag( DELETED_TAG );
								}
							}
						}
					}	
				}
			}
			m_TutotialRead.m_tutArray->removeObjectAtIndex(0);
		}
    }
}

void GameLayer::processTutorial_Body()
{
	if(!g_bTutorial/* || m_nTick == 0*/)
		return;

	if(m_TutotialRead.m_tutArray_Body->count() == 0) {
		//g_bTutorial = false;
	} else {
		for ( unsigned int i = 0; i < m_TutotialRead.m_tutArray_Body->count() - 1; i++ )
		{
			TUTORIAL *tut1 = (TUTORIAL*)m_TutotialRead.m_tutArray_Body->objectAtIndex(i);
			TUTORIAL *tut2 = (TUTORIAL*)m_TutotialRead.m_tutArray_Body->objectAtIndex(i+1);

			if ( tut1->m_Time < m_nTick && tut2->m_Time < m_nTick )
			{
				m_TutotialRead.m_tutArray_Body->removeObjectAtIndex(0);
				i--;
			}
		}
		TUTORIAL *tut1 = (TUTORIAL*)m_TutotialRead.m_tutArray_Body->objectAtIndex(0);
		TUTORIAL *tut2;
		if ( m_TutotialRead.m_tutArray_Body->count() > 1 )
			tut2 = (TUTORIAL*)m_TutotialRead.m_tutArray_Body->objectAtIndex(1);
		else
			return;

		//if ( tut1->m_Time > m_nTick && tut2->m_Time > m_nTick )
		//	tut2 = tut1;
		
		//Iterate over the bodies in the physics world
		for (b2Body* b = m_world->GetBodyList(); b; b = b->GetNext())
		{
			if (b->GetUserData() != NULL) 
			{
				//Synchronize the AtlasSprites position and rotation with the corresponding body
				CCSprite *myActor = (CCSprite*)b->GetUserData();

				if ( myActor != 0 )
				{
					//�������--Solution����
					int *sprInd = (int*)myActor->getUserData();
					if ( sprInd )
					{
						CCString* ptStr1 = (CCString*)tut1->m_BodyPosDict->valueForKey(*sprInd);
						CCString* ptStr2 = (CCString*)tut2->m_BodyPosDict->valueForKey(*sprInd);
						if ( ptStr1->m_sString != "" && ptStr2->m_sString != "" )
						{
							if ( ptStr1->m_sString == "" ) ptStr1->m_sString = ptStr2->m_sString;
							if ( ptStr2->m_sString == "" ) ptStr2->m_sString = ptStr1->m_sString;
							
							CCArray* paramArr1 = componentsSeparatedByString((CCString*)ptStr1,"@");
							CCPoint pt1 = LHPointFromString( ((CCString*)paramArr1->objectAtIndex(0))->getCString() );
							float angle1 = ((CCString*)paramArr1->objectAtIndex(1))->floatValue();

							CCArray* paramArr2 = componentsSeparatedByString((CCString*)ptStr2,"@");
							CCPoint pt2 = LHPointFromString( ((CCString*)paramArr2->objectAtIndex(0))->getCString() );
							float angle2 = ((CCString*)paramArr2->objectAtIndex(1))->floatValue();

							b2Vec2 vec;	float angle;
							vec.x = pt1.x + (pt2.x-pt1.x) * (m_nTick - tut1->m_Time) / (tut2->m_Time - tut1->m_Time);
							vec.y = pt1.y + (pt2.y-pt1.y) * (m_nTick - tut1->m_Time) / (tut2->m_Time - tut1->m_Time);
							angle = angle1 + (angle2-angle1) * (m_nTick - tut1->m_Time) / (tut2->m_Time - tut1->m_Time);
							if ( tut1->m_Time == tut2->m_Time )
							{
								vec.x = pt1.x;
								vec.y = pt1.y;
								angle = angle1;
							}

							bool bSet = true;
							if ( (vec.x - pt1.x) / abs(vec.x - pt1.x) == (vec.x - pt2.x) / abs(vec.x - pt2.x) )
								bSet = false;
							if ( (vec.y - pt1.y) / abs(vec.y - pt1.y) == (vec.y - pt2.y) / abs(vec.y - pt2.y) )
								bSet = false;
							
							if ( bSet )
							{
								if (VIRT_WIDTH / (float)VIRT_HEIGHT == 480.0f / 320.0f)
								{
									vec.x *= 1.035f;
									vec.y *= 0.925f;
								}
								b->SetTransform( vec,angle );
							}
						}
					}
				}
			}	
		}

		if ( tut1->m_Time < m_nTick )
			m_TutotialRead.m_tutArray_Body->removeObjectAtIndex(0);
	}
}


void GameLayer::processEnemyAfraid()
{
	CCArray* enemy_arr = m_level->spritesWithTag(ENEMY_TAG);
	CCObject *obj;
	CCARRAY_FOREACH(enemy_arr, obj)
	{
		LHSprite* sprEnemy = (LHSprite*)obj;
		
		float ang1 = watchAngle(m_heroManager->getHeroPos(),m_CurPoi);
		float ang2 = watchAngle(m_heroManager->getHeroPos(),sprEnemy->getPosition());

		if ( ang1 > (float)M_PI ) ang1 -= (float)(2*M_PI);
		if ( ang2 > (float)M_PI ) ang2 -= (float)(2*M_PI);

		sprEnemy->afraidState = 0;
		if ( abs(ang1-ang2) < 10*M_PI/180 )
			sprEnemy->afraidState = 1;		
	}
}

void GameLayer::flyClouds() {
    bool bAllFlied = true;
    for(int i = 0; i < 5; i++){       
        m_sprCloud[i]->setPosition(ccp(m_sprCloud[i]->getPosition().x + m_fCloudSpeed[i], m_sprCloud[i]->getPosition().y));
        if(m_sprCloud[i]->getPosition().x > SCREEN_WIDTH + iDevPixelX(213))
            m_sprCloud[i]->setPosition( ccp(iDevPixelX(-213),m_sprCloud[i]->getPosition().y) );
        //else
          //  bAllFlied &= false;
    }
    if (bAllFlied) {
        //composeClouds();
    }
    
}

bool GameLayer::isEmptyBullet()
{
    bool bIsEmpty = false;
    int nAllBulletCount = 0;
    for(int i = 0; i < 6; i++)
        nAllBulletCount += m_nBulletInfo[i];
    if(nAllBulletCount == 0)
        bIsEmpty = true;
	if ( m_btnGuns[6]->isVisible() )
		bIsEmpty = false;
    return bIsEmpty;
}

#define DIRECTION_OFFSET iDevPixelX(64)
void GameLayer::processDirection() {
    //if(g_bTutorial)
    //    return;
    if(g_nHeroWeaponType == HERO_PISTOL_BULLET || g_nHeroWeaponType == HERO_UZI_BULLET)
	{
		m_sprDirection2->setVisible(false);
		m_sprDirection1->setVisible(false);
        return;
	}
	
	if(m_fDirectionAngle != UNDEFINED_VALUE){
        m_sprDirection2->setVisible(true);
		m_sprDirection1->setVisible(true);
        m_sprDirection1->setRotation(m_fDirectionAngle);
		m_sprDirection2->setRotation(m_fDirectionAngle);
        CCPoint ptOffset = ccp(DIRECTION_OFFSET * cos(CC_DEGREES_TO_RADIANS(m_fDirectionAngle * -1)), DIRECTION_OFFSET * sin(CC_DEGREES_TO_RADIANS(m_fDirectionAngle * -1)));
        m_sprDirection1->setPosition(ccpAdd(m_heroManager->getHeroPos(), ptOffset));
		m_sprDirection2->setPosition(ccpAdd(m_heroManager->getHeroPos(), ptOffset));
        
//         if(m_nDirectionSubWidth > m_nDirectionWidth && m_nCode == 1){
//             m_nCode = -1;
//         }else if(m_nDirectionSubWidth <= 0 && m_nCode == -1)
//             m_nCode = 1;
//         
//         m_nDirectionSubWidth += m_nCode * 3;

        m_sprDirection2->setTextureRect(CCRectMake(0, 0, m_nDirectionSubWidth, m_sprDirection2->getContentSize().height));
    }else{
        m_fDirectionAngle = UNDEFINED_VALUE;
        m_sprDirection2->setVisible(false);
		m_sprDirection1->setVisible(false);
    }
}

void GameLayer::processSubLife() {
    if(g_bSubLife)
        g_nLife -= 1;
}

void GameLayer::drawCurLevelNumber() {
	m_lblLevel->setString(CCString::createWithFormat("Level %d-%d", g_nCurMission+1, g_nCurLevel%PER_LEVEL_COUNT+1)->getCString());
}

void GameLayer::displayGameScore() {
	m_lblGameScore->setString(CCString::createWithFormat("Score: %d", nGameScore)->getCString());
}

void GameLayer::drawLife() {
	//m_lblLife->setString(CCString::createWithFormat("%d", g_nLife)->getCString());
}

void GameLayer::createBullets(CCPoint ptTouchedPos, float fBulletForce) {
    if(m_nBulletInfo[g_nHeroWeaponType - HERO_HAND_GRENADE_BULLET] <= 0)
        return;
    m_nBulletInfo[g_nHeroWeaponType - HERO_HAND_GRENADE_BULLET] --;
    
    displayBulletCount();
    m_heroManager->createBullet(m_fDirectionAngle, fBulletForce, ptTouchedPos);
    
    CCLog("%d, %f, %f, %d, %f, %f", m_nTick, m_fDirectionAngle, fBulletForce, g_nHeroWeaponType, ptTouchedPos.x, ptTouchedPos.y);
}

void GameLayer::createBullets(CCPoint ptTouchedPos) {
    if(m_nBulletInfo[g_nHeroWeaponType - HERO_HAND_GRENADE_BULLET] <= 0)
        return;    
    
    float fBulletForce = (float)m_nDirectionSubWidth / (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad? 5.0f : (CC_CONTENT_SCALE_FACTOR() == 2.0f ? 2.0f : 5.0f));
	fBulletForce /= iDevSize(1);
    if(g_nHeroWeaponType == HERO_PISTOL_BULLET || g_nHeroWeaponType == HERO_UZI_BULLET)
        fBulletForce = 23.0f;

	if ( g_nHeroWeaponType == HERO_UZI_BULLET )
	{
		for ( int i = 0; i < 3; i++ )
		{
			BULLETINFO *bi = new BULLETINFO;
			bi->m_Index = i;
			bi->m_Type = HERO_UZI_BULLET;
			bi->m_Angle = (m_fDirectionAngle - 3*i);
			bi->m_BulletForce = fBulletForce;
			m_BulletArray->addObject(bi);
		}
		schedule(schedule_selector(GameLayer::shootUzi), 0.1f);
	}
    createBullets(ptTouchedPos, fBulletForce);
}

void GameLayer::shootUzi(float dt)
{
	if ( m_BulletArray->count() == 0 )
	{
		unschedule(schedule_selector(GameLayer::shootUzi));
		return;
	}
	
	BULLETINFO *bi = (BULLETINFO*)m_BulletArray->objectAtIndex(0);
	bi->m_Angle = m_fDirectionAngle - 2*bi->m_Index;
	m_heroManager->createBullet(bi->m_Angle, bi->m_BulletForce,ccp(0,0));
	m_BulletArray->removeObject(bi);
}

void GameLayer::checkGameResult() {
    if(m_enemyManager->isDestroyAllEnemies())
	{
		createScore(m_heroManager->getHeroPos(),10000);
		for ( int i = 0; i < 6; i++ )
		{
			int nCount = CCString::create( m_lblBulletCount[i]->getString() )->intValue();
			if ( nCount > 0 )
				createScore( m_menuGuns[i]->getPosition(),nCount * 4000 );
		}
        g_bGameSuccess = true;
	}
    if(g_bGameSuccess && m_nGameState == GAME_PROCESSING){
        m_nGameState = GAME_SUCCEESS;
		schedule(schedule_selector(GameLayer::displayGameResult),2.0f);
        submitScore();
    }
    if(g_bGameSuccess)
        return;
    
    if(g_nLife == 0 || (isEmptyBullet() && g_arrBulletsCount == 0) || 
		m_heroManager->getHeroTag() == DELETED_TAG || m_bHeroFriendDestroyed){
        g_bGameOver = true;
    }
    if(g_bGameOver && m_nGameState == GAME_PROCESSING){
        m_nGameState = GAME_FAILED;
        m_heroManager->dieAnimation();
		schedule(schedule_selector(GameLayer::displayGameResult),2.0f);
    }
}

void GameLayer::displayGameResult(float t){
    if(m_nGameState == GAME_SUCCEESS)
        displayGameSuccess();
    else if(m_nGameState == GAME_FAILED)
        flyGameOverText();
	unschedule(schedule_selector(GameLayer::displayGameResult));
}

void GameLayer::submitScore() {
    m_app->submitScore(nGameScore);
}

void GameLayer::ccTouchesBegan(CCSet * touches,CCEvent * event)
{
	bDown = true;
	if ( m_tapStartLabel->isVisible() )
	{
		m_tapColor->setVisible(false);
		m_tapStartLabel->setVisible(false);

		CCSprite* spR = ResourceManager::sharedResourceManager()->getSpriteWithName("game/gameresult/tutorial_mark");
		spR->setPosition( ccp(SCREEN_WIDTH/2,SCREEN_HEIGHT/4) );
		spR->setOpacity( 155 );
		addChild( spR,Z_DIALOG );
		return;
	}

	if(g_bTutorial && !event)
        return;
	
    if(isEmptyBullet())
        return;
    if(g_bGamePause || g_bGameOver || g_bGameSuccess)
        return;
    CCArray *arrTutorials = m_level->spritesWithTag(TUTORIAL_TAG);
    for(unsigned int i = 0; i < arrTutorials->count(); i++){
        ((CCNode*)(arrTutorials->objectAtIndex(i)))->setVisible(false);
    }
    CCTouch *uiTouch = (CCTouch*)touches->anyObject();
    CCPoint ptPos = uiTouch->getLocation();
	m_CurPoi = ptPos;
    m_fDirectionAngle = m_heroManager->getBulletAngleByHeroPos(ptPos);    
    ((CCNode*)(m_level->spritesWithTag(CURSOR_TAG)->objectAtIndex(0)))->setPosition(ptPos);
	//m_level->startAnimationWithUniqueName(CCString::create("game_cursor_anim"),(LHSprite*)m_level->spritesWithTag(CURSOR_TAG)->objectAtIndex(0));
	CCNode *curNode = ((CCNode*)(m_level->spritesWithTag(CURSOR_TAG)->objectAtIndex(0)));
	CCScaleTo *curScaleAct = CCScaleTo::create(0.2f,(2.0f));
	curNode->runAction(curScaleAct);

	m_TutotialWrite.addTutorial(m_nTick,TOUCHBEGAN,ptPos,ptPos,0);
}

void GameLayer::ccTouchesMoved(CCSet * touches,CCEvent * event) {
	if(g_bTutorial && !event)
		return;
    if(isEmptyBullet())
        return;
    if(g_bGamePause || g_bGameOver || g_bGameSuccess)
        return;
    CCTouch *uiTouch = (CCTouch*)touches->anyObject();
    CCPoint ptPos = uiTouch->getLocation();
	m_CurPoi = ptPos;
    
    ((CCNode*)(m_level->spritesWithTag(CURSOR_TAG)->objectAtIndex(0)))->setPosition(ptPos);

	//Bomb Force Control
	CCPoint prevPos = uiTouch->getPreviousLocation();
	//prevPos = CCDirector::sharedDirector()->convertToGL(prevPos);
	float dis = ccpDistance(m_heroManager->getHeroPos(),m_CurPoi) - ccpDistance(m_heroManager->getHeroPos(),prevPos);
	dis = -dis / 5;
	m_nDirectionSubWidth -= dis;
	if ( m_nDirectionSubWidth < 0 ) m_nDirectionSubWidth = 0;
	if ( m_nDirectionSubWidth > m_nDirectionWidth ) m_nDirectionSubWidth = m_nDirectionWidth;
	//////////////////////////////////////////////////////////////////////////
	m_TutotialWrite.addTutorial(m_nTick,TOUCHMOVE,prevPos,ptPos,0);
}

void GameLayer::ccTouchesEnded(CCSet * touches,CCEvent * event) {
	bDown = false;
	if(g_bTutorial && !event)
		return;
    if(isEmptyBullet())
        return;
    if(g_bGamePause || g_bGameOver || g_bGameSuccess)
        return;
	CCTouch *uiTouch = (CCTouch*)touches->anyObject();
	CCPoint ptPos = uiTouch->getLocation();
	m_fDirectionAngle = m_heroManager->getBulletAngleByHeroPos(ptPos);
    m_fDirectionAngle = UNDEFINED_VALUE;
    m_nCode = -1;
    m_level->stopAnimationOnSprite((LHSprite*)m_level->spritesWithTag(CURSOR_TAG)->objectAtIndex(0));
	m_level->startAnimationWithUniqueName(CCString::create("game_cursor_reset"), (LHSprite*)m_level->spritesWithTag(CURSOR_TAG)->objectAtIndex(0));
	CCNode *curNode = ((CCNode*)(m_level->spritesWithTag(CURSOR_TAG)->objectAtIndex(0)));
	CCScaleTo *curScaleAct = CCScaleTo::create(0.2f, (1.3f));
	curNode->runAction(curScaleAct);

	m_TutotialWrite.addTutorial(m_nTick,TOUCHENDED,ptPos,ptPos,0);
}

void GameLayer::flyGameOverText() {
    m_sprGameResultTxt->setPosition(ccp(iDevPixelX(-427), SCREEN_HEIGHT / 2));
    CCTexture2D *texture = NULL;
    if(isEmptyBullet() && g_arrBulletsCount == 0) {
		texture = CCTextureCache::sharedTextureCache()->addImage(m_resMgr->makeFileName("game/result2_txt","png"));
    }else
		texture = CCTextureCache::sharedTextureCache()->addImage(m_resMgr->makeFileName("game/result1_txt","png"));
    m_sprGameResultTxt->setTexture(texture);
    m_sprGameResultTxt->setTextureRect(CCRectMake(0, 0, texture->getContentSize().width, texture->getContentSize().height));
    
	CCMoveTo* move1 = CCMoveTo::create(0.5f,ccp(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2));
	CCMoveTo* move2 = CCMoveTo::create(0.5f,ccp(SCREEN_WIDTH + iDevPixelX(427), SCREEN_HEIGHT / 2));
	m_sprGameResultTxt->runAction(CCSequence::create(move1, CCDelayTime::create(0.8f), move2, CCCallFunc::create(this,callfunc_selector(GameLayer::displayGameOver)), NULL));
}

void GameLayer::displayGameOver() {
	GameOverLayer* l = GameOverLayer::create();
	l->setInfo(this,nGameScore);
	addChild(l, Z_DIALOG, GAME_RESULT_TAG);
}

void GameLayer::displayGameSuccess() {
	g_nGameCoins += COINPERLEVEL;
	CCString *str = CCString::createWithFormat("tutorials/%d_%d.tut",g_nCurMission+1,g_nCurLevel % PER_LEVEL_COUNT + 1 );
	m_TutotialWrite.saveToFile(str->getCString());

	GameSuccessLayer* l = GameSuccessLayer::create();
	l->setInfo(this,nGameScore);
	addChild(l, Z_DIALOG, GAME_RESULT_TAG);
}

void GameLayer::spriteMoveOnPathEnded(/*LHSprite* spr,CCString* pathName*/)
{

}

void GameLayer::createScore(CCPoint pos,int nScore)
{
	CCString* scoreStr = CCString::createWithFormat("+%d",nScore);

	//CCLabelTTF *scoreLabel = CCLabelTTF::create(scoreStr->getCString(),"font/Marker Felt.ttf",30);
	CCLabelBMFont *scoreLabel = CCLabelBMFont::create(scoreStr->getCString(),"font/PopupScreenTitleFont@2x.fnt");
	scoreLabel->setScale(iDevSize(1.3f));

	scoreLabel->setPosition( pos );
	scoreLabel->setOpacity(0);
	addChild(scoreLabel,10000);
	
	CCFadeIn * fadeIn = CCFadeIn::create(0.8f);
	CCFadeOut * fadeOut = CCFadeOut::create(0.8f);
	CCMoveBy * moveBy1 = CCMoveBy::create(0.8f,CCPoint(0,iDevPixelY(100)));
	CCMoveBy * moveBy2 = CCMoveBy::create(0.8f,CCPoint(0,iDevPixelY(100)));

	scoreLabel->runAction( CCSequence::create(fadeIn/*,CCDelayTime::create(0.1f)*/,fadeOut,NULL) );
	scoreLabel->runAction( CCSequence::create(moveBy1/*,CCDelayTime::create(0.1f)*/,moveBy2,NULL) );

	nGameScore += nScore;
	displayGameScore();
}

////////////////////////////////////////////////////////////////////////////////
void GameLayer::spriteAnimHasEnded(/*LHSprite* spr, CCString* animName*/)
{
    
}

////////////////////////////////////////////////////////////////////////////////
//FIX TIME STEPT------------>>>>>>>>>>>>>>>>>>
void GameLayer::tick(float dt)
{
	if(g_bGamePause)
		return;

	if ( !m_tapStartLabel->isVisible() )
		m_nTick += dt;

	step(dt);

	processTutorial_Body();
    
	CCDictionary* dict = new CCDictionary;
	//Iterate over the bodies in the physics world
	for (b2Body* b = m_world->GetBodyList(); b; b = b->GetNext())
	{
		if (b->GetUserData() != NULL) 
        {
			//Synchronize the AtlasSprites position and rotation with the corresponding body
			CCSprite *myActor = (CCSprite*)b->GetUserData();
            
			//�������--Solution����
			int *sprInd = (int*)myActor->getUserData();
			if ( !sprInd && ((b->GetType() == b2_dynamicBody && myActor->getTag() != 200000/*No EatItem*/ ) || 
							myActor->getTag() == ICE_TAG))
			{
				myActor->setUserData(&g_nBodyIndex[ m_nBodyCount ]);
				m_MemBodyDict->setObject(CCString::create(""),g_nBodyIndex[ m_nBodyCount ]);
				m_nBodyCount++;
			}

			sprInd = (int*)myActor->getUserData();
			if ( g_SaveTutorial /*&& !g_bTutorial*/ && sprInd )
			{
				CCString* newPtStr = LHStringWithCCPoint( CCPointMake(b->GetPosition().x,b->GetPosition().y) );
				newPtStr->m_sString += "@";
				newPtStr->m_sString += CCString::createWithFormat("%f",b->GetAngle())->getCString();
				const CCString* oldPtStr = m_MemBodyDict->valueForKey(*sprInd);
				if ( newPtStr->m_sString != oldPtStr->m_sString )
				{
					m_MemBodyDict->setObject(newPtStr,*sprInd);
					dict->setObject(newPtStr,*sprInd);
				}
			}

			//////////////////////////////////////////////////////////////////////////

			if(myActor != 0)
            {
                //THIS IS VERY IMPORTANT - GETTING THE POSITION FROM BOX2D TO COCOS2D
				myActor->setPosition(m_level->metersToPoints(b->GetPosition()));
				myActor->setRotation(-1 * CC_RADIANS_TO_DEGREES(b->GetAngle()));		
            }
        }	
	}
	
	if ( dict->count() > 0 && g_SaveTutorial /*&& !g_bTutorial*/ )
	{
		m_TutotialWrite.addTutorial( m_nTick,BODYPOSSET,CCPointZero,CCPointZero,0,dict );
	}
	else
		dict->release();
}

void GameLayer::draw()
{
	m_world->DrawDebugData();
}
////////////////////////////////////////////////////////////////////////////////
void GameLayer::step(float dt)
{
	if ( m_tapStartLabel->isVisible() )
		dt = 0.0;

	float32 frameTime = dt;
	int stepsPerformed = 0;
	while ( (frameTime > 0.0) && (stepsPerformed < MAXIMUM_NUMBER_OF_STEPS) ){
		float32 deltaTime = min( frameTime, FIXED_TIMESTEP );
		frameTime -= deltaTime;
		if (frameTime < MINIMUM_TIMESTEP) {
			deltaTime += frameTime;
			frameTime = 0.0f;
		}
		m_world->Step(deltaTime,VELOCITY_ITERATIONS,POSITION_ITERATIONS);
		stepsPerformed++;
		afterStep(); // process collisions and result from callbacks called by the step
	}
	m_world->ClearForces();
}

void GameLayer::hideTapStart(float dt)
{
	m_tapStartLabel->setVisible(false);
	m_tapColor->setVisible(false);
}

void GameLayer::afterStep() {
	// process collisions and result from callbacks called by the step
}

GameLayer::~GameLayer(){
    if(m_level != NULL){
        m_level->release();
        m_level = NULL;
    }
    
	m_BulletArray->release();
	m_MemBodyDict->removeAllObjects();
	m_MemBodyDict->release();
    unscheduleAllSelectors();
    stopAllActions();
    removeAllChildrenWithCleanup(true);
	CCTextureCache::sharedTextureCache()->removeUnusedTextures();
}
