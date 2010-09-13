/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010 Martin Zuther (http://www.mzuther.de/)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Thank you for using free software!

---------------------------------------------------------------------------- */

#include "plugin_parameters.h"


// The methods of this class may be called on the audio thread, so
// they are absolutely time-critical!

KmeterPluginParameters::KmeterPluginParameters()
{
  nParam = new int[nNumParameters];

  nParam[selHeadroom] = 20;
  nParam[selExpanded] = 0;
  nParam[selPeak] = 0;
  nParam[selHold] = 0;
  nParam[selMono] = 0;

  bParamChanged = new bool[nNumParameters];

  for (int nIndex=0; nIndex < nNumParameters; nIndex++)
	 UnmarkParameter(nIndex);
}


KmeterPluginParameters::~KmeterPluginParameters()
{
  removeAllChangeListeners();

  delete [] nParam;
  delete [] bParamChanged;
}


int KmeterPluginParameters::getNumParameters()
{
  return nNumParameters;
}


bool KmeterPluginParameters::getParameterAsBool(int nIndex)
{
  return (getParameterAsInt(nIndex) != 0) ? true : false;
}


float KmeterPluginParameters::getParameterAsFloat(int nIndex)
{
  int nValue = getParameterAsInt(nIndex);
  return translateParameterToFloat(nIndex, nValue);
}


int KmeterPluginParameters::getParameterAsInt(int nIndex)
{
  jassert((nIndex >= 0) && (nIndex < nNumParameters));

  return nParam[nIndex];
}


void KmeterPluginParameters::setParameterFromBool(int nIndex, bool bValue)
{
  setParameterFromInt(nIndex, bValue ? 1 : 0);
}


void KmeterPluginParameters::setParameterFromFloat(int nIndex, float fValue)
{
  int nValue = translateParameterToInt(nIndex, fValue);
  setParameterFromInt(nIndex, nValue);
}


void KmeterPluginParameters::setParameterFromInt(int nIndex, int nValue)
{
  jassert((nIndex >= 0) && (nIndex < nNumParameters));

  if (nParam[nIndex] != nValue)
  {
	 if (nIndex == selHeadroom)
	 {
		if ((nValue == 0) || (nValue == 12) || (nValue == 14))
		  nParam[nIndex] = nValue;
		else
		  nParam[nIndex] = 20;
	 }
	 else
		nParam[nIndex] = (nValue != 0) ? 1 : 0;

	 MarkParameter(nIndex);
	 sendChangeMessage(this);
  }
}


void KmeterPluginParameters::MarkParameter(int nIndex)
{
  jassert((nIndex >= 0) && (nIndex < nNumParameters));

  bParamChanged[nIndex] = true;
}


void KmeterPluginParameters::UnmarkParameter(int nIndex)
{
  jassert((nIndex >= 0) && (nIndex < nNumParameters));

  bParamChanged[nIndex] = false;
}


bool KmeterPluginParameters::isParameterMarked(int nIndex)
{
  jassert((nIndex >= 0) && (nIndex < nNumParameters));

  return bParamChanged[nIndex];
}


const String KmeterPluginParameters::getParameterName(int nIndex)
{
  switch (nIndex)
  {
  case selHeadroom:
	 return "Headroom";
	 break;

  case selExpanded:
	 return "Expand";
	 break;

  case selPeak:
	 return "Peak";
	 break;

  case selHold:
	 return "Hold";
	 break;

  case selMono:
	 return "Mono";
	 break;

  default:
	 return "invalid";
	 break;
  }
}


const String KmeterPluginParameters::getParameterText(int nIndex)
{
  jassert((nIndex >= 0) && (nIndex < nNumParameters));

  if (nIndex == selHeadroom)
  {
	 if (nParam[selHeadroom] == 0)
		return "Normal";
	 else if (nParam[selHeadroom] == 12)
		return "K-12";
	 else if (nParam[selHeadroom] == 14)
		return "K-14";
	 else
		return "K-20";
  }
  else
	 return getParameterAsBool(nIndex) ? "on" : "off";
}


float KmeterPluginParameters::translateParameterToFloat(int nIndex, int nValue)
{
  jassert((nIndex >= 0) && (nIndex < nNumParameters));

  if (nIndex == selHeadroom)
  {
	 if (nValue == 0)
		return (selNormal / float(nNumHeadrooms - 1));
	 else if (nValue == 12)
		return (selK12 / float(nNumHeadrooms - 1));
	 else if (nValue == 14)
		return (selK14 / float(nNumHeadrooms - 1));
	 else
		return (selK20 / float(nNumHeadrooms - 1));
  }
  else
	 return (nValue != 0) ? 1.0f : 0.0f;
}


int KmeterPluginParameters::translateParameterToInt(int nIndex, float fValue)
{
  jassert((nIndex >= 0) && (nIndex < nNumParameters));

  if (nIndex == selHeadroom)
  {
	 if (fValue < (selK12 / float(nNumHeadrooms)))
		return 0;
	 else if (fValue < (selK14 / float(nNumHeadrooms)))
		return 12;
	 else if (fValue < (selK20 / float(nNumHeadrooms)))
		return 14;
	 else
		return 20;
  }
  else
	 return (fValue > 0.5f) ? true : false;
}


XmlElement KmeterPluginParameters::storeAsXml()
{
  XmlElement xml("KMETER_SETTINGS");

  int nHeadroom = getParameterAsInt(selHeadroom);
  if (nHeadroom == 0)
	 xml.setAttribute("Headroom", 20);
  else
	 xml.setAttribute("Headroom", nHeadroom);

  xml.setAttribute("Expanded", getParameterAsInt(selExpanded));
  xml.setAttribute("Peak", getParameterAsInt(selPeak));
  xml.setAttribute("Hold", getParameterAsInt(selHold));
  xml.setAttribute("Mono", getParameterAsInt(selMono));

  return xml;
}


void KmeterPluginParameters::loadFromXml(XmlElement* xml)
{
  if (xml && xml->hasTagName("KMETER_SETTINGS"))
  {
	 int nHeadroom = xml->getIntAttribute("Headroom", getParameterAsInt(selHeadroom));
	 if (nHeadroom == 0)
		setParameterFromInt(selHeadroom, 20);
	 else
		setParameterFromInt(selHeadroom, nHeadroom);

	 setParameterFromInt(selExpanded, xml->getIntAttribute("Expanded", getParameterAsInt(selExpanded)));
	 setParameterFromInt(selPeak, xml->getIntAttribute("Peak", getParameterAsInt(selPeak)));
	 setParameterFromInt(selHold, xml->getIntAttribute("Hold", getParameterAsInt(selHold)));
	 setParameterFromInt(selMono, xml->getIntAttribute("Mono", getParameterAsInt(selMono)));
  }
}
