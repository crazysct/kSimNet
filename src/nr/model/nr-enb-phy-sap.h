/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Marco Miozzo <mmiozzo@cttc.es>
 */



#ifndef NR_ENB_PHY_SAP_H
#define NR_ENB_PHY_SAP_H

#include <ns3/packet.h>
#include <ns3/nr-ff-mac-common.h>
#include <ns3/nr-ff-mac-sched-sap.h>

namespace ns3 {

class NrControlMessage;

/**
 * Service Access Point (SAP) offered by the eNB-PHY to the eNB-MAC
 *
 * This is the PHY SAP Provider, i.e., the part of the SAP that contains
 * the PHY methods called by the MAC
 */
class NrEnbPhySapProvider
{
public:
  virtual ~NrEnbPhySapProvider ();

  /**
   * \brief Send the MAC PDU to the channel
   * \param p the MAC PDU to send
   * \return true if
   */
  virtual void SendMacPdu (Ptr<Packet> p) = 0;

  /** 
   * \brief Send SendNrControlMessage (PDCCH map, CQI feedbacks) using the ideal control channel
   * \param msg the Ideal Control Message to send
   */
  virtual void SendNrControlMessage (Ptr<NrControlMessage> msg) = 0;
  
  /**
  * \brief Get the delay from MAC to Channel expressed in TTIs
  *
  */
  virtual uint8_t GetMacChTtiDelay () = 0;


};


/**
 * Service Access Point (SAP) offered by the eNB-PHY to the eNB-MAC
 *
 * This is the MAC SAP User, i.e., the part of the SAP that contains the MAC
 * methods called by the PHY
 */
class NrEnbPhySapUser
{
public:
  virtual ~NrEnbPhySapUser ();


  /**
   * Called by the Phy to notify the MAC of the reception of a new PHY-PDU
   *
   * \param p
   */
  virtual void ReceivePhyPdu (Ptr<Packet> p) = 0;

  /**
   * \brief Trigger the start from a new frame (input from Phy layer)
   * \param frameNo frame number
   * \param subframeNo subframe number
   */
  virtual void SubframeIndication (uint32_t frameNo, uint32_t subframeNo) = 0;

  /**
   * \brief Receive SendNrControlMessage (PDCCH map, CQI feedbacks) using the ideal control channel
   * \param msg the Ideal Control Message to receive
   */
  virtual void ReceiveNrControlMessage (Ptr<NrControlMessage> msg) = 0;

  /** 
   * notify the reception of a RACH preamble on the PRACH 
   * 
   * \param prachId the ID of the preamble
   */
  virtual void ReceiveRachPreamble (uint32_t prachId) = 0;

  /**
   * \brief Returns to MAC level the UL-CQI evaluated
   * \param ulcqi the UL-CQI (see FF MAC API 4.3.29)
   */
  virtual void UlCqiReport (NrFfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi) = 0;

  /**
   * Notify the HARQ on the UL tranmission status
   *
   * \param params
   */
  virtual void UlInfoListElementHarqFeeback (NrUlInfoListElement_s params) = 0;


  /**
   * Notify the HARQ on the DL tranmission status
   *
   * \param params
   */
  virtual void DlInfoListElementHarqFeeback (NrDlInfoListElement_s params) = 0;

};



} // namespace ns3


#endif // NR_ENB_PHY_SAP_H
