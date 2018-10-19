/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "virt-5gc-helper.h"
#include "ns3/object.h"
#include "ns3/log.h"

namespace ns3 {

	NS_LOG_COMPONENT_DEFINE ("Virt5gcHelper");

	Virt5gcHelper::Virt5gcHelper()
	{
		m_inputModel = 0;
	}

	void
	Virt5gcHelper::SetTopoFile (const std::string fileName)
	{
		m_topoFile = fileName;
	}

	void
	Virt5gcHelper::SetInputFile (const std::string fileName)
	{
		m_inputFile = fileName;
	}

	void
	Virt5gcHelper::SetFileType (const std::string fileType)
	{
		m_fileType = fileType;
	}

	Ptr<Virt5gc>
	Virt5gcHelper::GetVirt5gc ()
	{
		NS_ASSERT_MSG(!m_fileType.empty(), "Missing File Type");
		NS_ASSERT_MSG(!m_topoFile.empty(), "Missing Topology File");
		NS_ASSERT_MSG(!m_inputFile.empty(), "Missing Input File");

		if (m_fileType == "Virt5gc")
		{
			NS_LOG_INFO("Creating Lte5g formated data input.");
			m_inputModel = CreateObject<Virt5gc> ();
		}
		else
		{
			NS_ASSERT_MSG(false, "Wrong (unknown) File Type");
		}

		m_inputModel->SetInputFile(m_inputFile);
		m_inputModel->SetTopoFile(m_topoFile);

		return m_inputModel;

	}


}

