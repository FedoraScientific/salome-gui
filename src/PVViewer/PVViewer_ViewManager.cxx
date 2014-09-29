// Copyright (C) 2010-2014  CEA/DEN, EDF R&D
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
#include "PVViewer_ViewManager.h"
#include "PVViewer_ViewModel.h"
#include "PVViewer_ViewWindow.h"
#include "PVViewer_LogWindowAdapter.h"
#include "PVViewer_GUIElements.h"
#include "PVViewer_Behaviors.h"
#include "PVViewer_EngineWrapper.h"

#include <utilities.h>
#include <SalomeApp_Application.h>
#include <SUIT_MessageBox.h>
#include <SUIT_Desktop.h>
#include <SUIT_Session.h>
#include <SUIT_ResourceMgr.h>
#include <PyInterp_Interp.h>
#include <PyConsole_Interp.h>
#include <PyConsole_Console.h>

#include <QApplication>
#include <QStringList>
#include <QDir>

#include <string>

#include <pqOptions.h>
#include <pqServer.h>
#include <pqSettings.h>
#include <pqServerDisconnectReaction.h>
#include <pqPVApplicationCore.h>
#include <pqTabbedMultiViewWidget.h>
#include <pqActiveObjects.h>
#include <pqServerConnectReaction.h>

#include <pqParaViewMenuBuilders.h>
#include <pqPipelineBrowserWidget.h>

//---------- Static init -----------------
pqPVApplicationCore* PVViewer_ViewManager::MyCoreApp = 0;
bool PVViewer_ViewManager::ConfigLoaded = false;
PVViewer_Behaviors * PVViewer_ViewManager::ParaviewBehaviors = NULL;

/*!
  Constructor
*/
PVViewer_ViewManager::PVViewer_ViewManager( SUIT_Study* study, SUIT_Desktop* desk )
: SUIT_ViewManager( study, desk, new PVViewer_Viewer() ),
  desktop(desk)
{
  MESSAGE("PARAVIS - view manager created ...")
  setTitle( tr( "PARAVIEW_VIEW_TITLE" ) );
  // Initialize minimal paraview stuff (if not already done)
  ParaviewInitApp(desk);

//  connect(this, SIGNAL(viewCreated(SUIT_ViewWindow*)), this, SLOT(onPVViewCreated(SUIT_ViewWindow*)));
}

pqPVApplicationCore * PVViewer_ViewManager::GetPVApplication()
{
  return MyCoreApp;
}

/*!
  \brief Static method, performs initialization of ParaView session.
  \param fullSetup whether to instanciate all behaviors or just the minimal ones.
  \return \c true if ParaView has been initialized successfully, otherwise false
*/
bool PVViewer_ViewManager::ParaviewInitApp(SUIT_Desktop * aDesktop)
{
  if ( ! MyCoreApp) {
      // Obtain command-line arguments
      int argc = 0;
      char** argv = 0;
      QString aOptions = getenv("PARAVIS_OPTIONS");
      QStringList aOptList = aOptions.split(":", QString::SkipEmptyParts);
      argv = new char*[aOptList.size() + 1];
      QStringList args = QApplication::arguments();
      argv[0] = (args.size() > 0)? strdup(args[0].toLatin1().constData()) : strdup("paravis");
      argc++;

      foreach (QString aStr, aOptList) {
        argv[argc] = strdup( aStr.toLatin1().constData() );
        argc++;
      }
      MyCoreApp = new pqPVApplicationCore (argc, argv);
      if (MyCoreApp->getOptions()->GetHelpSelected() ||
          MyCoreApp->getOptions()->GetUnknownArgument() ||
          MyCoreApp->getOptions()->GetErrorMessage() ||
          MyCoreApp->getOptions()->GetTellVersion()) {
          return false;
      }

      // Direct VTK log messages to our SALOME window - TODO: review this
      vtkOutputWindow::SetInstance(PVViewer_LogWindowAdapter::New());

      new pqTabbedMultiViewWidget(); // registers a "MULTIVIEW_WIDGET" on creation

      // At this stage, the pqPythonManager has been initialized, i.e. the current process has
      // activated the embedded Python interpreter. "paraview" package has also been imported once already.
      // Make sure the current process executes paraview's Python command with the "fromGUI" flag.
      // This is used in pvsimple.py to avoid reconnecting the GUI thread to the pvserver (when
      // user types "import pvsimple" in SALOME's console).
      SalomeApp_Application* app =
                  dynamic_cast< SalomeApp_Application* >(SUIT_Session::session()->activeApplication());
      PyConsole_Interp* pyInterp = app->pythonConsole()->getInterp();
      {
        PyLockWrapper aGil;
        std::string cmd = "import paraview;paraview.fromGUI = True";
        pyInterp->run(cmd.c_str());
      }

      for (int i = 0; i < argc; i++)
        free(argv[i]);
      delete[] argv;
  }
  // Initialize GUI elements if needed:
  PVViewer_GUIElements::GetInstance(aDesktop);
  return true;
}

void PVViewer_ViewManager::ParaviewInitBehaviors(bool fullSetup, SUIT_Desktop* aDesktop)
{
  if (!ParaviewBehaviors)
      ParaviewBehaviors = new PVViewer_Behaviors(aDesktop);

  if(fullSetup)
    ParaviewBehaviors->instanciateAllBehaviors(aDesktop);
  else
    ParaviewBehaviors->instanciateMinimalBehaviors(aDesktop);
}

void PVViewer_ViewManager::ParaviewLoadConfigurations()
{
  if (!ConfigLoaded)
    {
      SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
      QString aPath = resMgr->stringValue("resources", "PARAVIS", QString());
      if (!aPath.isNull()) {
          MyCoreApp->loadConfiguration(aPath + QDir::separator() + "ParaViewFilters.xml");
          MyCoreApp->loadConfiguration(aPath + QDir::separator() + "ParaViewReaders.xml");
          MyCoreApp->loadConfiguration(aPath + QDir::separator() + "ParaViewSources.xml");
          MyCoreApp->loadConfiguration(aPath + QDir::separator() + "ParaViewWriters.xml");
      }
      ConfigLoaded = true;
    }
}

void PVViewer_ViewManager::ParaviewCleanup()
{
  // Disconnect from server
  pqServer* server = pqActiveObjects::instance().activeServer();
  if (server && server->isRemote())
    {
      MESSAGE("~PVViewer_Module(): Disconnecting from remote server ...");
      pqServerDisconnectReaction::disconnectFromServer();
    }

  pqApplicationCore::instance()->settings()->sync();

  pqPVApplicationCore * app = GetPVApplication();
  // Schedule destruction of PVApplication singleton:
  if (app)
    app->deleteLater();
}

PVViewer_EngineWrapper * PVViewer_ViewManager::GetEngine()
{
  return PVViewer_EngineWrapper::GetInstance();
}

bool PVViewer_ViewManager::ConnectToExternalPVServer(SUIT_Desktop* aDesktop)
{
  SUIT_ResourceMgr* aResourceMgr = SUIT_Session::session()->resourceMgr();
  bool noConnect = aResourceMgr->booleanValue( "PARAVIS", "no_ext_pv_server", false );
  if (noConnect)
    return true;

  pqServer* server = pqActiveObjects::instance().activeServer();
  if (server && server->isRemote())
    {
      // Already connected to an external server, do nothing
      MESSAGE("connectToExternalPVServer(): Already connected to an external PVServer, won't reconnect.");
      return false;
    }

  if (GetEngine()->GetGUIConnected())
    {
      // Should never be there as the above should already tell us that we are connected.
      std::stringstream msg2;
      msg2 << "Internal error while connecting to the pvserver.";
      msg2 << "ParaView doesn't see a connection, but PARAVIS engine tells us there is already one!" << std::endl;
      qWarning(msg2.str().c_str());  // will go to the ParaView console (see ParavisMessageOutput below)
      SUIT_MessageBox::warning( aDesktop,
                                      QString("Error connecting to PVServer"), QString(msg2.str().c_str()));
      return false;
    }

  std::stringstream msg;

  // Try to connect to the external PVServer - gives priority to an externally specified URL:
  QString serverUrlEnv = getenv("PARAVIS_PVSERVER_URL");
  std::string serverUrl;
  if (!serverUrlEnv.isEmpty())
    serverUrl = serverUrlEnv.toStdString();
  else
    {
      // Get the URL from the engine (possibly starting the pvserver)
      serverUrl = GetEngine()->FindOrStartPVServer(0);  // take the first free port
    }

  msg << "connectToExternalPVServer(): Trying to connect to the external PVServer '" << serverUrl << "' ...";
  MESSAGE(msg.str());

  if (!pqServerConnectReaction::connectToServer(pqServerResource(serverUrl.c_str())))
    {
      std::stringstream msg2;
      msg2 << "Error while connecting to the requested pvserver '" << serverUrl;
      msg2 << "'. Might use default built-in connection instead!" << std::endl;
      qWarning(msg2.str().c_str());  // will go to the ParaView console (see ParavisMessageOutput below)
      SUIT_MessageBox::warning( aDesktop,
                                QString("Error connecting to PVServer"), QString(msg2.str().c_str()));
      return false;
    }
  else
    {
      MESSAGE("connectToExternalPVServer(): Connected!");
      GetEngine()->SetGUIConnected(true);
    }
  return true;
}

//void PVViewer_ViewManager::onPVViewCreated(SUIT_ViewWindow* w)
//{
//  PVViewer_ViewWindow * w2 = dynamic_cast<PVViewer_ViewWindow *>(w);
//  Q_ASSERT(w2 != NULL);
//  connect(w2, SIGNAL(applyRequest()), ParaviewBehaviors, SLOT(onEmulateApply()));
//}

void PVViewer_ViewManager::onEmulateApply()
{
  PVViewer_GUIElements * guiElements = PVViewer_GUIElements::GetInstance(desktop);
  guiElements->onEmulateApply();
}
