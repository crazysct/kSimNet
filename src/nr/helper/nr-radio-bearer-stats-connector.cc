/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 */


#include <ns3/log.h>


#include "nr-radio-bearer-stats-connector.h"
#include "nr-radio-bearer-stats-calculator.h"
#include <ns3/nr-enb-rrc.h>
#include <ns3/nr-enb-net-device.h>
#include <ns3/nr-ue-rrc.h>
#include <ns3/nr-ue-net-device.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrRadioBearerStatsConnector");

/**
  * Less than operator for CellIdRnti, because it is used as key in map
  */
bool
operator < (const NrRadioBearerStatsConnector::CellIdRnti& a, const NrRadioBearerStatsConnector::CellIdRnti& b)
{
  return ( (a.cellId < b.cellId) || ( (a.cellId == b.cellId) && (a.rnti < b.rnti) ) );
}

/**
 * This structure is used as interface between trace
 * sources and NrRadioBearerStatsCalculator. It stores
 * and provides calculators with cellId and IMSI,
 * because most trace sources do not provide it.
 */
struct BoundCallbackArgument : public SimpleRefCount<BoundCallbackArgument>
{
public:
  Ptr<NrRadioBearerStatsCalculator> stats;  //!< statistics calculator
  uint64_t imsi; //!< imsi
  uint16_t cellId; //!< cellId
};

struct BoundCallbackArgumentRetx : public SimpleRefCount<BoundCallbackArgumentRetx>
{
public:
  Ptr<NrRetxStatsCalculator> stats;  //!< statistics calculator
  uint64_t imsi; //!< imsi
  uint16_t cellId; //!< cellId
};

struct BoundCallbackArgumentMacTx : public SimpleRefCount<BoundCallbackArgumentMacTx>
{
public:
  Ptr<NrMacTxStatsCalculator> stats;  //!< statistics calculator
};

/**
 * Callback function for DL TX statistics for both RLC and PDCP
 * /param arg
 * /param path
 * /param rnti
 * /param lcid
 * /param packetSize
 */
void
DlTxPduCallback (Ptr<BoundCallbackArgument> arg, std::string path,
                 uint16_t rnti, uint8_t lcid, uint32_t packetSize)
{
  NS_LOG_FUNCTION (path << rnti << (uint16_t)lcid << packetSize);
  arg->stats->DlTxPdu (arg->cellId, arg->imsi, rnti, lcid, packetSize);
}

/**
 * Callback function for DL RX statistics for both RLC and PDCP
 * /param arg
 * /param path
 * /param rnti
 * /param lcid
 * /param packetSize
 * /param delay
 */
void
DlRxPduCallback (Ptr<BoundCallbackArgument> arg, std::string path,
                 uint16_t rnti, uint8_t lcid, uint32_t packetSize, uint64_t delay)
{
  NS_LOG_FUNCTION (path << rnti << (uint16_t)lcid << packetSize << delay);
  arg->stats->DlRxPdu (arg->cellId, arg->imsi, rnti, lcid, packetSize, delay);
}

/**
 * Callback function for UL TX statistics for both RLC and PDCP
 * /param arg
 * /param path
 * /param rnti
 * /param lcid
 * /param packetSize
 */
void
UlTxPduCallback (Ptr<BoundCallbackArgument> arg, std::string path,
                 uint16_t rnti, uint8_t lcid, uint32_t packetSize)
{
  NS_LOG_FUNCTION (path << rnti << (uint16_t)lcid << packetSize);
 
  arg->stats->UlTxPdu (arg->cellId, arg->imsi, rnti, lcid, packetSize);
}

/**
 * Callback function for UL RX statistics for both RLC and PDCP
 * /param arg
 * /param path
 * /param rnti
 * /param lcid
 * /param packetSize
 * /param delay
 */
void
UlRxPduCallback (Ptr<BoundCallbackArgument> arg, std::string path,
                 uint16_t rnti, uint8_t lcid, uint32_t packetSize, uint64_t delay)
{
  NS_LOG_FUNCTION (path << rnti << (uint16_t)lcid << packetSize << delay);
 
  arg->stats->UlRxPdu (arg->cellId, arg->imsi, rnti, lcid, packetSize, delay);
}

void
DlRetxCallback (Ptr<BoundCallbackArgumentRetx> arg, std::string path,
                 uint16_t rnti, uint8_t lcid, uint32_t packetSize, uint32_t numRetx)
{
  NS_LOG_FUNCTION(path << arg->stats << arg->imsi);
  arg->stats->RegisterRetxDl(arg->imsi, arg->cellId, rnti, lcid, packetSize, numRetx);
}

void
UlRetxCallback (Ptr<BoundCallbackArgumentRetx> arg, std::string path,
                 uint16_t rnti, uint8_t lcid, uint32_t packetSize, uint32_t numRetx)
{
  NS_LOG_FUNCTION(path << arg->stats << arg->imsi);
  arg->stats->RegisterRetxUl(arg->imsi, arg->cellId, rnti, lcid, packetSize, numRetx);
}

void
NotifyDlMacTx (Ptr<BoundCallbackArgumentMacTx> arg, std::string path, uint16_t rnti, uint16_t cellId, uint32_t packetSize, uint8_t numRetx)
{
  NS_LOG_FUNCTION(path << rnti << cellId << packetSize << (uint32_t)numRetx);
  arg->stats->RegisterMacTxDl(rnti, cellId, packetSize, numRetx);
}


NrRadioBearerStatsConnector::NrRadioBearerStatsConnector ()
  : m_connected (false)
{
  m_retxStats = CreateObject<NrRetxStatsCalculator> ();
  m_macTxStats = CreateObject<NrMacTxStatsCalculator> ();
}

void 
NrRadioBearerStatsConnector::EnableRlcStats (Ptr<NrRadioBearerStatsCalculator> rlcStats)
{
  m_rlcStats = rlcStats;
  EnsureConnected ();
}

void 
NrRadioBearerStatsConnector::EnablePdcpStats (Ptr<NrRadioBearerStatsCalculator> pdcpStats)
{
  m_pdcpStats = pdcpStats;
  EnsureConnected ();
}

// TypeId
// NrRadioBearerStatsConnector::GetTypeId (void)
// {
//   static TypeId tid =
//     TypeId ("ns3::NrRadioBearerStatsConnector") 
//     .SetParent<Object> ()
//     .AddConstructor<NrRadioBearerStatsConnector> ()
//     .SetGroupName("Nr");
//   return tid; 
// }

void 
NrRadioBearerStatsConnector::EnsureConnected ()
{
  NS_LOG_FUNCTION (this);
  if (!m_connected)
    {
      Config::Connect ("/NodeList/*/DeviceList/*/NrEnbRrc/NewUeContext",
		       MakeBoundCallback (&NrRadioBearerStatsConnector::NotifyNewUeContextEnb, this));
      Config::Connect ("/NodeList/*/DeviceList/*/NrUeRrc/RandomAccessSuccessful",
		       MakeBoundCallback (&NrRadioBearerStatsConnector::NotifyRandomAccessSuccessfulUe, this));
      Config::Connect ("/NodeList/*/DeviceList/*/NrEnbRrc/ConnectionReconfiguration",
		       MakeBoundCallback (&NrRadioBearerStatsConnector::NotifyConnectionReconfigurationEnb, this));
      Config::Connect ("/NodeList/*/DeviceList/*/NrUeRrc/ConnectionReconfiguration",
		       MakeBoundCallback (&NrRadioBearerStatsConnector::NotifyConnectionReconfigurationUe, this));
      Config::Connect ("/NodeList/*/DeviceList/*/NrEnbRrc/HandoverStart",
		       MakeBoundCallback (&NrRadioBearerStatsConnector::NotifyHandoverStartEnb, this));
      Config::Connect ("/NodeList/*/DeviceList/*/NrUeRrc/HandoverStart",
		       MakeBoundCallback (&NrRadioBearerStatsConnector::NotifyHandoverStartUe, this));
      Config::Connect ("/NodeList/*/DeviceList/*/NrEnbRrc/HandoverEndOk",
		       MakeBoundCallback (&NrRadioBearerStatsConnector::NotifyHandoverEndOkEnb, this));
      Config::Connect ("/NodeList/*/DeviceList/*/NrUeRrc/HandoverEndOk",
		       MakeBoundCallback (&NrRadioBearerStatsConnector::NotifyHandoverEndOkUe, this));
      // MAC related callback
      if(m_macTxStats)
      {
        Ptr<BoundCallbackArgumentMacTx> arg = Create<BoundCallbackArgumentMacTx>();
        arg->stats = m_macTxStats;
        Config::Connect ("/NodeList/*/DeviceList/*/MmWaveEnbMac/DlMacTxCallback",
           MakeBoundCallback (&NotifyDlMacTx, arg));
      }
      m_connected = true;
    }
}

void 
NrRadioBearerStatsConnector::NotifyRandomAccessSuccessfulUe (NrRadioBearerStatsConnector* c, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  c->ConnectSrb0Traces (context, imsi, cellId, rnti);
}

void 
NrRadioBearerStatsConnector::NotifyConnectionSetupUe (NrRadioBearerStatsConnector* c, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  c->ConnectSrb1TracesUe (context, imsi, cellId, rnti);
}

void 
NrRadioBearerStatsConnector::NotifyConnectionReconfigurationUe (NrRadioBearerStatsConnector* c, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  c->ConnectTracesUeIfFirstTime (context, imsi, cellId, rnti);
}

void 
NrRadioBearerStatsConnector::NotifyHandoverStartUe (NrRadioBearerStatsConnector* c, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti, uint16_t targetCellId)
{
  c->DisconnectTracesUe (context, imsi, cellId, rnti);
}

void 
NrRadioBearerStatsConnector::NotifyHandoverEndOkUe (NrRadioBearerStatsConnector* c, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  c->ConnectTracesUe (context, imsi, cellId, rnti);
}

void 
NrRadioBearerStatsConnector::NotifyNewUeContextEnb (NrRadioBearerStatsConnector* c, std::string context, uint16_t cellId, uint16_t rnti)
{
  c->StoreUeManagerPath (context, cellId, rnti);
}

void 
NrRadioBearerStatsConnector::NotifyConnectionReconfigurationEnb (NrRadioBearerStatsConnector* c, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  c->ConnectTracesEnbIfFirstTime (context, imsi, cellId, rnti);
}

void 
NrRadioBearerStatsConnector::NotifyHandoverStartEnb (NrRadioBearerStatsConnector* c, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti, uint16_t targetCellId)
{
  c->DisconnectTracesEnb (context, imsi, cellId, rnti);
}

void 
NrRadioBearerStatsConnector::NotifyHandoverEndOkEnb (NrRadioBearerStatsConnector* c, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  c->ConnectTracesEnb (context, imsi, cellId, rnti);
}

void 
NrRadioBearerStatsConnector::StoreUeManagerPath (std::string context, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this << context << cellId << rnti);
  std::ostringstream ueManagerPath;
  ueManagerPath <<  context.substr (0, context.rfind ("/")) << "/UeMap/" << (uint32_t) rnti;
  CellIdRnti key;
  key.cellId = cellId;
  key.rnti = rnti;
  m_ueManagerPathByCellIdRnti[key] = ueManagerPath.str ();
}

void 
NrRadioBearerStatsConnector::ConnectSrb0Traces (std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this << imsi << cellId << rnti);
  std::string ueRrcPath =  context.substr (0, context.rfind ("/"));
  CellIdRnti key;
  key.cellId = cellId;
  key.rnti = rnti;
  std::map<CellIdRnti, std::string>::iterator it = m_ueManagerPathByCellIdRnti.find (key);
  NS_ASSERT (it != m_ueManagerPathByCellIdRnti.end ());
  std::string ueManagerPath = it->second;  
  NS_LOG_LOGIC (this << " ueManagerPath: " << ueManagerPath);
  m_ueManagerPathByCellIdRnti.erase (it);

  if (m_rlcStats)
    {
      Ptr<BoundCallbackArgument> arg = Create<BoundCallbackArgument> ();
      arg->imsi = imsi;
      arg->cellId = cellId; 
      arg->stats = m_rlcStats;

      // diconnect eventually previously connected SRB0 both at UE and eNB
      Config::Disconnect (ueRrcPath + "/Srb0/NrRlc/TxPDU",
                          MakeBoundCallback (&UlTxPduCallback, arg));
      Config::Disconnect (ueRrcPath + "/Srb0/NrRlc/RxPDU",
                          MakeBoundCallback (&DlRxPduCallback, arg));
      Config::Disconnect (ueManagerPath + "/Srb0/NrRlc/TxPDU",
                          MakeBoundCallback (&DlTxPduCallback, arg));
      Config::Disconnect (ueManagerPath + "/Srb0/NrRlc/RxPDU",
                          MakeBoundCallback (&UlRxPduCallback, arg));

      // connect SRB0 both at UE and eNB
      Config::Connect (ueRrcPath + "/Srb0/NrRlc/TxPDU",
                       MakeBoundCallback (&UlTxPduCallback, arg));
      Config::Connect (ueRrcPath + "/Srb0/NrRlc/RxPDU",
                       MakeBoundCallback (&DlRxPduCallback, arg));
      Config::Connect (ueManagerPath + "/Srb0/NrRlc/TxPDU",
                       MakeBoundCallback (&DlTxPduCallback, arg));
      Config::Connect (ueManagerPath + "/Srb0/NrRlc/RxPDU",
                       MakeBoundCallback (&UlRxPduCallback, arg));

      // connect SRB1 at eNB only (at UE SRB1 will be setup later)
      Config::Connect (ueManagerPath + "/Srb1/NrRlc/TxPDU",
                       MakeBoundCallback (&DlTxPduCallback, arg));
      Config::Connect (ueManagerPath + "/Srb1/NrRlc/RxPDU",
                       MakeBoundCallback (&UlRxPduCallback, arg));
    }
  if (m_pdcpStats)
    {
      Ptr<BoundCallbackArgument> arg = Create<BoundCallbackArgument> ();
      arg->imsi = imsi;
      arg->cellId = cellId; 
      arg->stats = m_pdcpStats;

      // connect SRB1 at eNB only (at UE SRB1 will be setup later)
      Config::Connect (ueManagerPath + "/Srb1/NrPdcp/RxPDU",
		       MakeBoundCallback (&UlRxPduCallback, arg));
      Config::Connect (ueManagerPath + "/Srb1/NrPdcp/TxPDU",
		       MakeBoundCallback (&DlTxPduCallback, arg));
    }
}

void 
NrRadioBearerStatsConnector::ConnectSrb1TracesUe (std::string ueRrcPath, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this << imsi << cellId << rnti);
   if (m_rlcStats)
    {
      Ptr<BoundCallbackArgument> arg = Create<BoundCallbackArgument> ();
      arg->imsi = imsi;
      arg->cellId = cellId; 
      arg->stats = m_rlcStats;
      Config::Connect (ueRrcPath + "/Srb1/NrRlc/TxPDU",
                       MakeBoundCallback (&UlTxPduCallback, arg));
      Config::Connect (ueRrcPath + "/Srb1/NrRlc/RxPDU",
                       MakeBoundCallback (&DlRxPduCallback, arg));
    }
  if (m_pdcpStats)
    {
      Ptr<BoundCallbackArgument> arg = Create<BoundCallbackArgument> ();
      arg->imsi = imsi;
      arg->cellId = cellId; 
      arg->stats = m_pdcpStats;
      Config::Connect (ueRrcPath + "/Srb1/NrPdcp/RxPDU",
		       MakeBoundCallback (&DlRxPduCallback, arg));
      Config::Connect (ueRrcPath + "/Srb1/NrPdcp/TxPDU",
		       MakeBoundCallback (&UlTxPduCallback, arg));
    }
}
  
void 
NrRadioBearerStatsConnector::ConnectTracesUeIfFirstTime (std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this << context);
  if (m_imsiSeenUe.find (imsi) == m_imsiSeenUe.end ())
    {
      m_imsiSeenUe.insert (imsi);
      ConnectTracesUe (context, imsi, cellId, rnti);
    }
}
 
void 
NrRadioBearerStatsConnector::ConnectTracesEnbIfFirstTime (std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this << context);
   if (m_imsiSeenEnb.find (imsi) == m_imsiSeenEnb.end ())
    {
      m_imsiSeenEnb.insert (imsi);
      ConnectTracesEnb (context, imsi, cellId, rnti);
    }
}

void 
NrRadioBearerStatsConnector::ConnectTracesUe (std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this << context);
  NS_LOG_LOGIC (this << "expected context should match /NodeList/*/DeviceList/*/NrUeRrc/");
  std::string basePath = context.substr (0, context.rfind ("/"));
  if (m_rlcStats)
    {
      Ptr<BoundCallbackArgument> arg = Create<BoundCallbackArgument> ();
      arg->imsi = imsi;
      arg->cellId = cellId; 
      arg->stats = m_rlcStats;
      Config::Connect (basePath + "/DataRadioBearerMap/*/NrRlc/TxPDU",
		       MakeBoundCallback (&UlTxPduCallback, arg));
      Config::Connect (basePath + "/DataRadioBearerMap/*/NrRlc/RxPDU",
		       MakeBoundCallback (&DlRxPduCallback, arg));
      Config::Connect (basePath + "/Srb1/NrRlc/TxPDU",
		       MakeBoundCallback (&UlTxPduCallback, arg));
      Config::Connect (basePath + "/Srb1/NrRlc/RxPDU",
		       MakeBoundCallback (&DlRxPduCallback, arg));

    }
  if (m_pdcpStats)
    {
      Ptr<BoundCallbackArgument> arg = Create<BoundCallbackArgument> ();
      arg->imsi = imsi;
      arg->cellId = cellId; 
      arg->stats = m_pdcpStats;
      Config::Connect (basePath + "/DataRadioBearerMap/*/NrPdcp/RxPDU",
		       MakeBoundCallback (&DlRxPduCallback, arg));
      Config::Connect (basePath + "/DataRadioBearerMap/*/NrPdcp/TxPDU",
		       MakeBoundCallback (&UlTxPduCallback, arg));
      Config::Connect (basePath + "/Srb1/NrPdcp/RxPDU",
		       MakeBoundCallback (&DlRxPduCallback, arg));
      Config::Connect (basePath + "/Srb1/NrPdcp/TxPDU",
		       MakeBoundCallback (&UlTxPduCallback, arg));
    }
  if (m_retxStats) // TODO set condition
    {
      Ptr<BoundCallbackArgumentRetx> arg = Create<BoundCallbackArgumentRetx> ();
      arg->imsi = imsi;
      arg->cellId = cellId; 
      arg->stats = m_retxStats;
      Config::Connect (basePath + "/DataRadioBearerMap/*/NrRlc/TxCompletedCallback",
         MakeBoundCallback (&UlRetxCallback, arg));
    }
}

void 
NrRadioBearerStatsConnector::ConnectTracesEnb (std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this << context);
  NS_LOG_LOGIC (this << "expected context  should match /NodeList/*/DeviceList/*/NrEnbRrc/");
  std::ostringstream basePath;
  basePath <<  context.substr (0, context.rfind ("/")) << "/UeMap/" << (uint32_t) rnti;
  if (m_rlcStats)
    {
      Ptr<BoundCallbackArgument> arg = Create<BoundCallbackArgument> ();
      arg->imsi = imsi;
      arg->cellId = cellId; 
      arg->stats = m_rlcStats;
      Config::Connect (basePath.str () + "/DataRadioBearerMap/*/NrRlc/RxPDU",
		       MakeBoundCallback (&UlRxPduCallback, arg));
      Config::Connect (basePath.str () + "/DataRadioBearerMap/*/NrRlc/TxPDU",
		       MakeBoundCallback (&DlTxPduCallback, arg));
      Config::Connect (basePath.str () + "/Srb0/NrRlc/RxPDU",
		       MakeBoundCallback (&UlRxPduCallback, arg));
      Config::Connect (basePath.str () + "/Srb0/NrRlc/TxPDU",
		       MakeBoundCallback (&DlTxPduCallback, arg));
      Config::Connect (basePath.str () + "/Srb1/NrRlc/RxPDU",
		       MakeBoundCallback (&UlRxPduCallback, arg));
      Config::Connect (basePath.str () + "/Srb1/NrRlc/TxPDU",
		       MakeBoundCallback (&DlTxPduCallback, arg));
    }
  if (m_pdcpStats)
    {
      Ptr<BoundCallbackArgument> arg = Create<BoundCallbackArgument> ();
      arg->imsi = imsi;
      arg->cellId = cellId; 
      arg->stats = m_pdcpStats;
      Config::Connect (basePath.str () + "/DataRadioBearerMap/*/NrPdcp/TxPDU",
		       MakeBoundCallback (&DlTxPduCallback, arg));
      Config::Connect (basePath.str () + "/DataRadioBearerMap/*/NrPdcp/RxPDU",
		       MakeBoundCallback (&UlRxPduCallback, arg));
      Config::Connect (basePath.str () + "/Srb1/NrPdcp/TxPDU",
		       MakeBoundCallback (&DlTxPduCallback, arg));
      Config::Connect (basePath.str () + "/Srb1/NrPdcp/RxPDU",
		       MakeBoundCallback (&UlRxPduCallback, arg));
    }
  if (m_retxStats) 
    {
      Ptr<BoundCallbackArgumentRetx> arg = Create<BoundCallbackArgumentRetx> ();
      arg->imsi = imsi;
      arg->cellId = cellId; 
      arg->stats = m_retxStats;
      Config::Connect (basePath.str () + "/DataRadioBearerMap/*/NrRlc/TxCompletedCallback",
         MakeBoundCallback (&DlRetxCallback, arg));
    }
}

void 
NrRadioBearerStatsConnector::DisconnectTracesUe (std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
}


void 
NrRadioBearerStatsConnector::DisconnectTracesEnb (std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
}



} // namespace ns3
