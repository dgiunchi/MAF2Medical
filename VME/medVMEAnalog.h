/*=========================================================================

 Program: MAF2Medical
 Module: medVMEAnalog
 Authors: Roberto Mucci
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __medVMEAnalog_h
#define __medVMEAnalog_h
//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "medVMEDefines.h"
#include "mafVMEScalarMatrix.h"

//----------------------------------------------------------------------------
// forward declarations :
//----------------------------------------------------------------------------
class mafVMEOutputScalarMatrix;


/** VME child of mafVMEScalarMatrix, for EMG data. Refactor CreateGui and Print methods not to show parameters used for VTK representation */
class MED_VME_EXPORT medVMEAnalog : public mafVMEScalarMatrix
{
public:
  mafTypeMacro(medVMEAnalog, mafVMEScalarMatrix);

  /** print a dump of this object */
  virtual void Print(std::ostream& os, const int tabs=0);

  /** Get the time bounds for the time varying VME based on scalar matrix.*/
  void GetLocalTimeBounds(mafTimeStamp tbounds[2]);

  /** Return true if this VME has more than one time stamp, corresponding to the number of elements in the first row.*/
  bool IsAnimated();

  void GetTimeBounds(mafTimeStamp tbounds[2]); 

  /** Return the list of timestamps for this VME. Timestamps list is 
  obtained by extracting the first row of the scalar matrix.*/
  void GetLocalTimeStamps(std::vector<mafTimeStamp> &kframes);

protected:
  medVMEAnalog();
  virtual ~medVMEAnalog();

  /** Internally used to create a new instance of the GUI.*/
  virtual mafGUI *CreateGui();

  mafTimeStamp    m_CurrentTime;  ///< the time parameter for generation of the output

};
#endif
