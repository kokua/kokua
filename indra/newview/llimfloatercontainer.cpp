/** 
 * @file llimfloatercontainer.cpp
 * @brief Multifloater containing active IM sessions in separate tab container tabs
 *
 * $LicenseInfo:firstyear=2009&license=viewergpl$
 * 
 * Copyright (c) 2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */


#include "llviewerprecompiledheaders.h"

#include "llimfloatercontainer.h"

// TODO: mantipov: replace with floater_im_session
#include "llimpanel.h"

//
// LLIMFloaterContainer
//
LLIMFloaterContainer::LLIMFloaterContainer(const LLSD& seed)
:	LLMultiFloater(seed),
	mActiveVoiceFloater(NULL)
{
	mAutoResize = FALSE;
}

LLIMFloaterContainer::~LLIMFloaterContainer()
{
}

BOOL LLIMFloaterContainer::postBuild()
{
	// TODO: mantipov: verify if it is needed
	mTabContainer->lockTabs();
	return TRUE;
}

void LLIMFloaterContainer::onOpen(const LLSD& key)
{
	// TODO: mantipov: verify if it is needed
	if (key.isDefined())
	{
		LLFloaterIMPanel* impanel = gIMMgr->findFloaterBySession(key.asUUID());
		if (impanel)
		{
			impanel->openFloater();
		}
	}
}

void LLIMFloaterContainer::addFloater(LLFloater* floaterp, 
									BOOL select_added_floater, 
									LLTabContainer::eInsertionPoint insertion_point)
{
	if(!floaterp) return;

	// already here
	if (floaterp->getHost() == this)
	{
		openFloater(floaterp->getKey());
		return;
	}

	LLMultiFloater::addFloater(floaterp, select_added_floater, insertion_point);

	// make sure active voice icon shows up for new tab
	if (floaterp == mActiveVoiceFloater)
	{
		mTabContainer->setTabImage(floaterp, "active_voice_tab.tga");	
	}
}

// EOF
