/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef VIRT_5GC_H
#define VIRT_5GC_H

#include <list>

#include "ns3/object.h"
#include "ns3/node-container.h"
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/lte-module.h"
#include "ns3/position-allocator.h"
#include "ns3/mobility-module.h"
#include "ns3/trace-helper.h"
#include "ns3/callback.h"
#include "ns3/traced-value.h"
#include "ns3/core-module.h"
#include "ns3/ovs-point-to-point-epc-helper.h"

#include <math.h>
#include <iostream>
#include <chrono>
#include <random>

#include "virt-5gc-node.h"
#include "virt-5gc-vm.h"

namespace ns3 {

	class Virt5gc : public Object
	{
		public:

			static TypeId GetTypeId(void);
			Virt5gc();
			void SetInputFile (const std::string &fileName); // Read a VM information file
			void SetTopoFile (const std::string &fileName); // Read a Topology file
			std::string GetInputFile (void); // Get Vm information
			std::string GetTopoFile (void); // Get Topology information (ue-enb mapping or node's location
			
			void InitVirt5gc (void);
			NodeContainer GetEnbNodes (void);
			NodeContainer GetUeNodes (void);
			Ptr<LteHelper> GetLteHelper (void);
			//Ptr<PointToPointEpcHelper> GetEpcHelper (void);
			Ptr<OvsPointToPointEpcHelper> GetEpcHelper (void);	
			
			void Read (void);
			void ReadVm(void);
			void AllocNodesPosition(std::vector<std::pair<int, int>> ueCoords, std::vector<std::pair<int, int>> enbCoords);
			std::list<std::pair<int, int>> GetNodeMap (void); // return the mapping btw. UE nodes and Enb Nodes
			std::list<Virt5gcNode> GetNodesList (void);
			NetDeviceContainer GetEnbDevs (void);
			NetDeviceContainer GetUeDevs (void);
			std::list<Virt5gcVm> GetVmList (void);

			void DynamicLoadInit (double std);
			void DynamicLoad (void);
			void SetMigrationRate (double scaleIn, double scaleOut);
			void SetAllocationDelay (double delay);
			void Scaling (void);
			double scaleIn(std::list<Virt5gcVm*> *vms, std::pair<int, int> cpuInfo, std::pair<int, int> memInfo, std::pair<int, int> diskInfo);
			double scaleOut(std::list<Virt5gcVm*> *vms, std::pair<int, int> cpuInfo, std::pair<int, int> memInfo, std::pair<int, int> diskInfo);	
			//double memMigration(std::list<Virt5gcVm*> *vms, int capa, int load, bool in);
			double ScalingDelay (bool in, int migratedLoad, int bw, int mem);
			std::list<Virt5gcVm*> GetNodeVms(std::list<int> vms);

			TracedCallback<uint32_t> m_scalingTrace;
		private:
			std::string m_inputFile;
			std::string m_topoFile;

			std::list<Virt5gcNode> nodeList;
			std::list<std::pair<int, int>> nodeMapping;
			NodeContainer enbNodes;
			NodeContainer ueNodes;
			NetDeviceContainer enbDevs;
			NetDeviceContainer ueDevs;
			Ptr<LteHelper> lteHelper;
			//Ptr<PointToPointEpcHelper> epcHelper;
			Ptr<OvsPointToPointEpcHelper> epcHelper;
			std::list<Virt5gcVm> vmList;
			std::list<std::pair<int, int>> vm_nodeList;

			int pgwN;
			int mmeN;
			int enbN;
			int ueN;
			int totAttr;
			int loadStd;
			int mmeVmN;
			int pgwVmN;

			double scaleInRate;
			double scaleOutRate;
			double allocDelay;
			Ptr<OutputStreamWrapper> loadStream;
			Ptr<OutputStreamWrapper> scalingStream;

			static const char COMMENT_HEADER = '#';

			std::istream& getline (std::istream& is, std::string &str);

	};

};

#endif /* VIRT_5GC_H */

