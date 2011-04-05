/** 
 * @file kokuafloaterxsidebar.cpp
 */

#include "llviewerprecompiledheaders.h"

#include "kokuafloaterxsidebar.h"

// Library includes
#include "llfloaterreg.h"

// Viewer includes
#include "llbottomtray.h"
#include "llsidetray.h"



KOKUAFloaterXSidebar* KOKUAFloaterXSidebar::findInstance()
{
	return LLFloaterReg::findTypedInstance<KOKUAFloaterXSidebar>("x_sidebar");
}

void KOKUAFloaterXSidebar::onOpen(const LLSD& key)
{
	LLButton *anchor_panel = LLBottomTray::getInstance()->getChild<LLButton>("x_sidebar_btn");

	setDockControl(new LLDockControl(
		anchor_panel, this,
		getDockTongue(), LLDockControl::TOP));


	mClosed = FALSE;
}

void KOKUAFloaterXSidebar::onClose(bool app_quitting)
{
	if(app_quitting)
		return;

	if ( !isDocked() )
		setDocked(true);

	mClosed = TRUE;
}

KOKUAFloaterXSidebar::KOKUAFloaterXSidebar(const LLSD& val)
:	LLTransientDockableFloater(NULL, true, val),
	mClosed(FALSE),
	mBtnOpenClose(NULL),
	mBtnCurrent(NULL),
	mBtnLast(NULL)

{
 	mCommitCallbackRegistrar.add("XSidebar.ChangeTab", boost::bind(&KOKUAFloaterXSidebar::onClickXSidebar,this, _2));
}

// virtual
BOOL KOKUAFloaterXSidebar::postBuild()
{
	setIsChrome(TRUE);
	setTitleVisible(TRUE);
	mBtnOpenClose = getChild<LLButton>("btn_x_sidebar_openclose");

	updateTransparency(TT_ACTIVE);

	return LLDockableFloater::postBuild();
}


void KOKUAFloaterXSidebar::onClickXSidebar(const LLSD& param)
{
	std::string command = param.asString();
//	KOKUAFloaterXSidebar* x_sidebar = KOKUAFloaterXSidebar::findInstance();

	LLSideTray* sidetray= LLSideTray::getInstance();
	if (!sidetray) return;

	bool collapsed = sidetray->getCollapsed();

	if( "sidebar_openclose" == command)
	{
		//don't select the openclose tab
		if(collapsed)
		{
			sidetray->expandSideBar();
		}
		else
		{
			sidetray->collapseSideBar();
		}


	}
	else
	{

		if(sidetray->isTabAttached(command))
		{
			if(collapsed)
			{
				// open sidebar and select tab 
				sidetray->expandSideBar();
				sidetray->selectTabByName(command);
			}
			else if(sidetray->getActiveTabName() == command)
			{
				// close sidebar if currently selected tab is chosen
				sidetray->collapseSideBar();
			}
			else
			{
				// switch to newly selected tab
				sidetray->selectTabByName(command);
			}
		}
		else
		{
			LLFloater* floater_tab = LLFloaterReg::getInstance("side_bar_tab", command);
			if(floater_tab)
			{
				if(floater_tab->isMinimized())
				{
					floater_tab->setMinimized( FALSE );
					floater_tab->setFrontmost();
				}
				else
				{
					floater_tab->setMinimized( TRUE );
				}

			}
		}

	}
}



void KOKUAFloaterXSidebar::setArrow(bool collapsed)
{
	llwarns << "Floater X Sidebar: " << (collapsed ? "closed" : "open" ) << llendl;
	if(mBtnOpenClose)
	{
		if(collapsed)
		{
			mBtnOpenClose->setImageOverlay("TabIcon_Open_Off");

			if(mBtnCurrent)// yes, sometimes we don't have one and would crash
			{
				mBtnCurrent->setToggleState(false);
			}
		}
		else
		{
			mBtnOpenClose->setImageOverlay("TabIcon_Close_Off");

			if(mBtnCurrent)
			{
				mBtnCurrent->setToggleState(true);
			}
		}
	}
}

void KOKUAFloaterXSidebar::setButton(const std::string& selected)
{
	std::string button_name = "btn_x_"+selected;

	mBtnCurrent = getChild<LLButton>(button_name);
	if(mBtnCurrent)
	{
		mBtnCurrent->setToggleState(true);

		if(mBtnLast && mBtnLast != mBtnCurrent)
		{
			mBtnLast->setToggleState(false);
		}

		mBtnLast = mBtnCurrent;
	}

}
