/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 CTTC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Nicola Baldo <nbaldo@cttc.es>
 * Modified by Marco Miozzo <mmiozzo@cttc.es> (add data and ctrl diversity)
 */

#include <ns3/log.h>
#include <ns3/packet-burst.h>
#include <ns3/ptr.h>
#include <ns3/nr-spectrum-signal-parameters.h>
#include <ns3/nr-control-messages.h>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrSpectrumSignalParameters");

NrSpectrumSignalParameters::NrSpectrumSignalParameters ()
{
  NS_LOG_FUNCTION (this);
}

NrSpectrumSignalParameters::NrSpectrumSignalParameters (const NrSpectrumSignalParameters& p)
  : SpectrumSignalParameters (p)
{
  NS_LOG_FUNCTION (this << &p);
  packetBurst = p.packetBurst->Copy ();
}

Ptr<SpectrumSignalParameters>
NrSpectrumSignalParameters::Copy ()
{
  NS_LOG_FUNCTION (this);
  // Ideally we would use:
  //   return Copy<NrSpectrumSignalParameters> (*this);
  // but for some reason it doesn't work. Another alternative is 
  //   return Copy<NrSpectrumSignalParameters> (this);
  // but it causes a double creation of the object, hence it is less efficient.
  // The solution below is copied from the implementation of Copy<> (Ptr<>) in ptr.h
  Ptr<NrSpectrumSignalParameters> lssp (new NrSpectrumSignalParameters (*this), false);  
  return lssp;
}



NrSpectrumSignalParametersDataFrame::NrSpectrumSignalParametersDataFrame ()
{
  NS_LOG_FUNCTION (this);
}

NrSpectrumSignalParametersDataFrame::NrSpectrumSignalParametersDataFrame (const NrSpectrumSignalParametersDataFrame& p)
: SpectrumSignalParameters (p)
{
  NS_LOG_FUNCTION (this << &p);
  cellId = p.cellId;
  if (p.packetBurst)
    {
      packetBurst = p.packetBurst->Copy ();
    }
  ctrlMsgList = p.ctrlMsgList;
}

Ptr<SpectrumSignalParameters>
NrSpectrumSignalParametersDataFrame::Copy ()
{
  NS_LOG_FUNCTION (this);
  // Ideally we would use:
  //   return Copy<NrSpectrumSignalParametersDataFrame> (*this);
  // but for some reason it doesn't work. Another alternative is 
  //   return Copy<NrSpectrumSignalParametersDataFrame> (this);
  // but it causes a double creation of the object, hence it is less efficient.
  // The solution below is copied from the implementation of Copy<> (Ptr<>) in ptr.h
  Ptr<NrSpectrumSignalParametersDataFrame> lssp (new NrSpectrumSignalParametersDataFrame (*this), false);  
  return lssp;
}



NrSpectrumSignalParametersDlCtrlFrame::NrSpectrumSignalParametersDlCtrlFrame ()
{
  NS_LOG_FUNCTION (this);
}

NrSpectrumSignalParametersDlCtrlFrame::NrSpectrumSignalParametersDlCtrlFrame (const NrSpectrumSignalParametersDlCtrlFrame& p)
: SpectrumSignalParameters (p)
{
  NS_LOG_FUNCTION (this << &p);
  cellId = p.cellId;
  pss = p.pss;
  ctrlMsgList = p.ctrlMsgList;
}

Ptr<SpectrumSignalParameters>
NrSpectrumSignalParametersDlCtrlFrame::Copy ()
{
  NS_LOG_FUNCTION (this);
  // Ideally we would use:
  //   return Copy<NrSpectrumSignalParametersDlCtrlFrame> (*this);
  // but for some reason it doesn't work. Another alternative is 
  //   return Copy<NrSpectrumSignalParametersDlCtrlFrame> (this);
  // but it causes a double creation of the object, hence it is less efficient.
  // The solution below is copied from the implementation of Copy<> (Ptr<>) in ptr.h
  Ptr<NrSpectrumSignalParametersDlCtrlFrame> lssp (new NrSpectrumSignalParametersDlCtrlFrame (*this), false);  
  return lssp;
}


NrSpectrumSignalParametersUlSrsFrame::NrSpectrumSignalParametersUlSrsFrame ()
{
  NS_LOG_FUNCTION (this);
}

NrSpectrumSignalParametersUlSrsFrame::NrSpectrumSignalParametersUlSrsFrame (const NrSpectrumSignalParametersUlSrsFrame& p)
: SpectrumSignalParameters (p)
{
  NS_LOG_FUNCTION (this << &p);
  cellId = p.cellId;
}

Ptr<SpectrumSignalParameters>
NrSpectrumSignalParametersUlSrsFrame::Copy ()
{
  NS_LOG_FUNCTION (this);
  // Ideally we would use:
  //   return Copy<NrSpectrumSignalParametersUlSrsFrame> (*this);
  // but for some reason it doesn't work. Another alternative is 
  //   return Copy<NrSpectrumSignalParametersUlSrsFrame> (this);
  // but it causes a double creation of the object, hence it is less efficient.
  // The solution below is copied from the implementation of Copy<> (Ptr<>) in ptr.h
  Ptr<NrSpectrumSignalParametersUlSrsFrame> lssp (new NrSpectrumSignalParametersUlSrsFrame (*this), false);  
  return lssp;
}







} // namespace ns3
