 /* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
 /*
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
 *  
 *   Author: Marco Mezzavilla < mezzavilla@nyu.edu>
 *        	 Sourjya Dutta <sdutta@nyu.edu>
 *        	 Russell Ford <russell.ford@nyu.edu>
 *        	 Menglei Zhang <menglei@nyu.edu>
 */

#ifndef ANTENNA_ARRAY_MODEL_H_
#define ANTENNA_ARRAY_MODEL_H_
#include <ns3/antenna-model.h>
#include <complex>
#include <ns3/net-device.h>
#include <map>

namespace ns3 {

typedef std::vector< std::complex<double> > complexVector_t;
typedef std::vector<complexVector_t> complex2DVector_t; //180704-jskim14-for antenna weight matrix

class AntennaArrayModel: public AntennaModel {
public:
	AntennaArrayModel();
	virtual ~AntennaArrayModel();
	static TypeId GetTypeId ();
	virtual double GetGainDb (Angles a);
	void SetBeamformingVector (complexVector_t antennaWeights, Ptr<NetDevice> device = 0);
	void SetBeamformingVectorWithDelay (complexVector_t antennaWeights, Ptr<NetDevice> device = 0);

	void ChangeBeamformingVector (Ptr<NetDevice> device);
	void ChangeToOmniTx ();
	complexVector_t GetBeamformingVector ();
	complexVector_t GetBeamformingVector (Ptr<NetDevice> device);
	void SetToSector (uint32_t sector, uint32_t antennaNum);
	bool IsOmniTx ();
	Vector2D GetRadiationPattern_polar (double vangle, double hangle, uint16_t antInd); //input is expressed in radians
	double GetRadiationPattern_nonpolar (double vangle, double hangle = 0);
	Vector GetAntennaLocation (uint8_t index, uint8_t* antennaNum) ;
	Vector GetAntennaLocation (uint8_t index, uint8_t vAntennaNum, uint8_t hAntennaNum, uint8_t polarNum); //180704-jskim14-new antenna location function
  	void SetSector (uint8_t sector, uint8_t *antennaNum, double elevation = 90);

	void SetAntParams (uint8_t connectMode, uint8_t vAntNum, uint8_t hAntNum, uint8_t polarNum, uint8_t vTxrusNum, uint8_t hTxrusNum, Ptr<NetDevice> device); //180702-jskim14-antenna parameters setting function
    void SetDigitalBeamformingVector(); //180822-jskim14-set digital beamforming vector
	void SetAntennaWeightMatrix(); //180704-jskim14-set analog beamforming vector
	complex2DVector_t GetAntennaWeightMatrix(); //180709-jskim14-get analog beamforming weight matrix
	//void SetPrecodingVector(); //180726-jskim14-set digital precoding vector
	//complex2DVector_t GetPrecodingVector(); //180726-jskim14-get digital precoding vector
	void SetAntennaRotation (double alpha, double beta, double gamma, double pol); //180715-jskim14-add set antenna roation
	Ptr<NetDevice> GetNetDevice (); //180717-jskim14
	Vector GetTxruNum (); //180718-jskim14

private:
	bool m_omniTx;
	double m_minAngle;
	double m_maxAngle;
	complexVector_t m_beamformingVector;
	std::map<Ptr<NetDevice>, complexVector_t> m_beamformingVectorMap;

	double m_disV; //antenna spacing in the vertical direction in terms of wave length.
	double m_disH; //antenna spacing in the horizontal direction in terms of wave length.

	uint8_t m_connectMode; //180702-jskim14-add antenna connection mode
	uint8_t m_vAntennaNum; //180704-jskim14-add the number of vertical antenna elements
	uint8_t m_hAntennaNum; //180704-jskim14-add the number of horizontal antenna elements
	uint8_t m_polarNum;    //180702-jskim14-add the number of polarization dimensions
	uint8_t m_vTxruNum;   //180702-jskim14-add the number of vertical TXRUs
	uint8_t m_hTxruNum;   //180702-jskim14-add the number of horizontal TXRUs
	Ptr<NetDevice> m_netDevice; //180714-jskim14

	//uint64_t m_noAntennas; //180702-jskim14-add the number of antenna elements
	complex2DVector_t m_antennaWeightMat; //180704-jskim14-antenna weight matrix for anamlog bemaforming

	double m_alpha; // bearing angle in radian
    double m_beta;  // downtilt angle in radian
    double m_gamma; // slant angle in radian
    double m_pol;   //polarization slant angle purely vertical = 0, cross polarization = +45/-45 in radian
	
};

} /* namespace ns3 */

#endif /* SRC_ANTENNA_MODEL_ANTENNA_ARRAY_MODEL_H_ */
