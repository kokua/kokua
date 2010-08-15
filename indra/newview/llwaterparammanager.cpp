/**
 * @file llwaterparammanager.cpp
 * @brief Implementation for the LLWaterParamManager class.
 *
 * $LicenseInfo:firstyear=2007&license=viewerlgpl$
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

#include "llviewerprecompiledheaders.h"

#include "llwaterparammanager.h"

#include "llrender.h"

#include "pipeline.h"
#include "llsky.h"

#include "llfloaterreg.h"
#include "llsliderctrl.h"
#include "llspinctrl.h"
#include "llcheckboxctrl.h"
#include "lluictrlfactory.h"
#include "llviewercontrol.h"
#include "llviewercamera.h"
#include "llcombobox.h"
#include "lllineeditor.h"
#include "llsdserialize.h"

// For notecard loading
#include "llvfile.h"
#include "llnotecard.h"
#include "llmemorystream.h"
#include "llnotificationsutil.h"
#include "llagent.h"
#include "llinventorymodel.h"
#include "llviewerinventory.h"
#include "llviewerregion.h"
#include "llassetuploadresponders.h"

#include "v4math.h"
#include "llviewercontrol.h"
#include "lldrawpoolwater.h"
#include "llagent.h"
#include "llagentcamera.h"
#include "llviewerregion.h"

#include "llwlparammanager.h"
#include "llwaterparamset.h"
#include "llfloaterwater.h"

#include "curl/curl.h"

LLWaterParamManager * LLWaterParamManager::sInstance = NULL;

LLWaterParamManager::LLWaterParamManager() :
	mFogColor(22.f/255.f, 43.f/255.f, 54.f/255.f, 0.0f, 0.0f, "waterFogColor", "WaterFogColor"),
	mFogDensity(4, "waterFogDensity", 2),
	mUnderWaterFogMod(0.25, "underWaterFogMod"),
	mNormalScale(2.f, 2.f, 2.f, "normScale"),
	mFresnelScale(0.5f, "fresnelScale"),
	mFresnelOffset(0.4f, "fresnelOffset"),
	mScaleAbove(0.025f, "scaleAbove"),
	mScaleBelow(0.2f, "scaleBelow"),
	mBlurMultiplier(0.1f, "blurMultiplier"),
	mWave1Dir(.5f, .5f, "wave1Dir"),
	mWave2Dir(.5f, .5f, "wave2Dir"),
	mDensitySliderValue(1.0f),
	mWaterFogKS(1.0f)
{
}

LLWaterParamManager::~LLWaterParamManager()
{
}

void LLWaterParamManager::loadAllPresets(const std::string& file_name)
{
	std::string path_name(gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, "windlight/water", ""));
	LL_DEBUGS("WindLight") << "Loading Default water settings from " << path_name << LL_ENDL;
			
	bool found = true;			
	while(found) 
	{
		std::string name;
		found = gDirUtilp->getNextFileInDir(path_name, "*.xml", name);
		if(found)
		{

			name=name.erase(name.length()-4);

			// bugfix for SL-46920: preventing filenames that break stuff.
			char * curl_str = curl_unescape(name.c_str(), name.size());
			std::string unescaped_name(curl_str);
			curl_free(curl_str);
			curl_str = NULL;

			LL_DEBUGS("WindLight") << "name: " << name << LL_ENDL;
			loadPreset(unescaped_name,FALSE);
		}
	}

	// And repeat for user presets, note the user presets will modify any system presets already loaded

	std::string path_name2(gDirUtilp->getExpandedFilename( LL_PATH_USER_SETTINGS , "windlight/water", ""));
	LL_DEBUGS("WindLight") << "Loading User water settings from " << path_name2 << LL_ENDL;
			
	found = true;			
	while(found) 
	{
		std::string name;
		found = gDirUtilp->getNextFileInDir(path_name2, "*.xml", name);
		if(found)
		{
			name=name.erase(name.length()-4);

			// bugfix for SL-46920: preventing filenames that break stuff.
			char * curl_str = curl_unescape(name.c_str(), name.size());
			std::string unescaped_name(curl_str);
			curl_free(curl_str);
			curl_str = NULL;

			LL_DEBUGS("WindLight") << "name: " << name << LL_ENDL;
			loadPreset(unescaped_name,FALSE);
		}
	}

}

void LLWaterParamManager::loadPreset(const std::string & name,bool propagate)
{
	// Check if we already have the preset before we try loading it again.
	if(mParamList.find(name) != mParamList.end())
	{
		if(propagate)
		{
			getParamSet(name, mCurParams);
			propagateParameters();
		}
		return;
	}

	// bugfix for SL-46920: preventing filenames that break stuff.
	char * curl_str = curl_escape(name.c_str(), name.size());
	std::string escaped_filename(curl_str);
	curl_free(curl_str);
	curl_str = NULL;

	escaped_filename += ".xml";

	std::string pathName(gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, "windlight/water", escaped_filename));
	LL_DEBUGS("WindLight") << "Loading water settings from " << pathName << LL_ENDL;
	
	llifstream presetsXML;
	presetsXML.open(pathName.c_str());
	
	// That failed, try loading from the users area instead.
	if(!presetsXML)
	{
		pathName=gDirUtilp->getExpandedFilename( LL_PATH_USER_SETTINGS , "windlight/water", escaped_filename);
		LL_DEBUGS("WindLight") << "Loading User water setting from " << pathName << LL_ENDL;
		presetsXML.clear();
        presetsXML.open(pathName.c_str());
	}

	if (presetsXML)
	{
		loadPresetXML(name, presetsXML);
		presetsXML.close();
	} 
	else 
	{
		llwarns << "Can't find " << name << llendl;
		return;
	}

	if(propagate)
	{
		getParamSet(name, mCurParams);
		propagateParameters();
	}
}

bool LLWaterParamManager::loadPresetXML(const std::string& name, std::istream& preset_stream, bool propagate /* = false */, bool check_if_real /* = false */)
{
	LLSD paramsData(LLSD::emptyMap());
	
	LLPointer<LLSDParser> parser = new LLSDXMLParser();
	
	if(parser->parse(preset_stream, paramsData, LLSDSerialize::SIZE_UNLIMITED) == LLSDParser::PARSE_FAILURE)
	{
		return false;
	}
	
	if(check_if_real)
	{
		static const char* expected_windlight_settings[] = {
			"blurMultiplier",
			"fresnelOffset",
			"fresnelScale",
			"normScale",
			"normalMap",
			"scaleAbove",
			"scaleBelow",
			"waterFogColor",
			"waterFogDensity",
			"wave1Dir",
			"wave2Dir"
		};
		static S32 expected_count = LL_ARRAY_SIZE(expected_windlight_settings);
		for(S32 i = 0; i < expected_count; ++i)
		{
			if(!paramsData.has(expected_windlight_settings[i]))
			{
				LL_WARNS("WindLight") << "Attempted to load WindLight water param set without " << expected_windlight_settings[i] << LL_ENDL;
				return false;
			}
		}
	}
	
	std::map<std::string, LLWaterParamSet>::iterator mIt = mParamList.find(name);
	if(mIt == mParamList.end())
	{
		addParamSet(name, paramsData);
	}
	else 
	{
		setParamSet(name, paramsData);
	}
	
	if(propagate)
	{
		getParamSet(name, mCurParams);
		propagateParameters();
	}
	return true;
}

void LLWaterParamManager::loadPresetNotecard(const std::string& name, const LLUUID& asset_id, const LLUUID& inv_id)
{
	gAssetStorage->getInvItemAsset(LLHost::invalid,
								   gAgent.getID(),
								   gAgent.getSessionID(),
								   gAgent.getID(),
								   LLUUID::null,
								   inv_id,
								   asset_id,
								   LLAssetType::AT_NOTECARD,
								   &loadWaterNotecard,
								   (void*)&inv_id);
}

void LLWaterParamManager::savePreset(const std::string & name)
{
	// bugfix for SL-46920: preventing filenames that break stuff.
	char * curl_str = curl_escape(name.c_str(), name.size());
	std::string escaped_filename(curl_str);
	curl_free(curl_str);
	curl_str = NULL;

	escaped_filename += ".xml";

	// make an empty llsd
	LLSD paramsData(LLSD::emptyMap());
	std::string pathName(gDirUtilp->getExpandedFilename( LL_PATH_USER_SETTINGS , "windlight/water", escaped_filename));

	// fill it with LLSD windlight params
	paramsData = mParamList[name].getAll();

	// write to file
	llofstream presetsXML(pathName);
	LLPointer<LLSDFormatter> formatter = new LLSDXMLFormatter();
	formatter->format(paramsData, presetsXML, LLSDFormatter::OPTIONS_PRETTY);
	presetsXML.close();

	propagateParameters();
}


void LLWaterParamManager::propagateParameters(void)
{
	// bind the variables only if we're using shaders
	if(gPipeline.canUseVertexShaders())
	{
		LLViewerShaderMgr::shader_iter shaders_iter, end_shaders;
		end_shaders = LLViewerShaderMgr::instance()->endShaders();
		for(shaders_iter = LLViewerShaderMgr::instance()->beginShaders(); shaders_iter != end_shaders; ++shaders_iter)
		{
			if (shaders_iter->mProgramObject != 0
				&& shaders_iter->mShaderGroup == LLGLSLShader::SG_WATER)
			{
				shaders_iter->mUniformsDirty = TRUE;
			}
		}
	}

	bool err;
	F32 fog_density_slider = 
		log(mCurParams.getFloat(mFogDensity.mName, err)) / 
		log(mFogDensity.mBase);

	setDensitySliderValue(fog_density_slider);
}

void LLWaterParamManager::updateShaderUniforms(LLGLSLShader * shader)
{
	if (shader->mShaderGroup == LLGLSLShader::SG_WATER)
	{
		shader->uniform4fv(LLViewerShaderMgr::LIGHTNORM, 1, LLWLParamManager::instance()->getRotatedLightDir().mV);
		shader->uniform3fv("camPosLocal", 1, LLViewerCamera::getInstance()->getOrigin().mV);
		shader->uniform4fv("waterFogColor", 1, LLDrawPoolWater::sWaterFogColor.mV);
		shader->uniform4fv("waterPlane", 1, mWaterPlane.mV);
		shader->uniform1f("waterFogDensity", getFogDensity());
		shader->uniform1f("waterFogKS", mWaterFogKS);
		shader->uniform4f("distance_multiplier", 0, 0, 0, 0);
	}
}

static LLFastTimer::DeclareTimer FTM_UPDATE_WLPARAM("Update Windlight Params");

void LLWaterParamManager::update(LLViewerCamera * cam)
{
	LLFastTimer ftm(FTM_UPDATE_WLPARAM);
	
	// update the shaders and the menu
	propagateParameters();
	
	// sync menus if they exist
	LLFloaterWater* waterfloater = LLFloaterReg::findTypedInstance<LLFloaterWater>("env_water");
	if(waterfloater) 
	{
		waterfloater->syncMenu();
	}

	// only do this if we're dealing with shaders
	if(gPipeline.canUseVertexShaders()) 
	{
		//transform water plane to eye space
		glh::vec3f norm(0.f, 0.f, 1.f);
		glh::vec3f p(0.f, 0.f, gAgent.getRegion()->getWaterHeight()+0.1f);
		
		F32 modelView[16];
		for (U32 i = 0; i < 16; i++)
		{
			modelView[i] = (F32) gGLModelView[i];
		}

		glh::matrix4f mat(modelView);
		glh::matrix4f invtrans = mat.inverse().transpose();
		glh::vec3f enorm;
		glh::vec3f ep;
		invtrans.mult_matrix_vec(norm, enorm);
		enorm.normalize();
		mat.mult_matrix_vec(p, ep);

		mWaterPlane = LLVector4(enorm.v[0], enorm.v[1], enorm.v[2], -ep.dot(enorm));

		LLVector3 sunMoonDir;
		if (gSky.getSunDirection().mV[2] > LLSky::NIGHTTIME_ELEVATION_COS) 	 
		{ 	 
			sunMoonDir = gSky.getSunDirection(); 	 
		} 	 
		else  	 
		{ 	 
			sunMoonDir = gSky.getMoonDirection(); 	 
		}
		sunMoonDir.normVec();
		mWaterFogKS = 1.f/llmax(sunMoonDir.mV[2], WATER_FOG_LIGHT_CLAMP);

		LLViewerShaderMgr::shader_iter shaders_iter, end_shaders;
		end_shaders = LLViewerShaderMgr::instance()->endShaders();
		for(shaders_iter = LLViewerShaderMgr::instance()->beginShaders(); shaders_iter != end_shaders; ++shaders_iter)
		{
			if (shaders_iter->mProgramObject != 0
				&& shaders_iter->mShaderGroup == LLGLSLShader::SG_WATER)
			{
				shaders_iter->mUniformsDirty = TRUE;
			}
		}
	}
}

// static
void LLWaterParamManager::initClass(void)
{
	instance();
}

// static
void LLWaterParamManager::cleanupClass(void)
{
	delete sInstance;
	sInstance = NULL;
}

bool LLWaterParamManager::addParamSet(const std::string& name, LLWaterParamSet& param)
{
	// add a new one if not one there already
	std::map<std::string, LLWaterParamSet>::iterator mIt = mParamList.find(name);
	if(mIt == mParamList.end()) 
	{	
		mParamList[name] = param;
		return true;
	}

	return false;
}

BOOL LLWaterParamManager::addParamSet(const std::string& name, LLSD const & param)
{
	// add a new one if not one there already
	std::map<std::string, LLWaterParamSet>::const_iterator finder = mParamList.find(name);
	if(finder == mParamList.end())
	{
		mParamList[name].setAll(param);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

bool LLWaterParamManager::getParamSet(const std::string& name, LLWaterParamSet& param)
{
	// find it and set it
	std::map<std::string, LLWaterParamSet>::iterator mIt = mParamList.find(name);
	if(mIt != mParamList.end()) 
	{
		param = mParamList[name];
		param.mName = name;
		return true;
	}

	return false;
}

bool LLWaterParamManager::setParamSet(const std::string& name, LLWaterParamSet& param)
{
	mParamList[name] = param;

	return true;
}

bool LLWaterParamManager::setParamSet(const std::string& name, const LLSD & param)
{
	// quick, non robust (we won't be working with files, but assets) check
	if(!param.isMap()) 
	{
		return false;
	}
	
	mParamList[name].setAll(param);

	return true;
}

bool LLWaterParamManager::removeParamSet(const std::string& name, bool delete_from_disk)
{
	// remove from param list
	std::map<std::string, LLWaterParamSet>::iterator mIt = mParamList.find(name);
	if(mIt != mParamList.end()) 
	{
		mParamList.erase(mIt);
	}

	if(delete_from_disk)
	{

		std::string path_name(gDirUtilp->getExpandedFilename( LL_PATH_USER_SETTINGS , "windlight/water", ""));
		
		// use full curl escaped name
		char * curl_str = curl_escape(name.c_str(), name.size());
		std::string escaped_name(curl_str);
		curl_free(curl_str);
		curl_str = NULL;
		
		gDirUtilp->deleteFilesInDir(path_name, escaped_name + ".xml");
	}

	return true;
}

F32 LLWaterParamManager::getFogDensity(void)
{
	bool err;

	F32 fogDensity = mCurParams.getFloat("waterFogDensity", err);
	
	// modify if we're underwater
	const F32 water_height = gAgent.getRegion() ? gAgent.getRegion()->getWaterHeight() : 0.f;
	F32 camera_height = gAgentCamera.getCameraPositionAgent().mV[2];
	if(camera_height <= water_height)
	{
		// raise it to the underwater fog density modifier
		fogDensity = pow(fogDensity, mCurParams.getFloat("underWaterFogMod", err));
	}

	return fogDensity;
}

// static
LLWaterParamManager * LLWaterParamManager::instance()
{
	if(NULL == sInstance)
	{
		sInstance = new LLWaterParamManager();

		sInstance->loadAllPresets(LLStringUtil::null);

		sInstance->getParamSet("Default", sInstance->mCurParams);
	}

	return sInstance;
}

// static
void LLWaterParamManager::loadWaterNotecard(LLVFS *vfs, const LLUUID& asset_id, LLAssetType::EType asset_type, void *user_data, S32 status, LLExtStat ext_status)
{
	LLUUID inventory_id(*((LLUUID*)user_data));
	std::string name = "WindLight Setting.ww";
	LLViewerInventoryItem *item = gInventory.getLinkedItem(inventory_id);
	if(item)
	{
		inventory_id = item->getUUID();
		name = item->getName();
	}
	if(LL_ERR_NOERR == status)
	{
		LLVFile file(vfs, asset_id, asset_type, LLVFile::READ);
		S32 file_length = file.getSize();
		std::vector<char> buffer(file_length + 1);
		file.read((U8*)&buffer[0], file_length);
		buffer[file_length] = 0;
		LLNotecard notecard(LLNotecard::MAX_SIZE);
		LLMemoryStream str((U8*)&buffer[0], file_length + 1);
		notecard.importStream(str);
		std::string settings = notecard.getText();
		LLMemoryStream settings_str((U8*)settings.c_str(), settings.length());
		bool is_real_setting = sInstance->loadPresetXML(name, settings_str, true, true);
		if(!is_real_setting)
		{
			LLSD subs;
			subs["NAME"] = name;
			LLNotificationsUtil::add("KittyInvalidWaterlightNotecard", subs);
		}
		else
		{
			// We can do this because we know mCurParams 
			sInstance->mParamList[name].mInventoryID = inventory_id;
		}
	}
}
