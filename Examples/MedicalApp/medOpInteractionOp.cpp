/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: medOpInteractionOp.cpp,v $
  Language:  C++
  Date:      $Date: 2008-04-28 09:02:40 $
  Version:   $Revision: 1.1 $
  Authors:   Marco Petrone
==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------


#include "medOpInteractionOp.h"
#include "mmgGui.h"

#include "mmiCompositorMouse.h"
#include "mmiGenericMouse.h"

#include "mafVMEGizmo.h"

#include "vtkMAFSmartPointer.h"
#include "vtkSphereSource.h"
#include "vtkPolyData.h"

/******************************************************************************/
/** UNCOMMENT HERE TO DECLARE THE ACTIONS               (PASS 1)              */
/* *
//----------------------------------------------------------------------------
const char *medOpInteractionOp::m_Actions[] = {"MyCustomAction",NULL};
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
const char ** medOpInteractionOp::GetActions()
//----------------------------------------------------------------------------
{
  return m_Actions;
}
/******************************************************************************/


//----------------------------------------------------------------------------
medOpInteractionOp::medOpInteractionOp(wxString label)
: mafOp(label)
//----------------------------------------------------------------------------
{
  
}
//----------------------------------------------------------------------------
medOpInteractionOp::~medOpInteractionOp()
//----------------------------------------------------------------------------
{
}
//----------------------------------------------------------------------------
mafOp* medOpInteractionOp::Copy()
//----------------------------------------------------------------------------
{
  return new medOpInteractionOp(m_Label);
}

//----------------------------------------------------------------------------
void medOpInteractionOp::OpRun()
//----------------------------------------------------------------------------
{
  // CREATE A SPHERE GIZMO
  mafNEW(m_Gizmo);

  vtkMAFSmartPointer<vtkSphereSource> sphere;
  sphere->SetRadius(10);
  m_Gizmo->SetData(sphere->GetOutput());
  
  // add the VME Gizmo to the VME tree
  mafEventMacro( mafEvent( this, VME_ADD, m_Gizmo ) );



  /******************************************************************************/
  /** UNCOMMENT HERE TO CREATE INTERACTORS TO MOVE THE GIZMO  WITH MOUSE (PASS 2)*/
  /* *
  //----------------------------------------------------------------------------
	// compositor
	//----------------------------------------------------------------------------
  mafNEW(m_MouseInteractor); // mouse compositor 
   
	//----------------------------------------------------------------------------
	// translate
	//----------------------------------------------------------------------------
  mmiGenericMouse *mmi_translate = m_MouseInteractor->CreateBehavior(MOUSE_LEFT);
  
  // set to send transform matrix for the gizmo to the operation
  mmi_translate->SetVME(m_Gizmo);
  mmi_translate->SetListener(this);
  mmi_translate->EnableTranslation(true);
  /******************************************************************************/


  /******************************************************************************/
  /** UNCOMMENT HERE TO ATTACH THE INTERACTOR TO THE ACTION    (PASS 3)         */
	/* *
  mafEventMacro( mafEvent( this, INTERACTOR_ADD, m_MouseInteractor, &wxString(m_Actions[0]) ) );
  /******************************************************************************/

  /******************************************************************************/
  /** UNCOMMENT HERE TO ATTACH THE INTERACTOR TO THE VME       (PASS 4)        */
	/* *
  mafVmeData *toolData = (mafVmeData*)m_Gizmo->GetClientData();
  toolData->m_behavior = m_MouseInteractor;
  /******************************************************************************/
  
  
  // the interface
  m_Gui = new mmgGui(this);
  m_Gui->SetListener(this);
  m_Gui->OkCancel();
	m_Gui->Label("");
	ShowGui(); // display the GUI
}
//----------------------------------------------------------------------------
void medOpInteractionOp::OpDo()
//----------------------------------------------------------------------------
{
}
//----------------------------------------------------------------------------
void medOpInteractionOp::OpUndo()
//----------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------
void medOpInteractionOp::OpStop(int result)
{	
  HideGui();
  
  /******************************************************************************/
  /** UNCOMMENT HERE TO DETACH THE INTERACTOR FROM THE ACTION      (PASS 3)     */
	/* *
  mafEventMacro( mafEvent( this, INTERACTOR_REMOVE, m_MouseInteractor, &wxString(m_Actions[0]) ) );
  /******************************************************************************/


  /******************************************************************************/
  /** UNCOMMENT HERE TO DETACH THE INTERACTOR FROM THE VME         (PASS 4)     */
	/* *
  mafVmeData *toolData = (mafVmeData*)m_Gizmo->GetClientData();
  toolData->m_behavior = NULL;
  /******************************************************************************/

  /******************************************************************************/
  /** UNCOMMENT HERE TO DESTROY THE INTERACTOR                     (PASS 2)     */
	/* *
  mafDEL(m_MouseInteractor);
  /******************************************************************************/
  

  // REMOVE VME-GIZMO FROM THE TREE
  mafEventMacro( mafEvent( this, VME_REMOVE, m_Gizmo ) );
  vtkDEL(m_Gizmo);

  mafEventMacro(mafEvent(this,result));
}

//----------------------------------------------------------------------------
void medOpInteractionOp::OnEvent(mafEventBase *e) 
//----------------------------------------------------------------------------
{ 

  switch(e->GetId())
	{
	case wxOK:
		OpStop(OP_RUN_OK);
	break;
	case wxCANCEL:
		OpStop(OP_RUN_CANCEL);
  break;
  case ID_TRANSFORM:
  {
    mafEvent *event=mafEvent::SafeDownCast(e);
    assert(event);
    m_Gizmo->ApplyMatrix(*(event->GetMatrix()),POST_MULTIPLY);
    mafEventMacro(mafEvent(this,CAMERA_UPDATE));
  }    
  break;
	default:
		mafEventMacro(*e); 
  break;
	}
}






