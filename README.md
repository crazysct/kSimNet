# 5G Network Simulator (5G K-SimNet)
5G K-SimNet is implemented based on [ns-3](https://www.nsnam.org "ns-3 Website") and [ns-3 mmWave module](https://github.com/nyuwireless/ns3-mmwave "mmWave github"). ns-3 is one of representative network simulators in communication and computer network research. ns-3 models network entities and structs complete set of the network protocol stack, so it can predict network behaviors more accurately. However, conventional ns-3 lacks of consideration about virtualization effects or cutting edge of software-defined network (SDN) technologies, or support of multi-connectivity of 5G networks. For simulating 5G network system, it is necessary to see virtualized effects or SDN functions, or test multi-connectivity scenario for deployment of 5G network. Our 5G K-SimNet includes OpenFlow controller, OpenFlow switches, modules for evaluating virtualization effects and also includes mmWave module for evaluating multi-connectivity of 5G network. Users who want to simulator 5G network system can evaluate their own network system topology with our 5G K-SimNet.

## Features
1. Performance metrics
- User throughput: User throughput is an important metric for network simulators, while it can show general network performance for each user device.
- User round trip time: User RTT can show timely change of packet transfer performance. If RTT increase/decrease sharply in short period of time, there might be special reasons such as VNF scaling, and so on.
- TCP congestion window size: With TCP congestion window size, user can verify the operation of network simulator and changes of RTT or throughput.
- Scaling delay: Scaling delay means additional delay components compared to non-virtualized core networks. Additional delays depending on where network operators put VNFs (VNF topology) can be evaluated.
- SDN switch throughput per port: With SDN controller and switches, user can engineer traffic flowing the simulation networks. SDN switch throughput per port can show results of traffic engineering.

2. Simulation parameters for configuration

2.1 General parameters 
- X2 interface settings (src/lte/helper$ point-to-point-epc-helper.cc )
-- X2 link data rate (m_x2LinkDataRate)
-- X2 link delay (m_x2LinkDelay)
-- MTU of X2 link (m_x2LinkMtu)
- S1AP interface settings (src/lte/helper$ point-to-point-epc-helper.cc )
-- S1AP link data rate (m_s1apLinkDataRate)
-- S1AP link delay (m_s1apLinkDelay)
-- MTU of S1AP link (m_s1apLinkMtu)
- S1-U interface settings (src/lte/helper$ point-to-point-epc-helper.cc )
-- S1-U link data rate (m_s1uLinkDataRate)
-- S1-U link delay (m_s1uLinkDelay)
-- MTU of S1-U link (m_s1uLinkMtu)

## Authors

## Acknowledgement
This work was supported by "The Cross-Ministry Giga KOREA Project" grant funded by the Korea government (MSIT) (No. GK18S0400, Research and Development of Open 5G Reference Model).
