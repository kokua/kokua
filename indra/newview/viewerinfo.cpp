/**
 * @file viewerinfo.cpp
 * @brief Functions for querying the viewer name and version.
 * @author Jacek Antonelli
 *
 * Copyright (c) 2010, Jacek Antonelli
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */


#include "viewerinfo.h"

namespace ViewerInfo
{

	// These are intentionally defined here instead of in the header,
	// because they should NOT be read directly. Use the functions.
	const std::string NAME  = "Kokua";
	const S32         MAJOR = 0;
	const S32         MINOR = 1;
	const S32         PATCH = 0;
	const std::string EXTRA = "WIP";


	const std::string& viewerName()
	{
		return NAME;
	}

	S32 versionMajor()
	{
		return MAJOR;
	}

	S32 versionMinor()
	{
		return MINOR;
	}

	S32 versionPatch()
	{
		return PATCH;
	}

	const std::string& versionExtra()
	{
		return EXTRA;
	}

	const std::string& versionNumber()
	{
		static std::string s = llformat("%d.%d.%d", MAJOR, MINOR, PATCH);
		return s;
	}

	const std::string& versionFull()
	{
		static std::string s;
		if (s.length() > 0)
		{
			return s;
		}

		s = versionNumber();

		if (EXTRA.length() > 0)
		{
			s += " " + EXTRA;
		}

		return s;
	}

	const std::string& fullInfo()
	{
		static std::string s = NAME + " " + versionFull();
		return s;
	}

}
