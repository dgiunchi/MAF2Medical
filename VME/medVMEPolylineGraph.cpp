/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: medVMEPolylineGraph.cpp,v $
Language:  C++
Date:      $Date: 2007-07-03 10:00:31 $
Version:   $Revision: 1.1 $
Authors:   Matteo Giacomoni
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)

MafMedical Library use license agreement

The software named MafMedical Library and any accompanying documentation, 
manuals or data (hereafter collectively "SOFTWARE") is property of the SCS s.r.l.
This is an open-source copyright as follows:
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, 
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, 
this list of conditions and the following disclaimer in the documentation and/or 
other materials provided with the distribution.
* Modified source versions must be plainly marked as such, and must not be misrepresented 
as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

MafMedical is partially based on OpenMAF.
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "medVMEPolylineGraph.h"
#include "mafObject.h"
#include "mafPolylineGraph.h"
#include "mafVMEOutputPolyline.h"

#include "vtkDataSet.h"
#include "vtkPolyData.h"
#include "vtkCellArray.h"
#include "vtkMath.h"

//-------------------------------------------------------------------------
mafCxxTypeMacro(medVMEPolylineGraph)
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
medVMEPolylineGraph::medVMEPolylineGraph()
//-------------------------------------------------------------------------
{
	m_PolylineGraph = NULL;

	m_PolylineGraph = new mafPolylineGraph;
	m_PolylineGraph->AddNewBranch();

	m_CurrentBranch = 0;
}

//-------------------------------------------------------------------------
medVMEPolylineGraph::~medVMEPolylineGraph()
//-------------------------------------------------------------------------
{
	cppDEL(m_PolylineGraph);
}
//-------------------------------------------------------------------------
int medVMEPolylineGraph::SetData(vtkDataSet *data, mafTimeStamp t, int mode)
//-------------------------------------------------------------------------
{
	assert(data);
	vtkPolyData *polydata = vtkPolyData::SafeDownCast(data);

	if (polydata)
		polydata->Update();

	// check this is a polydata containing only lines
	if (polydata && polydata->GetPolys()->GetNumberOfCells()==0 && \
		polydata->GetStrips()->GetNumberOfCells()==0 && \
		polydata->GetVerts()->GetNumberOfCells()==0)
	{
		m_PolylineGraph->CopyFromPolydata(polydata);
		if(m_PolylineGraph->SelfCheck())
		{
			m_PolylineGraph->CopyToPolydata(polydata);
			return Superclass::SetData(data,t,mode);
		}
		else
			return MAF_ERROR;
	}

	mafErrorMacro("Trying to set the wrong type of fata inside a VME Polyline :"<< (data?data->GetClassName():"NULL"));
	return MAF_ERROR;
}
//-------------------------------------------------------------------------
int medVMEPolylineGraph::AddNewBranch(vtkIdType vertexID,mafString *name)
//-------------------------------------------------------------------------
{
	m_PolylineGraph->AddNewBranch(vertexID,(wxString*)name);
	vtkPolyData *polydata;
	vtkNEW(polydata);
	m_PolylineGraph->CopyToPolydata(polydata);
	int result;
	result=Superclass::SetData(polydata,GetTimeStamp());
	if(result==MAF_OK)
		m_CurrentBranch = GetNumberOfBranch()-1;
	vtkDEL(polydata);
	return result;
}
//-------------------------------------------------------------------------
int medVMEPolylineGraph::AddNewBranch(double vertexCoord[3],mafString *name)
//-------------------------------------------------------------------------
{
	double minDistance=VTK_DOUBLE_MAX;
	int iMin;
	for(int i=0;i<m_PolylineGraph->GetNumberOfVertices();i++)
	{
		double point[3];
		m_PolylineGraph->GetVertexCoords(i,point);
		double distance = sqrt(vtkMath::Distance2BetweenPoints(vertexCoord,point));
		if(distance<minDistance)
		{
			iMin=i;
			minDistance=distance;
		}
	}
	return AddNewBranch(iMin,name);
}
//-------------------------------------------------------------------------
int medVMEPolylineGraph::GetNumberOfBranch()
//-------------------------------------------------------------------------
{
	return m_PolylineGraph->GetNumberOfBranches();
}
//-------------------------------------------------------------------------
bool medVMEPolylineGraph::IsConnected()
//-------------------------------------------------------------------------
{
	return m_PolylineGraph->IsConnected();
}
//-------------------------------------------------------------------------
mafVMEOutput *medVMEPolylineGraph::GetOutput()
//-------------------------------------------------------------------------
{
	// allocate the right type of output on demand
	if (m_Output==NULL)
	{
		SetOutput(mafVMEOutputPolyline::New()); // create the output
	}
	return m_Output;
}
//-------------------------------------------------------------------------
int medVMEPolylineGraph::SelectBranch(vtkIdType v1,vtkIdType v2)
//-------------------------------------------------------------------------
{
	for(int i=0;i<m_PolylineGraph->GetNumberOfEdges();i++)
	{
		if(m_PolylineGraph->GetConstEdgePtr(i)->IsVertexPair(v1,v2))
		{
			m_CurrentBranch = m_PolylineGraph->GetConstEdgePtr(i)->GetBranchId();
			return MAF_OK;
		}
	}
	return MAF_ERROR;
}
//-------------------------------------------------------------------------
int medVMEPolylineGraph::SelectBranch(double v1[3],double v2[3])
//-------------------------------------------------------------------------
{
	int iMin;
	double distaceMin=VTK_DOUBLE_MAX;
	for(int i=0;i<m_PolylineGraph->GetNumberOfVertices();i++)
	{
		double point[3];
		m_PolylineGraph->GetConstVertexPtr(i)->GetCoords(point);
		double distance=sqrt(vtkMath::Distance2BetweenPoints(point,v1));
		if(distance<distaceMin)
		{
			distaceMin=distance;
			iMin=i;
		}
	}
	vtkIdType v1ID=iMin;

	for(int i=0;i<m_PolylineGraph->GetNumberOfVertices();i++)
	{
		double point[3];
		m_PolylineGraph->GetConstVertexPtr(i)->GetCoords(point);
		double distance=sqrt(vtkMath::Distance2BetweenPoints(point,v2));
		if(distance<distaceMin)
		{
			distaceMin=distance;
			iMin=i;
		}
	}

	vtkIdType v2ID=iMin;

	return SelectBranch(v1ID,v2ID);
}