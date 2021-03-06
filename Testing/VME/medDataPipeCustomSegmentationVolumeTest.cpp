/*=========================================================================

 Program: MAF2Medical
 Module: medDataPipeCustomSegmentationVolumeTest
 Authors: Alberto Losi, Gianluigi Crimi
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/


#include "mafDefines.h"
#include "medDefines.h"
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include <cppunit/config/SourcePrefix.h>
#include "mafVMEVolumeGray.h"
#include "mafSmartPointer.h"
#include "medDataPipeCustomSegmentationVolumeTest.h"
#include "medDataPipeCustomSegmentationVolume.h"
#include "vtkStructuredPointsReader.h"
#include "vtkRectilinearGridReader.h"
#include "vtkImageData.h"
#include "vtkRectilinearGrid.h"
#include "medVMESegmentationVolume.h"

//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::setUp()
//----------------------------------------------------------------------------
{
  vtkStructuredPointsReader *readerSP;
  vtkRectilinearGridReader *readerRG;
  
  vtkNEW(readerSP);
  mafString filename = MED_DATA_ROOT;
  filename << "/Test_DataPipeCustomSegmentationVolume/VolumeSP.vtk";
  readerSP->SetFileName(filename.GetCStr());
  readerSP->Update();
  // Volume with 11 slices and scalar range between 0 and 9
  mafNEW(m_VolumeSP);
  m_VolumeSP->SetData((vtkImageData*)readerSP->GetOutput(),0.0);
  m_VolumeSP->GetOutput()->GetVTKData()->Update();
  m_VolumeSP->GetOutput()->Update();
  m_VolumeSP->Update();
  vtkDEL(readerSP);

  vtkNEW(readerRG);
  filename = MED_DATA_ROOT;
  filename << "/Test_DataPipeCustomSegmentationVolume/VolumeRG.vtk";
  readerRG->SetFileName(filename.GetCStr());
  readerRG->Update();
  // Rectilinear Grid  10x10x10 values between 1 and 10
  mafNEW(m_VolumeRG);
  m_VolumeRG->SetData((vtkRectilinearGrid*)readerRG->GetOutput(),0.0);
  m_VolumeRG->GetOutput()->GetVTKData()->Update();
  m_VolumeRG->GetOutput()->Update();
  m_VolumeRG->Update();
  vtkDEL(readerRG);
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::tearDown()
//----------------------------------------------------------------------------
{
  mafDEL(m_VolumeSP);
  mafDEL(m_VolumeRG);
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::TestFixture()
//----------------------------------------------------------------------------
{
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::TestDynamicAllocation()
//----------------------------------------------------------------------------
{
  medDataPipeCustomSegmentationVolume *pipe;
  mafNEW(pipe);
  CPPUNIT_ASSERT(pipe != NULL);
  mafDEL(pipe);

//   pipe = new medDataPipeCustomSegmentationVolume();
//   CPPUNIT_ASSERT(pipe != NULL);
//   cppDEL(pipe);
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::DeepCopyTest()
//----------------------------------------------------------------------------
{
//   medDataPipeCustomSegmentationVolume *pipe;
//   mafNEW(pipe);
// 
//   medDataPipeCustomSegmentationVolume *pipe_copy;
//   mafNEW(pipe_copy);
// 
//   pipe_copy->DeepCopy(pipe);
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::SetGetVolumeTest()
//----------------------------------------------------------------------------
{
  mafSmartPointer<medDataPipeCustomSegmentationVolume>pipe;

  pipe->SetVolume(m_VolumeSP);

  CPPUNIT_ASSERT(pipe->GetVolume() == m_VolumeSP);
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::SetGetAutomaticSegmentationThresholdModalityTest()
//----------------------------------------------------------------------------
{
  medDataPipeCustomSegmentationVolume *pipe;
  mafNEW(pipe);
  
  int modality = medVMESegmentationVolume::RANGE;
  pipe->SetAutomaticSegmentationThresholdModality(modality);

  CPPUNIT_ASSERT(pipe->GetAutomaticSegmentationThresholdModality() == modality);

  modality = medVMESegmentationVolume::GLOBAL;
  pipe->SetAutomaticSegmentationThresholdModality(modality);

  CPPUNIT_ASSERT(pipe->GetAutomaticSegmentationThresholdModality() == modality);

  mafDEL(pipe);
}

//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::AddGetRangeTest()
//----------------------------------------------------------------------------
{
  medDataPipeCustomSegmentationVolume *pipe;
  mafNEW(pipe);

  pipe->SetVME(m_VolumeSP);
  //pipe->SetVolume(m_Volume);

  CPPUNIT_ASSERT(MAF_OK == pipe->AddRange(0,1,5));
  CPPUNIT_ASSERT(MAF_OK == pipe->AddRange(2,3,4));
  CPPUNIT_ASSERT(2 == pipe->GetNumberOfRanges());
  CPPUNIT_ASSERT(MAF_ERROR == pipe->AddRange(0,1,6));
  
  int startSlice;
  int endSlice;
  double threshold;

  CPPUNIT_ASSERT(MAF_OK == pipe->GetRange(0,startSlice,endSlice,threshold));
  CPPUNIT_ASSERT(0 == startSlice && 1 == endSlice && 5 == threshold);

  CPPUNIT_ASSERT(MAF_OK == pipe->GetRange(1,startSlice,endSlice,threshold));
  CPPUNIT_ASSERT(2 == startSlice && 3 == endSlice && 4 == threshold);

  CPPUNIT_ASSERT(MAF_ERROR == pipe->GetRange(2,startSlice,endSlice,threshold));

  mafDEL(pipe);
  delete wxLog::SetActiveTarget(NULL);
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::UpdateRangeTest()
//----------------------------------------------------------------------------
{
  medDataPipeCustomSegmentationVolume *pipe;
  mafNEW(pipe);

  pipe->SetVME(m_VolumeSP);
  //pipe->SetVolume(m_Volume);

  pipe->AddRange(0,1,5);
  pipe->AddRange(2,3,4);

  int startSlice = 2;
  int endSlice = 3;
  double threshold = 5;

  CPPUNIT_ASSERT(MAF_OK == pipe->UpdateRange(1,startSlice,endSlice,threshold));
  CPPUNIT_ASSERT(MAF_ERROR == pipe->UpdateRange(2,startSlice,endSlice,threshold));

  startSlice = 1;
  CPPUNIT_ASSERT(MAF_ERROR == pipe->UpdateRange(1,startSlice,endSlice,threshold));

  pipe->GetRange(1,startSlice,endSlice,threshold);
  CPPUNIT_ASSERT(2 == startSlice && 3 == endSlice && 5 == threshold);

  mafDEL(pipe);
  delete wxLog::SetActiveTarget(NULL);
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::DeleteRangeTest()
//----------------------------------------------------------------------------
{
  medDataPipeCustomSegmentationVolume *pipe;
  mafNEW(pipe);

  pipe->SetVME(m_VolumeSP);
  //pipe->SetVolume(m_Volume);

  pipe->AddRange(0,1,5);
  pipe->AddRange(2,3,4);

  int startSlice = 2;
  int endSlice = 3;
  double threshold = 5;

  CPPUNIT_ASSERT(MAF_OK == pipe->DeleteRange(1));
  CPPUNIT_ASSERT(1 == pipe->GetNumberOfRanges());

  CPPUNIT_ASSERT(MAF_ERROR == pipe->DeleteRange(2));

  CPPUNIT_ASSERT(MAF_ERROR == pipe->GetRange(1,startSlice,endSlice,threshold));

  mafDEL(pipe);
  delete wxLog::SetActiveTarget(NULL);
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::RemoveAllRangesTest()
//----------------------------------------------------------------------------
{
  medDataPipeCustomSegmentationVolume *pipe;
  mafNEW(pipe);

  pipe->SetVME(m_VolumeSP);
  //pipe->SetVolume(m_Volume);

  pipe->AddRange(0,1,5);
  pipe->AddRange(2,3,4);

  CPPUNIT_ASSERT(MAF_OK == pipe->RemoveAllRanges());

  CPPUNIT_ASSERT(0 == pipe->GetNumberOfRanges());

  mafDEL(pipe);
  delete wxLog::SetActiveTarget(NULL);
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::GetNumberOfRangesTest()
//----------------------------------------------------------------------------
{
  medDataPipeCustomSegmentationVolume *pipe;
  mafNEW(pipe);

  pipe->SetVME(m_VolumeSP);
  //pipe->SetVolume(m_Volume);

  pipe->AddRange(0,1,5);
  pipe->AddRange(2,3,4);
  CPPUNIT_ASSERT(2 == pipe->GetNumberOfRanges());

  pipe->DeleteRange(1);
  CPPUNIT_ASSERT(1 == pipe->GetNumberOfRanges());
  
  pipe->RemoveAllRanges();

  CPPUNIT_ASSERT(0 == pipe->GetNumberOfRanges());

  mafDEL(pipe);
  delete wxLog::SetActiveTarget(NULL);
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::SetGetAutomaticSegmentationGlobalThresholdTest()
//----------------------------------------------------------------------------
{
  medDataPipeCustomSegmentationVolume *pipe;
  mafNEW(pipe);

  pipe->SetAutomaticSegmentationGlobalThreshold(5.);
  CPPUNIT_ASSERT(5 == pipe->GetAutomaticSegmentationGlobalThreshold());

  mafDEL(pipe);
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::CheckNumberOfThresholdsTest()
//----------------------------------------------------------------------------
{
  medDataPipeCustomSegmentationVolume *pipe;
  mafNEW(pipe);

  pipe->SetVME(m_VolumeSP);
  pipe->SetVolume(m_VolumeSP);

  pipe->SetAutomaticSegmentationThresholdModality(medVMESegmentationVolume::GLOBAL);
  CPPUNIT_ASSERT(true == pipe->CheckNumberOfThresholds());

  pipe->SetAutomaticSegmentationThresholdModality(medVMESegmentationVolume::RANGE);
  CPPUNIT_ASSERT(false == pipe->CheckNumberOfThresholds());

  pipe->AddRange(0,9,5);
  CPPUNIT_ASSERT(false == pipe->CheckNumberOfThresholds());

  pipe->AddRange(10,11,6);
  CPPUNIT_ASSERT(true == pipe->CheckNumberOfThresholds());

  mafDEL(pipe);
  delete wxLog::SetActiveTarget(NULL);
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::SetGetManualVolumeMaskTest()
//----------------------------------------------------------------------------
{
  medDataPipeCustomSegmentationVolume *pipe;
  mafNEW(pipe);
  
  pipe->SetManualVolumeMask(m_VolumeSP);
  CPPUNIT_ASSERT(pipe->GetManualVolumeMask() == m_VolumeSP);

  mafDEL(pipe);
}
//----------------------------------------------------------------------------
// GIGI
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::GetAutomaticOutputTest()
//----------------------------------------------------------------------------
{
  
  medDataPipeCustomSegmentationVolume *pipe;

  //Check  without a volume
  mafNEW(pipe);
  CPPUNIT_ASSERT(pipe->GetAutomaticOutput()==NULL);
  mafDEL(pipe);

  //Check  the structured points case
  mafNEW(pipe);
  pipe->SetVolume(m_VolumeSP);
  CPPUNIT_ASSERT(pipe->GetAutomaticOutput()!=NULL);
  mafDEL(pipe);

  //Check  the rectilinear grid case
  mafNEW(pipe);
  pipe->SetVolume(m_VolumeRG);
  CPPUNIT_ASSERT(pipe->GetAutomaticOutput()!=NULL);
  mafDEL(pipe);

}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::GetManualOutputTest()
//----------------------------------------------------------------------------
{

  medDataPipeCustomSegmentationVolume *pipe;

  //Check  without a volume
  mafNEW(pipe);
  CPPUNIT_ASSERT(pipe->GetManualOutput()==NULL);
  mafDEL(pipe);

  //Check  the structured points case
  mafNEW(pipe);
  pipe->SetVolume(m_VolumeSP);
  CPPUNIT_ASSERT(pipe->GetManualOutput()!=NULL);
  mafDEL(pipe);

  //Check  the rectilinear grid case
  mafNEW(pipe);
  pipe->SetVolume(m_VolumeRG);
  CPPUNIT_ASSERT(pipe->GetManualOutput()!=NULL);
  mafDEL(pipe);

}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::GetRefinementOutputTest()
//----------------------------------------------------------------------------
{
  medDataPipeCustomSegmentationVolume *pipe;

  //Check  without a volume
  mafNEW(pipe);
  CPPUNIT_ASSERT(pipe->GetRefinementOutput()==NULL);
  mafDEL(pipe);

  //Check  the structured points case
  mafNEW(pipe);
  pipe->SetVolume(m_VolumeSP);
  CPPUNIT_ASSERT(pipe->GetRefinementOutput()!=NULL);
  mafDEL(pipe);

  //Check  the rectilinear grid case
  mafNEW(pipe);
  pipe->SetVolume(m_VolumeRG);
  CPPUNIT_ASSERT(pipe->GetRefinementOutput()!=NULL);
  mafDEL(pipe);
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::GetRegionGrowingOutputTest()
//----------------------------------------------------------------------------
{
  medDataPipeCustomSegmentationVolume *pipe;

  //Check  without a volume
  mafNEW(pipe);
  CPPUNIT_ASSERT(pipe->GetRegionGrowingOutput()==NULL);
  mafDEL(pipe);

  //Check  the structured points case
  mafNEW(pipe);
  pipe->SetVolume(m_VolumeSP);
  CPPUNIT_ASSERT(pipe->GetRegionGrowingOutput()!=NULL);
  pipe->SetRegionGrowingLowerThreshold(1);
  pipe->SetRegionGrowingUpperThreshold(7);
  CPPUNIT_ASSERT(pipe->GetRegionGrowingOutput()!=NULL);
  mafDEL(pipe);

  //Check  the rectilinear grid case
  mafNEW(pipe);
  pipe->SetVolume(m_VolumeRG);
  CPPUNIT_ASSERT(pipe->GetRegionGrowingOutput()!=NULL);
  pipe->SetRegionGrowingLowerThreshold(1);
  pipe->SetRegionGrowingUpperThreshold(7);
  CPPUNIT_ASSERT(pipe->GetRegionGrowingOutput()!=NULL);
  mafDEL(pipe);
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::SetGetRegionGrowingUpperThresholdTest()
//----------------------------------------------------------------------------
{
  medDataPipeCustomSegmentationVolume *pipe;
  mafNEW(pipe);

  pipe->SetRegionGrowingUpperThreshold(7);
  CPPUNIT_ASSERT(7 == pipe->GetRegionGrowingUpperThreshold());

  mafDEL(pipe);
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::SetGetRegionGrowingLowerThresholdTest()
//----------------------------------------------------------------------------
{
  medDataPipeCustomSegmentationVolume *pipe;
  mafNEW(pipe);
  
  pipe->SetRegionGrowingLowerThreshold(7);
  CPPUNIT_ASSERT(7 == pipe->GetRegionGrowingLowerThreshold());

  mafDEL(pipe);
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::SetGetRegionGrowingSliceRangeTest()
//----------------------------------------------------------------------------
{
  medDataPipeCustomSegmentationVolume *pipe;
  mafNEW(pipe);

  int start,end;

  pipe->SetRegionGrowingSliceRange(2,7);

  pipe->GetRegionGrowingSliceRange(start,end);

  CPPUNIT_ASSERT(2==start && 7==end);
  
  mafDEL(pipe);

}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::AddGetSeedTest()
//----------------------------------------------------------------------------
{
  int s1[]={1,2,3};
  int s2[]={4,5,6};
  int s3[]={7,8,9};

  int r2[3];

  medDataPipeCustomSegmentationVolume *pipe;
  mafNEW(pipe);

  CPPUNIT_ASSERT(pipe->AddSeed(s1)==MAF_OK);
  CPPUNIT_ASSERT(pipe->AddSeed(s2)==MAF_OK);
  CPPUNIT_ASSERT(pipe->AddSeed(s3)==MAF_OK);
  
  //out of bound error check
  CPPUNIT_ASSERT(pipe->GetSeed(-1,r2)==MAF_ERROR);
  CPPUNIT_ASSERT(pipe->GetSeed(3,r2)==MAF_ERROR);

  CPPUNIT_ASSERT(pipe->GetSeed(1,r2)==MAF_OK);

  CPPUNIT_ASSERT(s2[0]==r2[0] && s2[1]==r2[1] && s2[2]==r2[2]);
  
  mafDEL(pipe);
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::DeleteSeedTest()
//----------------------------------------------------------------------------
{
int s1[]={1,2,3};
  int s2[]={4,5,6};
  int s3[]={7,8,9};

  int r3[3];

  medDataPipeCustomSegmentationVolume *pipe;
  mafNEW(pipe);

  CPPUNIT_ASSERT(pipe->AddSeed(s1)==MAF_OK);
  CPPUNIT_ASSERT(pipe->AddSeed(s2)==MAF_OK);
  CPPUNIT_ASSERT(pipe->AddSeed(s3)==MAF_OK);
  

  //out of bound error check
  CPPUNIT_ASSERT(pipe->DeleteSeed(-1)==MAF_ERROR);
  CPPUNIT_ASSERT(pipe->DeleteSeed(3)==MAF_ERROR);

  
  CPPUNIT_ASSERT(pipe->DeleteSeed(1)==MAF_OK);
  //now s3 will be in 2-nd pos
  CPPUNIT_ASSERT(pipe->GetSeed(1,r3)==MAF_OK);
  CPPUNIT_ASSERT(s3[0]==r3[0] && s3[1]==r3[1] && s3[2]==r3[2]);
  
  mafDEL(pipe);
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::RemoveAllSeedsTest()
//----------------------------------------------------------------------------
{
  int s1[]={1,2,3};
  int s2[]={4,5,6};
  int s3[]={7,8,9};

  int r2[3];

  medDataPipeCustomSegmentationVolume *pipe;
  mafNEW(pipe);

  CPPUNIT_ASSERT(pipe->AddSeed(s1)==MAF_OK);
  CPPUNIT_ASSERT(pipe->AddSeed(s2)==MAF_OK);
  CPPUNIT_ASSERT(pipe->AddSeed(s3)==MAF_OK);

  CPPUNIT_ASSERT(pipe->RemoveAllSeeds()==MAF_OK);

  CPPUNIT_ASSERT(pipe->GetSeed(0,r2)==MAF_ERROR);

  
  mafDEL(pipe);
}
//----------------------------------------------------------------------------
void medDataPipeCustomSegmentationVolumeTest::GetNumberOfSeedsTest()
//----------------------------------------------------------------------------
{
  int s1[]={1,2,3};

  medDataPipeCustomSegmentationVolume *pipe;
  mafNEW(pipe);

  CPPUNIT_ASSERT(pipe->GetNumberOfSeeds()==0);

  CPPUNIT_ASSERT(pipe->AddSeed(s1)==MAF_OK);
  CPPUNIT_ASSERT(pipe->AddSeed(s1)==MAF_OK);
  CPPUNIT_ASSERT(pipe->AddSeed(s1)==MAF_OK);
  CPPUNIT_ASSERT(pipe->GetNumberOfSeeds()==3);

  CPPUNIT_ASSERT(pipe->DeleteSeed(1)==MAF_OK);
  CPPUNIT_ASSERT(pipe->GetNumberOfSeeds()==2);
  
  CPPUNIT_ASSERT(pipe->RemoveAllSeeds()==MAF_OK);
  CPPUNIT_ASSERT(pipe->GetNumberOfSeeds()==0);

  mafDEL(pipe);
}