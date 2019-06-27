/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <ns3/nstime.h>
#include <ns3/node-list.h>
#include <ns3/node.h>
#include <ns3/simulator.h>

#include "nr-rrc-protocol-ideal.h"
#include "nr-ue-rrc.h"
#include "nr-enb-rrc.h"
#include "nr-enb-net-device.h"
#include "nr-ue-net-device.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrRrcProtocolIdeal");

static const Time RRC_IDEAL_MSG_DELAY = MicroSeconds (500);

NS_OBJECT_ENSURE_REGISTERED (NrUeRrcProtocolIdeal);

NrUeRrcProtocolIdeal::NrUeRrcProtocolIdeal ()
  :  m_ueRrcSapProvider (0),
     m_enbRrcSapProvider (0)
{
  m_ueRrcSapUser = new MemberNrUeRrcSapUser<NrUeRrcProtocolIdeal> (this);
}

NrUeRrcProtocolIdeal::~NrUeRrcProtocolIdeal ()
{
}

void
NrUeRrcProtocolIdeal::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_ueRrcSapUser;
  m_rrc = 0;
}

TypeId
NrUeRrcProtocolIdeal::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrUeRrcProtocolIdeal")
    .SetParent<Object> ()
    .SetGroupName("Nr")
    .AddConstructor<NrUeRrcProtocolIdeal> ()
    ;
  return tid;
}

void 
NrUeRrcProtocolIdeal::SetNrUeRrcSapProvider (NrUeRrcSapProvider* p)
{
  m_ueRrcSapProvider = p;
}

NrUeRrcSapUser* 
NrUeRrcProtocolIdeal::GetNrUeRrcSapUser ()
{
  return m_ueRrcSapUser;
}

void 
NrUeRrcProtocolIdeal::SetUeRrc (Ptr<NrUeRrc> rrc)
{
  m_rrc = rrc;
}

void 
NrUeRrcProtocolIdeal::DoSetup (NrUeRrcSapUser::SetupParameters params)
{
  NS_LOG_FUNCTION (this);
  // We don't care about SRB0/SRB1 since we use ideal RRC messages.
}

void 
NrUeRrcProtocolIdeal::DoSendRrcConnectionRequest (NrRrcSap::RrcConnectionRequest msg)
{
  // initialize the RNTI and get the EnbNrRrcSapProvider for the
  // eNB we are currently attached to
  m_rnti = m_rrc->GetRnti ();
  SetEnbRrcSapProvider (); 
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
                       &NrEnbRrcSapProvider::RecvRrcConnectionRequest,
                       m_enbRrcSapProvider,
                       m_rnti, 
                       msg);
}

void 
NrUeRrcProtocolIdeal::DoSendRrcConnectionSetupCompleted (NrRrcSap::RrcConnectionSetupCompleted msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &NrEnbRrcSapProvider::RecvRrcConnectionSetupCompleted,
                       m_enbRrcSapProvider,
		       m_rnti, 
		       msg);
}

void 
NrUeRrcProtocolIdeal::DoSendRrcConnectionReconfigurationCompleted (NrRrcSap::RrcConnectionReconfigurationCompleted msg)
{
  // re-initialize the RNTI and get the EnbNrRrcSapProvider for the
  // eNB we are currently attached to
  m_rnti = m_rrc->GetRnti ();
  SetEnbRrcSapProvider ();
    
   Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
                        &NrEnbRrcSapProvider::RecvRrcConnectionReconfigurationCompleted,
                        m_enbRrcSapProvider,
                        m_rnti, 
                        msg);
}

// jhlim
void
NrUeRrcProtocolIdeal::DoSendRrcIdentityResponse (NrRrcSap::RrcIdentityResponse msg)
{
  m_rnti = m_rrc->GetRnti ();
  //SetEnbRrcSapProvider ();
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY,
  						&NrEnbRrcSapProvider::RecvRrcIdentityResponse,
						m_enbRrcSapProvider,
						m_rnti,
						msg);
}
void
NrUeRrcProtocolIdeal::DoSendRrcRegistrationComplete (NrRrcSap::RrcRegistrationComplete msg)
{
  m_rnti = m_rrc->GetRnti ();
  //SetEnbRrcSapProvider ();
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY,
  						&NrEnbRrcSapProvider::RecvRrcRegistrationComplete,
						m_enbRrcSapProvider,
						m_rnti,
						msg);
}

void 
NrUeRrcProtocolIdeal::DoSendRrcConnectionReestablishmentRequest (NrRrcSap::RrcConnectionReestablishmentRequest msg)
{
   Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &NrEnbRrcSapProvider::RecvRrcConnectionReestablishmentRequest,
                       m_enbRrcSapProvider,
		       m_rnti, 
                        msg);
}

void 
NrUeRrcProtocolIdeal::DoSendRrcConnectionReestablishmentComplete (NrRrcSap::RrcConnectionReestablishmentComplete msg)
{
   Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &NrEnbRrcSapProvider::RecvRrcConnectionReestablishmentComplete,
                       m_enbRrcSapProvider,
		       m_rnti, 
msg);
}

void 
NrUeRrcProtocolIdeal::DoSendMeasurementReport (NrRrcSap::MeasurementReport msg)
{
   Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
                        &NrEnbRrcSapProvider::RecvMeasurementReport,
                        m_enbRrcSapProvider,
                        m_rnti, 
                        msg);
}

void 
NrUeRrcProtocolIdeal::DoSendNotifySecondaryCellConnected (uint16_t mmWaveRnti, uint16_t mmWaveCellId)
{
   Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
                        &NrEnbRrcSapProvider::RecvRrcSecondaryCellInitialAccessSuccessful,
                        m_enbRrcSapProvider,
                        m_rnti, 
                        mmWaveRnti,
                        mmWaveCellId);
}

void 
NrUeRrcProtocolIdeal::SetEnbRrcSapProvider ()
{
uint16_t cellId = m_rrc->GetCellId ();  

  // walk list of all nodes to get the peer eNB
  Ptr<NrEnbNetDevice> enbDev;
  NodeList::Iterator listEnd = NodeList::End ();
  bool found = false;
  for (NodeList::Iterator i = NodeList::Begin (); 
       (i != listEnd) && (!found); 
       ++i)
    {
      Ptr<Node> node = *i;
      int nDevs = node->GetNDevices ();
      for (int j = 0; 
           (j < nDevs) && (!found);
           j++)
        {
          enbDev = node->GetDevice (j)->GetObject <NrEnbNetDevice> ();
          if (enbDev == 0)
            {
              continue;
            }
          else
            {
              if (enbDev->GetCellId () == cellId)
                {
                  found = true;          
                  break;
                }
            }
        }
    }
  NS_ASSERT_MSG (found, " Unable to find eNB with CellId =" << cellId);
  m_enbRrcSapProvider = enbDev->GetRrc ()->GetNrEnbRrcSapProvider ();  
  Ptr<NrEnbRrcProtocolIdeal> enbRrcProtocolIdeal = enbDev->GetRrc ()->GetObject<NrEnbRrcProtocolIdeal> ();
  enbRrcProtocolIdeal->SetUeRrcSapProvider (m_rnti, m_ueRrcSapProvider);  
  
}


NS_OBJECT_ENSURE_REGISTERED (NrEnbRrcProtocolIdeal);

NrEnbRrcProtocolIdeal::NrEnbRrcProtocolIdeal ()
  :  m_enbRrcSapProvider (0)
{
  NS_LOG_FUNCTION (this);
  m_enbRrcSapUser = new MemberNrEnbRrcSapUser<NrEnbRrcProtocolIdeal> (this);
}

NrEnbRrcProtocolIdeal::~NrEnbRrcProtocolIdeal ()
{
  NS_LOG_FUNCTION (this);
}

void
NrEnbRrcProtocolIdeal::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_enbRrcSapUser;  
}

TypeId
NrEnbRrcProtocolIdeal::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrEnbRrcProtocolIdeal")
    .SetParent<Object> ()
    .SetGroupName("Nr")
    .AddConstructor<NrEnbRrcProtocolIdeal> ()
    ;
  return tid;
}

void 
NrEnbRrcProtocolIdeal::SetNrEnbRrcSapProvider (NrEnbRrcSapProvider* p)
{
  m_enbRrcSapProvider = p;
}

NrEnbRrcSapUser* 
NrEnbRrcProtocolIdeal::GetNrEnbRrcSapUser ()
{
  return m_enbRrcSapUser;
}

void 
NrEnbRrcProtocolIdeal::SetCellId (uint16_t cellId)
{
  m_cellId = cellId;
}

NrUeRrcSapProvider* 
NrEnbRrcProtocolIdeal::GetUeRrcSapProvider (uint16_t rnti)
{
  std::map<uint16_t, NrUeRrcSapProvider*>::const_iterator it;
  it = m_enbRrcSapProviderMap.find (rnti);
  NS_ASSERT_MSG (it != m_enbRrcSapProviderMap.end (), "could not find RNTI = " << rnti);
  return it->second;
}

void 
NrEnbRrcProtocolIdeal::SetUeRrcSapProvider (uint16_t rnti, NrUeRrcSapProvider* p)
{
  std::map<uint16_t, NrUeRrcSapProvider*>::iterator it;
  it = m_enbRrcSapProviderMap.find (rnti);
  NS_ASSERT_MSG (it != m_enbRrcSapProviderMap.end (), "could not find RNTI = " << rnti);
  it->second = p;
}

void 
NrEnbRrcProtocolIdeal::DoSetupUe (uint16_t rnti, NrEnbRrcSapUser::SetupUeParameters params)
{
  NS_LOG_FUNCTION (this << rnti);

  // // walk list of all nodes to get the peer UE RRC SAP Provider
  // Ptr<NrUeRrc> ueRrc;
  // NodeList::Iterator listEnd = NodeList::End ();
  // bool found = false;
  // for (NodeList::Iterator i = NodeList::Begin (); (i != listEnd) && (found == false); i++)
  //   {
  //     Ptr<Node> node = *i;
  //     int nDevs = node->GetNDevices ();
  //     for (int j = 0; j < nDevs; j++)
  //       {
  //         Ptr<NrUeNetDevice> ueDev = node->GetDevice (j)->GetObject <NrUeNetDevice> ();
  //         if (!ueDev)
  //           {
  //             continue;
  //           }
  //         else
  //           {
  //             ueRrc = ueDev->GetRrc ();
  //             if ((ueRrc->GetRnti () == rnti) && (ueRrc->GetCellId () == m_cellId))
  //               {                 
  //       	  found = true;
  //       	  break;
  //               }
  //           }
  //       }
  //   }
  // NS_ASSERT_MSG (found , " Unable to find UE with RNTI=" << rnti << " cellId=" << m_cellId);
  // m_enbRrcSapProviderMap[rnti] = ueRrc->GetNrUeRrcSapProvider ();


  // just create empty entry, the UeRrcSapProvider will be set by the
  // ue upon connection request or connection reconfiguration
  // completed 
  m_enbRrcSapProviderMap[rnti] = 0;

}

void 
NrEnbRrcProtocolIdeal::DoRemoveUe (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
  m_enbRrcSapProviderMap.erase (rnti);
}

void 
NrEnbRrcProtocolIdeal::DoSendSystemInformation (NrRrcSap::SystemInformation msg)
{
  NS_LOG_FUNCTION (this << m_cellId);
  // walk list of all nodes to get UEs with this cellId
  Ptr<NrUeRrc> ueRrc;
  for (NodeList::Iterator i = NodeList::Begin (); i != NodeList::End (); ++i)
    {
      Ptr<Node> node = *i;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; ++j)
        {
          Ptr<NrUeNetDevice> ueDev = node->GetDevice (j)->GetObject <NrUeNetDevice> ();
          if (ueDev != 0)
            {
              Ptr<NrUeRrc> ueRrc = ueDev->GetRrc ();              
              NS_LOG_LOGIC ("considering UE IMSI " << ueDev->GetImsi () << " that has cellId " << ueRrc->GetCellId ());
              if (ueRrc->GetCellId () == m_cellId)
                {       
                  NS_LOG_LOGIC ("sending SI to IMSI " << ueDev->GetImsi ());
                  ueRrc->GetNrUeRrcSapProvider ()->RecvSystemInformation (msg);
                  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
                                       &NrUeRrcSapProvider::RecvSystemInformation,
                                       ueRrc->GetNrUeRrcSapProvider (), 
                                       msg);          
                }             
            }
        }
    } 
}

void 
NrEnbRrcProtocolIdeal::DoSendRrcConnectionSetup (uint16_t rnti, NrRrcSap::RrcConnectionSetup msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &NrUeRrcSapProvider::RecvRrcConnectionSetup,
		       GetUeRrcSapProvider (rnti), 
		       msg);
}

void 
NrEnbRrcProtocolIdeal::DoSendRrcConnectionReconfiguration (uint16_t rnti, NrRrcSap::RrcConnectionReconfiguration msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &NrUeRrcSapProvider::RecvRrcConnectionReconfiguration,
		       GetUeRrcSapProvider (rnti), 
		       msg);
}

// jhlim
void
NrEnbRrcProtocolIdeal::DoSendRrcIdentityRequest (uint16_t rnti, NrRrcSap::RrcIdentityRequest msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &NrUeRrcSapProvider::RecvRrcIdentityRequest,
		       GetUeRrcSapProvider (rnti), 
		       msg);
}
void
NrEnbRrcProtocolIdeal::DoSendRrcRegistrationAccept (uint16_t rnti, NrRrcSap::RrcRegistrationAccept msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &NrUeRrcSapProvider::RecvRrcRegistrationAccept,
		       GetUeRrcSapProvider (rnti), 
		       msg);
}

void 
NrEnbRrcProtocolIdeal::DoSendRrcConnectionReestablishment (uint16_t rnti, NrRrcSap::RrcConnectionReestablishment msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &NrUeRrcSapProvider::RecvRrcConnectionReestablishment,
		       GetUeRrcSapProvider (rnti), 
		       msg);
}

void 
NrEnbRrcProtocolIdeal::DoSendRrcConnectionReestablishmentReject (uint16_t rnti, NrRrcSap::RrcConnectionReestablishmentReject msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &NrUeRrcSapProvider::RecvRrcConnectionReestablishmentReject,
		       GetUeRrcSapProvider (rnti), 
		       msg);
}

void 
NrEnbRrcProtocolIdeal::DoSendRrcConnectionRelease (uint16_t rnti, NrRrcSap::RrcConnectionRelease msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &NrUeRrcSapProvider::RecvRrcConnectionRelease,
		       GetUeRrcSapProvider (rnti), 
		       msg);
}

void 
NrEnbRrcProtocolIdeal::DoSendRrcConnectionReject (uint16_t rnti, NrRrcSap::RrcConnectionReject msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &NrUeRrcSapProvider::RecvRrcConnectionReject,
		       GetUeRrcSapProvider (rnti), 
		       msg);
}

void 
NrEnbRrcProtocolIdeal::DoSendRrcConnectionSwitch (uint16_t rnti, NrRrcSap::RrcConnectionSwitch msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
           &NrUeRrcSapProvider::RecvRrcConnectionSwitch,
           GetUeRrcSapProvider (rnti), 
           msg);
}

void 
NrEnbRrcProtocolIdeal::DoSendRrcConnectToMmWave (uint16_t rnti, uint16_t mmWaveCellId, uint16_t mmWaveCellId_2)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
           &NrUeRrcSapProvider::RecvRrcConnectToMmWave,
           GetUeRrcSapProvider (rnti), 
           mmWaveCellId,mmWaveCellId_2);
}

/*
 * The purpose of NrEnbRrcProtocolIdeal is to avoid encoding
 * messages. In order to do so, we need to have some form of encoding for
 * inter-node RRC messages like HandoverPreparationInfo and HandoverCommand. Doing so
 * directly is not practical (these messages includes a lot of
 * information elements, so encoding all of them would defeat the
 * purpose of NrEnbRrcProtocolIdeal. The workaround is to store the
 * actual message in a global map, so that then we can just encode the
 * key in a header and send that between eNBs over X2.
 * 
 */

static std::map<uint32_t, NrRrcSap::HandoverPreparationInfo> g_handoverPreparationInfoMsgMap;
static uint32_t g_handoverPreparationInfoMsgIdCounter = 0;

/*
 * This header encodes the map key discussed above. We keep this
 * private since it should not be used outside this file.
 * 
 */
class IdealHandoverPreparationInfoHeader : public Header
{
public:
  uint32_t GetMsgId ();
  void SetMsgId (uint32_t id);
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  uint32_t m_msgId;
};

uint32_t 
IdealHandoverPreparationInfoHeader::GetMsgId ()
{
  return m_msgId;
}  

void 
IdealHandoverPreparationInfoHeader::SetMsgId (uint32_t id)
{
  m_msgId = id;
}  


TypeId
IdealHandoverPreparationInfoHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::IdealHandoverPreparationInfoHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<IdealHandoverPreparationInfoHeader> ()
  ;
  return tid;
}

TypeId
IdealHandoverPreparationInfoHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void IdealHandoverPreparationInfoHeader::Print (std::ostream &os)  const
{
  os << " msgId=" << m_msgId;
}

uint32_t IdealHandoverPreparationInfoHeader::GetSerializedSize (void) const
{
  return 4;
}

void IdealHandoverPreparationInfoHeader::Serialize (Buffer::Iterator start) const
{  
  start.WriteU32 (m_msgId);
}

uint32_t IdealHandoverPreparationInfoHeader::Deserialize (Buffer::Iterator start)
{
  m_msgId = start.ReadU32 ();
  return GetSerializedSize ();
}



Ptr<Packet> 
NrEnbRrcProtocolIdeal::DoEncodeHandoverPreparationInformation (NrRrcSap::HandoverPreparationInfo msg)
{
  uint32_t msgId = ++g_handoverPreparationInfoMsgIdCounter;
  NS_ASSERT_MSG (g_handoverPreparationInfoMsgMap.find (msgId) == g_handoverPreparationInfoMsgMap.end (), "msgId " << msgId << " already in use");
  NS_LOG_INFO (" encoding msgId = " << msgId);
  g_handoverPreparationInfoMsgMap.insert (std::pair<uint32_t, NrRrcSap::HandoverPreparationInfo> (msgId, msg));
  IdealHandoverPreparationInfoHeader h;
  h.SetMsgId (msgId);
  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (h);
  return p;
}

NrRrcSap::HandoverPreparationInfo 
NrEnbRrcProtocolIdeal::DoDecodeHandoverPreparationInformation (Ptr<Packet> p)
{
  IdealHandoverPreparationInfoHeader h;
  p->RemoveHeader (h);
  uint32_t msgId = h.GetMsgId ();
  NS_LOG_INFO (" decoding msgId = " << msgId);
  std::map<uint32_t, NrRrcSap::HandoverPreparationInfo>::iterator it = g_handoverPreparationInfoMsgMap.find (msgId);
  NS_ASSERT_MSG (it != g_handoverPreparationInfoMsgMap.end (), "msgId " << msgId << " not found");
  NrRrcSap::HandoverPreparationInfo msg = it->second;
  g_handoverPreparationInfoMsgMap.erase (it);
  return msg;
}



static std::map<uint32_t, NrRrcSap::RrcConnectionReconfiguration> g_handoverCommandMsgMap;
static uint32_t g_handoverCommandMsgIdCounter = 0;

/*
 * This header encodes the map key discussed above. We keep this
 * private since it should not be used outside this file.
 * 
 */
class IdealHandoverCommandHeader : public Header
{
public:
  uint32_t GetMsgId ();
  void SetMsgId (uint32_t id);
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  uint32_t m_msgId;
};

uint32_t 
IdealHandoverCommandHeader::GetMsgId ()
{
  return m_msgId;
}  

void 
IdealHandoverCommandHeader::SetMsgId (uint32_t id)
{
  m_msgId = id;
}  


TypeId
IdealHandoverCommandHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::IdealHandoverCommandHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<IdealHandoverCommandHeader> ()
  ;
  return tid;
}

TypeId
IdealHandoverCommandHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void IdealHandoverCommandHeader::Print (std::ostream &os)  const
{
  os << " msgId=" << m_msgId;
}

uint32_t IdealHandoverCommandHeader::GetSerializedSize (void) const
{
  return 4;
}

void IdealHandoverCommandHeader::Serialize (Buffer::Iterator start) const
{  
  start.WriteU32 (m_msgId);
}

uint32_t IdealHandoverCommandHeader::Deserialize (Buffer::Iterator start)
{
  m_msgId = start.ReadU32 ();
  return GetSerializedSize ();
}



Ptr<Packet> 
NrEnbRrcProtocolIdeal::DoEncodeHandoverCommand (NrRrcSap::RrcConnectionReconfiguration msg)
{
  uint32_t msgId = ++g_handoverCommandMsgIdCounter;
  NS_ASSERT_MSG (g_handoverCommandMsgMap.find (msgId) == g_handoverCommandMsgMap.end (), "msgId " << msgId << " already in use");
  NS_LOG_INFO (" encoding msgId = " << msgId);
  g_handoverCommandMsgMap.insert (std::pair<uint32_t, NrRrcSap::RrcConnectionReconfiguration> (msgId, msg));
  IdealHandoverCommandHeader h;
  h.SetMsgId (msgId);
  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (h);
  return p;
}

NrRrcSap::RrcConnectionReconfiguration
NrEnbRrcProtocolIdeal::DoDecodeHandoverCommand (Ptr<Packet> p)
{
  IdealHandoverCommandHeader h;
  p->RemoveHeader (h);
  uint32_t msgId = h.GetMsgId ();
  NS_LOG_INFO (" decoding msgId = " << msgId);
  std::map<uint32_t, NrRrcSap::RrcConnectionReconfiguration>::iterator it = g_handoverCommandMsgMap.find (msgId);
  NS_ASSERT_MSG (it != g_handoverCommandMsgMap.end (), "msgId " << msgId << " not found");
  NrRrcSap::RrcConnectionReconfiguration msg = it->second;
  g_handoverCommandMsgMap.erase (it);
  return msg;
}





} // namespace ns3
