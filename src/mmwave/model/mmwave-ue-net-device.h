 /* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
 /*
 *   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
 *  
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *  
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *  
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  
 *   Author: Marco Miozzo <marco.miozzo@cttc.es>
 *           Nicola Baldo  <nbaldo@cttc.es>
 *  
 *   Modified by: Marco Mezzavilla < mezzavilla@nyu.edu>
 *        	 	  Sourjya Dutta <sdutta@nyu.edu>
 *        	 	  Russell Ford <russell.ford@nyu.edu>
 *        		  Menglei Zhang <menglei@nyu.edu>
 */

#ifndef SRC_MMWAVE_MODEL_MMWAVE_UE_NET_DEVICE_H_
#define SRC_MMWAVE_MODEL_MMWAVE_UE_NET_DEVICE_H_

#include "mmwave-net-device.h"
#include "mmwave-enb-net-device.h"
#include "ns3/event-id.h"
#include "ns3/mac48-address.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "mmwave-phy.h"
#include "mmwave-ue-mac.h"
#include <ns3/lte-ue-rrc.h>
#include <ns3/epc-ue-nas.h>

namespace ns3
{

class Packet;
class PacketBurst;
class Node;
//class MmWavePhy;
class MmWaveUePhy;
class MmWaveUeMac;
class MmWaveEnbNetDevice;

/*
//180704-jskim14-add antenna parameters
struct AntennaParams 
{
    AntennaParams (): m_vAntennaNum (4), m_hAntennaNum (4), m_polarNum (1), m_vTxrusNum(2), m_hTxrusNum(4), m_connectMode(0)
	{
	}

	AntennaParams (uint8_t vAntennaNum, uint8_t hAntennaNum, uint8_t polarNum, uint8_t vTxrusNum, uint8_t hTxrusNum, uint8_t connectMode)
		: m_vAntennaNum (vAntennaNum), m_hAntennaNum (hAntennaNum), m_polarNum (polarNum), m_vTxrusNum (vTxrusNum), m_hTxrusNum (hTxrusNum), m_connectMode (connectMode)
	{
	}
  	uint8_t m_vAntennaNum; //The number of vertical antenna elements
   	uint8_t m_hAntennaNum; //The number of horizontal antenna elements
	uint8_t m_polarNum;    //The number of polarization dimension
   	uint8_t m_vTxrusNum;   //The number of vertical TXRUs
   	uint8_t m_hTxrusNum;   //The number of horizontal TXRUs
    uint8_t m_connectMode; //Antenna connection mode (0:1-D full, 1:2-D full, 2,3)
};
//jskim14-end
*/

class MmWaveUeNetDevice : public MmWaveNetDevice
{
  public:
	static TypeId GetTypeId(void);

	MmWaveUeNetDevice(void);
	virtual ~MmWaveUeNetDevice(void);
	virtual void DoDispose();

	uint32_t GetCsgId() const;
	void SetCsgId(uint32_t csgId);

	void UpdateConfig(void);

	virtual bool DoSend(Ptr<Packet> packet, const Address &dest, uint16_t protocolNumber);

	Ptr<MmWaveUePhy> GetPhy(void) const;

	Ptr<MmWaveUeMac> GetMac(void) const;

	uint64_t GetImsi() const;

	uint16_t GetEarfcn() const;

	Ptr<EpcUeNas> GetNas(void) const;

	Ptr<LteUeRrc> GetRrc() const;

	void SetEarfcn(uint16_t earfcn);

	void SetTargetEnb(Ptr<MmWaveEnbNetDevice> enb);

	Ptr<MmWaveEnbNetDevice> GetTargetEnb(void);

	void SetAntennaNum(uint8_t antennaNum);

	uint8_t GetAntennaNum() const;

	//180704-jskim14-add new functions
	void SetAntennaParams(uint8_t vAntennaNum, uint8_t hAntennaNum, uint8_t polarNum, uint8_t vTxruNum, uint8_t hTxruNum, uint8_t connectMode);
	void SetAntennaRotation (double alpha, double beta, double gamma, double pol); //180716-jskim14-input is degree
	uint8_t GetVAntennaNum();
	uint8_t GetHAntennaNum();
	uint8_t GetPolarNum();
	uint8_t GetVTxruNum();
	uint8_t GetHTxruNum();
	uint8_t GetConnectMode();
	Vector GetRotation();	 //180716-jskim14
	double GetPolarization(); //180716-jskim14
	//jskim14-end

  protected:
	// inherited from Object
	virtual void DoInitialize(void);

  private:
	Ptr<MmWaveEnbNetDevice> m_targetEnb;
	Ptr<MmWaveUePhy> m_phy;
	Ptr<MmWaveUeMac> m_mac;

	Ptr<LteUeRrc> m_rrc;
	Ptr<EpcUeNas> m_nas;

	uint64_t m_imsi;

	uint16_t m_earfcn;

	uint32_t m_csgId;
	bool m_isConstructed;
	uint8_t m_antennaNum;

	AntennaParams m_antennaParams; //180714-jskim14-Parameters of antenna
	//180716-jskim14-antenna rotation parameterd
	Vector m_rotation;
	double m_pol;
	//jskim14-end
};

} // namespace ns3
#endif /* SRC_MMWAVE_MODEL_MMWAVE_UE_NET_DEVICE_H_ */
