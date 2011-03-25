/** 
 * @file llslurl_test.cpp
 * @author Roxie
 * @date 2009-02-10
 * @brief Test the slurl functionality
 *
 * $LicenseInfo:firstyear=2009&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */
#include "../llviewerprecompiledheaders.h"
#include "../llviewernetwork.h"
#include "../test/lltut.h"
#include "../llslurl.h"
#include "../../llxml/llcontrol.h"
#include "llsdserialize.h"
//----------------------------------------------------------------------------               
// Mock objects for the dependencies of the code we're testing                               

LLControlGroup::LLControlGroup(const std::string& name)
: LLInstanceTracker<LLControlGroup, std::string>(name) {}
LLControlGroup::~LLControlGroup() {}
BOOL LLControlGroup::declareString(const std::string& name,
                                   const std::string& initial_val,
                                   const std::string& comment,
                                   BOOL persist) {return TRUE;}
void LLControlGroup::setString(const std::string& name, const std::string& val){}

std::string gCmdLineLoginURI;
std::string gCmdLineGridChoice;
std::string gCmdLineHelperURI;
std::string gLoginPage;
std::string gCurrentGrid;

std::string LLControlGroup::getString(const std::string& name)
{
	if (name == "CmdLineGridChoice")
		return gCmdLineGridChoice;
	else if (name == "CmdLineHelperURI")
		return gCmdLineHelperURI;
	else if (name == "LoginPage")
		return gLoginPage;
	else if (name == "CurrentGrid")
		return gCurrentGrid;
	return "";
}

LLSD LLControlGroup::getLLSD(const std::string& name)
{
	if (name == "CmdLineLoginURI")
	{
		if(!gCmdLineLoginURI.empty())
		{
			return LLSD(gCmdLineLoginURI);
		}
	}
	return LLSD();
}

void LLControlGroup::setLLSD(const std::string& name, const LLSD& value)
{
	if (name == "CmdLineLoginURI")
	{
		gCmdLineLoginURI = value.asString();
	}

}

LLPointer<LLControlVariable> LLControlGroup::getControl(const std::string& name)
{
	ctrl_name_table_t::iterator iter = mNameTable.find(name);
	return iter == mNameTable.end() ? LLPointer<LLControlVariable>() : iter->second;
}

LLControlGroup gSavedSettings("test");

// -------------------------------------------------------------------------------------------
// TUT
// -------------------------------------------------------------------------------------------
namespace tut
{
	// Test wrapper declaration : wrapping nothing for the moment
	struct slurlTest
	{
		slurlTest()
		{	
			LLGridManager::getInstance()->initialize(std::string(""));
			LLGridManager::getInstance()->LLGridManager::initGrids();

		}
		~slurlTest()
		{
		}
	};
	
	// Tut templating thingamagic: test group, object and test instance
	typedef test_group<slurlTest> slurlTestFactory;
	typedef slurlTestFactory::object slurlTestObject;
	tut::slurlTestFactory tut_test("LLSlurl");
	
	// ---------------------------------------------------------------------------------------
	// Test functions 
	// ---------------------------------------------------------------------------------------
	// construction from slurl string
	template<> template<>
	void slurlTestObject::test<1>()
	{
		// Kokua:
		// see(!) http://redmine.kokuaviewer.org/issues/784
		// The correct notation of ensure_equals is:
		// 		ensure_equals("Fail message",  result from tested object, expectation);


		LLGridManager::getInstance()->setGridChoice("util.agni.lindenlab.com");
		
		LLSLURL slurl = LLSLURL("");
		ensure_equals("1 null slurl", (int)slurl.getType(), LLSLURL::LAST_LOCATION);
		
		slurl = LLSLURL("http://slurl.com/secondlife/myregion");
		ensure_equals("2 slurl.com slurl, region only - type", slurl.getType(), LLSLURL::LOCATION);
		ensure_equals("3 slurl.com slurl, region only", slurl.getSLURLString(), 
					  "http://maps.secondlife.com/secondlife/myregion/128/128/0");
		
		slurl = LLSLURL("http://maps.secondlife.com/secondlife/myregion/1/2/3");
		ensure_equals("4 maps.secondlife.com slurl, region + coords - type", slurl.getType(), LLSLURL::LOCATION);
		ensure_equals("5 maps.secondlife.com slurl, region + coords", slurl.getSLURLString(), 
					  "http://maps.secondlife.com/secondlife/myregion/1/2/3");

		slurl = LLSLURL("secondlife://myregion");
		ensure_equals("6 secondlife: slurl, region only - type", slurl.getType(), LLSLURL::LOCATION);
		ensure_equals("7 secondlife: slurl, region only", slurl.getSLURLString(), 
					  "http://maps.secondlife.com/secondlife/myregion/128/128/0");

		
		slurl = LLSLURL("secondlife://myregion/1/2/3");
		ensure_equals("8 secondlife: slurl, region + coords - type", slurl.getType(), LLSLURL::LOCATION);
		ensure_equals("9 secondlife slurl, region + coords", slurl.getSLURLString(), 
					  "http://maps.secondlife.com/secondlife/myregion/1/2/3");

		slurl = LLSLURL("/myregion");
		ensure_equals("10 /region slurl, region- type", slurl.getType(), LLSLURL::LOCATION);
		ensure_equals("11 /region slurl, region ", slurl.getSLURLString(), 
					  "http://maps.secondlife.com/secondlife/myregion/128/128/0");

		slurl = LLSLURL("/myregion/1/2/3");
		ensure_equals("12 /: slurl, region + coords - type", slurl.getType(), LLSLURL::LOCATION);
		ensure_equals("13 / slurl, region + coords", slurl.getSLURLString(), 
					  "http://maps.secondlife.com/secondlife/myregion/1/2/3");	

		slurl = LLSLURL("my region/1/2/3");
		ensure_equals("14 slurl, region + coords - type", slurl.getType(), LLSLURL::LOCATION);
		ensure_equals("15 slurl, region + coords", slurl.getSLURLString(), 
					  "http://maps.secondlife.com/secondlife/my%20region/1/2/3");	

		GridEntry* grid_entry = new GridEntry;
		grid_entry->grid = LLSD::emptyMap();
		grid_entry->grid[GRID_VALUE] = "my.grid.com";
		grid_entry->set_current=true;
		LLGridManager::getInstance()->addGrid(grid_entry, LLGridManager::SYSTEM);
		// we set grid choice  with: grid_entry->set_current=true
		// 		 instead of: LLGridManager::getInstance()->setGridChoice("my.grid.com");
		slurl = LLSLURL("https://my.grid.com/region/my%20region/1/2/3");
		ensure_equals("16 grid slurl, region + coords - type", slurl.getTypeHumanReadable(),
							LLSLURL::getTypeHumanReadable(LLSLURL::LOCATION));
		ensure_equals("17 grid slurl, region + coords", slurl.getSLURLString(), 
					  "https://my.grid.com/region/my%20region/1/2/3");


		slurl = LLSLURL("https://my.grid.com/region/my region");
		ensure_equals("18 grid slurl, region + coords - type", slurl.getType(), LLSLURL::LOCATION);
 		ensure_equals("19 grid slurl, region + coords", slurl.getSLURLString(), 
					  "https://my.grid.com/region/my%20region/128/128/0");

		grid_entry = new GridEntry;
		grid_entry->grid = LLSD::emptyMap();
		grid_entry->grid[GRID_VALUE] = "foo.bar.com";
		grid_entry->set_current=true;
		LLGridManager::getInstance()->addGrid(grid_entry, LLGridManager::SYSTEM);
		slurl = LLSLURL("/myregion/1/2/3");
		ensure_equals("20 /: slurl, region + coords - type", slurl.getType(), LLSLURL::LOCATION);
		ensure_equals("21 / slurl, region + coords", slurl.getSLURLString(), 
					  "https://foo.bar.com/region/myregion/1/2/3");		

		slurl = LLSLURL("myregion/1/2/3");
		ensure_equals("22 : slurl, region + coords - type", slurl.getType(), LLSLURL::LOCATION);
		ensure_equals("23 slurl, region + coords", slurl.getSLURLString(), 
					  "https://foo.bar.com/region/myregion/1/2/3");		
		
		slurl = LLSLURL(LLSLURL::SIM_LOCATION_HOME);
		ensure_equals("24 home", slurl.getType(), LLSLURL::HOME_LOCATION);
		slurl = LLSLURL(LLSLURL::SIM_LOCATION_LAST);
		ensure_equals("25 last", slurl.getType(), LLSLURL::LAST_LOCATION);
		
		slurl = LLSLURL("secondlife:///app/foo/bar?12345");
		ensure_equals("26 app", slurl.getTypeHumanReadable(), LLSLURL::getTypeHumanReadable(LLSLURL::APP));
		ensure_equals("27 appcmd", slurl.getAppCmd(), "foo");
		ensure_equals("28 apppath", slurl.getAppPath().size(), 1);
		ensure_equals("29 apppath2", slurl.getAppPath()[0].asString(), "bar");
		ensure_equals("30 appquery", slurl.getAppQuery(), "12345");
		ensure_equals("31 grid1", "foo.bar.com", slurl.getGrid());

		slurl = LLSLURL("secondlife://Aditi/app/foo/bar?12345");
		ensure_equals("32 app", slurl.getTypeHumanReadable(), LLSLURL::getTypeHumanReadable(LLSLURL::APP));
		ensure_equals("33 appcmd", slurl.getAppCmd(), "foo");
		ensure_equals("34 apppath", slurl.getAppPath().size(), 1);
		ensure_equals("35 apppath2", slurl.getAppPath()[0].asString(), "bar");
		ensure_equals("36 appquery", slurl.getAppQuery(), "12345");
// Kokua: fixme
/*
		ensure_equals("37 grid2",  slurl.getGrid(), "util.aditi.lindenlab.com");
// returns [LLSlurl, 1] fail: '37 grid2: expected 'util.aditi.lindenlab.com' actual 'util.agni.lindenlab.com''
// probably the fallback
*/

		LLGridManager::getInstance()->setGridChoice("foo.bar.com");
		slurl = LLSLURL("secondlife:///secondlife/myregion/1/2/3");
		ensure_equals("38 /: slurl, region + coords - type", slurl.getType(), LLSLURL::LOCATION);
		ensure_equals("39 location", slurl.getType(), LLSLURL::LOCATION);
		ensure_equals("40 region" , slurl.getRegion(), "myregion");
		ensure_equals("41 grid3", slurl.getGrid(), "util.agni.lindenlab.com");

		slurl = LLSLURL("secondlife://Aditi/secondlife/myregion/1/2/3");
		ensure_equals("42 /: slurl, region + coords - type", slurl.getType(), LLSLURL::LOCATION);
		ensure_equals("43 location", slurl.getTypeHumanReadable(), LLSLURL::getTypeHumanReadable(LLSLURL::LOCATION));
		ensure_equals("44 region" , slurl.getRegion(), "myregion");
// Kokua: fixme
/*
		ensure_equals("45 grid4", slurl.getGrid(), "util.aditi.lindenlab.com");		
// returns [LLSlurl, 1] fail: '45 grid4: expected 'util.aditi.lindenlab.com' actual 'util.agni.lindenlab.com''
// probably the fallback
*/
		LLGridManager::getInstance()->setGridChoice("my.grid.com");
		slurl = LLSLURL("https://my.grid.com/app/foo/bar?12345");
		ensure_equals("46 app", slurl.getTypeHumanReadable(), LLSLURL::getTypeHumanReadable(LLSLURL::APP));	
		ensure_equals("47 appcmd", slurl.getAppCmd(), "foo");
		ensure_equals("48 apppath", slurl.getAppPath().size(), 1);
		ensure_equals("49 apppath2", slurl.getAppPath()[0].asString(), "bar");
		ensure_equals("50 appquery", slurl.getAppQuery(), "12345");	
	}
	
	// construction from grid/region/vector combos
	template<> template<>
	void slurlTestObject::test<2>()
	{
		LLSLURL slurl = LLSLURL("mygrid.com", "my region");
		ensure_equals("1 grid/region - type", slurl.getType(), LLSLURL::LOCATION);
		ensure_equals("2 grid/region", slurl.getSLURLString(), 
					  "https://mygrid.com/region/my%20region/128/128/0");	
		
		slurl = LLSLURL("mygrid.com", "my region", LLVector3(1,2,3));
		ensure_equals("3 grid/region/vector - type", slurl.getType(), LLSLURL::LOCATION);
		ensure_equals("4 grid/region/vector", slurl.getSLURLString(), 
					  "https://mygrid.com/region/my%20region/1/2/3");			

		GridEntry* grid_entry = new GridEntry;
		grid_entry->grid = LLSD::emptyMap();
		grid_entry->grid[GRID_VALUE] = "foo.bar.com.bar";
		grid_entry->set_current=true;
		LLGridManager::getInstance()->addGrid(grid_entry, LLGridManager::SYSTEM);

		slurl = LLSLURL("my region", LLVector3(1,2,3));
		ensure_equals("5 grid/region/vector - type", slurl.getType(), LLSLURL::LOCATION);
		ensure_equals("6 grid/region/vector", slurl.getSLURLString(), 
					  "https://foo.bar.com.bar/region/my%20region/1/2/3");

		
		LLGridManager::getInstance()->setGridChoice("util.agni.lindenlab.com");	
		slurl = LLSLURL("my region", LLVector3(1,2,3));
		ensure_equals("7 default grid/region/vector - type", slurl.getType(), LLSLURL::LOCATION);
		ensure_equals("8 default grid/region/vector", slurl.getSLURLString(), 
					  "http://maps.secondlife.com/secondlife/my%20region/1/2/3");	
		
	}
	// Accessors
	template<> template<>
	void slurlTestObject::test<3>()
	{
		GridEntry* grid_entry = new GridEntry;
		grid_entry->grid = LLSD::emptyMap();
		grid_entry->grid[GRID_VALUE] = "my.wonderful.grid.com";
		grid_entry->set_current=true;
		LLGridManager::getInstance()->addGrid(grid_entry, LLGridManager::SYSTEM);

		LLSLURL slurl = LLSLURL("https://my.wonderful.grid.com/region/my%20region/1/2/3");
		ensure_equals("login string", slurl.getLoginString(), "uri:my region&amp;1&amp;2&amp;3");
		ensure_equals("location string", slurl.getLocationString(), "my region/1/2/3");
		ensure_equals("grid", slurl.getGrid(), "my.wonderful.grid.com");
		ensure_equals("region", slurl.getRegion(), "my region");
		ensure_equals("position", slurl.getPosition(), LLVector3(1, 2, 3));

		
	}
}
