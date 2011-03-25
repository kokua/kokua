 /** 
 * @file KOKUAStreamingAudio.cpp
 * @brief Definition of KOKUAStreamingAudio base class for streaming audio support
 *
 * $LicenseInfo:firstyear=2011&license=viewerlgpl$
 * Kokua Viewer Source Code
 * Copyright (C) 2011, Armin.Weatherwax (at) googlemail.com
 * for the Kokua Viewer Team.
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
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"

#include "kokuastreamingaudio.h"
#include "llstreamingaudio.h"


KOKUAStreamingAudio* gAudioStream = NULL;

KOKUAStreamingAudio::KOKUAStreamingAudio(LLStreamingAudioInterface *impl) :
mStreamingAudioImpl (impl)
{

}

KOKUAStreamingAudio::~KOKUAStreamingAudio()
{
	if (mStreamingAudioImpl)
	{
		delete mStreamingAudioImpl;
		mStreamingAudioImpl = NULL;
	}
}

LLStreamingAudioInterface* KOKUAStreamingAudio::getStreamingAudioImpl()
{
	return mStreamingAudioImpl;
}

// void KOKUAStreamingAudio::setStreamingAudioImpl(LLStreamingAudioInterface *impl)
// {
// 	mStreamingAudioImpl = impl;
// }

// virtual
void KOKUAStreamingAudio::startInternetStream(const std::string& url)
{
	if (mStreamingAudioImpl)
		mStreamingAudioImpl->start(url);
}

// virtual
void KOKUAStreamingAudio::stopInternetStream()
{
	if (mStreamingAudioImpl)
		mStreamingAudioImpl->stop();
}

// virtual
void KOKUAStreamingAudio::pauseInternetStream(int pause)
{
	if (mStreamingAudioImpl)
		mStreamingAudioImpl->pause(pause);
}

// virtual
void KOKUAStreamingAudio::updateInternetStream()
{
	if (mStreamingAudioImpl)
		mStreamingAudioImpl->update();
}

// virtual
KOKUAStreamingAudio::KOKUAAudioPlayState KOKUAStreamingAudio::isInternetStreamPlaying()
{
	if (mStreamingAudioImpl)
		return (KOKUAStreamingAudio::KOKUAAudioPlayState) mStreamingAudioImpl->isPlaying();

	return KOKUAStreamingAudio::AUDIO_STOPPED; // Stopped
}

// virtual
void KOKUAStreamingAudio::setInternetStreamGain(F32 vol)
{
	if (mStreamingAudioImpl)
		mStreamingAudioImpl->setGain(vol);
}

F32 KOKUAStreamingAudio::getInternetStreamGain()
{
	if (mStreamingAudioImpl)
		return mStreamingAudioImpl->getGain();
	else
		return 1.0f;
}

// virtual
std::string KOKUAStreamingAudio::getInternetStreamURL()
{
	if (mStreamingAudioImpl)
		return mStreamingAudioImpl->getURL();
	else return std::string();
}

std::string KOKUAStreamingAudio::getVersion()
{
	if (mStreamingAudioImpl)
		return mStreamingAudioImpl->getVersion();
	else return std::string();
}