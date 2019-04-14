/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 TELEMATICS LAB, DEE - Politecnico di Bari
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
 * Author: Giuseppe Piro  <g.piro@poliba.it>
 *         Marco Miozzo <mmiozzo@cttc.es>
 *         Nicola Baldo <nbaldo@cttc.es>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Dual Connectivity functionalities
 */

#ifndef NR_PHY_H
#define NR_PHY_H


#include <ns3/spectrum-value.h>
#include <ns3/mobility-model.h>
#include <ns3/packet.h>
#include <ns3/nstime.h>
#include <ns3/spectrum-phy.h>
#include <ns3/spectrum-channel.h>
#include <ns3/spectrum-signal-parameters.h>
#include <ns3/spectrum-interference.h>
#include <ns3/generic-phy.h>
#include <ns3/nr-spectrum-phy.h>

namespace ns3 {

class PacketBurst;
class NrNetDevice;
class NrControlMessage;



/**
 * \ingroup nr
 *
 * The NrPhy models the physical layer of NR. It is composed by two
 * NrSpectrumPhy, one for the downlink and one for the uplink.
 */
class NrPhy : public Object
{

public:
  /** 
   * @warning the default constructor should not be used
   */
  NrPhy ();

  /** 
   * 
   * \param dlPhy the downlink NrSpectrumPhy instance
   * \param ulPhy the uplink NrSpectrumPhy instance
   */
  NrPhy (Ptr<NrSpectrumPhy> dlPhy, Ptr<NrSpectrumPhy> ulPhy);

  virtual ~NrPhy ();

  static TypeId GetTypeId (void);

  /**
   * \brief Set the device where the phy layer is attached
   * \param d the device
   */
  void SetDevice (Ptr<NetDevice> d);
  /**
   * \brief Get the device where the phy layer is attached
   * \return the pointer to the device
   */
  Ptr<NetDevice> GetDevice () const;

  /** 
   * 
   * \return a pointer to the NrSpectrumPhy instance that manages the downlink
   */
  Ptr<NrSpectrumPhy> GetDownlinkSpectrumPhy ();


  /** 
   * 
   * \return a pointer to the NrSpectrumPhy instance that manages the uplink
   */
  Ptr<NrSpectrumPhy> GetUplinkSpectrumPhy ();

  /**
  * \brief Queue the MAC PDU to be sent (according to m_macChTtiDelay)
  * \param p the MAC PDU to sent
  */
  virtual void DoSendMacPdu (Ptr<Packet> p) = 0;

  /**
   * Set the downlink channel
   * \param c the downlink channel
   */
  void SetDownlinkChannel (Ptr<SpectrumChannel> c);

  /**
   * Set the uplink channel
   * \param c the uplink channel
   */
  void SetUplinkChannel (Ptr<SpectrumChannel> c);


  /**
   * \brief Compute the TX Power Spectral Density
   * \return a pointer to a newly allocated SpectrumValue representing the TX Power Spectral Density in W/Hz for each Resource Block
   */
  virtual Ptr<SpectrumValue> CreateTxPowerSpectralDensity () = 0;

  void DoDispose ();

  /**
   * \param tti transmission time interval
   */
  void SetTti (double tti);
  /**
   * \returns transmission time interval
   */
  double GetTti (void) const;

  /** 
   * 
   * \param cellId the Cell Identifier
   */
  void DoSetCellId (uint16_t cellId);


  /**
  * \returns the RB gruop size according to the bandwidth
  */
  uint8_t GetRbgSize (void) const;
  
  
  /**
  * \returns the SRS periodicity (see Table 8.2-1 of 36.213)
  * \param srcCi the SRS Configuration Index
  */
  uint16_t GetSrsPeriodicity (uint16_t srcCi) const;
  
  /**
  * \returns the SRS Subframe offset (see Table 8.2-1 of 36.213)
  * \param srcCi the SRS Configuration Index
  */
  uint16_t GetSrsSubframeOffset (uint16_t srcCi) const;


  /**
  * \param p queue MAC PDU to be sent
  */
  void SetMacPdu (Ptr<Packet> p);

  /**
  * \returns the packet burst to be sent
  */
  Ptr<PacketBurst> GetPacketBurst (void);

  /**
  * \param m the control message to be sent
  */
  void SetControlMessages (Ptr<NrControlMessage> m);

  /**
  * \returns the list of control messages to be sent
  */
  std::list<Ptr<NrControlMessage> > GetControlMessages (void);


  /** 
   * generate a CQI report based on the given SINR of Ctrl frame
   * 
   * \param sinr the SINR vs frequency measured by the device
   */
  virtual void GenerateCtrlCqiReport (const SpectrumValue& sinr) = 0;
  
  /** 
  * generate a CQI report based on the given SINR of Data frame
  * (used for PUSCH CQIs)
  * 
  * \param sinr the SINR vs frequency measured by the device
  */
  virtual void GenerateDataCqiReport (const SpectrumValue& sinr) = 0;

  /**
  * generate a report based on the linear interference and noise power
  * perceived during DATA frame
  * NOTE: used only by eNB 
  *
  * \param interf the interference + noise power measured by the device
  */
  virtual void ReportInterference (const SpectrumValue& interf) = 0;

  /**
  * generate a report based on the linear RS power perceived during CTRL 
  * frame
  * NOTE: used only by UE for evaluating RSRP
  *
  * \param power the RS power measured by the device
  */
  virtual void ReportRsReceivedPower (const SpectrumValue& power) = 0;



protected:
  /// Pointer to the NetDevice where this PHY layer is attached.
  Ptr<NetDevice> m_netDevice;

  /**
   * The downlink NrSpectrumPhy associated to this NrPhy. Also available as
   * attribute `DlSpectrumPhy` in the child classes NrEnbPhy and NrUePhy.
   */
  Ptr<NrSpectrumPhy> m_downlinkSpectrumPhy;
  /**
   * The uplink NrSpectrumPhy associated to this NrPhy. Also available as
   * attribute `UlSpectrumPhy` in the child classes NrEnbPhy and NrUePhy.
   */
  Ptr<NrSpectrumPhy> m_uplinkSpectrumPhy;

  /**
   * Transmission power in dBm. Also available as attribute `TxPower` in the
   * child classes NrEnbPhy and NrUePhy.
   */
  double m_txPower;
  /**
   * Loss (dB) in the Signal-to-Noise-Ratio due to non-idealities in the
   * receiver. Also available as attribute `NoiseFigure` in the child classes
   * NrEnbPhy and NrUePhy.
   *
   * According to [Wikipedia](http://en.wikipedia.org/wiki/Noise_figure), this
   * is "the difference in decibels (dB) between the noise output of the actual
   * receiver to the noise output of an ideal receiver with the same overall
   * gain and bandwidth when the receivers are connected to sources at the
   * standard noise temperature T0." In this model, we consider T0 = 290K.
   */
  double m_noiseFigure;

  /// Transmission time interval.
  double m_tti;
  /**
   * The UL bandwidth in number of PRBs.
   * Specified by the upper layer through CPHY SAP.
   */
  uint8_t m_ulBandwidth;
  /**
   * The DL bandwidth in number of PRBs.
   * Specified by the upper layer through CPHY SAP.
   */
  uint8_t m_dlBandwidth;
  /// The RB gruop size according to the bandwidth.
  uint8_t m_rbgSize;
  /**
   * The downlink carrier frequency.
   * Specified by the upper layer through CPHY SAP.
   */
  uint16_t m_dlEarfcn;
  /**
   * The uplink carrier frequency.
   * Specified by the upper layer through CPHY SAP.
   */
  uint16_t m_ulEarfcn;

  /// A queue of packet bursts to be sent.
  std::vector< Ptr<PacketBurst> > m_packetBurstQueue;
  /// A queue of control messages to be sent.
  std::vector< std::list<Ptr<NrControlMessage> > > m_controlMessagesQueue;
  /**
   * Delay between MAC and channel layer in terms of TTIs. It is the delay that
   * occurs between a scheduling decision in the MAC and the actual start of
   * the transmission by the PHY. This is intended to be used to model the
   * latency of real PHY and MAC implementations.
   *
   * In NrEnbPhy, it is 2 TTIs by default and can be configured through the
   * `MacToChannelDelay` attribute. In NrUePhy, it is 4 TTIs.
   */
  uint8_t m_macChTtiDelay;

  /**
   * Cell identifier. In NrEnbPhy, this corresponds to the ID of the cell
   * which hosts this PHY layer. In NrUePhy, this corresponds to the ID of the
   * eNodeB which this PHY layer is synchronized with.
   */
  uint16_t m_cellId;

}; // end of `class NrPhy`


}

#endif /* NR_PHY_H */
