   void makeListFromDir(TFile* file, const char* dirname){

    TDirectory *inputDir = (TDirectory*)file->Get(Form("z-d-cqvectors/%s",dirname));
    TList* outlist = new TList(); 

    TIter nextKey(inputDir->GetListOfKeys());
    TKey *key;
    while ((key = (TKey*)nextKey())) {
        TObject *obj = key->ReadObj();
        outlist->Add(obj);
    }
     TFile* outfile = TFile::Open(Form("Calibration_constants_%s.root", dirname), "RECREATE"); 

     outlist->Write("ccdb_object",1); 
     outfile->Close(); 
   }


void makeTList(){
    TFile* file = TFile::Open("/dcache/alice/nkoster/PhD/ZDC/Run3/calibration/ZDC-qvector-pre-recentering/LHC23_zzh/pass4/AnalysisResults.root", "READ"); 
    // file->ls(); 

    if (!file || file->IsZombie() || !file->IsOpen()) {
      printf("File does not exist!! Abort mission...");
      return; 
    }

    makeListFromDir(file, "Energy"); 
    makeListFromDir(file, "vmean"); 
    file->Close(); 
  }

      
