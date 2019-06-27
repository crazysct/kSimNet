/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2016, University of Padova, Dep. of Information Engineering, SIGNET lab
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
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Dual Connectivity functionalities
 */

#ifndef NR_RADIO_BEARER_INFO_H
#define NR_RADIO_BEARER_INFO_H

#include <ns3/object.h>
#include <ns3/pointer.h>
#include <ns3/eps-bearer.h>
#include <ns3/nr-rrc-sap.h>
#include <ns3/ipv4-address.h>
#include <ns3/ngc-x2-sap.h>

namespace ns3 {

class NrRlc;
class NrPdcp;

/**
 * store information on active radio bearer instance
 * 
 */
class NrRadioBearerInfo : public Object
{

public:
  NrRadioBearerInfo (void);
  virtual ~NrRadioBearerInfo (void);
  static TypeId GetTypeId (void);

  Ptr<NrRlc> m_rlc;
  Ptr<NrPdcp> m_pdcp;  
};


/**
 * store information on active signaling radio bearer instance
 * 
 */
class NrSignalingRadioBearerInfo : public NrRadioBearerInfo
{

public:
  static TypeId GetTypeId (void);

  uint8_t m_srbIdentity;   
  NrRrcSap::LogicalChannelConfig m_logicalChannelConfig;  
};


/**
 * store information on active data radio bearer instance
 * 
 */
class NrDataRadioBearerInfo : public NrRadioBearerInfo
{

public:
  static TypeId GetTypeId (void);

  EpsBearer m_epsBearer;
  uint8_t m_epsBearerIdentity;
  uint8_t m_drbIdentity;
  NrRrcSap::RlcConfig m_rlcConfig;
  uint8_t m_logicalChannelIdentity;
  NrRrcSap::LogicalChannelConfig m_logicalChannelConfig;
  uint32_t m_gtpTeid; /**< N2-bearer GTP tunnel endpoint identifier, see 36.423 9.2.1 */
  Ipv4Address m_transportLayerAddress; /**< IP Address of the SMF, see 36.423 9.2.1 */
  NgcX2Sap::RlcSetupRequest m_rlcSetupRequest; // complete bearer with related info, for MC functionalities
  bool m_isMc; // true if a bearer is split
  bool m_isMc_2;
};

class RlcBearerInfo : public Object
{
public:
  RlcBearerInfo (void);
  virtual ~RlcBearerInfo (void);
  static TypeId GetTypeId (void);
    
    uint16_t    sourceCellId;
    uint16_t    targetCellId;
    uint32_t    gtpTeid;
    uint16_t    mmWaveRnti;
    uint16_t 		mmWaveRnti_73;
    uint16_t    nrRnti;
    uint8_t     drbid;
    uint8_t     logicalChannelIdentity;
    NrRrcSap::RlcConfig rlcConfig;
    NrRrcSap::LogicalChannelConfig logicalChannelConfig;
    NrEnbCmacSapProvider::LcInfo lcinfo;
    Ptr<NrRlc> m_rlc;
    Ptr<NrRlc> m_rlc_2; //sjkang1110
};




} // namespace ns3


#endif // NR_RADIO_BEARER_INFO_H
