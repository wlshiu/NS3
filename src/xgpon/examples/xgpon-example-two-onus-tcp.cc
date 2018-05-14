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
* This script will create an UDP echo client and server application
* running in the OLT and the ONU respectively. These ONUs and the OLT will be in
* the same subnet
*                                                            P2P
*                                      ------------ ONU1 ---------- TCP sink/sender 1
*                        P2P          /
*  TCP sender/sink 1 ---------- OLT ------------
*                                |    \                     P2P
*                                |     ------------ ONU2 --------- (TCP sink/sender 2)
*  (TCP sender/sink 2)------------
**************************************************************/


#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/object-factory.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"

#include "ns3/xgpon-helper.h"
#include "ns3/xgpon-config-db.h"

#include "ns3/xgpon-channel.h"
#include "ns3/xgpon-onu-net-device.h"
#include "ns3/xgpon-olt-net-device.h"


#include "ns3/xgpon-module.h"



//Stats module for tracing
#include "ns3/stats-module.h"



#define APP_START 0
#define APP_STOP 10
#define SIM_STOP 11

#define ONU_NUM 2
//2 ONUs with TCP traffics in both directions 

#define ONU_TX_TRACE_FILE_BASE "data/xgpon/OnuTraceTx"
#define ONU_RX_TRACE_FILE_BASE "data/xgpon/OnuTraceRx"
#define ONU_TRACE_FILE "data/xgpon/OnuTraceTest.txt"
#define OLT_TX_TRACE_FILE "data/xgpon/OltTxTraceTest.txt"
#define OLT_RX_TRACE_FILE "data/xgpon/OltRxTraceTest.txt"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("XgponExampleTwoOnusTcp");

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
  bool verbose = true;

  CommandLine cmd;
  cmd.AddValue ("verbose", "Tell application to log if true", verbose);

  cmd.Parse (argc,argv);

  LogComponentEnable ("BulkSendApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
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





  //////////////////////////////////////////Create the ONUs and OLT nodes
  NodeContainer oltNode, onuNodes;
  uint16_t nOnus = ONU_NUM;  //Number Of Onus in the PON
  oltNode.Create (1);
  onuNodes.Create (nOnus);

  NodeContainer xgponNodes;
  xgponNodes.Add(oltNode.Get(0));
  for(int i=0; i<nOnus; i++) { xgponNodes.Add (onuNodes.Get(i)); }



  //////////////////////////////////////////////////////////////////////////XgponHelper
  XgponHelper xgponHelper;
  XgponConfigDb& xgponConfigDb = xgponHelper.GetConfigDb ( );

  xgponConfigDb.SetOltNetmaskLen (16);
  xgponConfigDb.SetOnuNetmaskLen (24);
  xgponConfigDb.SetIpAddressFirstByteForXgpon (10);
  xgponConfigDb.SetIpAddressFirstByteForOnus (172);

  //Set TypeId String and other configuration related information through XgponConfigDb before the following call.
  xgponHelper.InitializeObjectFactories ( );

  //0: olt; i (>0): onu
  NetDeviceContainer xgponDevices = xgponHelper.Install (xgponNodes);






  //////////////////////////////////////////Other P2P Links
  NodeContainer clientNodes, serverNodes;
  clientNodes.Create (nOnus);
  serverNodes.Create (nOnus);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pCDevices[nOnus];
  NetDeviceContainer p2pSDevices[nOnus];
  NodeContainer p2pCNodes[nOnus];
  NodeContainer p2pSNodes[nOnus];
  Ipv4InterfaceContainer p2pCInterfaces[nOnus];
  Ipv4InterfaceContainer p2pSInterfaces[nOnus];

  for(int i=0; i<nOnus; i++) 
  { 
    p2pCNodes[i].Add(clientNodes.Get(i));
    p2pCNodes[i].Add(oltNode.Get(0));
    p2pCDevices[i] = pointToPoint.Install (p2pCNodes[i]);

    p2pSNodes[i].Add(serverNodes.Get(i));
    p2pSNodes[i].Add(onuNodes.Get(i));
    p2pSDevices[i] = pointToPoint.Install (p2pSNodes[i]);
  }



  //install internet protocol stack
  InternetStackHelper stack;
  stack.Install (xgponNodes);
  stack.Install (serverNodes);
  stack.Install (clientNodes);


  //Assign OLT and ONU IP addresses
  Ipv4AddressHelper addressHelper;
  Ptr<XgponOltNetDevice> tmpDevice = DynamicCast<XgponOltNetDevice, NetDevice> (xgponDevices.Get(0));
  std::string xgponIpbase = xgponHelper.GetXgponIpAddressBase ( );
  std::string xgponNetmask = xgponHelper.GetOltAddressNetmask();
  addressHelper.SetBase (xgponIpbase.c_str(), xgponNetmask.c_str());

  Ipv4InterfaceContainer xgponInterfaces = addressHelper.Assign (xgponDevices);
  for(int i=0; i<(nOnus+1);i++)
  {
    Ipv4Address addr = xgponInterfaces.GetAddress(i);
    Ptr<XgponNetDevice> tmpDevice = DynamicCast<XgponNetDevice, NetDevice> (xgponDevices.Get(i));
    tmpDevice->SetAddress (addr);

    if(i==0) std::cout << "OLT IP Address: ";
    else std::cout << "ONU " << (i-1) <<" IP Address: ";
    addr.Print(std::cout);
    std::cout << std::endl;
  }


  for(int i=0; i<nOnus; i++) 
  {
    Ipv4Address addr;

    std::string clientIpbase = xgponHelper.GetIpAddressBase (160, i, 24);
    std::string clientNetmask = xgponHelper.GetIpAddressNetmask (24); 
    addressHelper.SetBase (clientIpbase.c_str(), clientNetmask.c_str());

    p2pCInterfaces[i] = addressHelper.Assign (p2pCDevices[i]);

    addr = p2pCInterfaces[i].GetAddress(0);
    std::cout << "Client " << i <<" IP Address: ";
    addr.Print(std::cout);
    std::cout << std::endl;

    addr = p2pCInterfaces[i].GetAddress(1);
    std::cout << "Corresponding IP Address at OLT: ";
    addr.Print(std::cout);
    std::cout << std::endl;




    Ptr<XgponOnuNetDevice> tmpDevice = DynamicCast<XgponOnuNetDevice, NetDevice> (xgponDevices.Get(i+1));
    std::string onuIpbase = xgponHelper.GetOnuIpAddressBase (tmpDevice);
    std::string onuNetmask = xgponHelper.GetOnuAddressNetmask();     
    addressHelper.SetBase (onuIpbase.c_str(), onuNetmask.c_str());

    p2pSInterfaces[i] = addressHelper.Assign (p2pSDevices[i]);

    addr = p2pSInterfaces[i].GetAddress(0);
    std::cout << "Server " << i <<" IP Address: ";
    addr.Print(std::cout);
    std::cout << std::endl;

    addr = p2pSInterfaces[i].GetAddress(1);
    std::cout << "IP Address at the Corresponding ONU: ";
    addr.Print(std::cout);
    std::cout << std::endl;
  }

  //set attributes for connections to be added as OMCI channel
  //we need get the address before setting OMCI channel.
  for(int i=0; i<nOnus; i++) 
  { 
    Ptr<XgponOltNetDevice> oltDevice = DynamicCast<XgponOltNetDevice, NetDevice> (xgponDevices.Get(0));
    Ptr<XgponOnuNetDevice> onuDevice = DynamicCast<XgponOnuNetDevice, NetDevice> (xgponDevices.Get(i+1));
    xgponHelper.AddOmciConnectionsForOnu (onuDevice, oltDevice); 
  }


  //add xgem ports for server nodes connected to ONUs
  for(int i=0; i<nOnus; i++) 
  {
    Address addr = p2pSInterfaces[i].GetAddress(0);

    Ptr<XgponOltNetDevice> oltDevice = DynamicCast<XgponOltNetDevice, NetDevice> (xgponDevices.Get(0));
    Ptr<XgponOnuNetDevice> onuDevice = DynamicCast<XgponOnuNetDevice, NetDevice> (xgponDevices.Get(i+1));

    uint16_t allocId = xgponHelper.AddOneTcontForOnu (onuDevice, oltDevice);
    uint16_t upPortId = xgponHelper.AddOneUpstreamConnectionForOnu (onuDevice, oltDevice, allocId, addr);
    uint16_t downPortId = xgponHelper.AddOneDownstreamConnectionForOnu (onuDevice, oltDevice, addr); 

    std::cout << "ONU-ID = "<<onuDevice->GetOnuId() << ";   ALLOC-ID = " << allocId << ";   UP-PORT-ID = " << upPortId << ";   DOWN-PORT-ID = " << downPortId << std::endl;
    
  }


  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


  //////////////////////////////////////////////////////////////////////////Configure Traffics
  uint16_t serverPort=9000;     //TCP server port
  


  PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), serverPort));
  ApplicationContainer sinkApps = sink.Install (serverNodes);
  sinkApps.Start (Seconds (0));
  sinkApps.Stop (Seconds (APP_STOP));



  PacketSinkHelper sink2 ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), serverPort));
  ApplicationContainer sinkApps2 = sink2.Install (clientNodes);
  sinkApps2.Start (Seconds (0));
  sinkApps2.Stop (Seconds (APP_STOP));



  for(int i=0; i<nOnus; i++)
  {
    BulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress (p2pSInterfaces[i].GetAddress (0), serverPort));
    source.SetAttribute ("MaxBytes", UintegerValue (0));  // Set the amount of data to send in bytes.  Zero is unlimited.
    ApplicationContainer sourceApps = source.Install (clientNodes.Get (i));
    sourceApps.Start (Seconds(APP_START + i * 0.01));
    sourceApps.Stop (Seconds(APP_STOP));


    BulkSendHelper source2 ("ns3::TcpSocketFactory",
                         InetSocketAddress (p2pCInterfaces[i].GetAddress (0), serverPort));
    source2.SetAttribute ("MaxBytes", UintegerValue (0));  // Set the amount of data to send in bytes.  Zero is unlimited.
    ApplicationContainer sourceApps2 = source2.Install (serverNodes.Get (i));
    sourceApps2.Start (Seconds(APP_START + i * 0.01));
    sourceApps2.Stop (Seconds(APP_STOP));
  }


  //xgponHelper.EnableAsciiAll("xgpon-two-onus-tcp-ascii");
  //xgponHelper.EnablePcapAll("xgpon-two-onus-tcp-pcap");
  //pointToPoint.EnableAsciiAll("p2p-two-onus-tcp-ascii");




  Ptr<XgponOltNetDevice> oltDevice = DynamicCast<XgponOltNetDevice, NetDevice> (xgponDevices.Get(0));
  // Tracing stuff
  //devOnus[0]->TraceConnect ("PhyTxEnd", "test1", MakeCallback(&TxTrace));
  //devOnus[0]->TraceConnect ("PhyRxEnd", "test2", MakeCallback(&RxTrace));
  //devOlt->TraceConnect ("PhyRxEnd", "test4", MakeCallback(&RxTrace));
  //oltDevice->TraceConnect ("PhyTxEnd", OLT_TX_TRACE_FILE, MakeCallback(&TxTrace));
  oltDevice->TraceConnectWithoutContext ("DeviceStatistics", MakeCallback(&DeviceStatisticsTrace));


  std::cout<<std::endl;
  Simulator::Stop(Seconds(SIM_STOP));
  Simulator::Run ();
  Simulator::Destroy ();
  std::cout<<std::endl;

  for(int i=0; i<nOnus; i++)
  {
    Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApps.Get (i));
    std::cout << "Total Bytes Received: " << sink1->GetTotalRx () << std::endl;  

    Ptr<PacketSink> sink2 = DynamicCast<PacketSink> (sinkApps2.Get (i));
    std::cout << "Total Bytes Received: " << sink2->GetTotalRx () << std::endl;  
  }


  return 0;
}


