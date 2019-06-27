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

#ifndef NR_SPECTRUM_SIGNAL_PARAMETERS_H
#define NR_SPECTRUM_SIGNAL_PARAMETERS_H


#include <ns3/spectrum-signal-parameters.h>

namespace ns3 {

class PacketBurst;
class NrControlMessage;


/**
 * \ingroup nr
 *
 * Signal parameters for Nr
 */
struct NrSpectrumSignalParameters : public SpectrumSignalParameters
{

  // inherited from SpectrumSignalParameters
  virtual Ptr<SpectrumSignalParameters> Copy ();

  /**
   * default constructor
   */
  NrSpectrumSignalParameters ();

  /**
   * copy constructor
   */
  NrSpectrumSignalParameters (const NrSpectrumSignalParameters& p);

  /**
   * The packet burst being transmitted with this signal
   */
  Ptr<PacketBurst> packetBurst;
};


/**
* \ingroup nr
*
* Signal parameters for Nr Data Frame (PDSCH), and eventually after some 
* control messages through other control channel embedded in PDSCH
* (i.e. PBCH)
*/
struct NrSpectrumSignalParametersDataFrame : public SpectrumSignalParameters
{
  
  // inherited from SpectrumSignalParameters
  virtual Ptr<SpectrumSignalParameters> Copy ();
  
  /**
  * default constructor
  */
  NrSpectrumSignalParametersDataFrame ();
  
  /**
  * copy constructor
  */
  NrSpectrumSignalParametersDataFrame (const NrSpectrumSignalParametersDataFrame& p);
  
  /**
  * The packet burst being transmitted with this signal
  */
  Ptr<PacketBurst> packetBurst;
  
  std::list<Ptr<NrControlMessage> > ctrlMsgList;
  
  uint16_t cellId;
};


/**
* \ingroup nr
*
* Signal parameters for Nr DL Ctrl Frame (RS, PCFICH and PDCCH)
*/
struct NrSpectrumSignalParametersDlCtrlFrame : public SpectrumSignalParameters
{
  
  // inherited from SpectrumSignalParameters
  virtual Ptr<SpectrumSignalParameters> Copy ();
  
  /**
  * default constructor
  */
  NrSpectrumSignalParametersDlCtrlFrame ();
  
  /**
  * copy constructor
  */
  NrSpectrumSignalParametersDlCtrlFrame (const NrSpectrumSignalParametersDlCtrlFrame& p);


  std::list<Ptr<NrControlMessage> > ctrlMsgList;
  
  uint16_t cellId;
  bool pss; // primary synchronization signal
};



/**
* \ingroup nr
*
* Signal parameters for Nr SRS Frame
*/
struct NrSpectrumSignalParametersUlSrsFrame : public SpectrumSignalParameters
{
  
  // inherited from SpectrumSignalParameters
  virtual Ptr<SpectrumSignalParameters> Copy ();
  
  /**
  * default constructor
  */
  NrSpectrumSignalParametersUlSrsFrame ();
  
  /**
  * copy constructor
  */
  NrSpectrumSignalParametersUlSrsFrame (const NrSpectrumSignalParametersUlSrsFrame& p);
  
  uint16_t cellId;
};


}  // namespace ns3


#endif /* NR_SPECTRUM_SIGNAL_PARAMETERS_H */
