/**
 * @file viewerinfo.h
 * @brief Functions for querying the viewer name and version.
 * @author Jacek Antonelli
 *
 * Copyright (c) 2010, Jacek Antonelli
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
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

#ifndef VERSIONINFO_H
#define VERSIONINFO_H

#include "linden_common.h"

namespace ViewerInfo
{
	/// Returns the name of the viewer.
  const std::string& viewerName();

	/// Returns the major (first) version number.
  S32 versionMajor();

	/// Returns the minor (second) version number.
  S32 versionMinor();

	/// Returns the patch (third) version number.
  S32 versionPatch();

	/// Returns the extra version string (e.g. "beta 1", "RC1").
	/// May be empty, if no extra string was set.
  const std::string& versionExtra();

	/// Returns a dot-separated string with all three version numbers.
  const std::string& versionNumber();

	/// Returns a string with the version number and extra version
	/// string (if not empty).
  const std::string& versionFull();

	/// Returns a string with the viewer name and full version.
  const std::string& fullInfo();
}

#endif // VERSIONINFO_H
