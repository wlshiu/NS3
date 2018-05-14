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

// Include a header file from your module to test.
//#include "ns3/xgpon-olt-dba-engine-giant.h"
//#include "ns3/xgpon-olt-dba-parameters-giant.h"

// Support files to build test OLT
#include "ns3/xgpon-helper.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/ptr.h"
#include "ns3/xgpon-olt-dba-engine-giant.h"
#include "ns3/object.h"
#include "ns3/simulator.h"

// An essential include is test.h
#include "ns3/test.h"

// Do not put your test classes in namespace ns3.  You may find it useful
// to use the using directive to access the ns3 namespace directly
using namespace ns3;

/*
 * This class will be the base for the GIANT tests allowing to save some copy/pasting in the code.
 * It is used to initialize the network for tests
 */
class XgponTestOltDbaEngineGiantBase : public TestCase
{
public:
  XgponTestOltDbaEngineGiantBase (uint32_t nOnus, std::string name);
  virtual ~XgponTestOltDbaEngineGiantBase ();




protected:
  void InitializeTest(  std::vector<uint64_t> fixedBw,
                    std::vector<uint64_t> assuredBw,
                    std::vector<uint64_t> nonAssuredBw,
                    std::vector<uint64_t> maxBw,
                    std::vector<uint16_t> siMax,
                    std::vector<uint16_t> siMin,
                    std::vector<uint16_t> fixedInitialTimers,
                    std::vector<uint16_t> assuredInitialTimers,
                    std::vector<uint16_t> nonAssuredInitialTimers,
                    std::vector<uint16_t> bestEffortInitialTimers,
                    std::vector<uint16_t> buffOcc,
                    bool fec);

  /*
   * \brief This function is used to schedule a sequence of GenerateBwMaps() store the result in vector m_testBwMapsRecord.
   * Since the GenerateBwMaps() function needs the current time, this allows to test the BwMap generation without the need to set up
   * up the whole network and test only the BwMap generation.
   */
  void ScheduleGenerateBwMap();

  void CompareBwMaps(Ptr<XgponXgtcBwmap> exptBwMap, Ptr<XgponXgtcBwmap> bwMap);

public:

protected:
  // Number of ONUs and T-CONTs
  uint16_t m_nOnus;

  //Variables for Qos Info
  std::vector<uint64_t> m_fixedBw;
  std::vector<uint64_t> m_assuredBw;
  std::vector<uint64_t> m_nonAssuredBw;
  std::vector<uint64_t> m_maxBw;
  std::vector<uint16_t> m_siMax;
  std::vector<uint16_t> m_siMin;
  std::vector<uint16_t> m_fixedInitialTimers;
  std::vector<uint16_t> m_assuredInitialTimers;
  std::vector<uint16_t> m_nonAssuredInitialTimers;
  std::vector<uint16_t> m_bestEffortInitialTimers;
  std::vector<uint16_t> m_buffOcc;

  std::vector<Ptr<XgponTcontOlt> > m_tcontsList;
  std::vector<Ptr<XgponConnectionReceiver> > m_xgemPortsList;
  std::vector<Ptr<XgponXgtcDbru> > m_dbrus;

  bool m_fec;

  NetDeviceContainer m_xgponDevices;

  Ptr<XgponOltNetDevice> m_oltDevice;
  Ptr<XgponOltDbaEngine> m_dbaEngine;

  std::vector< Ptr<XgponXgtcBwmap> > m_testBwMapsRecord;



};

XgponTestOltDbaEngineGiantBase::XgponTestOltDbaEngineGiantBase (uint32_t nOnus, std::string name)
  : TestCase ("Test Case of the XGPON GIANT scheduler: "+name),
    m_nOnus(nOnus),
    m_fixedBw(m_nOnus),
    m_assuredBw(m_nOnus),
    m_nonAssuredBw(m_nOnus),
    m_maxBw(m_nOnus),
    m_siMax(m_nOnus),
    m_siMin(m_nOnus),
    m_fixedInitialTimers(m_nOnus),
    m_assuredInitialTimers(m_nOnus),
    m_nonAssuredInitialTimers(m_nOnus),
    m_bestEffortInitialTimers(m_nOnus),
    m_buffOcc(m_nOnus),
    m_tcontsList(m_nOnus),
    m_xgemPortsList(m_nOnus),
    m_dbrus(m_nOnus),
    m_fec(false)
{
}

XgponTestOltDbaEngineGiantBase::~XgponTestOltDbaEngineGiantBase ()
{
}

void
XgponTestOltDbaEngineGiantBase::InitializeTest(std::vector<uint64_t> fixedBw,
                                           std::vector<uint64_t> assuredBw,
                                           std::vector<uint64_t> nonAssuredBw,
                                           std::vector<uint64_t> maxBw,
                                           std::vector<uint16_t> siMax,
                                           std::vector<uint16_t> siMin,
                                           std::vector<uint16_t> fixedInitialTimers,
                                           std::vector<uint16_t> assuredInitialTimers,
                                           std::vector<uint16_t> nonAssuredInitialTimers,
                                           std::vector<uint16_t> bestEffortInitialTimers,
                                           std::vector<uint16_t> buffOcc,
                                           bool fec)
{


  m_fixedBw=fixedBw;
  m_assuredBw=assuredBw;
  m_nonAssuredBw=nonAssuredBw;
  m_maxBw=maxBw;
  m_siMax=siMax;
  m_siMin=siMin;
  m_fixedInitialTimers=fixedInitialTimers;
  m_assuredInitialTimers=assuredInitialTimers;
  m_nonAssuredInitialTimers=nonAssuredInitialTimers;
  m_bestEffortInitialTimers=bestEffortInitialTimers;
  m_buffOcc=buffOcc;
  m_fec=fec;

/****************************************
  *
  * Create the OLT nodes and set the engines
  *
  ****************************************/

  Ptr<XgponOltNetDevice> oltDevice = CreateObject<XgponOltNetDevice>();

  Ptr<XgponOltXgemEngine> xgemEngine = CreateObject<XgponOltXgemEngine>();
  oltDevice->SetXgemEngine (xgemEngine);
  xgemEngine->SetXgponOltNetDevice (oltDevice);

  Ptr<XgponOltFramingEngine> framingEngine = CreateObject<XgponOltFramingEngine>();
  oltDevice->SetFramingEngine (framingEngine);
  framingEngine->SetXgponOltNetDevice (oltDevice);

  //Create Phy-adapter and set PON-ID.
  Ptr<XgponOltPhyAdapter> phyAdapter = CreateObject<XgponOltPhyAdapter>();
  oltDevice->SetPhyAdapter (phyAdapter);
  phyAdapter->SetXgponOltNetDevice (oltDevice);
  phyAdapter->SetPonid (1);

  Ptr<XgponPhy> commonPhy = CreateObject<XgponPhy>();
  oltDevice->SetXgponPhy (commonPhy);


  Ptr<XgponOltOmciEngine> omciEngine = CreateObject<XgponOltOmciEngine>();
  oltDevice->SetOmciEngine (omciEngine);
  omciEngine->SetXgponOltNetDevice (oltDevice);

  Ptr<XgponOltPloamEngine> ploamEngine = CreateObject<XgponOltPloamEngine>();
  oltDevice->SetPloamEngine (ploamEngine);
  ploamEngine->SetXgponOltNetDevice (oltDevice);


  //Create the GIANT DBA.
  Ptr<XgponOltDbaEngine> dbaEngine = CreateObject<XgponOltDbaEngineGiant>();
  oltDevice->SetDbaEngine (dbaEngine);
  dbaEngine->SetXgponOltNetDevice (oltDevice);

  //7. Channel
  Ptr<XgponChannel> xgponChannel = CreateObject<XgponChannel> ( );
  oltDevice->SetChannel (xgponChannel);
  xgponChannel->SetOlt (oltDevice);

  m_oltDevice = oltDevice;
  m_dbaEngine = m_oltDevice->GetDbaEngine();

  /****************************************
   *
   * Set up XGEM ports and AllocIds in the OLT
   * They are set up manually so we can play with the amount of data in the buffers
   * and create different BwMaps accordingly
   *
   ****************************************/

  for(int i=0; i<m_nOnus; i++)
  {
    //////////////////////////////
    int16_t onuId=i;
    uint16_t allocId=onuId;

    m_tcontsList[allocId] = CreateObject<XgponTcontOlt> ( );
    m_tcontsList[allocId]->SetOnuId (onuId);
    m_tcontsList[allocId]->SetAllocId(allocId);

    // XGEM Ports
    m_xgemPortsList[allocId] = CreateObject<XgponConnectionReceiver> ( );
    m_xgemPortsList[allocId]->SetDirection (XgponConnection::UPSTREAM_CONN);
    m_xgemPortsList[allocId]->SetBroadcast (false);
    m_xgemPortsList[allocId]->SetXgemPort (allocId);
    m_xgemPortsList[allocId]->SetAllocId (allocId);
    m_xgemPortsList[allocId]->SetOnuId (onuId);

    // Set up the T-CONT's Link Info
    Ptr<XgponLinkInfo> linkInfo = CreateObject<XgponLinkInfo>();
    m_oltDevice->GetPloamEngine()->AddLinkInfo (linkInfo, allocId);
    Ptr<XgponKey> key1 = CreateObject<XgponKey>();
    Ptr<XgponKey> key2 = CreateObject<XgponKey>();
    linkInfo->AddNewDsKey (key1, 0);
    linkInfo->AddNewUsKey (key2, 0);
    linkInfo->SetCurrentDsKeyIndex (0);
    linkInfo->SetCurrentUsKeyIndex (0);

    // Set up the T-CONT's burst profile
    Ptr<XgponBurstProfile> profile = CreateObject<XgponBurstProfile>();
    profile->SetPreambleLen (XgponBurstProfile::PSBU_PREAMBLE_DEFAULT_LEN);
    profile->SetDelimiterLen (XgponBurstProfile::PSBU_DELIMITER_DEFAULT_LEN);
    profile->SetFec (m_fec);
    linkInfo->AddNewProfile (profile, 0);
    linkInfo->SetCurrentProfileIndex (0);

    // Create and set connection receiver
    Ptr<XgponConnectionReceiver> connReceiver = CreateObject<XgponConnectionReceiver> ( );
    connReceiver->SetDirection (XgponConnection::UPSTREAM_CONN);
    connReceiver->SetBroadcast (false);
    connReceiver->SetXgemPort (allocId);
    connReceiver->SetAllocId (allocId);
    connReceiver->SetOnuId (onuId);

    // Set up the connection QoS parameters
    Ptr<XgponQosParameters> qos=CreateObject<XgponQosParameters>();
    qos->SetTcontType (XgponQosParameters::XGPON_TCONT_TYPE_1);
    qos->SetFixedBw (m_fixedBw[allocId]);
    qos->SetAssuredBw (m_assuredBw[allocId]);
    qos->SetNonAssuredBw (m_nonAssuredBw[allocId]);
    qos->SetMaxBw (m_maxBw[allocId]);
    qos->SetMaxInterval (m_siMax[allocId]);
    qos->SetMinInterval(m_siMin[allocId]);

    m_dbrus[allocId]=Create<XgponXgtcDbru>(m_buffOcc[allocId]);

    m_xgemPortsList[allocId]->SetQosParameters(qos);

    m_tcontsList[allocId]->AddOneConnection(m_xgemPortsList[allocId]);
    m_tcontsList[allocId]->AddNewBufOccupancyReport(m_dbrus[allocId]);
    m_dbaEngine->AddTcontToDbaEngine (m_tcontsList[allocId]);
  }

  /****************************************
   *
   * Initializing DBA engine
   *
   ****************************************/

  DynamicCast<XgponOltDbaEngineGiant> (m_dbaEngine)->GenerateAllocOltGiantParameterPairs();

  std::vector< Ptr<XgponTcontOlt> >::iterator it;
  uint32_t timerIt;

  for (it = m_tcontsList.begin(), timerIt=0; it!=m_tcontsList.end();it++, timerIt++)
  {
    DynamicCast<XgponOltDbaEngineGiant,XgponOltDbaEngine> (m_dbaEngine)->SetTimerStartValue((*it)->GetAllocId(), XGPON_GIANT_BW_FIXED, m_fixedInitialTimers[timerIt]);
    DynamicCast<XgponOltDbaEngineGiant,XgponOltDbaEngine> (m_dbaEngine)->SetTimerStartValue((*it)->GetAllocId(), XGPON_GIANT_BW_ASSURED, m_assuredInitialTimers[timerIt]);
    DynamicCast<XgponOltDbaEngineGiant,XgponOltDbaEngine> (m_dbaEngine)->SetTimerStartValue((*it)->GetAllocId(), XGPON_GIANT_BW_NON_ASSURED, m_nonAssuredInitialTimers[timerIt]);
    DynamicCast<XgponOltDbaEngineGiant,XgponOltDbaEngine> (m_dbaEngine)->SetTimerStartValue((*it)->GetAllocId(), XGPON_GIANT_BW_BEST_EFFORT, m_bestEffortInitialTimers[timerIt]);
  }

}

void
XgponTestOltDbaEngineGiantBase::ScheduleGenerateBwMap()
{
  Ptr<XgponXgtcBwmap> bwMap=m_dbaEngine->GenerateBwMap();
  m_testBwMapsRecord.push_back(bwMap);

}

void
XgponTestOltDbaEngineGiantBase::CompareBwMaps(Ptr<XgponXgtcBwmap> exptBwMap, Ptr<XgponXgtcBwmap> bwMap)
{
  NS_TEST_ASSERT_MSG_EQ (exptBwMap->GetNumberOfBwAllocation(), bwMap->GetNumberOfBwAllocation() , "Number of Allocations is different");

  for(uint32_t i=0; i<bwMap->GetNumberOfBwAllocation(); i++)
  {
    NS_TEST_ASSERT_MSG_EQ (exptBwMap->GetBwAllocationByIndex(i)->GetAllocId(), bwMap->GetBwAllocationByIndex(i)->GetAllocId(),"Different AllocId from expected in BwAllocation: "<<i);
    NS_TEST_ASSERT_MSG_EQ (exptBwMap->GetBwAllocationByIndex(i)->GetBurstProfileIndex(), bwMap->GetBwAllocationByIndex(i)->GetBurstProfileIndex(),"Different Burst Profile Index from expected in BwAllocation: "<<i);
    NS_TEST_ASSERT_MSG_EQ (exptBwMap->GetBwAllocationByIndex(i)->GetDbruFlag(), bwMap->GetBwAllocationByIndex(i)->GetDbruFlag(),"Different DBRu from expected in BwAllocation: "<<i);
    NS_TEST_ASSERT_MSG_EQ (exptBwMap->GetBwAllocationByIndex(i)->GetFwi(), bwMap->GetBwAllocationByIndex(i)->GetFwi(),"Different Forced Wake Up Indication Bit from expected in BwAllocation: "<<i);
    NS_TEST_ASSERT_MSG_EQ (exptBwMap->GetBwAllocationByIndex(i)->GetGrantSize(), bwMap->GetBwAllocationByIndex(i)->GetGrantSize(),"Different Grant Size from expected in BwAllocation: "<<i);
    NS_TEST_ASSERT_MSG_EQ (exptBwMap->GetBwAllocationByIndex(i)->GetPloamuFlag(), bwMap->GetBwAllocationByIndex(i)->GetPloamuFlag(),"Different Grant Size from expected in BwAllocation: "<<i);
    NS_TEST_ASSERT_MSG_EQ (exptBwMap->GetBwAllocationByIndex(i)->GetStartTime(), bwMap->GetBwAllocationByIndex(i)->GetStartTime(),"Different Grant Size from expected in BwAllocation: "<<i);
  }



}



/*
 * This test case is part of a class of tests used to test the GIANT DbaEngine.
 * It will test an OLT with 16 ONUs, each ONU with one T-CONT, each T-CONT with one XGEM-Port.
 *
 * Each T-CONT will have:
 *    1 - Fixed bandwith of 128 kbps
 *    2 - A service interval of 2 frames. 
 *
 * This means the each T-CONT will have 1 word of fixed bandwidth allocated every two frames. This is
 * the minimum allocation possible in XG-PON and this will transmit no DATA only one Buffer Ocuppancy
 * Report. This mechanism can be used to force periodic polling with the scheduler.
 *
 * NOTE: By Fixed Bandwidth we refer only to XGTC Layer bandwidth; i.e., no PHY overhead are
 * considered. When assigning Fixed and assured words on upstream frames, the user must make sure he
 * is not overbooking the frame, taking into account the guards. 
 *
 */

class XgponTestOltDbaEngineGiantMinimumFixed : public XgponTestOltDbaEngineGiantBase
{
public:
  XgponTestOltDbaEngineGiantMinimumFixed ();
  virtual ~XgponTestOltDbaEngineGiantMinimumFixed ();

private:
  virtual void DoRun (void);

};

// Add some help text to this case to describe what it is intended to test
XgponTestOltDbaEngineGiantMinimumFixed::XgponTestOltDbaEngineGiantMinimumFixed ()
  : XgponTestOltDbaEngineGiantBase (16, "Minimum Amount of Fixed bandwidth needed for pooling") //16 ONUs
{

  std::vector<uint64_t> fixedBw(m_nOnus);
  std::vector<uint64_t> assuredBw(m_nOnus);
  std::vector<uint64_t> nonAssuredBw(m_nOnus);
  std::vector<uint64_t> maxBw(m_nOnus);
  std::vector<uint16_t> siMax(m_nOnus);
  std::vector<uint16_t> siMin(m_nOnus);
  std::vector<uint16_t> fixedInitialTimers(m_nOnus);
  std::vector<uint16_t> assuredInitialTimers(m_nOnus);
  std::vector<uint16_t> nonAssuredInitialTimers(m_nOnus);
  std::vector<uint16_t> bestEffortInitialTimers(m_nOnus);
  std::vector<uint16_t> buffOcc(m_nOnus);
  bool fec=false;

  //Setting up the bandwidth of the T-CONTs
  uint16_t siValue=2;

  for(uint32_t i=0; i<m_nOnus; i++)
  {
    fixedBw[i]=128000;
    assuredBw[i]=0;
    nonAssuredBw[i]=0;
    maxBw[i]=fixedBw[i]+assuredBw[i]+nonAssuredBw[i]+0;//Zero best effort bandwidth
    siMax[i]=siValue;
    siMin[i]=siValue;
  }

  //Setting up initial timers
  NS_ASSERT_MSG(m_nOnus%siValue==0, "Number of ONUs is not equally distributed among the frames in a SI, you must change the code and manually set up the network");

  for(uint32_t j=0; j<siValue; j++)
  {
    for(uint32_t i=0; i<(m_nOnus/siValue); i++)
    {
      fixedInitialTimers[i+(j*m_nOnus/siValue)]=j;
      assuredInitialTimers[i+(j*m_nOnus/siValue)]=j;
      nonAssuredInitialTimers[i+(j*m_nOnus/siValue)]=j;
      bestEffortInitialTimers[i+(j*m_nOnus/siValue)]=j;
    }
  }

  //Set up initial bufferOccupancies
  for(uint32_t i=0; i<m_nOnus; i++)
  {
    buffOcc[i]=0;
  }
  InitializeTest( fixedBw,
                  assuredBw,
                  nonAssuredBw,
                  maxBw,
                  siMax,
                  siMin,
                  fixedInitialTimers,
                  assuredInitialTimers,
                  nonAssuredInitialTimers,
                  bestEffortInitialTimers,
                  buffOcc,
                  fec);

}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
XgponTestOltDbaEngineGiantMinimumFixed::~XgponTestOltDbaEngineGiantMinimumFixed ()
{
}

//
// This method is the pure virtual method from class TestCase that every
// TestCase must implement
//
void
XgponTestOltDbaEngineGiantMinimumFixed::DoRun (void)
{

  bool dbru=true;
  bool ploamu=false;
  uint8_t fwi=0;
  uint8_t burstProfile=0;
  uint16_t startTime=0;
  uint16_t grantSize=1;

  uint16_t phyOverhead=m_oltDevice->GetXgponPhy()->GetUsMinimumGuardTime()+(XgponBurstProfile::PSBU_PREAMBLE_DEFAULT_LEN+XgponBurstProfile::PSBU_DELIMITER_DEFAULT_LEN)/4; //(2+5+1)
  uint16_t xgtcOverhead=1+1; //(XgtcHeader (no PLOAM)+ XGTC trailer)
  uint16_t lastEndTime=0;

  //Create expected BwMaps:
  Ptr<XgponXgtcBwmap> exptBwMap1 = Create<XgponXgtcBwmap>();
  for(uint16_t i=0; i<8;i++ )
  {
    startTime=lastEndTime+phyOverhead;
    lastEndTime=startTime+xgtcOverhead+grantSize;
    Ptr<XgponXgtcBwAllocation> exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
    exptBwMap1->AddOneBwAllocation(exptBwAllocation);
  }

  startTime=0;
  lastEndTime=0;

  Ptr<XgponXgtcBwmap> exptBwMap2 = Create<XgponXgtcBwmap>();
  for(uint16_t i=8; i<16;i++ )
  {
    startTime=lastEndTime+phyOverhead;
    lastEndTime=startTime+xgtcOverhead+grantSize;
    Ptr<XgponXgtcBwAllocation> exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
    exptBwMap2->AddOneBwAllocation(exptBwAllocation);
  }

  //Generate the bandwidth map:

  for(uint32_t j=0;j<2;j++)
  {
    Ptr<XgponXgtcBwmap> bwMap1 = m_dbaEngine->GenerateBwMap();
    Ptr<XgponXgtcBwmap> bwMap2 = m_dbaEngine->GenerateBwMap();

    //Compare with the
    // A wide variety of test macros are available in src/core/test.h

    CompareBwMaps(exptBwMap1,bwMap1);
    CompareBwMaps(exptBwMap2,bwMap2);

    NS_TEST_ASSERT_MSG_EQ (exptBwMap2->GetNumberOfBwAllocation(), bwMap2->GetNumberOfBwAllocation() , "Number of Allocations is different");

  }
}





/*
 * This test case is part of a class of tests used to test the GIANT DbaEngine.
 * It will test an OLT with 16 ONUs, each ONU with one T-CONT, each T-CONT with one XGEM-Port.
 *
 * Each T-CONT will have:
 *    1 - Assured bandwidth of 154.240 Mbps
 *    2 - A service interval of 2 frames.
 *    3 - 300 words reported in the buffers
 *
 * This means the each T-CONT will have 1205 words of assured bandwidth allocated every two frames and requires 301
 * words to be served (300 data + 1 DBRu). This will test if assured bandwidth works when requests are smaller than
 * the available words.
 *
 * NOTE: By Assured Bandwidth we refer only to XGTC Layer bandwidth; i.e., no PHY overhead are
 * considered. When assigning Fixed and assured words on upstream frames, the user must make sure he
 * is not overbooking the frame, taking into account the guards.
 *
 */

class XgponTestOltDbaEngineGiantAssuredLowOccBuff : public XgponTestOltDbaEngineGiantBase
{
public:
  XgponTestOltDbaEngineGiantAssuredLowOccBuff ();
  virtual ~XgponTestOltDbaEngineGiantAssuredLowOccBuff ();

private:
  virtual void DoRun (void);


public:

private:


};

// Add some help text to this case to describe what it is intended to test
XgponTestOltDbaEngineGiantAssuredLowOccBuff::XgponTestOltDbaEngineGiantAssuredLowOccBuff ()
  : XgponTestOltDbaEngineGiantBase(16, "Test of Assured Bandwidth with low OccBuff")
{

  std::vector<uint64_t> fixedBw(m_nOnus);
  std::vector<uint64_t> assuredBw(m_nOnus);
  std::vector<uint64_t> nonAssuredBw(m_nOnus);
  std::vector<uint64_t> maxBw(m_nOnus);
  std::vector<uint16_t> siMax(m_nOnus);
  std::vector<uint16_t> siMin(m_nOnus);
  std::vector<uint16_t> fixedInitialTimers(m_nOnus);
  std::vector<uint16_t> assuredInitialTimers(m_nOnus);
  std::vector<uint16_t> nonAssuredInitialTimers(m_nOnus);
  std::vector<uint16_t> bestEffortInitialTimers(m_nOnus);
  std::vector<uint16_t> buffOcc(m_nOnus);
  bool fec=false;

  /*
   * Establish the QoS Parameters here:
   *
   */
  //Setting up the bandwidth of the T-CONTs
  uint16_t siValue=2;

  for(uint32_t i=0; i<m_nOnus; i++)
  {
    fixedBw[i]=0;
    assuredBw[i]=154240000; //154.240 Mbps
    nonAssuredBw[i]=0;
    maxBw[i]=fixedBw[i]+assuredBw[i]+nonAssuredBw[i]+0;//Zero best effort bandwidth
    siMax[i]=siValue;
    siMin[i]=siValue;
  }

  //Setting up initial timers
  NS_ASSERT_MSG(m_nOnus%siValue==0, "Number of ONUs is not equally distributed among the frames in a SI, you must change the code and manually set up the network");

  for(uint32_t j=0; j<siValue; j++)
  {
    for(uint32_t i=0; i<(m_nOnus/siValue); i++)
    {
      fixedInitialTimers[i+(j*m_nOnus/siValue)]=j;
      assuredInitialTimers[i+(j*m_nOnus/siValue)]=j;
      nonAssuredInitialTimers[i+(j*m_nOnus/siValue)]=j;
      bestEffortInitialTimers[i+(j*m_nOnus/siValue)]=j;
    }
  }


  //Set up initial bufferOccupancies
  for(uint32_t i=0; i<m_nOnus; i++)
  {
    buffOcc[i]=300;
  }

  /*
   * Call the InitializeTest function from GiantTestCaseBase to initialize the test.
   *
   */
  InitializeTest( fixedBw,
                    assuredBw,
                    nonAssuredBw,
                    maxBw,
                    siMax,
                    siMin,
                    fixedInitialTimers,
                    assuredInitialTimers,
                    nonAssuredInitialTimers,
                    bestEffortInitialTimers,
                    buffOcc,
                    fec);

  /*
   * DoRun will actually run the tests later on.
   */
}

XgponTestOltDbaEngineGiantAssuredLowOccBuff::~XgponTestOltDbaEngineGiantAssuredLowOccBuff ()
{
}


void
XgponTestOltDbaEngineGiantAssuredLowOccBuff::DoRun (void)
{


  bool dbru=true;
  bool ploamu=false;
  uint8_t fwi=0;
  uint8_t burstProfile=0;
  uint16_t startTime=0;
  uint16_t grantSize=301;

  uint16_t phyOverhead=m_oltDevice->GetXgponPhy()->GetUsMinimumGuardTime()+(XgponBurstProfile::PSBU_PREAMBLE_DEFAULT_LEN+XgponBurstProfile::PSBU_DELIMITER_DEFAULT_LEN)/4; //(2+5+1)
  uint16_t xgtcOverhead=1+1; //(XgtcHeader (no PLOAM)+ XGTC trailer)
  uint16_t lastEndTime=0;

  //Create expected BwMaps:
  Ptr<XgponXgtcBwmap> exptBwMap1 = Create<XgponXgtcBwmap>();
  for(uint16_t i=0; i<8;i++ )
  {
    startTime=lastEndTime+phyOverhead;
    lastEndTime=startTime+xgtcOverhead+grantSize;
    Ptr<XgponXgtcBwAllocation> exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
    exptBwMap1->AddOneBwAllocation(exptBwAllocation);
  }

  startTime=0;
  lastEndTime=0;

  Ptr<XgponXgtcBwmap> exptBwMap2 = Create<XgponXgtcBwmap>();
  for(uint16_t i=8; i<16;i++ )
  {
    startTime=lastEndTime+phyOverhead;
    lastEndTime=startTime+xgtcOverhead+grantSize;
    Ptr<XgponXgtcBwAllocation> exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
    exptBwMap2->AddOneBwAllocation(exptBwAllocation);
  }


  Ptr<XgponXgtcBwmap> bwMap1 = m_dbaEngine->GenerateBwMap();
  Ptr<XgponXgtcBwmap> bwMap2 = m_dbaEngine->GenerateBwMap();
  Ptr<XgponXgtcBwmap> bwMap3 = m_dbaEngine->GenerateBwMap();
  Ptr<XgponXgtcBwmap> bwMap4 = m_dbaEngine->GenerateBwMap();


  CompareBwMaps(exptBwMap1,bwMap1);
  CompareBwMaps(exptBwMap2,bwMap2);

  NS_TEST_ASSERT_MSG_EQ (0, bwMap3->GetNumberOfBwAllocation() , "Number of Allocations is different");
  NS_TEST_ASSERT_MSG_EQ (0, bwMap4->GetNumberOfBwAllocation() , "Number of Allocations is different");

}


/*
 * This test case is part of a class of tests used to test the GIANT DbaEngine.
 * It will test an OLT with 16 ONUs, each ONU with one T-CONT, each T-CONT with one XGEM-Port.
 *
 * Each T-CONT will have:
 *    1 - Assured bandwidth of 154.240 Mbps
 *    2 - A service interval of 2 frames.
 *    3 - 1205 words reported in the buffers
 *
 * This means the each T-CONT will have 1205 words of assured bandwidth allocated every two frames and requires two
 * bandwidth maps to be served, one with 1205 words (1204 data + 1 DBRu) and another with 2 words (1 data + 1 DBRu)
 * This will test if assured bandwidth works when requests are larger than the available words.
 *  *
 * NOTE: By Assured Bandwidth we refer only to XGTC Layer bandwidth; i.e., no PHY overhead are
 * considered. When assigning Fixed and assured words on upstream frames, the user must make sure he
 * is not overbooking the frame, taking into account the guards.
 *
 */

class XgponTestOltDbaEngineGiantAssuredHighOccBuff : public XgponTestOltDbaEngineGiantBase
{
public:
  XgponTestOltDbaEngineGiantAssuredHighOccBuff ();
  virtual ~XgponTestOltDbaEngineGiantAssuredHighOccBuff ();

private:
  virtual void DoRun (void);


public:

private:


};

// Add some help text to this case to describe what it is intended to test
XgponTestOltDbaEngineGiantAssuredHighOccBuff::XgponTestOltDbaEngineGiantAssuredHighOccBuff ()
  : XgponTestOltDbaEngineGiantBase(16, "Test of Assured Bandwidth with high OccBuff")
{

  std::vector<uint64_t> fixedBw(m_nOnus);
  std::vector<uint64_t> assuredBw(m_nOnus);
  std::vector<uint64_t> nonAssuredBw(m_nOnus);
  std::vector<uint64_t> maxBw(m_nOnus);
  std::vector<uint16_t> siMax(m_nOnus);
  std::vector<uint16_t> siMin(m_nOnus);
  std::vector<uint16_t> fixedInitialTimers(m_nOnus);
  std::vector<uint16_t> assuredInitialTimers(m_nOnus);
  std::vector<uint16_t> nonAssuredInitialTimers(m_nOnus);
  std::vector<uint16_t> bestEffortInitialTimers(m_nOnus);
  std::vector<uint16_t> buffOcc(m_nOnus);
  bool fec=false;

  /*
   * Establish the QoS Parameters here:
   *
   */
  //Setting up the bandwidth of the T-CONTs
  uint16_t siValue=2;

  for(uint32_t i=0; i<m_nOnus; i++)
  {
    fixedBw[i]=0;
    assuredBw[i]=154240000; //154.240 Mbps
    nonAssuredBw[i]=0;
    maxBw[i]=fixedBw[i]+assuredBw[i]+nonAssuredBw[i]+0;//Zero best effort bandwidth
    siMax[i]=siValue;
    siMin[i]=siValue;
  }

  //Setting up initial timers
  NS_ASSERT_MSG(m_nOnus%siValue==0, "Number of ONUs is not equally distributed among the frames in a SI, you must change the code and manually set up the network");

  for(uint32_t j=0; j<siValue; j++)
  {
    for(uint32_t i=0; i<(m_nOnus/siValue); i++)
    {
      fixedInitialTimers[i+(j*m_nOnus/siValue)]=j;
      assuredInitialTimers[i+(j*m_nOnus/siValue)]=j;
      nonAssuredInitialTimers[i+(j*m_nOnus/siValue)]=j;
      bestEffortInitialTimers[i+(j*m_nOnus/siValue)]=j;
    }
  }


  //Set up initial bufferOccupancies
  for(uint32_t i=0; i<m_nOnus; i++)
  {
    buffOcc[i]=1205; //Just enough words to be needed to be sent in two frames
  }

  /*
   * Call the InitializeTest function from GiantTestCaseBase to initialize the test.
   *
   */
  InitializeTest( fixedBw,
                    assuredBw,
                    nonAssuredBw,
                    maxBw,
                    siMax,
                    siMin,
                    fixedInitialTimers,
                    assuredInitialTimers,
                    nonAssuredInitialTimers,
                    bestEffortInitialTimers,
                    buffOcc,
                    fec);

  /*
   * DoRun will actually run the tests later on.
   */
}

XgponTestOltDbaEngineGiantAssuredHighOccBuff::~XgponTestOltDbaEngineGiantAssuredHighOccBuff ()
{
}

void
XgponTestOltDbaEngineGiantAssuredHighOccBuff::DoRun (void)
{


  bool dbru=true;
  bool ploamu=false;
  uint8_t fwi=0;
  uint8_t burstProfile=0;
  uint16_t startTime=0;
  uint16_t grantSize=1205;

  uint16_t phyOverhead=m_oltDevice->GetXgponPhy()->GetUsMinimumGuardTime()+(XgponBurstProfile::PSBU_PREAMBLE_DEFAULT_LEN+XgponBurstProfile::PSBU_DELIMITER_DEFAULT_LEN)/4; //(2+5+1)
  uint16_t xgtcOverhead=1+1; //(XgtcHeader (no PLOAM)+ XGTC trailer)
  uint16_t lastEndTime=0;

  //Create expected BwMaps:
  Ptr<XgponXgtcBwmap> exptBwMap1 = Create<XgponXgtcBwmap>();
  for(uint16_t i=0; i<8;i++ )
  {
    startTime=lastEndTime+phyOverhead;
    lastEndTime=startTime+xgtcOverhead+grantSize;
    Ptr<XgponXgtcBwAllocation> exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
    exptBwMap1->AddOneBwAllocation(exptBwAllocation);
  }

  startTime=0;
  lastEndTime=0;

  Ptr<XgponXgtcBwmap> exptBwMap2 = Create<XgponXgtcBwmap>();
  for(uint16_t i=8; i<16;i++ )
  {
    startTime=lastEndTime+phyOverhead;
    lastEndTime=startTime+xgtcOverhead+grantSize;
    Ptr<XgponXgtcBwAllocation> exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
    exptBwMap2->AddOneBwAllocation(exptBwAllocation);
  }

  startTime=0;
  lastEndTime=0;
  grantSize=5; //The minimum size a grant can have that has data and a DBRu

  Ptr<XgponXgtcBwmap> exptBwMap3 = Create<XgponXgtcBwmap>();
  for(uint16_t i=0; i<8;i++ )
  {
    startTime=lastEndTime+phyOverhead;
    lastEndTime=startTime+xgtcOverhead+grantSize;
    Ptr<XgponXgtcBwAllocation> exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
    exptBwMap3->AddOneBwAllocation(exptBwAllocation);
  }

  startTime=0;
  lastEndTime=0;
  grantSize=5; //The minimum size a grant can have that has data and a DBRu

  Ptr<XgponXgtcBwmap> exptBwMap4 = Create<XgponXgtcBwmap>();
  for(uint16_t i=8; i<16;i++ )
  {
    startTime=lastEndTime+phyOverhead;
    lastEndTime=startTime+xgtcOverhead+grantSize;
    Ptr<XgponXgtcBwAllocation> exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
    exptBwMap4->AddOneBwAllocation(exptBwAllocation);
  }

  Ptr<XgponXgtcBwmap> bwMap1 = m_dbaEngine->GenerateBwMap();
  Ptr<XgponXgtcBwmap> bwMap2 = m_dbaEngine->GenerateBwMap();
  Ptr<XgponXgtcBwmap> bwMap3 = m_dbaEngine->GenerateBwMap();
  Ptr<XgponXgtcBwmap> bwMap4 = m_dbaEngine->GenerateBwMap();

  CompareBwMaps(exptBwMap1,bwMap1);
  CompareBwMaps(exptBwMap2,bwMap2);
  CompareBwMaps(exptBwMap3,bwMap3);
  CompareBwMaps(exptBwMap4,bwMap4);

  //m_dbaEngine->PrintAllActiveBwmaps();
}

/*
 * This test case is part of a class of tests used to test the GIANT DbaEngine.
 * It will test an OLT with 16 ONUs, each ONU with one T-CONT, each T-CONT with one XGEM-Port.
 *
 * Each T-CONT will have:
 *    1 - Assured bandwidth of 154.240 Mbps
 *    2 - A service interval of 2 frames.
 *    3 - 1205 words reported in the buffers
 *
 * This means the each T-CONT will have 1205 words of assured bandwidth allocated every two frames and requires two
 * bandwidth maps to be served, one with 1205 words (1204 data + 1 DBRu) and another with 2 words (1 data + 1 DBRu)
 * This will test if assured bandwidth works when requests are larger than the available words.
 *  *
 * NOTE: By Assured Bandwidth we refer only to XGTC Layer bandwidth; i.e., no PHY overhead are
 * considered. When assigning Fixed and assured words on upstream frames, the user must make sure he
 * is not overbooking the frame, taking into account the guards.
 *
 */

class XgponTestOltDbaEngineGiantFixedPlusAssured : public XgponTestOltDbaEngineGiantBase
{
public:
  XgponTestOltDbaEngineGiantFixedPlusAssured ();
  virtual ~XgponTestOltDbaEngineGiantFixedPlusAssured ();

private:
  virtual void DoRun (void);


public:

private:


};

// Add some help text to this case to describe what it is intended to test
XgponTestOltDbaEngineGiantFixedPlusAssured::XgponTestOltDbaEngineGiantFixedPlusAssured ()
  : XgponTestOltDbaEngineGiantBase(16, "Test T-CONT with both Fixed and Assured Bandwidth")
{

  std::vector<uint64_t> fixedBw(m_nOnus);
  std::vector<uint64_t> assuredBw(m_nOnus);
  std::vector<uint64_t> nonAssuredBw(m_nOnus);
  std::vector<uint64_t> maxBw(m_nOnus);
  std::vector<uint16_t> siMax(m_nOnus);
  std::vector<uint16_t> siMin(m_nOnus);
  std::vector<uint16_t> fixedInitialTimers(m_nOnus);
  std::vector<uint16_t> assuredInitialTimers(m_nOnus);
  std::vector<uint16_t> nonAssuredInitialTimers(m_nOnus);
  std::vector<uint16_t> bestEffortInitialTimers(m_nOnus);
  std::vector<uint16_t> buffOcc(m_nOnus);
  bool fec=false;
  /*
   * Establish the QoS Parameters here:
   *
   */
  //Setting up the bandwidth of the T-CONTs
  uint16_t siValue=2;

  for(uint32_t i=0; i<m_nOnus; i++)
  {
    fixedBw[i]=128000;      //128 kbps
    assuredBw[i]=154112000; //154.112 Mbps
    nonAssuredBw[i]=0;
    maxBw[i]=fixedBw[i]+assuredBw[i]+nonAssuredBw[i]+0;//Zero best effort bandwidth
    siMax[i]=siValue;
    siMin[i]=siValue;
  }

  //Setting up initial timers
  NS_ASSERT_MSG(m_nOnus%siValue==0, "Number of ONUs is not equally distributed among the frames in a SI, you must change the code and manually set up the network");

  for(uint32_t j=0; j<siValue; j++)
  {
    for(uint32_t i=0; i<(m_nOnus/siValue); i++)
    {
      fixedInitialTimers[i+(j*m_nOnus/siValue)]=j;
      assuredInitialTimers[i+(j*m_nOnus/siValue)]=j;
      nonAssuredInitialTimers[i+(j*m_nOnus/siValue)]=j;
      bestEffortInitialTimers[i+(j*m_nOnus/siValue)]=j;
    }
  }


  //Set up initial bufferOccupancies
  for(uint32_t i=0; i<m_nOnus; i++)
  {
    buffOcc[i]=1300; //Enough words so that it needs to be sent in two frames
  }

  /*
   * Call the InitializeTest function from GiantTestCaseBase to initialize the test.
   *
   */
  InitializeTest(   fixedBw,
                    assuredBw,
                    nonAssuredBw,
                    maxBw,
                    siMax,
                    siMin,
                    fixedInitialTimers,
                    assuredInitialTimers,
                    nonAssuredInitialTimers,
                    bestEffortInitialTimers,
                    buffOcc,
                    fec);

  /*
   * DoRun will actually run the tests later on.
   */
}

XgponTestOltDbaEngineGiantFixedPlusAssured::~XgponTestOltDbaEngineGiantFixedPlusAssured ()
{
}

void
XgponTestOltDbaEngineGiantFixedPlusAssured::DoRun (void)
{

  bool dbru=true;
  bool ploamu=false;
  uint8_t fwi=0;
  uint8_t burstProfile=0;
  uint16_t startTime=0;
  uint16_t grantSize=1205;

  uint16_t phyOverhead=m_oltDevice->GetXgponPhy()->GetUsMinimumGuardTime()+(XgponBurstProfile::PSBU_PREAMBLE_DEFAULT_LEN+XgponBurstProfile::PSBU_DELIMITER_DEFAULT_LEN)/4; //(2+5+1)
  uint16_t xgtcOverhead=1+1; //(XgtcHeader (no PLOAM)+ XGTC trailer)
  uint16_t lastEndTime=0;

  //Create expected BwMaps:
  Ptr<XgponXgtcBwmap> exptBwMap1 = Create<XgponXgtcBwmap>();
  for(uint16_t i=0; i<8;i++ )
  {
    startTime=lastEndTime+phyOverhead;
    lastEndTime=startTime+xgtcOverhead+grantSize;
    Ptr<XgponXgtcBwAllocation> exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
    exptBwMap1->AddOneBwAllocation(exptBwAllocation);
  }

  startTime=0;
  lastEndTime=0;

  Ptr<XgponXgtcBwmap> exptBwMap2 = Create<XgponXgtcBwmap>();
  for(uint16_t i=8; i<16;i++ )
  {
    startTime=lastEndTime+phyOverhead;
    lastEndTime=startTime+xgtcOverhead+grantSize;
    Ptr<XgponXgtcBwAllocation> exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
    exptBwMap2->AddOneBwAllocation(exptBwAllocation);
  }

  startTime=0;
  lastEndTime=0;
  grantSize=97; //1300-1204+1=97 (Original Buffer Occupancy + Words served on previous BwMap + DBRu)

  Ptr<XgponXgtcBwmap> exptBwMap3 = Create<XgponXgtcBwmap>();
  for(uint16_t i=0; i<8;i++ )
  {
    startTime=lastEndTime+phyOverhead;
    lastEndTime=startTime+xgtcOverhead+grantSize;
    Ptr<XgponXgtcBwAllocation> exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
    exptBwMap3->AddOneBwAllocation(exptBwAllocation);
  }

  startTime=0;
  lastEndTime=0;
  grantSize=97; //The minimum size a grant can have that has data and a DBRu

  Ptr<XgponXgtcBwmap> exptBwMap4 = Create<XgponXgtcBwmap>();
  for(uint16_t i=8; i<16;i++ )
  {
    startTime=lastEndTime+phyOverhead;
    lastEndTime=startTime+xgtcOverhead+grantSize;
    Ptr<XgponXgtcBwAllocation> exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
    exptBwMap4->AddOneBwAllocation(exptBwAllocation);
  }

  Ptr<XgponXgtcBwmap> bwMap1 = m_dbaEngine->GenerateBwMap();
  Ptr<XgponXgtcBwmap> bwMap2 = m_dbaEngine->GenerateBwMap();
  Ptr<XgponXgtcBwmap> bwMap3 = m_dbaEngine->GenerateBwMap();
  Ptr<XgponXgtcBwmap> bwMap4 = m_dbaEngine->GenerateBwMap();

  CompareBwMaps(exptBwMap1,bwMap1);
  CompareBwMaps(exptBwMap2,bwMap2);
  CompareBwMaps(exptBwMap3,bwMap3);
  CompareBwMaps(exptBwMap4,bwMap4);

  //m_dbaEngine->PrintAllActiveBwmaps();
}


/*
 * This test case is part of a class of tests used to test the GIANT DbaEngine.
 * It will test an OLT with 32 ONUs, each ONU with one T-CONT, each T-CONT with one XGEM-Port.
 *
 * 16 of the T-CONTs will have:
 *    1 - Assured bandwidth of 154.240 Mbps
 *    2 - A service interval of 2 frames.
 *    3 - 1300 words reported in the buffers
 *
 * The other 16 will have:
 *    1 - Non Assured bandwidth of 154.240 Mbps
 *    2 - Service interval of 2 frame
 *    3 - 1300 words reported in the buffers
 *
 * This means the each T-CONT will have 1205 words of assured bandwidth allocated every two frames and requires two
 * bandwidth maps to be served, one with 1205 words (1204 data + 1 DBRu) and another with 2 words (1 data + 1 DBRu)
 * This will test if assured bandwidth works when requests are larger than the available words.
 *  *
 * NOTE: By Assured Bandwidth we refer only to XGTC Layer bandwidth; i.e., no PHY overhead are
 * considered. When assigning Fixed and assured words on upstream frames, the user must make sure he
 * is not overbooking the frame, taking into account the guards.
 *
 */

class XgponTestOltDbaEngineGiantAssuredPlusNonAssured : public XgponTestOltDbaEngineGiantBase
{
public:
  XgponTestOltDbaEngineGiantAssuredPlusNonAssured ();
  virtual ~XgponTestOltDbaEngineGiantAssuredPlusNonAssured ();

private:
  virtual void DoRun (void);


public:

private:


};

// Add some help text to this case to describe what it is intended to test
XgponTestOltDbaEngineGiantAssuredPlusNonAssured::XgponTestOltDbaEngineGiantAssuredPlusNonAssured ()
  : XgponTestOltDbaEngineGiantBase(32, "Test T-CONT with both Assured and Non-Assured Bandwidth")
{

  std::vector<uint64_t> fixedBw(m_nOnus);
  std::vector<uint64_t> assuredBw(m_nOnus);
  std::vector<uint64_t> nonAssuredBw(m_nOnus);
  std::vector<uint64_t> maxBw(m_nOnus);
  std::vector<uint16_t> siMax(m_nOnus);
  std::vector<uint16_t> siMin(m_nOnus);
  std::vector<uint16_t> fixedInitialTimers(m_nOnus);
  std::vector<uint16_t> assuredInitialTimers(m_nOnus);
  std::vector<uint16_t> nonAssuredInitialTimers(m_nOnus);
  std::vector<uint16_t> bestEffortInitialTimers(m_nOnus);
  std::vector<uint16_t> buffOcc(m_nOnus);
  bool fec=false;

  /*
   * Establish the QoS Parameters here:
   *
   */
  //Setting up the bandwidth of the T-CONTs
  uint16_t siValue=2;

  for(uint32_t i=0; i<m_nOnus/2; i++)
  {
    fixedBw[i]=0;
    assuredBw[i]=154240000; //154.112 Mbps
    nonAssuredBw[i]=0;
    maxBw[i]=fixedBw[i]+assuredBw[i]+nonAssuredBw[i]+0;//Zero best effort bandwidth
    siMax[i]=siValue;
    siMin[i]=siValue;
  }

  siValue=2;

  for(uint32_t i=m_nOnus/2; i<m_nOnus; i++)
  {
    fixedBw[i]=0;
    assuredBw[i]=0;
    nonAssuredBw[i]=154240000; //154.112 Mbps
    maxBw[i]=fixedBw[i]+assuredBw[i]+nonAssuredBw[i]+0;//Zero best effort bandwidth
    siMax[i]=siValue;
    siMin[i]=siValue;
  }

  //Setting up initial timers
  for(uint32_t i=0; i<8; i++)
    fixedInitialTimers[i]=assuredInitialTimers[i]=nonAssuredInitialTimers[i]=bestEffortInitialTimers[i]=0;
  for(uint32_t i=8; i<16; i++)
      fixedInitialTimers[i]=assuredInitialTimers[i]=nonAssuredInitialTimers[i]=bestEffortInitialTimers[i]=1;
  for(uint32_t i=16; i<32; i++)
      fixedInitialTimers[i]=assuredInitialTimers[i]=nonAssuredInitialTimers[i]=bestEffortInitialTimers[i]=0;

  //Set up initial bufferOccupancies
  for(uint32_t i=0; i<m_nOnus; i++)
    buffOcc[i]=1300; //Enough words so that it needs to be sent in two frames

  /*
   * Call the InitializeTest function from GiantTestCaseBase to initialize the test.
   *
   */
  InitializeTest(   fixedBw,
                    assuredBw,
                    nonAssuredBw,
                    maxBw,
                    siMax,
                    siMin,
                    fixedInitialTimers,
                    assuredInitialTimers,
                    nonAssuredInitialTimers,
                    bestEffortInitialTimers,
                    buffOcc,
                    fec);

  /*
   * DoRun will actually run the tests later on.
   */
}

XgponTestOltDbaEngineGiantAssuredPlusNonAssured::~XgponTestOltDbaEngineGiantAssuredPlusNonAssured ()
{
}

void
XgponTestOltDbaEngineGiantAssuredPlusNonAssured::DoRun (void)
{

  bool dbru=true;
  bool ploamu=false;
  uint8_t fwi=0;
  uint8_t burstProfile=0;
  uint16_t startTime=0;
  uint16_t grantSize=1205;
  Ptr<XgponXgtcBwAllocation> exptBwAllocation;

  uint16_t phyOverhead=m_oltDevice->GetXgponPhy()->GetUsMinimumGuardTime()+(XgponBurstProfile::PSBU_PREAMBLE_DEFAULT_LEN+XgponBurstProfile::PSBU_DELIMITER_DEFAULT_LEN)/4; //(2+5+1)
  uint16_t xgtcOverhead=1+1; //(XgtcHeader (no PLOAM)+ XGTC trailer)
  uint16_t lastEndTime=0;

  //Create expected BwMaps:
  Ptr<XgponXgtcBwmap> exptBwMap1 = Create<XgponXgtcBwmap>();
  for(uint16_t i=0; i<8;i++ )
  {
    startTime=lastEndTime+phyOverhead;
    lastEndTime=startTime+xgtcOverhead+grantSize;
    exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
    exptBwMap1->AddOneBwAllocation(exptBwAllocation);
  }

  startTime=0;
  lastEndTime=0;

  Ptr<XgponXgtcBwmap> exptBwMap2 = Create<XgponXgtcBwmap>();
  for(uint16_t i=8; i<16;i++ )
  {
    startTime=lastEndTime+phyOverhead;
    lastEndTime=startTime+xgtcOverhead+grantSize;
    exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
    exptBwMap2->AddOneBwAllocation(exptBwAllocation);
  }

  startTime=0;
  lastEndTime=0;
  grantSize=97; //1300-1204+1=97 (Original Buffer Occupancy + Words served on previous BwMap + DBRu)

  Ptr<XgponXgtcBwmap> exptBwMap3 = Create<XgponXgtcBwmap>();
  for(uint16_t i=0; i<8;i++ )
  {
    startTime=lastEndTime+phyOverhead;
    lastEndTime=startTime+xgtcOverhead+grantSize;
    exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
    exptBwMap3->AddOneBwAllocation(exptBwAllocation);
  }
  grantSize=1205;
  for(uint16_t i=16; i<23;i++ )
  {
      startTime=lastEndTime+phyOverhead;
      lastEndTime=startTime+xgtcOverhead+grantSize;
      exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
      exptBwMap3->AddOneBwAllocation(exptBwAllocation);
  }
  startTime=lastEndTime+phyOverhead;
  lastEndTime=startTime+xgtcOverhead+grantSize;
  exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[23]->GetAllocId(), dbru, ploamu, startTime, 349, fwi, burstProfile);
  exptBwMap3->AddOneBwAllocation(exptBwAllocation);


  startTime=0;
  lastEndTime=0;
  grantSize=97; //The minimum size a grant can have that has data and a DBRu

  Ptr<XgponXgtcBwmap> exptBwMap4 = Create<XgponXgtcBwmap>();
  for(uint16_t i=8; i<16;i++ )
  {
    startTime=lastEndTime+phyOverhead;
    lastEndTime=startTime+xgtcOverhead+grantSize;
    exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
    exptBwMap4->AddOneBwAllocation(exptBwAllocation);
  }

  grantSize=1205;
  for(uint16_t i=24; i<31;i++ )
  {
    startTime=lastEndTime+phyOverhead;
    lastEndTime=startTime+xgtcOverhead+grantSize;
    exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
    exptBwMap4->AddOneBwAllocation(exptBwAllocation);
  }

  startTime=lastEndTime+phyOverhead;
  lastEndTime=startTime+xgtcOverhead+grantSize;
  exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[31]->GetAllocId(), dbru, ploamu, startTime, 349, fwi, burstProfile);
  exptBwMap4->AddOneBwAllocation(exptBwAllocation);


  //exptBwMap4->Print(std::cout);

  Ptr<XgponXgtcBwmap> bwMap1 = m_dbaEngine->GenerateBwMap();
  Ptr<XgponXgtcBwmap> bwMap2 = m_dbaEngine->GenerateBwMap();
  Ptr<XgponXgtcBwmap> bwMap3 = m_dbaEngine->GenerateBwMap();
  Ptr<XgponXgtcBwmap> bwMap4 = m_dbaEngine->GenerateBwMap();
  Ptr<XgponXgtcBwmap> bwMap5 = m_dbaEngine->GenerateBwMap();

  CompareBwMaps(exptBwMap1,bwMap1);
  CompareBwMaps(exptBwMap2,bwMap2);
  CompareBwMaps(exptBwMap3,bwMap3);
  CompareBwMaps(exptBwMap4,bwMap4);

  //m_dbaEngine->PrintAllActiveBwmaps();
}

/*
 * This test case is part of a class of tests used to test the GIANT DbaEngine.
 * It will test an OLT with 16 ONUs, each ONU with one T-CONT, each T-CONT with one XGEM-Port.
 *
 * 16 of the T-CONTs will have:
 *    1 - Best effort bandwidth of 204.8 Mbps
 *    2 - A service interval of 1 frame.
 *    3 - 1300 words reported in the buffers
 *
 * This means the each T-CONT will have 1204 words of best effort bandwidth available every frame. This test case will test
 * that the round robin mechanisms of best effort and if the remaining size in the upstream frame is computed correctly, even if
 * FEC is enabled on the T-CONTs
 */

class XgponTestOltDbaEngineGiantBestEffortFecOn : public XgponTestOltDbaEngineGiantBase
{
public:
  XgponTestOltDbaEngineGiantBestEffortFecOn ();
  virtual ~XgponTestOltDbaEngineGiantBestEffortFecOn ();

private:
  virtual void DoRun (void);


public:

private:


};

// Add some help text to this case to describe what it is intended to test
XgponTestOltDbaEngineGiantBestEffortFecOn::XgponTestOltDbaEngineGiantBestEffortFecOn ()
  : XgponTestOltDbaEngineGiantBase(16, "Test T-CONT Best Effort Bandwidth and FEC")
{

  std::vector<uint64_t> fixedBw(m_nOnus);
  std::vector<uint64_t> assuredBw(m_nOnus);
  std::vector<uint64_t> nonAssuredBw(m_nOnus);
  std::vector<uint64_t> maxBw(m_nOnus);
  std::vector<uint16_t> siMax(m_nOnus);
  std::vector<uint16_t> siMin(m_nOnus);
  std::vector<uint16_t> fixedInitialTimers(m_nOnus);
  std::vector<uint16_t> assuredInitialTimers(m_nOnus);
  std::vector<uint16_t> nonAssuredInitialTimers(m_nOnus);
  std::vector<uint16_t> bestEffortInitialTimers(m_nOnus);
  std::vector<uint16_t> buffOcc(m_nOnus);
  bool fec=true;

  /*
   * Establish the QoS Parameters here:
   *
   */
  //Setting up the bandwidth of the T-CONTs
  uint16_t siValue=1;

  for(uint32_t i=0; i<m_nOnus; i++)
  {
    fixedBw[i]=0;
    assuredBw[i]=0;
    nonAssuredBw[i]=0;
    maxBw[i]=fixedBw[i]+assuredBw[i]+nonAssuredBw[i]+ 204800000; //204.8 Mbps best effort bandwidth
    siMax[i]=siValue;
    siMin[i]=siValue;
  }

  //Setting up initial timers
  for(uint32_t i=0; i<m_nOnus; i++)
    fixedInitialTimers[i]=assuredInitialTimers[i]=nonAssuredInitialTimers[i]=bestEffortInitialTimers[i]=0;

  //Set up initial bufferOccupancies
  for(uint32_t i=0; i<m_nOnus; i++)
    buffOcc[i]=1300; //Enough words so that it needs to be sent in two frames

  /*
   * Call the InitializeTest function from GiantTestCaseBase to initialize the test.
   *
   */
  InitializeTest(   fixedBw,
                    assuredBw,
                    nonAssuredBw,
                    maxBw,
                    siMax,
                    siMin,
                    fixedInitialTimers,
                    assuredInitialTimers,
                    nonAssuredInitialTimers,
                    bestEffortInitialTimers,
                    buffOcc,
                    fec);

  /*
   * DoRun will actually run the tests later on.
   */
}

XgponTestOltDbaEngineGiantBestEffortFecOn::~XgponTestOltDbaEngineGiantBestEffortFecOn ()
{
}

void
XgponTestOltDbaEngineGiantBestEffortFecOn::DoRun (void)
{

  bool dbru=true;
  bool ploamu=false;
  uint8_t fwi=0;
  uint8_t burstProfile=0;
  uint16_t startTime=0;
  uint16_t grantSize=1205;
  Ptr<XgponXgtcBwAllocation> exptBwAllocation;

  uint16_t phyOverhead=m_oltDevice->GetXgponPhy()->GetUsMinimumGuardTime()+(XgponBurstProfile::PSBU_PREAMBLE_DEFAULT_LEN+XgponBurstProfile::PSBU_DELIMITER_DEFAULT_LEN)/4; //(2+5+1)
  uint16_t xgtcOverhead=1+1; //(XgtcHeader (no PLOAM)+ XGTC trailer)
  uint16_t lastEndTime=0;

  //Create expected BwMaps:
  Ptr<XgponXgtcBwmap> exptBwMap1 = Create<XgponXgtcBwmap>();
  for(uint16_t i=0; i<m_nOnus;i++ )
  {
    startTime=lastEndTime+phyOverhead;
    lastEndTime=startTime+xgtcOverhead+grantSize;
    exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
    exptBwMap1->AddOneBwAllocation(exptBwAllocation);
  }



  //exptBwMap4->Print(std::cout);

  Ptr<XgponXgtcBwmap> bwMap1 = m_dbaEngine->GenerateBwMap();
  Ptr<XgponXgtcBwmap> bwMap2 = m_dbaEngine->GenerateBwMap();
  //Ptr<XgponXgtcBwmap> bwMap3 = m_dbaEngine->GenerateBwMap();
  //Ptr<XgponXgtcBwmap> bwMap4 = m_dbaEngine->GenerateBwMap();
  //Ptr<XgponXgtcBwmap> bwMap5 = m_dbaEngine->GenerateBwMap();

  //m_dbaEngine->PrintAllActiveBwmaps();
}

/*
 * This test case is part of a class of tests used to test the GIANT DbaEngine.
 * It will test an OLT with 16 ONUs, each ONU with one T-CONT, each T-CONT with one XGEM-Port.
 *
 * 16 of the T-CONTs will have:
 *    1 - Best effort bandwidth of 204.8 Mbps
 *    2 - A service interval of 1 frame.
 *    3 - 1300 words reported in the buffers
 *
 * This means the each T-CONT will have 1204 words of best effort bandwidth available every frame. This test case will test
 * that the round robin mechanisms of best effort and if the remaining size in the upstream frame is computed correctly, even if
 * FEC is enabled on the T-CONTs
 */

class XgponTestOltDbaEngineGiantFixedPlusBestEffortFecOn : public XgponTestOltDbaEngineGiantBase
{
public:
  XgponTestOltDbaEngineGiantFixedPlusBestEffortFecOn ();
  virtual ~XgponTestOltDbaEngineGiantFixedPlusBestEffortFecOn ();

private:
  virtual void DoRun (void);


public:

private:


};

// Add some help text to this case to describe what it is intended to test
XgponTestOltDbaEngineGiantFixedPlusBestEffortFecOn::XgponTestOltDbaEngineGiantFixedPlusBestEffortFecOn ()
  : XgponTestOltDbaEngineGiantBase(16, "Test T-CONT width Fixed and Best Effort Bandwidth plus FEC")
{

  std::vector<uint64_t> fixedBw(m_nOnus);
  std::vector<uint64_t> assuredBw(m_nOnus);
  std::vector<uint64_t> nonAssuredBw(m_nOnus);
  std::vector<uint64_t> maxBw(m_nOnus);
  std::vector<uint16_t> siMax(m_nOnus);
  std::vector<uint16_t> siMin(m_nOnus);
  std::vector<uint16_t> fixedInitialTimers(m_nOnus);
  std::vector<uint16_t> assuredInitialTimers(m_nOnus);
  std::vector<uint16_t> nonAssuredInitialTimers(m_nOnus);
  std::vector<uint16_t> bestEffortInitialTimers(m_nOnus);
  std::vector<uint16_t> buffOcc(m_nOnus);
  bool fec=true;

  /*
   * Establish the QoS Parameters here:
   *
   */
  //Setting up the bandwidth of the T-CONTs
  uint16_t siValue=1;

  for(uint32_t i=0; i<m_nOnus; i++)
  {
    fixedBw[i]=256000;
    assuredBw[i]=0;
    nonAssuredBw[i]=0;
    maxBw[i]=fixedBw[i]+assuredBw[i]+nonAssuredBw[i]+ 204800000; //204.8 Mbps best effort bandwidth
    siMax[i]=siValue;
    siMin[i]=siValue;
  }

  //Setting up initial timers
  for(uint32_t i=0; i<m_nOnus; i++)
    fixedInitialTimers[i]=assuredInitialTimers[i]=nonAssuredInitialTimers[i]=bestEffortInitialTimers[i]=0;

  //Set up initial bufferOccupancies
  for(uint32_t i=0; i<m_nOnus; i++)
    buffOcc[i]=1300; //Enough words so that it needs to be sent in two frames

  /*
   * Call the InitializeTest function from GiantTestCaseBase to initialize the test.
   *
   */
  InitializeTest(   fixedBw,
                    assuredBw,
                    nonAssuredBw,
                    maxBw,
                    siMax,
                    siMin,
                    fixedInitialTimers,
                    assuredInitialTimers,
                    nonAssuredInitialTimers,
                    bestEffortInitialTimers,
                    buffOcc,
                    fec);

  /*
   * DoRun will actually run the tests later on.
   */
}

XgponTestOltDbaEngineGiantFixedPlusBestEffortFecOn::~XgponTestOltDbaEngineGiantFixedPlusBestEffortFecOn ()
{
}

void
XgponTestOltDbaEngineGiantFixedPlusBestEffortFecOn::DoRun (void)
{

  bool dbru=true;
  bool ploamu=false;
  uint8_t fwi=0;
  uint8_t burstProfile=0;
  uint16_t startTime=0;
  uint16_t grantSize=1205;
  Ptr<XgponXgtcBwAllocation> exptBwAllocation;

  uint16_t phyOverhead=m_oltDevice->GetXgponPhy()->GetUsMinimumGuardTime()+(XgponBurstProfile::PSBU_PREAMBLE_DEFAULT_LEN+XgponBurstProfile::PSBU_DELIMITER_DEFAULT_LEN)/4; //(2+5+1)
  uint16_t xgtcOverhead=1+1; //(XgtcHeader (no PLOAM)+ XGTC trailer)
  uint16_t lastEndTime=0;

  //Create expected BwMaps:
  Ptr<XgponXgtcBwmap> exptBwMap1 = Create<XgponXgtcBwmap>();
  for(uint16_t i=0; i<m_nOnus;i++ )
  {
    startTime=lastEndTime+phyOverhead;
    lastEndTime=startTime+xgtcOverhead+grantSize;
    exptBwAllocation =  Create<XgponXgtcBwAllocation>(m_tcontsList[i]->GetAllocId(), dbru, ploamu, startTime, grantSize, fwi, burstProfile);
    exptBwMap1->AddOneBwAllocation(exptBwAllocation);
  }



  //exptBwMap4->Print(std::cout);

  Ptr<XgponXgtcBwmap> bwMap1 = m_dbaEngine->GenerateBwMap();
  Ptr<XgponXgtcBwmap> bwMap2 = m_dbaEngine->GenerateBwMap();
  //Ptr<XgponXgtcBwmap> bwMap3 = m_dbaEngine->GenerateBwMap();
  //Ptr<XgponXgtcBwmap> bwMap4 = m_dbaEngine->GenerateBwMap();
  //Ptr<XgponXgtcBwmap> bwMap5 = m_dbaEngine->GenerateBwMap();

  m_dbaEngine->PrintAllActiveBwmaps();
}








// The TestSuite class names the TestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run.  Typically, only the constructor for
// this class must be defined
//
class XgponTestOltDbaGiantSuite : public TestSuite
{
public:
    XgponTestOltDbaGiantSuite ();
};

XgponTestOltDbaGiantSuite::XgponTestOltDbaGiantSuite()
  : TestSuite ("xgpon-dba-giant", UNIT)
{
  AddTestCase (new XgponTestOltDbaEngineGiantMinimumFixed, TestCase::QUICK);
  AddTestCase (new XgponTestOltDbaEngineGiantAssuredLowOccBuff, TestCase::QUICK);
  AddTestCase (new XgponTestOltDbaEngineGiantAssuredHighOccBuff, TestCase::QUICK);
  AddTestCase (new XgponTestOltDbaEngineGiantFixedPlusAssured, TestCase::QUICK);
  AddTestCase (new XgponTestOltDbaEngineGiantAssuredPlusNonAssured, TestCase::QUICK);
  //AddTestCase (new XgponTestOltDbaEngineGiantAssuredPlusNonAssuredPlusBestEffort, TestCase::QUICK);
  AddTestCase (new XgponTestOltDbaEngineGiantBestEffortFecOn, TestCase::QUICK);
  AddTestCase (new XgponTestOltDbaEngineGiantFixedPlusBestEffortFecOn, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static XgponTestOltDbaGiantSuite xgponTestOltDbaGiantSuite;
