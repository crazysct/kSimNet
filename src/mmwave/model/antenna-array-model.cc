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



#include "antenna-array-model.h"
#include <ns3/log.h>
#include <ns3/math.h>
#include <ns3/simulator.h>
#include "ns3/double.h"
#include <ns3/node.h>//180714-jskim14
#include <ns3/mobility-model.h> //180714-jskim14
#include <ns3/mmwave-ue-net-device.h> //180718-jskim14
#include <ns3/mc-ue-net-device.h> //180718-jskim14

NS_LOG_COMPONENT_DEFINE ("AntennaArrayModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (AntennaArrayModel);

static const double Enb22DegreeBFVectorReal[8][64] = {
		{1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,},
		{1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,},
		{1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,},
		{1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,},
		{1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,},
		{1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,},
		{1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,},
		{1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,},
};
static const double Enb22DegreeBFVectorImag[8][64] = {
		{-0.000000,-0.060328,0.120437,-0.180106,0.239120,-0.297262,0.354322,-0.410091,-0.000000,-0.060328,0.120437,-0.180106,0.239120,-0.297262,0.354322,-0.410091,-0.000000,-0.060328,0.120437,-0.180106,0.239120,-0.297262,0.354322,-0.410091,-0.000000,-0.060328,0.120437,-0.180106,0.239120,-0.297262,0.354322,-0.410091,-0.000000,-0.060328,0.120437,-0.180106,0.239120,-0.297262,0.354322,-0.410091,-0.000000,-0.060328,0.120437,-0.180106,0.239120,-0.297262,0.354322,-0.410091,-0.000000,-0.060328,0.120437,-0.180106,0.239120,-0.297262,0.354322,-0.410091,-0.000000,-0.060328,0.120437,-0.180106,0.239120,-0.297262,0.354322,-0.410091,},
		{-0.000000,-0.505062,0.871821,-0.999846,0.854079,-0.474436,-0.035123,0.535065,-0.000000,-0.505062,0.871821,-0.999846,0.854079,-0.474436,-0.035123,0.535065,-0.000000,-0.505062,0.871821,-0.999846,0.854079,-0.474436,-0.035123,0.535065,-0.000000,-0.505062,0.871821,-0.999846,0.854079,-0.474436,-0.035123,0.535065,-0.000000,-0.505062,0.871821,-0.999846,0.854079,-0.474436,-0.035123,0.535065,-0.000000,-0.505062,0.871821,-0.999846,0.854079,-0.474436,-0.035123,0.535065,-0.000000,-0.505062,0.871821,-0.999846,0.854079,-0.474436,-0.035123,0.535065,-0.000000,-0.505062,0.871821,-0.999846,0.854079,-0.474436,-0.035123,0.535065,},
		{-0.000000,-0.984800,0.342107,0.865956,-0.642929,-0.642611,0.866164,0.341717,-0.000000,-0.984800,0.342107,0.865956,-0.642929,-0.642611,0.866164,0.341717,-0.000000,-0.984800,0.342107,0.865956,-0.642929,-0.642611,0.866164,0.341717,-0.000000,-0.984800,0.342107,0.865956,-0.642929,-0.642611,0.866164,0.341717,-0.000000,-0.984800,0.342107,0.865956,-0.642929,-0.642611,0.866164,0.341717,-0.000000,-0.984800,0.342107,0.865956,-0.642929,-0.642611,0.866164,0.341717,-0.000000,-0.984800,0.342107,0.865956,-0.642929,-0.642611,0.866164,0.341717,-0.000000,-0.984800,0.342107,0.865956,-0.642929,-0.642611,0.866164,0.341717,},
		{-0.000000,-0.575237,-0.941073,-0.964332,-0.636549,-0.077045,0.510506,0.912219,-0.000000,-0.575237,-0.941073,-0.964332,-0.636549,-0.077045,0.510506,0.912219,-0.000000,-0.575237,-0.941073,-0.964332,-0.636549,-0.077045,0.510506,0.912219,-0.000000,-0.575237,-0.941073,-0.964332,-0.636549,-0.077045,0.510506,0.912219,-0.000000,-0.575237,-0.941073,-0.964332,-0.636549,-0.077045,0.510506,0.912219,-0.000000,-0.575237,-0.941073,-0.964332,-0.636549,-0.077045,0.510506,0.912219,-0.000000,-0.575237,-0.941073,-0.964332,-0.636549,-0.077045,0.510506,0.912219,-0.000000,-0.575237,-0.941073,-0.964332,-0.636549,-0.077045,0.510506,0.912219,},
		{0.000000,0.575237,0.941073,0.964332,0.636549,0.077045,-0.510506,-0.912219,-0.000000,0.575237,0.941073,0.964332,0.636549,0.077045,-0.510506,-0.912219,-0.000000,0.575237,0.941073,0.964332,0.636549,0.077045,-0.510506,-0.912219,-0.000000,0.575237,0.941073,0.964332,0.636549,0.077045,-0.510506,-0.912219,-0.000000,0.575237,0.941073,0.964332,0.636549,0.077045,-0.510506,-0.912219,-0.000000,0.575237,0.941073,0.964332,0.636549,0.077045,-0.510506,-0.912219,-0.000000,0.575237,0.941073,0.964332,0.636549,0.077045,-0.510506,-0.912219,-0.000000,0.575237,0.941073,0.964332,0.636549,0.077045,-0.510506,-0.912219,},
		{0.000000,0.984800,-0.342107,-0.865956,0.642929,0.642611,-0.866164,-0.341717,-0.000000,0.984800,-0.342107,-0.865956,0.642929,0.642611,-0.866164,-0.341717,-0.000000,0.984800,-0.342107,-0.865956,0.642929,0.642611,-0.866164,-0.341717,-0.000000,0.984800,-0.342107,-0.865956,0.642929,0.642611,-0.866164,-0.341717,-0.000000,0.984800,-0.342107,-0.865956,0.642929,0.642611,-0.866164,-0.341717,-0.000000,0.984800,-0.342107,-0.865956,0.642929,0.642611,-0.866164,-0.341717,-0.000000,0.984800,-0.342107,-0.865956,0.642929,0.642611,-0.866164,-0.341717,-0.000000,0.984800,-0.342107,-0.865956,0.642929,0.642611,-0.866164,-0.341717,},
		{0.000000,0.505062,-0.871821,0.999846,-0.854079,0.474436,0.035123,-0.535065,-0.000000,0.505062,-0.871821,0.999846,-0.854079,0.474436,0.035123,-0.535065,-0.000000,0.505062,-0.871821,0.999846,-0.854079,0.474436,0.035123,-0.535065,-0.000000,0.505062,-0.871821,0.999846,-0.854079,0.474436,0.035123,-0.535065,-0.000000,0.505062,-0.871821,0.999846,-0.854079,0.474436,0.035123,-0.535065,-0.000000,0.505062,-0.871821,0.999846,-0.854079,0.474436,0.035123,-0.535065,-0.000000,0.505062,-0.871821,0.999846,-0.854079,0.474436,0.035123,-0.535065,-0.000000,0.505062,-0.871821,0.999846,-0.854079,0.474436,0.035123,-0.535065,},
		{0.000000,0.060328,-0.120437,0.180106,-0.239120,0.297262,-0.354322,0.410091,-0.000000,0.060328,-0.120437,0.180106,-0.239120,0.297262,-0.354322,0.410091,-0.000000,0.060328,-0.120437,0.180106,-0.239120,0.297262,-0.354322,0.410091,-0.000000,0.060328,-0.120437,0.180106,-0.239120,0.297262,-0.354322,0.410091,-0.000000,0.060328,-0.120437,0.180106,-0.239120,0.297262,-0.354322,0.410091,-0.000000,0.060328,-0.120437,0.180106,-0.239120,0.297262,-0.354322,0.410091,-0.000000,0.060328,-0.120437,0.180106,-0.239120,0.297262,-0.354322,0.410091,-0.000000,0.060328,-0.120437,0.180106,-0.239120,0.297262,-0.354322,0.410091,},
};

static const double Ue45DegreeBFVectorReal[4][16] = {
		{1.000000,-0.971542,0.887788,-0.753505,1.000000,-0.971542,0.887788,-0.753505,1.000000,-0.971542,0.887788,-0.753505,1.000000,-0.971542,0.887788,-0.753505,},
		{1.000000,0.360273,-0.740406,-0.893771,1.000000,0.360273,-0.740406,-0.893771,1.000000,0.360273,-0.740406,-0.893771,1.000000,0.360273,-0.740406,-0.893771,},
		{1.000000,0.360273,-0.740406,-0.893771,1.000000,0.360273,-0.740406,-0.893771,1.000000,0.360273,-0.740406,-0.893771,1.000000,0.360273,-0.740406,-0.893771,},
		{1.000000,-0.971542,0.887788,-0.753505,1.000000,-0.971542,0.887788,-0.753505,1.000000,-0.971542,0.887788,-0.753505,1.000000,-0.971542,0.887788,-0.753505,},
};
static const double Ue45DegreeBFVectorImag[4][16] = {
		{-0.000000,-0.236867,0.460252,-0.657442,-0.000000,-0.236867,0.460252,-0.657442,-0.000000,-0.236867,0.460252,-0.657442,-0.000000,-0.236867,0.460252,-0.657442,},
		{-0.000000,-0.932847,-0.672160,0.448524,-0.000000,-0.932847,-0.672160,0.448524,-0.000000,-0.932847,-0.672160,0.448524,-0.000000,-0.932847,-0.672160,0.448524,},
		{0.000000,0.932847,0.672160,-0.448524,-0.000000,0.932847,0.672160,-0.448524,-0.000000,0.932847,0.672160,-0.448524,-0.000000,0.932847,0.672160,-0.448524,},
		{0.000000,0.236867,-0.460252,0.657442,-0.000000,0.236867,-0.460252,0.657442,-0.000000,0.236867,-0.460252,0.657442,-0.000000,0.236867,-0.460252,0.657442,},
};

static const double All90DegreeBFVectorReal[2][4] = {
		{1.000000,-0.605700,1.000000,-0.605700,},
		{1.000000,-0.605700,1.000000,-0.605700,},
};
static const double All90DegreeBFVectorImag[2][4] = {
		{-0.000000,-0.795693,-0.000000,-0.795693,},
		{0.000000,0.795693,-0.000000,0.795693,},
};


AntennaArrayModel::AntennaArrayModel()
	:m_minAngle (0),m_maxAngle(2*M_PI), m_alpha(0), m_beta(0*M_PI/180), m_gamma(0), m_pol(45*M_PI/180)
{
	m_omniTx = false;
}

AntennaArrayModel::~AntennaArrayModel()
{

}

TypeId
AntennaArrayModel::GetTypeId ()
{
	static TypeId tid = TypeId ("ns3::AntennaArrayModel")
	.SetParent<Object> ()
	.AddConstructor<AntennaArrayModel> ()
	.AddAttribute ("AntennaHorizontalSpacing",
			"Horizontal spacing between antenna elements, in multiples of lambda",
			DoubleValue (0.5),
			MakeDoubleAccessor (&AntennaArrayModel::m_disH),
		    MakeDoubleChecker<double> ())
	.AddAttribute ("AntennaVerticalSpacing",
			"Vertical spacing between antenna elements, in multiples of lambda",
			DoubleValue (0.5),
			MakeDoubleAccessor (&AntennaArrayModel::m_disV),
		    MakeDoubleChecker<double> ())
	;
	return tid;
}

double
AntennaArrayModel::GetGainDb (Angles a)
{
	/*NS_ASSERT (m_minAngle<=m_maxAngle);
	double gain;
	if (m_maxAngle <= M_PI)
	{
		if(a.phi < m_minAngle || a.phi > m_maxAngle)
		{
			gain = -500;
			//NS_LOG_UNCOND ("++++++++++++++++++++++blocked");
		}
		else
		{
			gain = 0;
		}
	}
	else
	{
		double maxAngle = m_maxAngle - 2*M_PI;
		if(a.phi < m_minAngle || a.phi > maxAngle)
		{
			gain = -500;
			//NS_LOG_UNCOND ("++++++++++++++++++++++blocked");
			}
		else
		{
			gain = 0;
		}

	}

	return gain;*/
	return 0;
}
void
AntennaArrayModel::SetBeamformingVectorWithDelay (complexVector_t antennaWeights, Ptr<NetDevice> device)
{
	Simulator::Schedule (MilliSeconds(8), &AntennaArrayModel::SetBeamformingVector,this,antennaWeights,device);
}


void
AntennaArrayModel::SetBeamformingVector (complexVector_t antennaWeights, Ptr<NetDevice> device)
{
	m_omniTx = false;
	if (device != 0)
	{
		std::map< Ptr<NetDevice>, complexVector_t >::iterator iter = m_beamformingVectorMap.find (device);
		if (iter != m_beamformingVectorMap.end ())
		{
			(*iter).second = antennaWeights;
		}
		else
		{
			m_beamformingVectorMap.insert (std::make_pair (device, antennaWeights) );
		}
	}
	m_beamformingVector = antennaWeights;
}

void
AntennaArrayModel::ChangeBeamformingVector (Ptr<NetDevice> device)
{
	m_omniTx = false;
	std::map< Ptr<NetDevice>, complexVector_t >::iterator it = m_beamformingVectorMap.find (device);
	NS_ASSERT_MSG (it != m_beamformingVectorMap.end (), "could not find");
	m_beamformingVector = it->second;
}

complexVector_t
AntennaArrayModel::GetBeamformingVector ()
{
	if(m_omniTx)
	{
		NS_FATAL_ERROR ("omni transmission do not need beamforming vector");
	}
	return m_beamformingVector;
}

void
AntennaArrayModel::ChangeToOmniTx ()
{
	m_omniTx = true;
}

bool
AntennaArrayModel::IsOmniTx ()
{
	return m_omniTx;
}


complexVector_t
AntennaArrayModel::GetBeamformingVector (Ptr<NetDevice> device)
{
	complexVector_t weights;
	std::map< Ptr<NetDevice>, complexVector_t >::iterator it = m_beamformingVectorMap.find (device);
	if (it != m_beamformingVectorMap.end ())
	{
		weights = it->second;
	}
	else
	{
		weights = m_beamformingVector;
	}
	return weights;
}

void
AntennaArrayModel::SetToSector (uint32_t sector, uint32_t antennaNum)
{
	m_omniTx = false;
	complexVector_t cmplxVector;
	switch(antennaNum)
	{
		case 64:
		{
			if(sector == 0 || sector == 1 || sector == 14 || sector == 15)
			{
				m_minAngle = -0.5*M_PI;
				m_maxAngle = 0.5*M_PI;
			}
			else if(sector == 2 || sector == 3 || sector == 4 || sector == 5)
			{
				m_minAngle = 0;
				m_maxAngle = M_PI;
			}
			else if(sector == 6 || sector == 7 || sector == 8 || sector == 9)
			{
				m_minAngle = 0.5*M_PI;
				m_maxAngle = 1.5*M_PI;
			}

			else if(sector == 10 || sector == 11 || sector == 12 || sector == 13)
			{
				m_minAngle = -1*M_PI;
				m_maxAngle = 0;
			}
			else
			{
				NS_FATAL_ERROR ("64 antenna only need 16 sectors");

			}

			if(sector > 7)
			{
				sector = 15-sector;
			}
			for(unsigned int i = 0; i< antennaNum; i++)
			{
				std::complex<double> cmplx (Enb22DegreeBFVectorReal[sector][i],Enb22DegreeBFVectorImag[sector][i]);
				cmplxVector.push_back(cmplx);
			}
			break;
		}
		case 16:
		{
			if(sector == 0 || sector == 7)
			{
				m_minAngle = -0.5*M_PI;
				m_maxAngle = 0.5*M_PI;
			}
			else if(sector == 1 || sector == 2)
			{
				m_minAngle = 0;
				m_maxAngle = M_PI;
			}
			else if(sector == 3 || sector == 4)
			{
				m_minAngle = 0.5*M_PI;
				m_maxAngle = 1.5*M_PI;
			}
			else if(sector == 5 || sector == 6)
			{
				m_minAngle = -1*M_PI;
				m_maxAngle = 0;
			}
			else
			{
				NS_FATAL_ERROR ("16 antenna only need 8 sectors");

			}
			if(sector > 3)
			{
				sector = 7-sector;
			}
			for(unsigned int i = 0; i< antennaNum; i++)
			{
				std::complex<double> cmplx (Ue45DegreeBFVectorReal[sector][i],Ue45DegreeBFVectorImag[sector][i]);
				cmplxVector.push_back(cmplx);
			}
			break;
		}
		case 4:
		{
			if(sector == 0)
			{
				m_minAngle = 0;
				m_maxAngle = 0.5*M_PI;
			}
			else if(sector == 1)
			{
				m_minAngle = 0.5*M_PI;
				m_maxAngle = M_PI;
			}
			else if(sector == 2)
			{
				m_minAngle = -1*M_PI;
				m_maxAngle = -0.5*M_PI;
			}
			else if(sector == 3)
			{
				m_minAngle = -0.5*M_PI;
				m_maxAngle = 0;
			}
			else
			{
				NS_FATAL_ERROR ("4 antenna only need 4 sectors");
			}
			if(sector > 1)
			{
				sector = 3-sector;
			}
			for(unsigned int i = 0; i< antennaNum; i++)
			{
				std::complex<double> cmplx (All90DegreeBFVectorReal[sector][i],All90DegreeBFVectorImag[sector][i]);
				cmplxVector.push_back(cmplx);
			}
			break;
		}
		default:
		{
			NS_FATAL_ERROR ("the antenna number should only be 64 or 16");
		}
	}

	//normalize antennaWeights;
	double weightSum = 0;
	for (unsigned int i = 0; i< antennaNum; i++)
	{
		weightSum += pow (std::abs(cmplxVector. at(i)),2);
	}
	for (unsigned int i = 0; i< antennaNum; i++)
	{
		cmplxVector. at(i) = cmplxVector. at(i)/sqrt(weightSum);
	}
	m_beamformingVector = cmplxVector;
}

// We add the two 'Get radiation pattern function' for implementing polarization. 2018.07.11 shlim.
Vector2D
AntennaArrayModel::GetRadiationPattern_polar (double vAngle, double hAngle, uint16_t antInd)
{
	//180716-jskim14-for cross polarization
	uint64_t antNum = m_vAntennaNum*m_hAntennaNum*m_polarNum;
	double pol=m_pol;
	if (antInd >= antNum/m_polarNum)
	{
		if (m_pol!=0) pol = -m_pol;
		else pol += M_PI/2;
	}
	//jskim14-end

 	double theta = vAngle;
    double phi = hAngle;
    //double theta_prime = acos(cos(phi)*sin(theta)*sin(m_beta)+cos(theta)*cos(m_beta));
	//180713-jskim14-revise above equation
	double theta_prime = acos(cos(m_beta)*cos(m_gamma)*cos(theta)+(sin(m_beta)*cos(m_gamma)*cos(phi-m_alpha)-sin(m_gamma)*sin(phi-m_alpha))*sin(theta));

    //std::complex<double> temp(cos(phi)*sin(theta)*cos(m_beta), sin(phi)*sin(theta));
    //180713-jskim14-revise above equation
	double real = cos(m_beta)*sin(theta)*cos(phi-m_alpha)-sin(m_beta)*cos(theta);
	double imag = cos(m_beta)*sin(m_gamma)*cos(theta)+(sin(m_beta)*sin(m_gamma)*cos(phi-m_alpha)+cos(m_gamma)*sin(phi-m_alpha))*sin(theta);
	std::complex<double> temp(real, imag);
    double phi_prime = std::arg(temp);
	//jskim14-end
    
    double cosPsi = (cos(m_beta)*cos(m_gamma)*sin(theta) - (sin(m_beta)*cos(m_gamma)*cos(phi-m_alpha)-sin(m_gamma)*
			sin(phi-m_alpha))*cos(theta))/ sqrt(1-pow(cos(m_beta)*cos(m_gamma)*cos(theta)+(sin(m_beta)*cos(m_gamma)*
			cos(phi-m_alpha)-sin(m_gamma)*sin(phi-m_alpha))*sin(theta),2));
    double sinPsi = (sin(m_beta)*cos(m_gamma)*sin(phi-m_alpha)+sin(m_gamma)*cos(phi-m_alpha))/sqrt(1-pow(cos(m_beta)*
			cos(m_gamma)*cos(theta)+(sin(m_beta)*cos(m_gamma)*cos(phi-m_alpha)-sin(m_gamma)*sin(phi-m_alpha))*sin(theta),2));
    
    double SLAv = 30;
    double theta3dB = 65; // in degrees
    double phi3dB = 65;
    double AMax = 30;
    double A_theta_double_db = (-1) * std::min(12*pow((RadiansToDegrees(theta_prime)-90 ) / theta3dB, 2),SLAv); //_double means double prime
	
    //if(RadiansToDegrees(phi_prime) > 180) {phi_prime = phi_prime-360;} 
    //180713-jskim14-revise above code
	if(RadiansToDegrees(phi_prime) > 180) {phi_prime = phi_prime-2*M_PI;} 
	//jskim14-end
    double A_phi_double_db = (-1) * std::min(12*pow(RadiansToDegrees(phi_prime)/ phi3dB, 2), AMax);
    
	//double A_double = (-1) * std::min((-1)*std::abs(A_theta_double+A_phi_double),AMax);
    //180713-jskim14-revise above equation and add conver to linear
	double A_double_db;
	A_double_db = (-1) * std::min((-1)*(A_theta_double_db + A_phi_double_db), AMax) + 8; //180716-jskim14=plus antenna gain 8 dBi
	//1807187-jskim14-for UE device
	Ptr<MmWaveUeNetDevice> ueDev = DynamicCast<MmWaveUeNetDevice>(m_netDevice);
	Ptr<McUeNetDevice> mcUeDev = DynamicCast<McUeNetDevice>(m_netDevice);
	if (ueDev || mcUeDev)
	{
		A_double_db = 8;
	}
	//jskim14-end
	double A_double = pow(10, A_double_db/10);
    
	//180713-jskim14-revise below equations-we don't need to calculate F_angle_double because A_double is equal to A_prime (A'=A'')
	// double F_theta_double = sqrt(A_double)*cos(pol); 
    // double F_phi_double = sqrt(A_double)*sin(pol);
    // double cosPsi_prime = cos(pol)*sin(theta_prime)+sin(pol)*sin(phi_prime)*cos(theta_prime) 
	// 						/ sqrt(1-pow(cos(pol)*cos(theta_prime)-sin(pol)*sin(phi_prime)*sin(theta_prime),2));
    // double sinPsi_prime = sin(pol)*cos(phi_prime) / sqrt(1-pow(cos(pol)*cos(theta_prime)-sin(pol)*sin(phi_prime)*sin(theta_prime),2));
    // double F_theta_prime = F_theta_double * cosPsi_prime - F_phi_double * sinPsi_prime;
    // double F_phi_prime = F_theta_double *sinPsi_prime + F_phi_double * cosPsi_prime;
    
	double F_theta_prime = sqrt(A_double) * cos(pol);
    double F_phi_prime = sqrt(A_double) * sin(pol);
	//jskim14-end

	NS_LOG_INFO("Total antenna elements=" << antNum << ", antenna index=" << antInd << ", polarization in degree=" << pol*180/M_PI);
 
	double F_theta = F_theta_prime*cosPsi - F_phi_prime*sinPsi;
    double F_phi = F_theta_prime*sinPsi + F_phi_prime*cosPsi; //180724-jskim14-bug fix, F_theta --> F_phi
    
    Vector2D radiationField(F_theta, F_phi);

    return radiationField;
}

double
AntennaArrayModel::GetRadiationPattern_nonpolar (double vAngle, double hAngle)
{
	NS_ASSERT_MSG(vAngle>=0&&vAngle<=180, "the vertical angle should be the range of [0,180]");
	NS_ASSERT_MSG(hAngle>=-180&&vAngle<=180, "the horizontal angle should be the range of [-180,180]");
	//for testing
	/*
	double A_EV = -1*std::min(12*pow((vAngle-90)/65,2),30.0);
	if(hAngle !=0)
	{
		double A_EH = -1*std::min(12*pow(hAngle/65,2),30.0);
		double A = -1*std::min(-1*A_EV-1*A_EH,30.0);
		return pow(10, A/10); //convert to linear;
	}
	else
	{
		return pow(10, A_EV/10); // convert to linear;
	}
	*/
	return 1;
}

Vector
AntennaArrayModel::GetAntennaLocation(uint8_t index, uint8_t* antennaNum)
{
	//assume the left bottom corner is (0,0,0), and the rectangular antenna array is on the y-z plane.
	Vector loc;
	loc.x = 0;
	loc.y = m_disH* (index % antennaNum[0]);
	loc.z = m_disV* (index / antennaNum[1]);
	return loc;
}

//180704-jskim14-new antenna location function
Vector
AntennaArrayModel::GetAntennaLocation(uint8_t index, uint8_t vAntennaNum, uint8_t hAntennaNum, uint8_t polarNum)
{
	//assume the left bottom corner is (0,0,0), and the rectangular antenna array is on the y-z plane.
	//assume the antenna index order (1.vertical, 2.hrizontal, 3.polar)
	//Vector location = m_netDevice->GetNode()->GetObject<MobilityModel>()->GetPosition();

	if(polarNum==1)
	{
		Vector loc;
		loc.x = 0;
		loc.y = m_disH* (index / vAntennaNum);
		loc.z = m_disV* (index % vAntennaNum);
		return loc;
	}
	else
	{
		if (index >= vAntennaNum*hAntennaNum)
		{
			index = index - vAntennaNum*hAntennaNum;
		}
		Vector loc;
		loc.x = 0;
		loc.y = m_disH * (index / vAntennaNum);
		loc.z = m_disV * (index % vAntennaNum);
		NS_LOG_INFO ("Antenna Index=" << (unsigned)index << " -> location: x=" << (double)loc.x << ", y=" << (double)loc.y << ", z=" << (double)loc.z);
		return loc;
	}
}
//jskim14

void
AntennaArrayModel::SetSector (uint8_t sector, uint8_t *antennaNum, double elevation)
{
	complexVector_t tempVector;
	double hAngle_radian = M_PI*(double)sector/(double)antennaNum[1]-0.5*M_PI;
	double vAngle_radian = elevation*M_PI/180;
	uint16_t size = antennaNum[0]*antennaNum[1];
	NS_LOG_INFO("Antenna size: " << (double)size << " horizontal: " << (double)antennaNum[1] << " vertical: " << (double)antennaNum[0]);
	double power = 1/sqrt(size);
	for(int ind=0; ind<size; ind++)
	{
		Vector loc = GetAntennaLocation(ind, antennaNum);
		double phase = -2*M_PI*(sin(vAngle_radian)*cos(hAngle_radian)*loc.x
							+ sin(vAngle_radian)*sin(hAngle_radian)*loc.y
							+ cos(vAngle_radian)*loc.z);
		tempVector.push_back(exp(std::complex<double>(0, phase))*power);
	}
	m_beamformingVector = tempVector;
}

//180702-jskim14-antenna parameters setting function
void
AntennaArrayModel::SetAntParams (uint8_t connectMode, uint8_t vAntNum, uint8_t hAntNum, uint8_t polarNum, uint8_t vTxruNum, uint8_t hTxruNum, Ptr<NetDevice> device)
{
	//NS_LOG_UNCOND("Antenna Array " << " jskim test " << (unsigned)connectMode << " " << (unsigned)(noAntennas/noHTxrus/noPolar));
	m_connectMode = connectMode;
	m_vAntennaNum = vAntNum;
	m_hAntennaNum = hAntNum;
	m_polarNum = polarNum;
	m_vTxruNum = vTxruNum;
	m_hTxruNum = hTxruNum;
	m_netDevice = device;
	SetAntennaWeightMatrix();
}
//jskim14-end

//180822-jskim14-digital beamforming vector setting
void
AntennaArrayModel::SetDigitalBeamformingVector()
{
	//TBD
}
//

//180705-jskim14-analog beamforming vector setting
void
AntennaArrayModel::SetAntennaWeightMatrix ()
{
	m_antennaWeightMat.clear();
	double beamNum = 0;
	uint16_t antNum = 0;
	//Vector location = m_netDevice->GetNode()->GetObject<MobilityModel>()->GetPosition();
	switch (m_connectMode)
	{
		//1-D full connection mode
		case 0:
		{
			beamNum = m_vTxruNum*m_hTxruNum*m_polarNum;
			antNum = m_vAntennaNum*m_hAntennaNum*m_polarNum;
			NS_LOG_INFO("1-D full connection mode: " << "# of beams=" << (double)beamNum << " # of antenna elements=" << (unsigned)antNum);	
			complex2DVector_t tempWeightMat(antNum, complexVector_t(beamNum, 0));
			for(int beamInd=0; beamInd<beamNum; beamInd++)
			{
				double elevation = 180/beamNum*(beamInd+1);
				double vAngle_radian = elevation*M_PI/180;
				NS_LOG_INFO("1-D full connection mode: " << "elevation angle: " << (double)elevation << " degree");	
				//complex2DVector_t tempVec(antNum, complexVector_t(beamNum, 0));
				for(int antInd=0; antInd<antNum; antInd++)
				{
					int startAntInd = m_vAntennaNum*(beamInd/m_vTxruNum);
					int endAntInd = startAntInd + (m_vAntennaNum-1);
					if(antInd>=startAntInd && antInd<=endAntInd)
					{
						Vector loc = GetAntennaLocation(antInd, m_vAntennaNum, m_hAntennaNum, m_polarNum);
						double phase = -2*M_PI*(cos(vAngle_radian)*(loc.z));
						double power = 1/sqrt(m_vAntennaNum);
						NS_LOG_INFO("1-D full connection mode: " << " antenna ind: " << (int)antInd << " antenna location: x=" << (double)loc.x << ", y=" << (double)loc.y << ", z=" << (double)loc.z);	
						tempWeightMat[antInd][beamInd] = exp(std::complex<double>(0, phase))*power;
						NS_LOG_INFO("Antenna weight=" << tempWeightMat[antInd][beamInd]);
					}
					else
					{
						tempWeightMat[antInd][beamInd] = 0;
					}
				}
				m_antennaWeightMat = tempWeightMat;
			}
			break;
		}
		//2-D full connection mode
		case 1:
		{
			double vBeamNum = m_vTxruNum;
			double hBeamNum = m_hTxruNum*m_polarNum;
			beamNum = vBeamNum*hBeamNum;
			antNum = m_vAntennaNum*m_hAntennaNum*m_polarNum;
			double azimuthRange = 120;
			//1807187-jskim14-for UE device
			Ptr<MmWaveUeNetDevice> ueDev = DynamicCast<MmWaveUeNetDevice>(m_netDevice);
			Ptr<McUeNetDevice> mcUeDev = DynamicCast<McUeNetDevice>(m_netDevice);
			if (ueDev || mcUeDev)
			{
				azimuthRange = 360;
			}
			//jskim14-end
			NS_LOG_INFO("2-D full connection mode: " << "# of beams=" << (double)beamNum << " # of antenna elements=" << (unsigned)antNum);	
			complex2DVector_t tempWeightMat(antNum, complexVector_t(beamNum, 0));
			for(int vBeamInd=0; vBeamInd<vBeamNum; vBeamInd++)
			{
				double elevation = 180/vBeamNum*(vBeamInd+1);
				double vAngle_radian = elevation*M_PI/180;
				for(int hBeamInd=0; hBeamInd<hBeamNum; hBeamInd++)
				{
					double azimuth = azimuthRange/hBeamNum*(hBeamInd+1)-(azimuthRange/2);
					double hAngle_radian = azimuth*M_PI/180;
					NS_LOG_INFO("2-D full connection mode: " << "elevation angle=" << (double)elevation << " degree, azimuth angle=" << (double)azimuth);	
					for(int antInd=0; antInd<antNum; antInd++)
					{
						int pInd;
						if (antInd >= m_vAntennaNum*m_hAntennaNum) pInd=1;		
						else pInd= 2;
						Vector loc = GetAntennaLocation(antInd, m_vAntennaNum, m_hAntennaNum, m_polarNum);
						double vPhase = -2*M_PI*(cos(vAngle_radian)*(loc.z));
						double vPower = 1/sqrt(m_vAntennaNum);
						double hPhase = -2*M_PI*(sin(hAngle_radian)*(loc.y));
						double hPower = 1/sqrt(m_hAntennaNum*m_polarNum);
						double real = sin(m_gamma)*cos(vAngle_radian)*sin(hAngle_radian-m_alpha)+cos(m_gamma)*(cos(m_beta)*sin(vAngle_radian)-sin(m_beta)*cos(vAngle_radian)*cos(hAngle_radian-m_alpha));
						double imag = sin(m_gamma)*cos(hAngle_radian-m_alpha)+sin(m_beta)*cos(m_gamma)*sin(hAngle_radian-m_alpha);
						std::complex<double> temp = std::complex<double>(real, imag);
						double psi = std::arg(temp);
						double pPhase = -(pInd-1)*psi;					
						NS_LOG_INFO("2-D full connection mode: " << " antenna ind: " << (int)antInd << " antenna location: x=" << (double)loc.x << ", y=" << (double)loc.y << ", z=" << (double)loc.z);	
						tempWeightMat[antInd][hBeamInd*m_vTxruNum+vBeamInd] = exp(std::complex<double>(0, vPhase))*vPower*exp(std::complex<double>(0, hPhase))*hPower*exp(std::complex<double>(0,pPhase));
						NS_LOG_INFO("Antenna weight=" << tempWeightMat[antInd][hBeamInd*m_vTxruNum+vBeamInd]);
					}
				}
				m_antennaWeightMat = tempWeightMat;
			}
			break;
		}
		case 2:
		case 3:
		default:
			NS_FATAL_ERROR ("Do not support this Antenna connection mode !");
	}
}
//jskim14-end

//180709-jskim14-add get antenna weight matrix
complex2DVector_t
AntennaArrayModel::GetAntennaWeightMatrix()
{
	return m_antennaWeightMat;
}
//jskim14-end

//180715-jskim14-add set antenna rotation
void
AntennaArrayModel::SetAntennaRotation (double alpha, double beta, double gamma, double pol) //input is degree
{
	m_alpha = alpha*M_PI/180;
	m_beta = beta*M_PI/180;
	m_gamma = gamma*M_PI/180;
	m_pol = pol*M_PI/180;
}
//jskim14-end

//180717-jskim14-add get net device
Ptr<NetDevice>
AntennaArrayModel::GetNetDevice ()
{
	return m_netDevice;
}
//jskim14-end

//180718-jskim14-add get number of TXRUs
Vector
AntennaArrayModel::GetTxruNum ()
{
	Vector txrus;
	txrus.x = m_vTxruNum;
	txrus.y = m_hTxruNum;
	txrus.z = m_polarNum;
	return txrus;
}
//jskim14-end

} /* namespace ns3 */
