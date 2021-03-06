IlcCentralitySelectionTask *AddTaskCentrality(Bool_t fillHistos=kTRUE)
{
// Macro to connect a centrality selection task to an existing analysis manager.
  IlcAnalysisManager *mgr = IlcAnalysisManager::GetAnalysisManager();
  if (!mgr) {
    ::Error("AddTaskCentrality", "No analysis manager to connect to.");
    return NULL;
  }    
  // Check the analysis type using the event handlers connected to the analysis manager.
  //==============================================================================
  if (!mgr->GetInputEventHandler()) {
    ::Error("AddTaskCentrality", "This task requires an input event handler");
    return NULL;
  }
  TString inputDataType = mgr->GetInputEventHandler()->GetDataType(); // can be "ESD" or "AOD"
  if (inputDataType != "ESD") {
    ::Error("AddTaskCentrality", "This task works only on ESD analysis");
    return NULL;
  }
  IlcCentralitySelectionTask *centralityTask = new IlcCentralitySelectionTask("CentralitySelection");
  centralityTask->SelectCollisionCandidates(IlcVEvent::kAny);
  mgr->AddTask(centralityTask);
  
  mgr->ConnectInput(centralityTask, 0, mgr->GetCommonInputContainer());
  if (fillHistos) {
    centralityTask->SetFillHistos();
    IlcAnalysisDataContainer *coutput1 = mgr->CreateContainer("CentralityStat",
                                                              TList::Class(), 
                                                              IlcAnalysisManager::kOutputContainer,
                                                              "EventStat_temp.root");
    mgr->ConnectOutput(centralityTask,1,coutput1);
  }

  return centralityTask;
}   
