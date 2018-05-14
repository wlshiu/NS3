/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c)  2012 The Provost, Fellows and Scholars of the 
 * College of the Holy and Undivided Trinity of Queen Elizabeth near Dublin. 
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
 * Author: Xiuchao Wu <xw2@cs.ucc.ie>
 */

/**************************************************************
*
* This script will create an TCP client and server application at both sides to generate two-way traffics.
* 
*                                                                            P2P
*                                                       ------------ ONU1 ---------- TCP sink/sender 1
*                       P2P              P2P (CN)      /
*  TCP sender/sink 1 ---------- Gateway ------------ OLT
*                                |                     \                     P2P
*                        P2P     |                      ------------ ONU2 ---------- TCP sink/sender 2
*   TCP sender/sink 2 ------------
**************************************************************/


#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/object-factory.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"

#include "ns3/xgpon-module.h"

//Stats module for tracing
#include "ns3/stats-module.h"





#define APP_START_TIME   0
#define APP_STOP_TIME   100
#define SIM_STOP_TIME   (APP_STOP_TIME + 1)

#define TCP_PKT_SIZE    1000



//2 ONUs with TCP traffics in both directions 
//The data rate of a TCP connection is detrmined by RTT (window = 64KB).
#define ONU_NUM          2  


#define ONU_TX_TRACE_FILE_BASE "data/xgpon/OnuTraceTx"
#define ONU_RX_TRACE_FILE_BASE "data/xgpon/OnuTraceRx"
#define ONU_TRACE_FILE "data/xgpon/OnuTraceTest.txt"
#define OLT_TX_TRACE_FILE "data/xgpon/OltTxTraceTest.txt"
#define OLT_RX_TRACE_FILE "data/xgpon/OltRxTraceTest.txt"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("XgponSimulationSpeedFanTcp");

void
TxTrace (std::string path, Ptr<const XgponDsFrame> pkt, Time t)
{
  std::ofstream myfile;
  myfile.open (path.c_str(), std::ios::app);
  myfile<<"Tx Time: "<<t<<"\n";
  pkt->Print(myfile);
  myfile<<"\n\n";
  myfile.close();
}

void
RxTrace (std::string path, Ptr<const Packet> pkt, Time t)
{
  std::ofstream myfile;
  myfile.open (path.c_str(), std::ios::app);
  myfile<<"Rx Time: "<<t<<"\n";
  pkt->Print(myfile);
  myfile<<"\n\n";
  myfile.close();
}



void
DeviceStatisticsTrace (const XgponNetDeviceStatistics& stat)
{
  static uint64_t time2print = 1000000000;    //1,000,000,000 nanoseconds per second.

  if(stat.m_currentTime > time2print) 
  {
    std::cout << (stat.m_currentTime / 1000000000L) << " seconds have been simulated.";
    std::cout << "   DS-BYTES: " << stat.m_passToXgponBytes;
    std::cout << ";   US-BYTES: " << stat.m_rxFromXgponBytes;
    std::cout << ";   FROM-CN-DS-BYTES: " << stat.m_rxFromUpperLayerBytes;
    std::cout << ";   DROPPED-DS-BYTES: " << stat.m_overallQueueDropBytes;
    std::cout << std::endl;

    time2print += 1000000000;
  }
}



int 
main (int argc, char *argv[])
{
  bool p_verbose = false;
  uint16_t p_nOnus = ONU_NUM;  
  uint16_t p_appStartTime = APP_START_TIME;
  uint16_t p_appStopTime = APP_STOP_TIME;
  uint16_t p_simStopTime = SIM_STOP_TIME;
  uint16_t p_pktSize = TCP_PKT_SIZE;
  


  CommandLine cmd;
  cmd.AddValue ("verbose", "Tell application to log if true", p_verbose);
  cmd.AddValue ("onus", "the number of onus", p_nOnus);
  cmd.AddValue ("astarttime", "the start time of applications", p_appStartTime);
  cmd.AddValue ("astoptime", "the stop time of applications", p_appStopTime);
  cmd.AddValue ("sstoptime", "the stop time of whole simulation", p_simStopTime);
  cmd.AddValue ("pktsize", "the TCP packet size (byte)", p_pktSize);



  cmd.Parse (argc,argv);

  if(p_verbose)
  {
    LogComponentEnable ("BulkSendApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  }
  //LogComponentEnable ("XgponChannel", LOG_LEVEL_FUNCTION);
  //LogComponentEnable ("XgponOltNetDevice", LOG_LEVEL_FUNCTION);
  //LogComponentEnable ("XgponOnuNetDevice", LOG_LEVEL_FUNCTION);
  //LogComponentEnable ("XgponXgemEngine", LOG_LEVEL_FUNCTION);
  //LogComponentEnable ("XgponOltXgemEngine", LOG_LEVEL_FUNCTION);
  //LogComponentEnable ("XgponOltFramingEngine", LOG_LEVEL_FUNCTION);
  //LogComponentEnable ("XgponPhy", LOG_LEVEL_FUNCTION);
  //LogComponentEnable ("XgponOnuXgemEngine", LOG_LEVEL_LOGIC);
  //LogComponentEnable ("XgponOnuDbaEngine", LOG_LEVEL_LOGIC);
  //LogComponentEnable ("XgponOnuFramingEngine", LOG_LEVEL_LOGIC);
  //LogComponentEnable ("XgponOnuPhy", LOG_LEVEL_LOGIC);
  //LogComponentEnable ("XgponOltSimpleDsScheduler", LOG_LEVEL_LOGIC);
  //LogComponentEnable ("XgponOltDbaEngineRoundRobin", LOG_LEVEL_LOGIC);
  //LogComponentEnable ("XgponOltDsSchedulerRoundRobin", LOG_LEVEL_LOGIC);

  Packet::EnablePrinting ();

  std::cout << "number of ONUs: " << p_nOnus << std::endl;



  //////////////////////////////////////////Create all nodes and organize into the corresponding containers for installing network devices.
  ///////the ONUs, OLT nodes, and container for all xgpon nodes
  NodeContainer oltNode, onuNodes, xgponNodes;
  oltNode.Create (1);
  onuNodes.Create (p_nOnus);
  xgponNodes.Add(oltNode.Get(0));
  for(int i=0; i<p_nOnus; i++) { xgponNodes.Add (onuNodes.Get(i)); }


  ///////the gateway node, OLT node, and container for the link to simulate Internet's core network
  NodeContainer gatewayNode, cnNodes;
  gatewayNode.Create (1);
  cnNodes.Add(oltNode.Get(0));
  cnNodes.Add(gatewayNode.Get(0));


  ///////the end hosts at both sides of the networks  
  NodeContainer leftServerNode, leftClientNode, rightNodes;
  NodeContainer leftServerLinkNodes, leftClientLinkNodes;
  NodeContainer rightLinkNodes[p_nOnus];
  leftServerNode.Create (1);
  leftClientNode.Create (1);
  leftServerLinkNodes.Add(leftServerNode.Get(0));
  leftServerLinkNodes.Add(gatewayNode.Get(0));
  leftClientLinkNodes.Add(leftClientNode.Get(0));
  leftClientLinkNodes.Add(gatewayNode.Get(0));

  rightNodes.Create (p_nOnus);
  for(int i=0; i<p_nOnus; i++) 
  { 
    rightLinkNodes[i].Add(rightNodes.Get(i));
    rightLinkNodes[i].Add(onuNodes.Get(i));
  }









  /////////////////////////////////////////Create all links used to connect the above nodes
  ///////Xgpon network configuration through XgponHelper
  XgponHelper xgponHelper;
  XgponConfigDb& xgponConfigDb = xgponHelper.GetConfigDb ( );

  xgponConfigDb.SetOltNetmaskLen (16);
  xgponConfigDb.SetOnuNetmaskLen (24);
  xgponConfigDb.SetIpAddressFirstByteForXgpon (10);
  xgponConfigDb.SetIpAddressFirstByteForOnus (172);


  //other configuration related information 
  Config::SetDefault("ns3::XgponOltDbaEngineRoundRobin::MaxServiceSize", UintegerValue(1000));  //1000 words
  Config::SetDefault("ns3::XgponOltDsSchedulerRoundRobin::MaxServiceSize", UintegerValue(10000));  //10K bytes
  Config::SetDefault("ns3::XgponOnuUsSchedulerRoundRobin::MaxServiceSize", UintegerValue(4000));  //4K bytes


  //Set TypeId String for object factories through XgponConfigDb before the following call.
  //initialize object factories
  xgponHelper.InitializeObjectFactories ( );

  //configuration through object factory
  xgponHelper.SetQueueAttribute ("MaxBytes", UintegerValue(50000));  //queue size is 50KBytes

  //install xgpon network devices
  NetDeviceContainer xgponDevices = xgponHelper.Install (xgponNodes);





  ///////Internet core network through PointToPointHelper
  PointToPointHelper p2pHelper;

  p2pHelper.SetDeviceAttribute ("DataRate", StringValue ("20000Mbps"));
  p2pHelper.SetChannelAttribute ("Delay", StringValue ("10ms"));
  p2pHelper.SetQueue ("ns3::DropTailQueue", "MaxPackets", UintegerValue (2000));

  NetDeviceContainer cnDevices = p2pHelper.Install (cnNodes);


  ///////access links for end hosts through PointToPointHelper
  p2pHelper.SetDeviceAttribute ("DataRate", StringValue ("20000Mbps"));
  p2pHelper.SetChannelAttribute ("Delay", StringValue ("2ms"));
  p2pHelper.SetQueue ("ns3::DropTailQueue", "MaxPackets", UintegerValue (2000));

  NetDeviceContainer leftServerLinkDevices, leftClientLinkDevices;
  leftServerLinkDevices = p2pHelper.Install (leftServerLinkNodes);
  leftClientLinkDevices = p2pHelper.Install (leftClientLinkNodes);


  p2pHelper.SetDeviceAttribute ("DataRate", StringValue ("20000Mbps"));
  p2pHelper.SetChannelAttribute ("Delay", StringValue ("2ms"));
  p2pHelper.SetQueue ("ns3::DropTailQueue", "MaxPackets", UintegerValue (100));

  NetDeviceContainer rightLinkDevices[p_nOnus];
  for(int i=0; i<p_nOnus; i++) 
  { 
    rightLinkDevices[i] = p2pHelper.Install (rightLinkNodes[i]);
  }






  /////////////////////////////////////////////////////////////////////////////////install internet protocol stack
  InternetStackHelper stack;
  stack.Install (xgponNodes);
  stack.Install (leftServerNode);
  stack.Install (leftClientNode);
  stack.Install (rightNodes);
  stack.Install (gatewayNode);






  ///////////////////////////////////////////////////////////////////////////////Assign IP addresses to all interfaces of all nodes
  Ipv4AddressHelper addressHelper;

  ///////////Assign IP addresses to core network nodes (point-to-point link)
  std::string cnIpbase = xgponHelper.GetIpAddressBase (150, 0, 24);
  std::string cnNetmask = xgponHelper.GetIpAddressNetmask (24); 
  addressHelper.SetBase (cnIpbase.c_str(), cnNetmask.c_str());
  Ipv4InterfaceContainer cnInterfaces = addressHelper.Assign (cnDevices);
  if(p_verbose)
  {
    Ipv4Address tmpAddr = cnInterfaces.GetAddress(0);
    std::cout << "OLT Internet Interface's IP Address: ";
    tmpAddr.Print(std::cout);
    std::cout << std::endl;

    tmpAddr = cnInterfaces.GetAddress(1);
    std::cout << "Internet Gateway's IP Address: ";
    tmpAddr.Print(std::cout);
    std::cout << std::endl;
  }

  
  /////////Assign IP addresses to end hosts at the left side (point-to-point link)
  Ipv4InterfaceContainer leftServerLinkInterfaces, leftClientLinkInterfaces;

  std::string leftServerIpbase = xgponHelper.GetIpAddressBase (160, 1, 24);
  std::string leftServerNetmask = xgponHelper.GetIpAddressNetmask (24); 
  addressHelper.SetBase (leftServerIpbase.c_str(), leftServerNetmask.c_str());
  leftServerLinkInterfaces = addressHelper.Assign (leftServerLinkDevices);

  std::string leftClientIpbase = xgponHelper.GetIpAddressBase (160, 2, 24);
  std::string leftClientNetmask = xgponHelper.GetIpAddressNetmask (24); 
  addressHelper.SetBase (leftClientIpbase.c_str(), leftClientNetmask.c_str());
  leftClientLinkInterfaces = addressHelper.Assign (leftClientLinkDevices);






  ///////////Assign IP addresses to OLT and ONU (for xgpon network devices)
  Ptr<XgponOltNetDevice> tmpDevice = DynamicCast<XgponOltNetDevice, NetDevice> (xgponDevices.Get(0));
  std::string xgponIpbase = xgponHelper.GetXgponIpAddressBase ( );
  std::string xgponNetmask = xgponHelper.GetOltAddressNetmask();
  addressHelper.SetBase (xgponIpbase.c_str(), xgponNetmask.c_str());

  Ipv4InterfaceContainer xgponInterfaces = addressHelper.Assign (xgponDevices);
  for(int i=0; i<(p_nOnus+1);i++)
  {
    Ipv4Address addr = xgponInterfaces.GetAddress(i);
    Ptr<XgponNetDevice> tmpDevice = DynamicCast<XgponNetDevice, NetDevice> (xgponDevices.Get(i));
    tmpDevice->SetAddress (addr);

    if(p_verbose)
    {
      if(i==0) std::cout << "OLT IP Address: ";
      else std::cout << "ONU " << (i-1) <<" IP Address: ";
      addr.Print(std::cout);
      std::cout << std::endl;
    }
  }



  /////////Assign IP addresses to end hosts at the right side (point-to-point link)
  Ipv4InterfaceContainer rightLinkInterfaces[p_nOnus];

  for(int i=0; i<p_nOnus; i++) 
  {
    Ptr<XgponOnuNetDevice> tmpDevice = DynamicCast<XgponOnuNetDevice, NetDevice> (xgponDevices.Get(i+1));
    std::string onuIpbase = xgponHelper.GetOnuIpAddressBase (tmpDevice);
    std::string onuNetmask = xgponHelper.GetOnuAddressNetmask();     
    addressHelper.SetBase (onuIpbase.c_str(), onuNetmask.c_str());

    rightLinkInterfaces[i] = addressHelper.Assign (rightLinkDevices[i]);
    if(p_verbose)
    {
      Ipv4Address addr = rightLinkInterfaces[i].GetAddress(0);
      std::cout << "Right Node " << i <<" IP Address: ";
      addr.Print(std::cout);
      std::cout << std::endl;

      addr = rightLinkInterfaces[i].GetAddress(1);
      std::cout << "IP Address at the Corresponding ONU: ";
      addr.Print(std::cout);
      std::cout << std::endl;
    }
  }




  //////////////////////////////////////////////////////////////////////////////////////Add OMCI Channels
  //set attributes (QoS parameters, etc.) for connections to be added as OMCI channel
  //we need get the address before setting OMCI channel.
  //for(int i=0; i<p_nOnus; i++) 
  //{ 
  //  Ptr<XgponOltNetDevice> oltDevice = DynamicCast<XgponOltNetDevice, NetDevice> (xgponDevices.Get(0));
  //  Ptr<XgponOnuNetDevice> onuDevice = DynamicCast<XgponOnuNetDevice, NetDevice> (xgponDevices.Get(i+1));
  //  xgponHelper.AddOmciConnectionsForOnu (onuDevice, oltDevice); 
  //}




  /////////////////////////////////////////////////////////////////////add xgem ports for end hosts connected to ONUs
  for(int i=0; i<p_nOnus; i++) 
  {
    Address addr = rightLinkInterfaces[i].GetAddress(0);

    Ptr<XgponOltNetDevice> oltDevice = DynamicCast<XgponOltNetDevice, NetDevice> (xgponDevices.Get(0));
    Ptr<XgponOnuNetDevice> onuDevice = DynamicCast<XgponOnuNetDevice, NetDevice> (xgponDevices.Get(i+1));

    uint16_t allocId = xgponHelper.AddOneTcontForOnu (onuDevice, oltDevice);
    uint16_t upPortId = xgponHelper.AddOneUpstreamConnectionForOnu (onuDevice, oltDevice, allocId, addr);
    uint16_t downPortId = xgponHelper.AddOneDownstreamConnectionForOnu (onuDevice, oltDevice, addr); 

    if(p_verbose) 
    {
      std::cout << "ONU-ID = "<<onuDevice->GetOnuId() << ";   ALLOC-ID = " << allocId << ";   UP-PORT-ID = " << upPortId << ";   DOWN-PORT-ID = " << downPortId << std::endl;
    }    
  }




  //////////////////////////////////////////////////////////Populate routing tables for all nodes
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();




  //////////////////////////////////////////////////////////////////////////Configure Traffics
  uint16_t leftServerPort=9000;      //TCP server port
  uint16_t rightServerPort=9001;     //TCP server port

  Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(2000000));
  Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(2000000));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(p_pktSize));

  PacketSinkHelper leftSink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), leftServerPort));
  ApplicationContainer leftServer = leftSink.Install (leftServerNode);
  leftServer.Start (Seconds (0));
  leftServer.Stop (Seconds (p_appStopTime));

  PacketSinkHelper rightSink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), rightServerPort));
  ApplicationContainer rightServers = rightSink.Install (rightNodes);
  rightServers.Start (Seconds (0));
  rightServers.Stop (Seconds (p_appStopTime));

  for(int i=0; i<p_nOnus; i++)
  {
    BulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress (rightLinkInterfaces[i].GetAddress (0), rightServerPort));
    source.SetAttribute ("MaxBytes", UintegerValue (0));  // Set the amount of data to send in bytes.  Zero is unlimited.
    ApplicationContainer sourceApps = source.Install (leftClientNode.Get (0));
    sourceApps.Start (Seconds(p_appStartTime + i * 0.001));
    sourceApps.Stop (Seconds(p_appStopTime));


    BulkSendHelper source2 ("ns3::TcpSocketFactory",
                         InetSocketAddress (leftServerLinkInterfaces.GetAddress (0), leftServerPort));
    source2.SetAttribute ("MaxBytes", UintegerValue (0));  // Set the amount of data to send in bytes.  Zero is unlimited.
    ApplicationContainer sourceApps2 = source2.Install (rightNodes.Get (i));
    sourceApps2.Start (Seconds(p_appStartTime + i * 0.001));
    sourceApps2.Stop (Seconds(p_appStopTime));
  }

  if(p_verbose)
  {
    xgponHelper.EnableAsciiAll("xgpon-simulation-speed-fan-tcp-ascii");
    xgponHelper.EnablePcapAll("xgpon-simulation-speed-fan-tcp-pcap");
  }


  // Tracing stuff
  Ptr<XgponOltNetDevice> oltDevice = DynamicCast<XgponOltNetDevice, NetDevice> (xgponDevices.Get(0));
  //oltDevice->TraceConnect ("PhyRxEnd", "test4", MakeCallback(&RxTrace));
  //oltDevice->TraceConnect ("PhyTxEnd", OLT_TX_TRACE_FILE, MakeCallback(&TxTrace));
  oltDevice->TraceConnectWithoutContext ("DeviceStatistics", MakeCallback(&DeviceStatisticsTrace));

 

  std::cout<<std::endl;
  Simulator::Stop(Seconds(p_simStopTime));
  Simulator::Run ();
  Simulator::Destroy ();
  std::cout<<std::endl;

  Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (leftServer.Get (0));
  std::cout << "Total Bytes Received (upstream): " << sink1->GetTotalRx () << std::endl;  

  for(int i=0; i<p_nOnus; i++)
  {
    Ptr<PacketSink> sink2 = DynamicCast<PacketSink> (rightServers.Get (i));
    std::cout << "Total Bytes Received: " << sink2->GetTotalRx () << std::endl;  
  }
  
  return 0;
}


