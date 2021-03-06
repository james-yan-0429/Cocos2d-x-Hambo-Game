/*
 * ModalAlert - Customizable popup dialogs/alerts for Cocos2D
 *
 * For details, visit the Rombos blog:
 * http://rombosblog.wordpress.com/2012/02/28/modal-alerts-for-cocos2d/ 
 *
 * Copyright (c) 2012 Hans-Juergen Richstein, Rombos
 * http://www.rombos.de
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "cocos2d.h"

USING_NS_CC;

class ModalAlert : public CCObject
{
public:
	static void Ask(const char * question,CCLayer * layer,SEL_CallFuncO yesBlock,SEL_CallFuncO noBlock);
	static void Confirm(const char * question,CCLayer *layer,SEL_CallFuncO okBlock,SEL_CallFuncO cancelBlock);
	static void Tell(const char * statement,CCLayer *layer,SEL_CallFuncO okBlock);

	static void CloseAlert(CCSprite* alertDialog,CCLayer* coverLayer,SEL_CallFuncO block);
	static void ShowAlert(const char* message,CCLayer * layer,const char* opt1,SEL_CallFuncO opt1Block,const char* opt2,SEL_CallFuncO opt2Block);
};

class CoverLayer: public CCLayerColor
{
public:
	CCSprite *mainDialog;

	SEL_CallFuncO callFunc1;
	SEL_CallFuncO callFunc2;

public:
	CoverLayer();

	void onBut1(CCObject *object);
	void onBut2(CCObject *object);

private:
	bool ccTouchBegan(CCTouch *touch,CCEvent *event);
	void registerWithTouchDispatcher();
};