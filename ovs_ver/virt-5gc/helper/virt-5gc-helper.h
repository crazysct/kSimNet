/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef VIRT_5GC_HELPER_H
#define VIRT_5GC_HELPER_H

#include "ns3/virt-5gc.h"

namespace ns3 {

	class Virt5gcHelper {

		public:
			Virt5gcHelper();
			void SetTopoFile (const std::string fileName);
			void SetInputFile (const std::string fileName);
			void SetFileType (const std::string fileType);
			Ptr<Virt5gc> GetVirt5gc();

		private:
			Ptr<Virt5gc> m_inputModel;
			std::string m_topoFile;
			std::string m_inputFile;
			std::string m_fileType;
	};

}

#endif /* VIRT_5GC_HELPER_H */

