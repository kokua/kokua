/** 
 * @file kokuastreamingaudio.h
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
 *
 * $/LicenseInfo$
 */
#ifndef KOKUA_STREAMINGAUDIO_H
#define KOKUA_STREAMINGAUDIO_H

#include "linden_common.h"

class LLStreamingAudioInterface;

class KOKUAStreamingAudio 
{
public:
	enum KOKUAAudioPlayState
	{
		// isInternetStreamPlaying() returns an *int*, with
		// 0 = stopped, 1 = playing, 2 = paused.
		AUDIO_STOPPED = 0,
		AUDIO_PLAYING = 1,
		AUDIO_PAUSED = 2
	};

	KOKUAStreamingAudio(LLStreamingAudioInterface *impl);
	virtual ~KOKUAStreamingAudio();

	LLStreamingAudioInterface *getStreamingAudioImpl();
// 	void setStreamingAudioImpl(LLStreamingAudioInterface *impl);
	void startInternetStream(const std::string& url);
	void stopInternetStream();
	void pauseInternetStream(int pause);
	void updateInternetStream(); 
	KOKUAAudioPlayState isInternetStreamPlaying();
	void setInternetStreamGain(F32 vol);
	F32 getInternetStreamGain();
	std::string getInternetStreamURL();
	std::string getVersion();	
private:
	LLStreamingAudioInterface *mStreamingAudioImpl;
};

extern KOKUAStreamingAudio* gAudioStream;

#endif
