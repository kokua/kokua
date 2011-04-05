/** 
 * @file kokuafloaterxsidebar.h
 */

#ifndef KOKUA_FLOATER_X_SIDEBAR_H
#define KOKUA_FLOATER_X_SIDEBAR_H

#include "lltransientdockablefloater.h"


class KOKUAFloaterXSidebar
	:	public LLTransientDockableFloater
{
	friend class LLFloaterReg;
	
public:



	virtual void onOpen(const LLSD& key);
	virtual void onClose(bool app_quitting);

	void onClickXSidebar(const LLSD& param);

	void setArrow(bool collapsed);
	void setButton(const std::string& selected);
	static KOKUAFloaterXSidebar* findInstance();


private:

	KOKUAFloaterXSidebar(const LLSD& val);
	~KOKUAFloaterXSidebar() {};



	/*virtual*/ BOOL postBuild();

	LLButton* mBtnOpenClose;
	LLButton* mBtnCurrent;
	LLButton* mBtnLast;

	BOOL mClosed;
};



#endif //KOKUA_FLOATER_X_SIDEBAR_H
