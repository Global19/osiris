/*
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
*  OSIRIS is a desktop tool working on your computer with your own data.
*  Your sample profile data is processed on your computer and is not sent
*  over the internet.
*
*  For quality monitoring, OSIRIS sends some information about usage
*  statistics  back to NCBI.  This information is limited to use of the
*  tool, without any sample, profile or batch data that would reveal the
*  context of your analysis.  For more details and instructions on opting
*  out, see the Privacy Information section of the OSIRIS User's Guide.
*
* ===========================================================================
*
*
*  FileName: CFramePlot.cpp
*  Author:   Douglas Hoffman
*
*/
#include <memory>
#include <wx/printdlg.h>
#include "CPrintOutPlot.h"
#include "mainApp.h"
#include "CFramePlot.h"
#include "CParmOsiris.h"

// static data and functions

class CPrintPreview : public wxPrintPreview
{
public:
  CPrintPreview(wxPrintout *printout, wxPrintout *printoutForPrinting = NULL, wxPrintDialogData *data = NULL) :
    wxPrintPreview(printout, printoutForPrinting, data)
  {
    _SetDefaultZoom();
  }
  CPrintPreview(wxPrintout *printout, wxPrintout *printoutForPrinting, wxPrintData *data) :
    wxPrintPreview(printout, printoutForPrinting, data)
  {
    _SetDefaultZoom();
  }
  virtual ~CPrintPreview() {}
  virtual bool Print(bool interactive)
  {
    bool bRtn = wxPrintPreview::Print(interactive);
    wxChar *psStatus = bRtn ? wxT("OK") : wxT("NOT_OK");
    mainApp::Ping3(PING_EVENT, wxT("Print"), wxT("Status"), psStatus, wxT("FromPreview"), wxT("1"));
    return bRtn;
  }
  virtual void SetZoom(int n)
  {
    CParmOsirisGlobal parm;
    parm->SetPrintPreviewZoom(n);
    wxPrintPreview::SetZoom(n);
  }
private:
  void _SetDefaultZoom()
  {
    CParmOsirisGlobal parm;
    int n = parm->GetPrintPreviewZoom();
    if (n > 10)
    {
      wxPrintPreview::SetZoom(n);
    }
  }
};

wxPrintData *CPrintOutPlot::g_printData = NULL;
wxPageSetupDialogData* CPrintOutPlot::g_pageSetupData = NULL;

wxPrintData *CPrintOutPlot::GetPrintData()
{
  if (g_printData == NULL)
  {
    CParmOsiris *pParm = CParmOsiris::GetGlobal();
    g_printData = new wxPrintData();
    g_printData->SetOrientation(
      pParm->GetPrintPlotLandscape() ? wxLANDSCAPE : wxPORTRAIT
    );
    int nPaperID = pParm->GetPrintPlotPaperType();
    bool bSetPaperSize = true;
    if (nPaperID >= 0)
    {
      wxPaperSize nID = wxPaperSize(nPaperID);
      g_printData->SetPaperId(nID);
      bSetPaperSize = (nID == wxPAPER_NONE);
    }
    if(bSetPaperSize)
    {
      int nw = pParm->GetPrintPlotPaperWidth();
      int nh = pParm->GetPrintPlotPaperHeight();
      if (nw > 0 && nh > 0)
      {
        g_printData->SetPaperSize(wxSize(nw, nh));
      }
    }
  }
  return g_printData;
}
  
wxPageSetupDialogData *CPrintOutPlot::GetPageSetupData()
{
  if(g_pageSetupData == NULL)
  {
    CParmOsiris *pParm = CParmOsiris::GetGlobal();
    g_pageSetupData = new wxPageSetupDialogData(*GetPrintData());
    // set saved margins, to do save/retrieve paper ID or sice
    g_pageSetupData->SetMarginTopLeft(
      wxPoint(
        pParm->GetPrintPlotMarginLeft(),
        pParm->GetPrintPlotMarginTop()
      ));
    g_pageSetupData->SetMarginBottomRight(
        wxPoint(
          pParm->GetPrintPlotMarginRight(),
          pParm->GetPrintPlotMarginBottom()
        ));
    *g_pageSetupData = *GetPrintData();
  }
  return g_pageSetupData;
}
void CPrintOutPlot::UpdatePageSetup()
{
  // update page setup in user osiris parameters
  CParmOsirisGlobal pParm;
  if (g_pageSetupData != NULL)
  {
    wxPoint pt;
    pt = g_pageSetupData->GetMarginTopLeft();
    pParm->SetPrintPlotMarginLeft((unsigned int)pt.x);
    pParm->SetPrintPlotMarginTop((unsigned int)pt.y);
    pt = g_pageSetupData->GetMarginBottomRight();
    pParm->SetPrintPlotMarginRight((unsigned int)pt.x);
    pParm->SetPrintPlotMarginBottom((unsigned int)pt.y);
  }
  if (g_printData != NULL)
  {
    wxSize szPaperSize(-1, -1);
    wxPaperSize nPaperID = g_printData->GetPaperId();
    pParm->SetPrintPlotPaperType(int(nPaperID));
    if (nPaperID == wxPAPER_NONE)
    {
      szPaperSize = g_printData->GetPaperSize();
    }
    pParm->SetPrintPlotPaperWidth(szPaperSize.GetWidth());
    pParm->SetPrintPlotPaperHeight(szPaperSize.GetHeight());
    pParm->SetPrintPlotLandscape(
      g_printData->GetOrientation() == wxLANDSCAPE);
  }
}
void CPrintOutPlot::UpdatePageSetupData(wxPrintData *pPrintData, wxPageSetupDialogData *pSetupData)
{
  bool bUpdate = false;
  if (pPrintData != NULL)
  {
    bUpdate = true;
    *(GetPrintData()) = *pPrintData;
  }
  if (pSetupData != NULL)
  {
    bUpdate = true;
    *(GetPageSetupData()) = *pSetupData;
  }
  if (bUpdate)
  {
    UpdatePageSetup(); // save user settings
  }
}

void CPrintOutPlot::DoPrintPreview(CFramePlot *pPlot)
{
  // using the wxWidgets printing sample program
  wxPrintDialogData printDialogData(*GetPrintData());
  wxPrintPreview *preview =
    new CPrintPreview(
      new CPrintOutPlot(pPlot, true),
      new CPrintOutPlot(pPlot),
      &printDialogData);
  wxChar *psStatus = NULL;
  if (!preview->IsOk())
  {
    delete preview;
    mainApp::ShowError(
      wxT("There was a problem with print preview.\nPlease check your printer setup."),
      pPlot);
    psStatus = wxT("error");
  }
  else
  {
    psStatus = wxT("ok");
    CPrintPlotPreview *frame =
      new CPrintPlotPreview(preview, pPlot, wxT("Print Preview"));
    frame->Show();
  }
  mainApp::Ping2(PING_EVENT, wxT("PrintPreview"), wxT("Status"), psStatus);
}
void CPrintOutPlot::DoPageSetup(CFramePlot *pPlot)
{
  wxPageSetupDialogData *pSetupData = GetPageSetupData();
  wxPrintData *pPrintData = GetPrintData();
  *pSetupData = *pPrintData;

  wxPageSetupDialog pageSetupDialog(pPlot, pSetupData);
  pageSetupDialog.ShowModal();
  UpdatePageSetupData(
    &pageSetupDialog.GetPageSetupDialogData().GetPrintData(),
    &pageSetupDialog.GetPageSetupDialogData()
  );

  mainApp::Ping(PING_EVENT, wxT("PrintPageSetup"));
}
void CPrintOutPlot::DoPrint(CFramePlot *pPlot)
{
  wxPrintDialogData printDialogData(*GetPrintData());

  wxPrinter printer(&printDialogData);
  CPrintOutPlot printout(pPlot);
  wxString sStatus(wxT("OK"));
  if (printer.Print(pPlot, &printout, true /*prompt*/))
  {
    // OK
    SetPrintData(printer.GetPrintDialogData().GetPrintData());
  }
  else if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
  {
    // error
    mainApp::LogMessage(wxT("Printer error"));
    sStatus = wxT("ERROR");
  }
  else
  {
    // printout canceled
    sStatus = wxT("CANCEL");
  }
  mainApp::Ping2(PING_EVENT, wxT("Print"), wxT("Status"), sStatus); 
}

// end static functions - begin virtual wxPrintout functions

CPrintOutPlot::~CPrintOutPlot() { ; }

bool CPrintOutPlot::OnPrintPage(int page)
{
  bool bRtn = true;
  if (page == 1)
  {
    double dPPIscale = 1.0, dScalePixel = 1.0;
    enum
    {
      SCALE_NONE,
      SCALE_X,
      SCALE_Y
    } nScale = SCALE_NONE;
    int nPPIx, nPPIy, nMinPPI, nUsePPI, nX, nY;
    bool bFit = true;
    GetPPIPrinter(&nPPIx, &nPPIy);
    if (nPPIx == nPPIy)
    {
      nMinPPI = nPPIx;
    }
    else if (nPPIx < nPPIy)
    {
      nMinPPI = nPPIx;
      nScale = SCALE_Y;
      dScalePixel = double(nPPIx) / double(nPPIy);
    }
    else
    {
      nMinPPI = nPPIy;
      nScale = SCALE_X;
      dScalePixel = double(nPPIy) / double(nPPIx);
    }
    wxRect rectFit = GetLogicalPageMarginsRect(*GetPageSetupData());
    if (MAX_PPI < 20)
    {
      // this is print preview, get PPI for screen
      int nx, ny;
      GetPPIScreen(&nx, &ny);
      MAX_PPI = (nx < ny) ? nx : ny;
      if (MAX_PPI < 72)
      {
        MAX_PPI = 72;
      }
    }

    if (nMinPPI > MAX_PPI)
    {
      bFit = true;
      nUsePPI = MAX_PPI;
      dPPIscale = MAX_PPI / float(nMinPPI);
      nX = int(rectFit.GetWidth() * dPPIscale);
      nY = int(rectFit.GetHeight() * dPPIscale);
    }
    else
    {
      nUsePPI = nMinPPI;
      nX = rectFit.GetWidth();
      nY = rectFit.GetHeight();
    }
    switch (nScale)
    {
    case SCALE_NONE:
      break;
    case SCALE_X:
      nX = int(dScalePixel * nX);
      bFit = true;
      break;
    case SCALE_Y:
      nY = int(dScalePixel * nY);
      bFit = true;
      break;
    }
    if (bFit)
    {
      FitThisSizeToPageMargins(wxSize(nX, nY), *GetPageSetupData());
    }

    std::unique_ptr<wxBitmap> px(m_pFramePlot->CreateBitmap(nX, nY, nUsePPI, m_pFramePlot->GetPrintTitle()));
    wxDC *pdc = GetDC();
    pdc->DrawBitmap(*px, wxPoint(0,0));
  }
  else
  {
    bRtn = false;
    mainApp::LogMessageV(wxT("Invalid page number %d"), page);
  }
  return bRtn;
}
bool CPrintOutPlot::HasPage(int page)
{
  return (page == 1);
}
void CPrintOutPlot::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
  *minPage = *maxPage = *selPageFrom = *selPageTo = 1;
}

// CPrintPlotPreview

CPrintPlotPreview::CPrintPlotPreview(wxPrintPreview *pPrev, wxWindow *parent,
  const wxString &title, bool bPageButtons) :
    wxPreviewFrame(pPrev, parent, title,
      GET_PERSISTENT_POSITION(CPrintPlotPreview),
      GET_PERSISTENT_SIZE_DEFAULT(CPrintPlotPreview, wxSize(600,800))),
  m_bPageButtons(bPageButtons)
{
  InitializeWithModality(wxPreviewFrame_AppModal);
}
void CPrintPlotPreview::CreateControlBar()
{
  // copied from wxWidgets prntbase.cpp and modified
  // to change buttons
  // and should be checked when updating version of wx
  long buttons = m_bPageButtons ? wxPREVIEW_DEFAULT : wxPREVIEW_ZOOM;
  if (m_printPreview->GetPrintoutForPrinting())
    buttons |= wxPREVIEW_PRINT;

  m_controlBar = new wxPreviewControlBar(m_printPreview, buttons, this);
  m_controlBar->CreateButtons();
}

IMPLEMENT_ABSTRACT_CLASS(CPrintPlotPreview, wxPreviewFrame)
IMPLEMENT_PERSISTENT_SIZE_POSITION(CPrintPlotPreview)
BEGIN_EVENT_TABLE(CPrintPlotPreview, wxPreviewFrame)
EVT_PERSISTENT_SIZE_POSITION(CPrintPlotPreview)
END_EVENT_TABLE()