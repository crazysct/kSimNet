#ifndef VIRT_5GC_VM_H
#define VIRT_5GC_VM_H

#include "ns3/object.h"

namespace ns3 {

	class Virt5gcVm : public Object
	{
		public:
			static TypeId GetTypeId (void);
			Virt5gcVm (int vmId, int torId, int pmId);
			Virt5gcVm (const Virt5gcVm&);
			//bool operator<(const Virt5gcVm& rhs) const; 
			bool operator==(const Virt5gcVm& rhs) const;

			void SetId (int vmId);
			void SetNodeId (int nodeId);	// which node is run on this VM
			void SetCpuInfo (int size, int util);
			void SetMemInfo (int size, int util);
			void SetDiskInfo (int size, int util);
			void SetBwInfo (int size, int util);

			void ChangeCpuLoad (int util);
			void ChangeMemLoad (int util);
			void ChangeDiskLoad (int util);

			void ChangeToR (int tor);
			void ChangePm (int pmId);
	
			int GetVmId (void);
			int GetToRId (void);
			int GetPmId (void);
			int GetNodeId (void);
			std::pair<int, int> GetCpuInfo (void);
			std::pair<int, int> GetMemInfo (void);
			std::pair<int, int> GetDiskInfo (void);
			std::pair<int, int> GetBwInfo (void);


		private:
			int id;
			int ToR;
			int pm;
			int node;
			int cpuSize;
			int cpuUtil;
			int memSize;
			int memUtil;
			int diskSize;
			int diskUtil;
			int bwSize;
			int bwUtil;

	};
};

#endif
