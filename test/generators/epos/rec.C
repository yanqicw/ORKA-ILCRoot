void rec() {
  IlcReconstruction reco;

  reco.SetWriteESDfriend();
  reco.SetWriteAlignmentData();

  reco.SetDefaultStorage("local://$ILC_ROOT/OCDB");
  reco.SetSpecificStorage("GRP/GRP/Data",
			  Form("local://%s",gSystem->pwd()));

  // High multiplicity settings
  Double_t cuts[]={33, 0.1, 0.1, 0.05, 0.99, 0.9, 100}; 
  IlcV0vertexer::SetDefaultCuts(cuts); 
  Double_t cts[]={33., 0.05, 0.008, 0.035, 0.1, 0.9985, 0.9,100}; 
  IlcCascadeVertexer::SetDefaultCuts(cts); 

  TStopwatch timer;
  timer.Start();
  reco.Run();
  timer.Stop();
  timer.Print();
}
