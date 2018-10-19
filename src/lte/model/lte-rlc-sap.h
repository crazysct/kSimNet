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
 * Author: Manuel Requena <manuel.requena@cttc.es>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Dual Connectivity functionalities
 */

#ifndef LTE_RLC_SAP_H
#define LTE_RLC_SAP_H

#include "ns3/packet.h"
#include "ns3/log.h"
#include <ns3/epc-x2-sap.h>
#include <ns3/lte-pdcp.h>
namespace ns3 {

/**
 * Service Access Point (SAP) offered by the UM-RLC and AM-RLC entities to the PDCP entity
 * See 3GPP 36.322 Radio Link Control (RLC) protocol specification
 *
 * This is the RLC SAP Provider
 * (i.e. the part of the SAP that contains the RLC methods called by the PDCP)
 */

/*class LteRlcAssistantUser{ //sjkang for sendig assistant info from lte Rlc
	friend class LtePdcp;
public:
	~LteRlcAssistantUser();
	LteRlcAssistantUser(Ptr<LtePdcp> m_pdcp);
	void ReceiveLteAssistantInfo(EpcX2Sap::AssistantInformationForSplitting info);
private:
	Ptr<LtePdcp>  m_pdcp;
};


LteRlcAssistantUser::LteRlcAssistantUser(Ptr<LtePdcp> pdcp){
	m_pdcp = pdcp;
}
void LteRlcAssistantUser::ReceiveLteAssistantInfo(EpcX2Sap::AssistantInformationForSplitting info){
//m_pdcp->DoRecieveLteAssistantInfo(info);
}


LteRlcAssistantUser::~LteRlcAssistantUser(){

}*/


class LteRlcSapProvider
{
public:
  virtual ~LteRlcSapProvider ();

  /**
   * Parameters for LteRlcSapProvider::TransmitPdcpPdu
   */
  struct TransmitPdcpPduParameters
  {
    Ptr<Packet> pdcpPdu;  /**< the PDCP PDU */
    uint16_t    rnti; /**< the C-RNTI identifying the UE */
    uint8_t     lcid; /**< the logical channel id corresponding to the sending RLC instance */
  };


  /**
   * Send a PDCP PDU to the RLC for transmission
   * This method is to be called
   * when upper PDCP entity has a PDCP PDU ready to send
   */
  virtual void TransmitPdcpPdu (TransmitPdcpPduParameters params) = 0;
  virtual void RequestAssistantInfo()=0; //sjkang
};


/**
 * Service Access Point (SAP) offered by the UM-RLC and AM-RLC entities to the PDCP entity
 * See 3GPP 36.322 Radio Link Control (RLC) protocol specification
 *
 * This is the RLC SAP User
 * (i.e. the part of the SAP that contains the PDCP methos called by the RLC)
 */
class LteRlcSapUser
{
public:
  virtual ~LteRlcSapUser ();

  /**
  * Called by the RLC entity to notify the PDCP entity of the reception of a new PDCP PDU
  *
  * \param p the PDCP PDU
  */
  virtual void SendLteAssi(EpcX2Sap::AssistantInformationForSplitting)=0;
  EpcX2Sap::AssistantInformationForSplitting info; //sjkang
  virtual void ReceivePdcpPdu (Ptr<Packet> p) = 0;
};
/*void
LteRlcSapUser::print(){

}*/
///////////////////////////////////////

template <class C>
class LteRlcSpecificLteRlcSapProvider : public LteRlcSapProvider
{
public:
  LteRlcSpecificLteRlcSapProvider (C* rlc);

  // Interface implemented from LteRlcSapProvider
  virtual void TransmitPdcpPdu (TransmitPdcpPduParameters params);
  virtual void RequestAssistantInfo(); //sjkang
private:
  LteRlcSpecificLteRlcSapProvider ();
  C* m_rlc;

};

template <class C>
LteRlcSpecificLteRlcSapProvider<C>::LteRlcSpecificLteRlcSapProvider (C* rlc)
  : m_rlc (rlc)
{
}

template <class C>
LteRlcSpecificLteRlcSapProvider<C>::LteRlcSpecificLteRlcSapProvider ()
{
}

template <class C>
void LteRlcSpecificLteRlcSapProvider<C>::TransmitPdcpPdu (TransmitPdcpPduParameters params)
{
  m_rlc->DoTransmitPdcpPdu (params.pdcpPdu);
}
template  <class C>
void LteRlcSpecificLteRlcSapProvider<C>::RequestAssistantInfo(){ //sjkang
m_rlc->DoRequestAssistantInfo();
}
///////////////////////////////////////

template <class C>
class LteRlcSpecificLteRlcSapUser : public LteRlcSapUser
{
public:
  LteRlcSpecificLteRlcSapUser (C* pdcp);

  // Interface implemented from LteRlcSapUser
  virtual void ReceivePdcpPdu (Ptr<Packet> p);
  virtual void SendLteAssi(EpcX2Sap::AssistantInformationForSplitting); //sjkang
private:
  LteRlcSpecificLteRlcSapUser ();
  C* m_pdcp;
};

template <class C>
LteRlcSpecificLteRlcSapUser<C>::LteRlcSpecificLteRlcSapUser (C* pdcp)
  : m_pdcp (pdcp)
{
}

template <class C>
LteRlcSpecificLteRlcSapUser<C>::LteRlcSpecificLteRlcSapUser ()
{
}

template <class C>
void LteRlcSpecificLteRlcSapUser<C>::ReceivePdcpPdu (Ptr<Packet> p)
{
  m_pdcp->DoReceivePdcpPdu (p);
}
template <class C>
void LteRlcSpecificLteRlcSapUser<C>::SendLteAssi (EpcX2Sap::AssistantInformationForSplitting info)
{
 m_pdcp->DoReceiveLteAssistantInfo (info);
}


} // namespace ns3

#endif // LTE_RLC_SAP_H
