/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2016, University of Padova, Dep. of Information Engineering, SIGNET lab. 
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
 * Extension to DC devices by Michele Polese <michele.polese@gmail.com>
 */

#include "ns3/log.h"
#include "ns3/simulator.h"

#include "ns3/nr-mc-enb-pdcp.h"
#include "ns3/nr-pdcp-header.h"
#include "ns3/nr-pdcp-sap.h"
#include "ns3/nr-pdcp-tag.h"
#include "ns3/ngc-x2-sap.h"
#include <ctime>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrMcEnbPdcp");

class McPdcpSpecificNrRlcSapUser : public NrRlcSapUser
{
public:
  McPdcpSpecificNrRlcSapUser (NrMcEnbPdcp* pdcp);

  // Interface provided to lower RLC entity (implemented from NrRlcSapUser)
  virtual void ReceivePdcpPdu (Ptr<Packet> p);
  virtual void SendNrAssi(NgcX2Sap::AssistantInformationForSplitting);
private:
  McPdcpSpecificNrRlcSapUser ();
  NrMcEnbPdcp* m_pdcp;
};

McPdcpSpecificNrRlcSapUser::McPdcpSpecificNrRlcSapUser (NrMcEnbPdcp* pdcp)
  : m_pdcp (pdcp)
{
}

McPdcpSpecificNrRlcSapUser::McPdcpSpecificNrRlcSapUser ()
{
}

void
McPdcpSpecificNrRlcSapUser::ReceivePdcpPdu (Ptr<Packet> p)
{
  m_pdcp->DoReceivePdu (p);
}
void
McPdcpSpecificNrRlcSapUser::SendNrAssi(NgcX2Sap::AssistantInformationForSplitting info){
	//std::cout << "receive nr info"<<std::endl;
	m_pdcp->DoReceiveNrAssistantInfo(info);

}

/////////////////////////////
class AssistantInfoNrRlcSapUser : public NrRlcSapUser
{
public:
  AssistantInfoNrRlcSapUser (NrMcEnbPdcp* pdcp);

  // Interface provided to lower RLC entity (implemented from NrRlcSapUser)
  virtual void ReceivePdcpPdu (Ptr<Packet> p);
  virtual void ReceiveNrAssistantInfo(NgcX2Sap::AssistantInformationForSplitting info);
  virtual void  SendNrAssi(NgcX2Sap::AssistantInformationForSplitting info);
private:
  AssistantInfoNrRlcSapUser ();
  NrMcEnbPdcp* m_pdcp;
};
void
AssistantInfoNrRlcSapUser::ReceiveNrAssistantInfo(NgcX2Sap::AssistantInformationForSplitting info){
//	m_pdcp->DoReceiveNrAssistantInfo(info);
}
AssistantInfoNrRlcSapUser::AssistantInfoNrRlcSapUser (NrMcEnbPdcp* pdcp)
  : m_pdcp (pdcp)
{
}

AssistantInfoNrRlcSapUser::AssistantInfoNrRlcSapUser ()
{
}

void
AssistantInfoNrRlcSapUser::ReceivePdcpPdu (Ptr<Packet> p)
{
 // m_pdcp->DoReceiveNrAssistantInfo() (p);
}
void
AssistantInfoNrRlcSapUser:: SendNrAssi(NgcX2Sap::AssistantInformationForSplitting info){
std::cout << info.Tx_On_Q_Size<< std::endl;
}
///////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NrMcEnbPdcp);

NrMcEnbPdcp::NrMcEnbPdcp ()
  : m_pdcpSapUser (0),
    m_rlcSapProvider (0),
    m_rnti (0),
    m_lcid (0),
    m_ngcX2PdcpProvider (0),
    m_txSequenceNumber (0),
    m_rxSequenceNumber (0),
    m_useMmWaveConnection (false)
{
  NS_LOG_FUNCTION (this);
  m_pdcpSapProvider = new NrPdcpSpecificNrPdcpSapProvider<NrMcEnbPdcp> (this);
  m_rlcSapUser = new McPdcpSpecificNrRlcSapUser (this);
  m_assistant_rlcSapUser = new McPdcpSpecificNrRlcSapUser(this); //sjkang
  m_ngcX2PdcpUser = new NgcX2PdcpSpecificUser<NrMcEnbPdcp> (this);
  targetCellId_1 =0; //sjkang
  targetCellId_2=0; //sjkang
  isAlternative = true;
  isTargetCellId_1 = false;
  isTargetCellId_2 = false;
  eta = 0.5;
  t_1 = 0; t_2= 0;
  print_Eta.open("Pvalue.txt");
  m_isNrMmWaveDC = false;
  RequestAssistantInfoNR = false;
  m_isEnableDuplicate = false;
  //RequestAssistantInfoMmWave = false;
}
void
NrMcEnbPdcp::SetTargetCellIds(uint16_t targetID2, uint16_t targetID1, uint16_t nrCellId){ //sjkang
	this->nrCellId = nrCellId;
/*
 if(targetCellId_1 != targetID1 && targetCellId_1 != targetCellId_2){
	 std::cout << targetID1<<"\t"<<targetID2<<std::endl;
		std::cout << "will forward backup buffer to mmEnb " << targetID1 << std::endl;
				for(uint16_t count =0 ; count < bufferOfTargetEnb1.size() ; count ++){
				NrPdcpHeader pdcpHeader;
					PdcpTag pdcpTag;
						 bufferOfTargetEnb1.at(count)->RemoveAllByteTags();
									PdcpTag newpdcpTag (Simulator::Now ());
									bufferOfTargetEnb1.at(count)->AddByteTag (newpdcpTag);
					bufferOfTargetEnb1.at(count)->PeekHeader(pdcpHeader);
					pdcpHeader.SetSourceCellId(targetID1);
					NgcX2Sap::UeDataParams m_ueDataParams;
					m_ueDataParams.ueData =bufferOfTargetEnb1.at(count);
					m_ueDataParams.targetCellId = targetID1;
					m_ueDataParams.sourceCellId = this->m_ueDataParams.sourceCellId;
					m_ueDataParams.gtpTeid = this -> m_ueDataParams.gtpTeid;
					std::cout << " will foward bufferd data to mmEnb " << targetID1 << std::endl;
					m_ngcX2PdcpProvider->SendMcPdcpPdu (m_ueDataParams);
				}
				isTargetCellId_1 = true;
				isTargetCellId_2 = false;
				bufferOfTargetEnb1.clear();
	}
 if( bufferOfTargetEnb1.size() > 100){
		bufferOfTargetEnb1.clear();
	    std::cout << " will clear backup buffer in mmEnb " << targetID1 << std::endl;
	}

	 if ( targetCellId_2 != targetID2 && targetCellId_1 != targetCellId_2){
		// std::cout << targetID1<<"\t"<<targetID2<<std::endl;
		// std::cout << targetCellId_1<<"\t"<<targetCellId_2 <<std::endl;
		std::cout << "will forward backup buffer to mmEnb--- " << targetID2 << std::endl;
			for(uint16_t count =0 ; count < bufferOfTargetEnb2.size() ; count ++){
						NrPdcpHeader pdcpHeader;
								 bufferOfTargetEnb2.at(count)->RemoveAllByteTags();
									PdcpTag newpdcpTag (Simulator::Now ());
									bufferOfTargetEnb2.at(count)->AddByteTag (newpdcpTag);
								bufferOfTargetEnb2.at(count)->PeekHeader(pdcpHeader);
								pdcpHeader.SetSourceCellId(targetID2);
								NgcX2Sap::UeDataParams m_ueDataParams;
								m_ueDataParams.ueData =bufferOfTargetEnb2.at(count);
								m_ueDataParams.targetCellId = targetID2;
								m_ueDataParams.sourceCellId = this->m_ueDataParams.sourceCellId;
								m_ueDataParams.gtpTeid = this -> m_ueDataParams.gtpTeid;
									m_ngcX2PdcpProvider->SendMcPdcpPdu (m_ueDataParams);
								}
				isTargetCellId_1 = false;
				isTargetCellId_2 = true;
				bufferOfTargetEnb2.clear();
		}
	 if( bufferOfTargetEnb2.size() > 100){
			bufferOfTargetEnb2.clear();
			std::cout << " will clear backup buffer in mmEnb " << targetID2 << std::endl;
			}
*/

	targetCellId_1  =targetID1;
	targetCellId_2 = targetID2;
	//std::cout <<"sjkang0----" << targetCellId_1 << "\t" <<targetCellId_2 <<std::endl;
}
void
NrMcEnbPdcp::SetPacketDuplicateMode( bool isDuplicate){
	m_isEnableDuplicate = isDuplicate;
}
uint16_t
NrMcEnbPdcp::GetTargetCellId_1(){ //sjkang
	return this->targetCellId_1;
}
uint16_t
NrMcEnbPdcp::GetTargetCellId_2(){ //sjkang
	return this->targetCellId_2;
}
NrMcEnbPdcp::~NrMcEnbPdcp ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
NrMcEnbPdcp::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrMcEnbPdcp")
    .SetParent<Object> ()
	.AddAttribute("numberOfAlgorithm",
			"this variable indicates kind of splitting algorithms (p-split, SDF, SQF and so on )",
			UintegerValue(2),
			MakeUintegerAccessor(&NrMcEnbPdcp::m_isSplitting),
			MakeUintegerChecker<uint16_t> ())
	.AddAttribute("enableNrMmWaveDC", "this value means if Nr - mmWave DC is enable or not", BooleanValue(false),
			MakeBooleanAccessor(&NrMcEnbPdcp::m_isNrMmWaveDC),
			MakeBooleanChecker())
    .AddTraceSource ("TxPDU",
                     "PDU transmission notified to the RLC.",
                     MakeTraceSourceAccessor (&NrMcEnbPdcp::m_txPdu),
                     "ns3::NrMcEnbPdcp::PduTxTracedCallback")
    .AddTraceSource ("RxPDU",
                     "PDU received.",
                     MakeTraceSourceAccessor (&NrMcEnbPdcp::m_rxPdu),
                     "ns3::NrMcEnbPdcp::PduRxTracedCallback")
    ;
  return tid;
}

void
NrMcEnbPdcp::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete (m_pdcpSapProvider);
  delete (m_rlcSapUser);
  delete (m_ngcX2PdcpUser);
}

void
NrMcEnbPdcp::SetNgcX2PdcpProvider (NgcX2PdcpProvider * s)
{
  NS_LOG_FUNCTION(this);
  m_ngcX2PdcpProvider = s;
}
void
NrMcEnbPdcp::SetNgcX2PdcpUser(NgcX2PdcpUser *s ){ //sjkang
	NS_LOG_FUNCTION(this);
	m_ngcX2PdcpUser =s; //sjkang
}
  
NgcX2PdcpUser* 
NrMcEnbPdcp::GetNgcX2PdcpUser ()
{
  NS_LOG_FUNCTION(this);
  return m_ngcX2PdcpUser;
}

void
NrMcEnbPdcp::SetMmWaveRnti (uint16_t rnti)
{
  m_mmWaveRnti = rnti;
}

void
NrMcEnbPdcp::SetRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << (uint32_t) rnti);
  m_rnti = rnti;
}

void
NrMcEnbPdcp::SetLcId (uint8_t lcId)
{
  NS_LOG_FUNCTION (this << (uint32_t) lcId);
  m_lcid = lcId;
}

void
NrMcEnbPdcp::SetNrPdcpSapUser (NrPdcpSapUser * s)
{
  NS_LOG_FUNCTION (this << s);
  m_pdcpSapUser = s;
}

NrPdcpSapProvider*
NrMcEnbPdcp::GetNrPdcpSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_pdcpSapProvider;
}

void
NrMcEnbPdcp::SetNrRlcSapProvider (NrRlcSapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  NS_LOG_INFO("Change NrRlcSapProvider");
  m_rlcSapProvider = s;
}

NrRlcSapUser*
NrMcEnbPdcp::GetNrRlcSapUser ()
{
  NS_LOG_FUNCTION (this);
  return m_rlcSapUser;
}
NrRlcSapUser*
NrMcEnbPdcp::GetAssi_NrRlcSapUser(){  //sjkang
	return m_assistant_rlcSapUser;
}
NrMcEnbPdcp::Status 
NrMcEnbPdcp::GetStatus ()
{
  Status s;
  s.txSn = m_txSequenceNumber;
  s.rxSn = m_rxSequenceNumber;
  return s;
}

void 
NrMcEnbPdcp::SetStatus (Status s)
{
  m_txSequenceNumber = s.txSn;
  m_rxSequenceNumber = s.rxSn;
}

void 
NrMcEnbPdcp::SetUeDataParams(NgcX2Sap::UeDataParams params)
{
  m_ueDataParams = params;
 // std::cout << params.sourceCellId << "\t" <<params.targetCellId << std::endl;
}


////////////////////////////////////////
//int count=0;
void
NrMcEnbPdcp::DoTransmitPdcpSdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << m_rnti << (uint32_t) m_lcid << p->GetSize ());

  NrPdcpHeader pdcpHeader;
  pdcpHeader.SetSequenceNumber (m_txSequenceNumber);

//std::cout<< (unsigned)pdcpHeader.GetDcBit() << std::endl;
  m_txSequenceNumber++;
  if (m_txSequenceNumber > m_maxPdcpSn)
    {
      m_txSequenceNumber = 0;
    }
//std::cout << this << "\t"<<"transmit  " << m_txSequenceNumber << std::endl;
  pdcpHeader.SetDcBit (NrPdcpHeader::DATA_PDU);

  NS_LOG_LOGIC ("PDCP header: " << pdcpHeader);


  NrRlcSapProvider::TransmitPdcpPduParameters params;
  params.rnti = m_rnti;
  params.lcid = m_lcid;
//m_useMmWaveConnection=true;
  //std::cout << this <<"\t" <<m_ngcX2PdcpProvider << "\t "  << m_useMmWaveConnection << std :: endl;
  if(m_ngcX2PdcpProvider == 0 || (!m_useMmWaveConnection))
  {
    //NS_LOG_UNCOND(this << Simulator::Now().GetSeconds() << "\t" <<" NrMcEnbPdcp: Tx packet to downlink local stack");

    // Sender timestamp. We will use this to measure the delay on top of RLC
	  p->AddHeader (pdcpHeader);
    PdcpTag pdcpTag (Simulator::Now ());
    p->AddByteTag (pdcpTag);
    // m_txPdu (m_rnti, m_lcid, p->GetSize ());
    params.pdcpPdu = p;

    NS_LOG_LOGIC("Params.rnti " << params.rnti);
    NS_LOG_LOGIC("Params.m_lcid " << params.lcid);
    NS_LOG_LOGIC("Params.pdcpPdu " << params.pdcpPdu);
  //   std::cout << " MC PDCP will send to NR RLC ------>" << std::endl;
    m_rlcSapProvider->TransmitPdcpPdu (params);
  } 
  else if (m_useMmWaveConnection ){

	  // Do not add sender time stamp: we are not interested in adding X2 delay for MC connections
    NS_LOG_INFO(this << Simulator::Now().GetSeconds() << "\t" << " NrMcEnbPdcp: Tx packet to downlink MmWave stack on remote cell " << m_ueDataParams.targetCellId);
    m_ueDataParams.ueData = p;


    	if(m_isNrMmWaveDC){
    		targetCellId_2 = nrCellId;

    							if(!RequestAssistantInfoNR){ ///sjkang
    		    							m_rlcSapProvider->RequestAssistantInfo(); //sjkang
    		    							RequestAssistantInfoNR = true; //sjkang
    		    						}

    		if (splitingAlgorithm() == nrCellId ){
    				  p->AddHeader (pdcpHeader);
    			    PdcpTag pdcpTag (Simulator::Now ());
    			    p->AddByteTag (pdcpTag);
    				    params.pdcpPdu = p;
    				m_rlcSapProvider->TransmitPdcpPdu (params);

    			  }
    			 else
    			  {
    				  m_ueDataParams.targetCellId = targetCellId_1;
    				  PdcpTag pdcpTag (Simulator::Now ());
    			//	 std::cout<<"will send to MmWave "<< std::endl;
    				 	 	  	  p->AddByteTag (pdcpTag);
    			    		pdcpHeader.SetSourceCellId(targetCellId_1); ///sjkang1116
    			    		p->AddHeader (pdcpHeader);

    				  m_ngcX2PdcpProvider->SendMcPdcpPdu (m_ueDataParams);
    			  }
    		 // count ++;

    	}else
    	{
    		if (count >=100){
    			if (m_isEnableDuplicate){
    				//	std::cout << "----------------Duplicate Mode ---------------" <<std::endl;
    				NgcX2Sap::UeDataParams m_ueDataParams_copy = m_ueDataParams;
    				  PdcpTag pdcpTag (Simulator::Now ());
    			    	 p->AddByteTag (pdcpTag);
    			    	 p->AddHeader (pdcpHeader);
    			    	 Ptr<Packet> p_copy = p->Copy();
    			    	 m_ueDataParams_copy.ueData = p_copy;
    			    	 m_ueDataParams.targetCellId = targetCellId_1;
    			    	 m_ueDataParams_copy.targetCellId = targetCellId_2;
    			    		params.pdcpPdu = p;
    			    	   if (targetCellId_1 == nrCellId or targetCellId_2 == nrCellId)
    			    	    		  m_rlcSapProvider->TransmitPdcpPdu (params);
    			    	  else{

    			    	 m_ngcX2PdcpProvider->SendMcPdcpPdu(m_ueDataParams);
    			    	 m_ngcX2PdcpProvider->SendMcPdcpPdu(m_ueDataParams_copy);
    			    	  }

    			}else{
    					uint16_t Cellid = splitingAlgorithm();

    					m_ueDataParams.targetCellId = Cellid;
    				//	std::cout<<Simulator::Now().GetSeconds()<<"\t"<<Cellid << std::endl;
    		    		  	  PdcpTag pdcpTag (Simulator::Now ());
    		    		    p->AddByteTag (pdcpTag);
    		    		    pdcpHeader.SetSourceCellId(Cellid); ///sjkang1116
    		    	    	p->AddHeader (pdcpHeader);
    		    	    	params.pdcpPdu = p;
    		    	    	if (Cellid == nrCellId)
    		    	    	   	m_rlcSapProvider->TransmitPdcpPdu (params);
    		    	    	 else{
    		    	    	m_ngcX2PdcpProvider->SendMcPdcpPdu (m_ueDataParams);
    		    	    	  }
    			}
    			}
    		else if(count < 100){
    			if (count %2==1 ){
    		    m_ueDataParams.targetCellId=targetCellId_1;
    			    pdcpHeader.SetSourceCellId(targetCellId_1); //sjakng1116
    		    p->AddHeader (pdcpHeader);
    		    }
    		    else{

    		   	m_ueDataParams.targetCellId = targetCellId_2;
      		    pdcpHeader.SetSourceCellId(targetCellId_2); ///sjkang1116
    		   	p->AddHeader (pdcpHeader);

    		    }

    			count ++;
    		    m_ngcX2PdcpProvider->SendMcPdcpPdu (m_ueDataParams);
    		  }
    		}

    	//
  }

  else 
  {
    NS_FATAL_ERROR("Invalid combination");
  }
}

void
NrMcEnbPdcp::DoReceivePdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << m_rnti << (uint32_t) m_lcid << p->GetSize ());

  NS_LOG_INFO(this << " NrMcEnbPdcp received uplink Pdu");
  // Receiver timestamp
  PdcpTag pdcpTag;
  Time delay;
  if (p->FindFirstMatchingByteTag (pdcpTag))
    {
      delay = Simulator::Now() - pdcpTag.GetSenderTimestamp ();
    }
  m_rxPdu(m_rnti, m_lcid, p->GetSize (), delay.GetNanoSeconds ());

  p->RemoveAllByteTags();
  NS_LOG_LOGIC("ALL BYTE TAGS REMOVED. NetAmin and FlowMonitor won't work");
  
  NrPdcpHeader pdcpHeader;
  p->RemoveHeader (pdcpHeader);
  NS_LOG_LOGIC ("PDCP header: " << pdcpHeader);
if (pdcpHeader.GetDcBit() ==NrPdcpHeader::CONTROL_PDU){
	//std::cout <<"I received Contrl packet -->"<<"\t"<<(unsigned)pdcpHeader.GetSourceCellId() <<std::endl;
	if((unsigned) pdcpHeader.GetSourceCellId() == 100){
		isAlternative = true;
		this->isTargetCellId_1 = false;
		this -> isTargetCellId_2 =false;

	}else if ((uint16_t)pdcpHeader.GetSourceCellId() == targetCellId_1){
			isAlternative = false;
			this->isTargetCellId_1 = true;
			this -> isTargetCellId_2 =false;
			//std::cout <<"I received Contrl packet "<<std::endl;
	}else if ((uint16_t)pdcpHeader.GetSourceCellId() == targetCellId_2){
		isAlternative = false;
			this->isTargetCellId_1 = false;
			this -> isTargetCellId_2 = true;
			//std::cout <<"I received Contrl packet "<<std::endl;
		}
	}
  m_rxSequenceNumber = pdcpHeader.GetSequenceNumber () + 1;
  if (m_rxSequenceNumber > m_maxPdcpSn)
    {
      m_rxSequenceNumber = 0;
    }
  if(p->GetSize() > 20 + 8 + 12)
  {
    NrPdcpSapUser::ReceivePdcpSduParameters params;
    params.pdcpSdu = p;
    params.rnti = m_rnti;
    params.lcid = m_lcid;
    m_pdcpSapUser->ReceivePdcpSdu (params);
  }
}

void
NrMcEnbPdcp::DoReceiveMcPdcpPdu (NgcX2Sap::UeDataParams params)
{
  NS_LOG_FUNCTION(this << m_mmWaveRnti << (uint32_t) m_lcid);
  DoReceivePdu(params.ueData);
}

void
NrMcEnbPdcp::SwitchConnection (bool useMmWaveConnection)
{
  m_useMmWaveConnection = useMmWaveConnection;
}

bool
NrMcEnbPdcp::GetUseMmWaveConnection() const
{
  return m_useMmWaveConnection && (m_ngcX2PdcpProvider != 0);
}


void
NrMcEnbPdcp::UpdateEta(){

}
uint16_t
NrMcEnbPdcp::splitingAlgorithm(){

switch(m_isSplitting){
case 0:
{
	return targetCellId_1;
	break;
}
case 1:
{
	return targetCellId_2;
	break;
}
case 2: //alternative
{
	count ++;
	if (count % 2 == 0)
		return targetCellId_1;
	else
		return targetCellId_2;


	break;
}
case 3: // p-split
{
	double randomValue = (double) rand()/ RAND_MAX;
	if (q_Delay[targetCellId_1] >0.003 || q_Delay[targetCellId_2]>0.003){
		if(q_Delay[targetCellId_1]> q_Delay[targetCellId_2]){
			eta -= 0.001;
			if (eta < 0.0) eta =0.0;
		}else if (q_Delay[targetCellId_2] >= q_Delay[targetCellId_1]){
			eta += 0.001;
			if (eta >= 1) eta = 1.0;
		}
	}
 print_Eta << Simulator::Now().GetSeconds() << "\t"<<eta << std::endl;
	if (randomValue < eta){
		return targetCellId_1;
	}else
		return targetCellId_2;

break;
		}
case 4: //SDF
	//	std::cout <<targetCellId_1 <<"\t"<< q_Delay[targetCellId_1] <<"\t"<<targetCellId_2<<"\t"<<q_Delay[targetCellId_2]<< std::endl;
			if (q_Delay[targetCellId_1] > q_Delay[targetCellId_2]){
				t_2++;
				targetCellID = targetCellId_2;
			//std::cout << q_Delay[targetCellId_1] <<"\t"<<q_Delay[targetCellId_2]<<"\t"<<targetCellID << std::endl;
			}else if (q_Delay[targetCellId_2] > q_Delay[targetCellId_1]){
				t_1 ++;
				targetCellID = targetCellId_1;
				//std::cout << q_Delay[targetCellId_1] <<"\t"<<q_Delay[targetCellId_2]<<"\t"<<targetCellID << std::endl;

			}
			else if (q_Delay[targetCellId_1] == q_Delay[targetCellId_2]){
					if(count % 2 == 0){
						t_1 ++;
						targetCellID = targetCellId_1;
						//std::cout << q_Delay[targetCellId_1] <<"\t"<<q_Delay[targetCellId_2]<<"\t"<<targetCellID << std::endl;

					}else{
						t_2 ++;
						targetCellID = targetCellId_2;
						//std::cout << q_Delay[targetCellId_1] <<"\t"<<q_Delay[targetCellId_2]<<"\t"<<targetCellID << std::endl;

					}
					count ++;
			}
			print_Eta << Simulator::Now().GetSeconds() <<"\t"<<(double) t_1/(t_1+t_2)<<std::endl;
		return targetCellID;
	break;
case 5: //SQF
	///std::cout << q_Size[targetCellId_1] <<"\t"<<q_Size[targetCellId_2]<< "\t"<< targetCellId_1<<"\t" << targetCellId_2 <<std::endl;
	if(q_Size[targetCellId_1] > q_Size[targetCellId_2]){
		t_2++;
		targetCellID = targetCellId_2;
	}else if (q_Size[targetCellId_2] >q_Size[targetCellId_1]){
		t_1 ++;
		targetCellID = targetCellId_1;
	}else if(q_Size[targetCellId_1] == q_Size[targetCellId_2])
	    {
			if (count %2 ==0){
				t_1 ++;
				targetCellID = targetCellId_1;
			//	std::cout << count<< "\t"<< targetCellID<<std::endl;
			}else{
				t_2 ++;
				targetCellID = targetCellId_2;
				//std::cout << count<< "\t"<< targetCellID<<std::endl;

			}
			count ++;

	}
	print_Eta << Simulator::Now().GetSeconds() <<"\t"<< (double)t_1/(t_1+t_2)<<std::endl;
	return targetCellID;
	break;
case 6:{
	double randomValue = (double) rand()/ RAND_MAX;
	if (randomValue >0.05)
		return targetCellId_1;

	else
		return targetCellId_2;
		break;

}
 } // switch

return 1;
}
void
NrMcEnbPdcp::DoReceiveAssistantInformation(NgcX2Sap::AssistantInformationForSplitting info){

//	q_Size[info.sourceCellId] = info.Re_TX_Q_Size+info.Tx_On_Q_Size+info.Txed_Q_Size;
	q_Size[info.sourceCellId] = info.Tx_On_Q_Size+info.Txed_Q_Size;
	//q_Delay[info.sourceCellId] =(double) (info.Txed_Q_Delay+info.Tx_On_Q_Delay+info.Re_Tx_Q_Delay)/10e6;
	double alpha=0.9;
	q_Delay[info.sourceCellId] =(1-alpha)*(double) (info.Tx_On_Q_Delay)/10e3 + alpha*q_Delay[info.sourceCellId];
//std::cout <<info.Re_TX_Q_Size <<std::endl;
	//std::cout<<"mmWave received-->" <<info.Tx_On_Q_Delay<<std::endl;;
	//std::cout << "MmWave : "<<info.Tx_On_Q_Delay << "\t"<< info.Re_Tx_Q_Delay << info.Txed_Q_Delay <<std::endl;

}
void
NrMcEnbPdcp::DoReceiveNrAssistantInfo(NgcX2Sap::AssistantInformationForSplitting info){
	q_Size[nrCellId] = info.Tx_On_Q_Size+info.Txed_Q_Size;
	//q_Size[nrCellId] = info.Re_TX_Q_Size+info.Tx_On_Q_Size+info.Txed_Q_Size;
	//q_Delay[nrCellId] =(double) (info.Txed_Q_Delay+info.Tx_On_Q_Delay+info.Re_Tx_Q_Delay)/10e6;
	double alpha = 0.9;
	q_Delay[nrCellId]=(1-alpha)*(double) (info.Tx_On_Q_Delay)/10e3 + alpha* q_Delay[nrCellId];

//	std::cout << "nr received -->"<<q_Delay[nrCellId]<<std::endl;;
   // std::cout << "NR : "<<info.Tx_On_Q_Delay << "\t"<< info.Re_Tx_Q_Delay <<"\t"<< info.Txed_Q_Delay <<std::endl;
}

} // namespace ns3
