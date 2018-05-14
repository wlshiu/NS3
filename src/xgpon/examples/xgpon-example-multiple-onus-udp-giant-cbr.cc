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
 * Author: Pedro Alvarez <pinheirp@tcd.ie>
 */

/**************************************************************
*
* This script will create an UDP onoff application at each ONU. There are packet sinks for the data from each
* of the ONUs at the olt on different ports.
* These ONUs and the OLT will be in the same subnet.
*
*                                       --------ONU1 --- UDP onoff 1
*                                     /
*      packet sink 1 --- OLT ----------
*      packet sink 2 ---              \
*                                       -------ONU2 --- UDP onoff 2
*
*
* This example will show how to set up the giant scheduler. Each ONU will have one T-CONT with
* fixed, assured, non-assured and best effort bandwidth.
*
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



using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("XgponExampleMultipleOnusUdpGiantPoisson");


//onoff trace
void
appTxTrace (std::string path, Ptr<const Packet> pkt)
{
  std::ofstream myfile;
  myfile.open (path.c_str(), std::ios::app);
  myfile<< "Tx Time: " << Simulator::Now();// <<"\n";
  myfile<<"Pkt UId*"<< pkt->GetUid()<<"&";
  //myfile<<" Pkt UId: "<< pkt->GetUid();// <<"\n";
  pkt->Print(myfile);

//  myfile<< Simulator::Now();
  myfile<<std::endl;
  myfile.close();
}

//packet sink trace
void
appRxTrace(std::string path,Ptr<const Packet> pkt, const Address & addr)
//m_rxTrace(std::string path, Ptr<const Packet> pkt)
{
  std::ofstream myfile;
  myfile.open (path.c_str(), std::ios::app);
  myfile<<"Rx Time: "<< Simulator::Now();// <<"\n";
  myfile<<"Pkt UId*"<< pkt->GetUid()<<"&";
  //myfile<<"Pkt UId: "<< pkt->GetUid();// <<"\n";
  pkt->Print(myfile);
  myfile<<"Address: "<< addr;// << "\n";

 // myfile<< Simulator::Now();
  myfile<<std::endl;
  myfile.close();
}



void
xgponOltTxTrace(std::string path, Ptr<const XgponDsFrame> frame)
//m_rxTrace(std::string path, Ptr<const Packet> pkt)
{
  std::ofstream myfile;
  myfile.open (path.c_str(), std::ios::app);
  myfile<<"Tx Time: "<< Simulator::Now()<<" Frame: ";
  frame->Print(myfile);

  myfile<<std::endl;
  myfile.close();
}

void
xgponOltRxTrace(std::string path, Ptr<const XgponUsBurst> frame)
//m_rxTrace(std::string path, Ptr<const Packet> pkt)
{
  std::ofstream myfile;
  myfile.open (path.c_str(), std::ios::app);
  myfile<<"Rx Time: "<< Simulator::Now()<<" Frame: ";
  frame->Print(myfile);

  myfile<<std::endl;
  myfile.close();
}

void
xgponOnuTxTrace(std::string path, Ptr<const XgponUsBurst> frame)
//m_rxTrace(std::string path, Ptr<const Packet> pkt)
{
  std::ofstream myfile;
  myfile.open (path.c_str(), std::ios::app);
  myfile<<"Tx Time: "<< Simulator::Now()<<" Frame: ";
  frame->Print(myfile);

  myfile<<std::endl;
  myfile.close();
}

void
xgponOnuRxTrace(std::string path, Ptr<const XgponUsBurst> frame)
//m_rxTrace(std::string path, Ptr<const Packet> pkt)
{
  std::ofstream myfile;
  myfile.open (path.c_str(), std::ios::app);
  myfile<<"Rx Time: "<< Simulator::Now()<<" Frame: ";
  frame->Print(myfile);

  myfile<<std::endl;
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

  uint32_t appStart=0;
  uint32_t appStop=5;
  uint32_t simStop=5;

  bool verbose = false;
  bool record = false;
  std::string filenameSuffix="0";
  float dataRate=120e6;
  uint64_t maxBytes=0;
  uint32_t packetSize=1472;

  uint32_t fixedBwValue=128000; //Minimum for DBRu
  uint32_t assuredBwValue=140.928e6;
  uint32_t nonAssuredBwValue=0;
  uint32_t bestEffortBwValue=0;

  uint32_t siValue=2;

  /****************************************
   *
   * Number of ONUs in simulation
   *
   ****************************************/
  uint16_t nOnus = 16;  //Number Of Onus in the PON

  CommandLine cmd;
  cmd.AddValue ("verbose", "Tell application to log if true", verbose);
  cmd.AddValue ("record", "Tell application to record downstream and upstream frames", record);
  cmd.AddValue("FilenameSuffix", "suffix after file name", filenameSuffix);
  cmd.AddValue("MaxBytes", "Max bytes that can be sent in simulation", maxBytes);
  cmd.AddValue("PacketSize", "Size of each packet", packetSize);
  cmd.AddValue("DataRate", "Data Rate in bps", dataRate);
  cmd.AddValue("NumOnus", "Number Of ONUs in simulation", nOnus);
  cmd.AddValue("SimTime", "Simulated Time in seconds", simStop);
  cmd.AddValue("AppTime", "Time Application is running, in seconds", appStop);

  cmd.AddValue("FixedBandwidth", "The amount of fixed bandwidth for each ONU's T-CONT in bps", fixedBwValue);
  cmd.AddValue("AssuredBandwidth", "The amount of assured bandwidth for each ONU's T-CONT in bps", assuredBwValue);
  cmd.AddValue("NonAssuredBandwidth", "The amount of non-Assured bandwidth for each ONU's T-CONT in bps", nonAssuredBwValue);
  cmd.AddValue("BestEffortBandwidth", "Time Application is running, in seconds", bestEffortBwValue);


  cmd.Parse (argc,argv);

  if(verbose)
  {
    LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
    LogComponentEnable ("XgponOltDbaEngineGiant", LOG_LEVEL_LOGIC);
  }

  Packet::EnablePrinting ();


  /****************************************
   *
   * Output filenames
   *
   ****************************************/
  //Application traces
  std::string onOffFilename = "data/giant-data/OnOffTx";
  std::string pktSinkFilename = "data/giant-data/PktSinkRx";

  //ascii traces
  std::string asciiEnqueueFilename = "data/giant-data/onuEnqueue";
  std::string asciiDropFilename = "data/giant-data/onuDrop";
  std::string asciiDequeueFilename = "data/giant-data/onuDequeue";

  //Frame/Bursts traces
  std::string onuBurstTxFilename = "data/giant-data/onuBurstTx";
  std::string oltFrameTxFilename = "data/giant-data/oltFrameTx";

  /****************************************
   *
   * Defining the QoS parameters
   *
   ****************************************/

  std::vector<uint16_t > allocIdList;
  std::vector<uint16_t> serverPorts(nOnus);

  std::vector<uint64_t> fixedBw(nOnus);
  std::vector<uint64_t> assuredBw(nOnus);
  std::vector<uint64_t> nonAssuredBw(nOnus);
  std::vector<uint64_t> maxBw(nOnus);
  std::vector<uint16_t> siMax(nOnus);
  std::vector<uint16_t> siMin(nOnus);

  std::vector<uint16_t> fixedInitialTimers(nOnus);
  std::vector<uint16_t> assuredInitialTimers(nOnus);
  std::vector<uint16_t> nonAssuredInitialTimers(nOnus);
  std::vector<uint16_t> bestEffortInitialTimers(nOnus);


  //QoS parameters
  for(uint32_t i=0; i<nOnus; i++)
  {
    fixedBw[i]=fixedBwValue;
    assuredBw[i]=assuredBwValue;
    nonAssuredBw[i]=nonAssuredBwValue;
    maxBw[i]=fixedBw[i]+assuredBw[i]+nonAssuredBw[i]+bestEffortBwValue;//Zero best effort bandwidth
    siMax[i]=siValue;
    siMin[i]=siValue;
  }

  //Setting up initial timers
  for(uint32_t j=0; j<siValue; j++)
  {
    for(uint32_t i=0; i<(nOnus/siValue); i++)
    {
      fixedInitialTimers[i+(j*nOnus/siValue)]=j;
      assuredInitialTimers[i+(j*nOnus/siValue)]=j;
      nonAssuredInitialTimers[i+(j*nOnus/siValue)]=j;
      bestEffortInitialTimers[i+(j*nOnus/siValue)]=j;
    }
  }

  std::cout<<"Fixed Bandwidth:"<<fixedBwValue<<std::endl;
  std::cout<<"Assured Bandwidth:"<<assuredBwValue<<std::endl;
  std::cout<<"Non-Assured Bandwidth:"<<nonAssuredBwValue<<std::endl;
  std::cout<<"Best Effort Bandwidth:"<<bestEffortBwValue<<std::endl;
  /****************************************
   *
   * Create the ONUs and OLT nodes
   *
   ****************************************/

  NodeContainer oltNode, onuNodes;
  oltNode.Create (1);
  onuNodes.Create (nOnus);

  NodeContainer xgponNodes;
  xgponNodes.Add(oltNode.Get(0));
  for(int i=0; i<nOnus; i++) { xgponNodes.Add (onuNodes.Get(i)); }

  /****************************************
   *
   * Set up XGPON helper and initialize factories
   *
   ****************************************/

  XgponHelper xgponHelper;
  XgponConfigDb& xgponConfigDb = xgponHelper.GetConfigDb ( );

  xgponConfigDb.SetOltNetmaskLen (16);
  xgponConfigDb.SetOnuNetmaskLen (24);
  xgponConfigDb.SetIpAddressFirstByteForXgpon (10);
  xgponConfigDb.SetIpAddressFirstByteForOnus (172);
  xgponConfigDb.SetAllocateIds4Speed (false);
  xgponConfigDb.SetOltDbaEngineTypeIdStr ("ns3::XgponOltDbaEngineGiant");


  //Set TypeId String and other configuration related information through XgponConfigDb before the following call.
  xgponHelper.InitializeObjectFactories ( );

  //Set Queues' attributes
  //xgponHelper.SetQueueAttribute("Mode", EnumValue(XgponQueue::XGPON_QUEUE_MODE_PACKETS));
  //xgponHelper.SetQueueAttribute("MaxPackets", UintegerValue (2) );


  //0: olt; i (>0): onu
  NetDeviceContainer xgponDevices = xgponHelper.Install (xgponNodes);


  /****************************************
   *
   * Set up IP addresses on the network
   *
   ****************************************/

  //install Internet protocol stack
  InternetStackHelper stack;
  stack.Install (xgponNodes);

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
    if(verbose)
    {
      if(i==0) std::cout << "OLT IP Address: ";
      else std::cout << "ONU " << (i-1) <<" IP Address: ";
      addr.Print(std::cout);
      std::cout << std::endl;
    }
  }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


  /****************************************
   *
   * Set up XGEM ports and AllocIds
   *
   ****************************************/

  for(int i=0; i<nOnus; i++)
  {
    Address addr = xgponInterfaces.GetAddress(i+1); //addr i+1 because olt is GetAddress(0)

    Ptr<XgponOltNetDevice> oltDevice = DynamicCast<XgponOltNetDevice, NetDevice> (xgponDevices.Get(0));
    Ptr<XgponOnuNetDevice> onuDevice = DynamicCast<XgponOnuNetDevice, NetDevice> (xgponDevices.Get(i+1));

    //set attributes of the per xgem port qos parameters
    xgponHelper.SetQosParametersAttribute ("FixedBandwidth", UintegerValue (fixedBw[i]) );
    xgponHelper.SetQosParametersAttribute ("AssuredBandwidth", UintegerValue (assuredBw[i]) );
    xgponHelper.SetQosParametersAttribute ("NonAssuredBandwidth", UintegerValue (nonAssuredBw[i]) );
    xgponHelper.SetQosParametersAttribute ("MaxBandwidth", UintegerValue (maxBw[i]) );
    xgponHelper.SetQosParametersAttribute ("MaxServiceInterval", UintegerValue (siMax[i]) );
    xgponHelper.SetQosParametersAttribute ("MinServiceInterval", UintegerValue (siMin[i]) );
    xgponHelper.SetQosParametersAttribute ("TcontType", EnumValue (XgponQosParameters::XGPON_TCONT_TYPE_5) );

    uint16_t allocId = xgponHelper.AddOneTcontForOnu (onuDevice, oltDevice);
    uint16_t upPortId = xgponHelper.AddOneUpstreamConnectionForOnu (onuDevice, oltDevice, allocId, addr);
    uint16_t downPortId = xgponHelper.AddOneDownstreamConnectionForOnu (onuDevice, oltDevice, addr);

    if(verbose)
      std::cout << "ONU-ID = "<<onuDevice->GetOnuId() << ";   ALLOC-ID = " << allocId << ";   UP-PORT-ID = " << upPortId << ";   DOWN-PORT-ID = " << downPortId << std::endl;

    allocIdList.push_back(allocId);
  }

  /****************************************
   *
   * Set up OnOff and PacketSink applications
   *
   ****************************************/

  //Containers
  ApplicationContainer onuApps;
  ApplicationContainer oltApps;

  //set up server ports
  for(uint32_t i=0; i<nOnus;i++)
  {
    serverPorts[i]= 9000+i;
  }


  //OnOff at the onus so setting i to 1 as 0 is the olt
  for(int i=0; i<nOnus; i++)
  {
    OnOffHelper onOff ("ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address (xgponInterfaces.GetAddress(0)), serverPorts[i]));
    if(verbose)
      std::cout << "Setting up onOff to ipv4 address: " << i << ": "<< xgponInterfaces.GetAddress(0)<<std::cout<<std::endl;
    onOff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1000]"));
    onOff.SetAttribute ("OffTime",  StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    onOff.SetAttribute("DataRate", DataRateValue(DataRate(dataRate)));
    onOff.SetAttribute("PacketSize", UintegerValue(packetSize));
    onOff.SetAttribute("MaxBytes", UintegerValue(maxBytes));

    onuApps.Add(onOff.Install(xgponNodes.Get(i+1)));
  }


  //Set up packets sinks at the OLT for each of the ONUs at different ports
  for(int i=0; i<nOnus; i++)
  {
    PacketSinkHelper packetSink ("ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address (xgponInterfaces.GetAddress(0)), serverPorts[i]));
    if(verbose)
      std::cout << "Packet sink at OLT at address" << ": "<< xgponInterfaces.GetAddress(0) << ", port number : "<< serverPorts[i]<<std::endl;

    oltApps.Add(packetSink.Install(xgponNodes.Get(0)));
  }

  //Starting applications
  onuApps.Start (Seconds (appStart));
  onuApps.Stop (Seconds (appStop));
  oltApps.Start (Seconds (appStart));
  oltApps.Stop (Seconds (appStop));

  /****************************************
   *
   * Initializing DBA engine
   *
   ****************************************/

  Ptr<XgponOltNetDevice> oltDevice = DynamicCast<XgponOltNetDevice, NetDevice> (xgponDevices.Get(0));
  Ptr<XgponOltDbaEngine> dbaEngine = oltDevice->GetDbaEngine();

  DynamicCast<XgponOltDbaEngineGiant,XgponOltDbaEngine> (dbaEngine)->GenerateAllocOltGiantParameterPairs();

  std::vector< uint16_t >::iterator it;
  uint32_t timerIt;

  for (it = allocIdList.begin(), timerIt=0; it!=allocIdList.end();it++, timerIt++)
  {
    DynamicCast<XgponOltDbaEngineGiant,XgponOltDbaEngine> (dbaEngine)->SetTimerStartValue(*it, XGPON_GIANT_BW_FIXED, fixedInitialTimers[timerIt]);
    DynamicCast<XgponOltDbaEngineGiant,XgponOltDbaEngine> (dbaEngine)->SetTimerStartValue(*it, XGPON_GIANT_BW_ASSURED, assuredInitialTimers[timerIt]);
    DynamicCast<XgponOltDbaEngineGiant,XgponOltDbaEngine> (dbaEngine)->SetTimerStartValue(*it, XGPON_GIANT_BW_NON_ASSURED, nonAssuredInitialTimers[timerIt]);
    DynamicCast<XgponOltDbaEngineGiant,XgponOltDbaEngine> (dbaEngine)->SetTimerStartValue(*it, XGPON_GIANT_BW_BEST_EFFORT, bestEffortInitialTimers[timerIt]);
  }

  /****************************************
   *
   * Tracing stuff
   *
   ****************************************/

  //Aplication Tracing
  Ptr<Application> onuApp0, oltApp0; //onu2, olt2;
  onuApp0 = onuApps.Get(0);
  oltApp0 = oltApps.Get(0);
  //onu2 = onuApps.Get(1);
  //olt2 = oltApps.Get(1);

  onuApp0->TraceConnect("Tx", onOffFilename+filenameSuffix, MakeCallback(&appTxTrace));
  oltApp0->TraceConnect("Rx", pktSinkFilename+filenameSuffix, MakeCallback(&appRxTrace));
  //onu2->TraceConnect("Tx",ONU_2_TX_TRACE_FILE, MakeCallback(&m_txTrace));
  //olt2->TraceConnect("Rx", OLT_2_RX_TRACE_FILE, MakeCallback(&m_rxTrace));

  //Ascii enqueue, dequeue and drop tracing
  xgponHelper.EnableAsciiInternal(0,"data/giant-data/giant-onu",xgponDevices.Get(1),false);

  //XGPON Frames/Burst Tracing
  if(record)
  {
    Ptr<XgponOltNetDevice> oltDevice = DynamicCast<XgponOltNetDevice, NetDevice> (xgponDevices.Get(0));
    Ptr<XgponOnuNetDevice> onuDevice0 = DynamicCast<XgponOnuNetDevice, NetDevice> (xgponDevices.Get(1));

    oltDevice->TraceConnect("PhyTxEnd", oltFrameTxFilename+filenameSuffix, MakeCallback(&xgponOltTxTrace));
    onuDevice0->TraceConnect("PhyTxEnd", onuBurstTxFilename+filenameSuffix, MakeCallback(&xgponOnuTxTrace));
  }

  //OLT statistics
  oltDevice->TraceConnectWithoutContext ("DeviceStatistics", MakeCallback(&DeviceStatisticsTrace));


  /****************************************
   *
   * Run Simulation
   *
   ****************************************/

  Simulator::Stop(Seconds(simStop));
  Simulator::Run ();
  Simulator::Destroy ();


  /****************************************
   *
   * Simulation Finished
   *
   ****************************************/

  std::cout<<std::endl;
  std::cout<<"GIANT simulation finished. Application data rate: "<<(dataRate*1e-6)<<" Mbps"<<std::endl;
  std::cout<<std::endl;

  return 0;
}


