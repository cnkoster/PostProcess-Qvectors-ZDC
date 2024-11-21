#if !defined(__CINT__) || defined(__MAKECINT__)

#include <stdio.h>
#include <stdlib.h>
#include <TROOT.h>
#include <Riostream.h>
#include <TClassTable.h>
#include <TStyle.h>
#include <TMath.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <TF1.h>
#include <TTree.h>
#include <TLegend.h>
#include <TString.h>
#include <TLatex.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TMinuit.h>
#include <TDirectoryFile.h>

#endif

template <typename T>
void writeTList(int iter, int step, int RUN, std::vector<T*> hists, const char* filename = "Calibration_constants", bool makelist = true, const char* mode="RECREATE"){

// Write the TList to the output file
TFile* output = TFile::Open(Form("%s_it%i_step%i.root", filename, iter, step), mode); 
if (!output || output->IsZombie()) {
    printf("Error: Could not open file for writing at specified path.\n");
    return; // or handle the error accordingly
}

if(makelist) { 
  TList* ccdb_object = new TList();

  // Add each histogram to the TList
  for (int i = 0; i < hists.size(); i++) {
    if(hists[i]==nullptr || hists[i]->GetEntries()<1) std::cout<<"nullptr... or empty hist.. this is a problem."<<std::endl; 
    ccdb_object->Add(hists[i]);
  }
  ccdb_object->Write("ccdb_object",1);
  output->Close();
  delete ccdb_object; 
} else {
  for (int i = 0; i < hists.size(); i++) {
    hists[i]->Write(); 
    std::cout<<"Writing: "<<hists[i]->GetName()<<std::endl;
  }
  output->Close();
}

}

template <typename T>
std::vector<double> getCorrection(int runnumber, float centrality, float v1, float v2, float v3, T* hist)
{
double calibConstant = 0; 
std::vector<double> calib = {0,0};

if (!hist) {
    printf("not available.. Abort..");
}

if (auto* h1d = dynamic_cast<TProfile*>(hist)) {
  int bin; 
  TString name = h1d->GetName(); 
    if(name.Contains("vx")) bin = h1d->GetXaxis()->FindBin(v1); 
    if(name.Contains("vy")) bin = h1d->GetXaxis()->FindBin(v2); 
    if(name.Contains("vz")) bin = h1d->GetXaxis()->FindBin(v3); 
    if(name.Contains("cent")) bin = h1d->GetXaxis()->FindBin(centrality); 
    calib[1] = h1d->GetBinError(bin);
    if (calib[1]>0) calib[0] = h1d->GetBinContent(bin);

} else if (auto* hn = dynamic_cast<THnSparseD*>(hist)) {
    std::vector<int> sparsePars(5);
    sparsePars[0]=hn->GetAxis(0)->FindBin(runnumber);
    sparsePars[1]=hn->GetAxis(1)->FindBin(centrality);
    sparsePars[2]=hn->GetAxis(2)->FindBin(v1);
    sparsePars[3]=hn->GetAxis(3)->FindBin(v2);
    sparsePars[4]=hn->GetAxis(4)->FindBin(v3);
    for (int i = 0; i < sparsePars.size(); i++) {
    hn->GetAxis(i)->SetRange(sparsePars[i], sparsePars[i]);
    }
    TH1::AddDirectory(kFALSE);
    TH1D* proj = hn->Projection(5);//, uniqueProjName);
    if(proj){
      calib[0] = proj->GetMean();
      calib[1] = proj->GetStdDev(); 
      if(proj->GetEntries()<2){
        printf("1 entry in sparse bin! Not used... (increase binsize) \n");
        calib = {0,0}; 
      }
    }
    delete proj;
}

return calib;
}

//
int do_recentering (TString inputfilename = "/dcache/alice/nkoster/PhD/q-vectors/LHC23zzh_pass4_small/step0/544122/AO2D.root", int RUN = 544122, int npart = 100, int part=0)
{
  TFile *fin = TFile::Open(inputfilename.Data(),"READ");
  if(!fin) {
    printf("  +++ FILE %s not found!!!\n\n", inputfilename.Data());
    return -1;
  }

TDirectoryFile *df = nullptr;

TIter next(fin->GetListOfKeys());
TKey *key;
while ((key = (TKey*)next())) {
    if (key->GetClassName() == std::string("TDirectoryFile")) {
        df = (TDirectoryFile*)key->ReadObj();
        break;
    }
}
  
const char* df_name = df->GetName();

  if(!df){
	  printf("Directory missing -> PLEASE CHECK FILE NAME!!! \n");
	  return -1;
  }

  TTree* tree = (TTree*)df->Get("O2spzdc");
  if(!tree){
	  printf("  + tree O2spzdc is not there!!!\n");
	  return -1;
  }  

  std::vector<std::vector<TString>> names(5, std::vector<TString>()); //(1x 4d 4x 1d)

  std::vector<const char*> sides = {"A", "C"};
  std::vector<const char*> coords = {"x", "y", "z"};
  std::vector<const char*> COORDS = {"X", "Y"};

  std::vector<THnSparseD*> hist4D;
  std::vector<TProfile*> histCent;
  std::vector<TProfile*> histVx;
  std::vector<TProfile*> histVy;
  std::vector<TProfile*> histVz;

  std::vector<std::vector<TProfile*>> corrHist(6,std::vector<TProfile*>()); 
  std::vector<std::vector<TProfile*>> corrHist1D(4,std::vector<TProfile*>()); 

  // HERE DEFINE THE HISTOS 

  // Define axes parameters
  int binsVwide = 4;
  int binsVfine = 100; 


  double vx_max = tree->GetMaximum("fVx");
  double vx_min = tree->GetMinimum("fVx");

  double vy_max = tree->GetMaximum("fVy");
  double vy_min = tree->GetMinimum("fVy");

  double vz_max = 10;
  double vz_min = -10;  

  int nBins[] = {static_cast<int>(1e6), 4, binsVwide, binsVwide, binsVwide, 100}; // Number of bins for each axis
  double minVals[] = {0, 0, vx_min, vy_min, vz_min, -2};    // Minimum values for each axis
  double maxVals[] = {1e6, 90, vx_max, vy_max, vz_max, 2};     // Maximum values for each axis

  for (int step = 0; step < 6; step++) {
    for (const char* side : sides) {
      for (const char* coord : COORDS) {
        if (step == 1) {
          TString name = TString::Format("hQ%s%s_mean_Cent_V_run", coord, side);
          // Correct THnSparseD creation by passing array of axes
          THnSparseD* hist = new THnSparseD(name.Data(), name.Data(), 6, nBins, minVals, maxVals);
          hist4D.push_back(hist); 
          names[step - 1].push_back(name);

        // make <Q> vs vx/y/z/cent -> ONLY ONCE!
        TString name_vx = TString::Format("hQ%s%s_vs_vx", coord, side);
        TProfile* hist_vx = new TProfile(name_vx.Data(), name_vx.Data(), binsVfine, vx_min, vx_max);
        corrHist1D[0].push_back(hist_vx);
        TString name_vy = TString::Format("hQ%s%s_vs_vy", coord, side);
        TProfile* hist_vy = new TProfile(name_vy.Data(), name_vy.Data(), binsVfine, vy_min, vy_max);
        corrHist1D[1].push_back(hist_vy);
        TString name_vz = TString::Format("hQ%s%s_vs_vz", coord, side);
        TProfile* hist_vz = new TProfile(name_vz.Data(), name_vz.Data(), binsVfine, -10, 10);
        corrHist1D[2].push_back(hist_vz);
        TString name_cent = TString::Format("hQ%s%s_vs_cent", coord, side);
        TProfile* hist_cent = new TProfile(name_cent.Data(), name_cent.Data(), binsVfine, 0, 80);
        corrHist1D[3].push_back(hist_cent);
        }
        if (step == 2) {
          TString name = TString::Format("hQ%s%s_mean_cent_run", coord, side);
          // Correct TProfile2D constructor
          TProfile* hist = new TProfile(name.Data(), name.Data(), binsVfine, 0, 90);
          hist->GetXaxis()->SetTitle("Centrality (%)");
          names[step - 1].push_back(name);
          histCent.push_back(hist); 
        }
        if (step == 3) {
          TString name = TString::Format("hQ%s%s_mean_vx_run", coord, side);
          TProfile* hist = new TProfile(name.Data(), name.Data(), binsVfine , vx_min, vx_max);
          hist->GetXaxis()->SetTitle("Vx");
          names[step - 1].push_back(name);
          histVx.push_back(hist); 
        }
        if (step == 4) {
          TString name = TString::Format("hQ%s%s_mean_vy_run", coord, side);
          TProfile* hist = new TProfile(name.Data(), name.Data(),  binsVfine , vy_min, vy_max);
          hist->GetXaxis()->SetTitle("Vy");
          names[step - 1].push_back(name);
          histVy.push_back(hist); 
        }
        if (step == 5) {
          TString name = TString::Format("hQ%s%s_mean_vz_run", coord, side);
          TProfile* hist = new TProfile(name.Data(), name.Data(),  binsVfine , -10, 10);
          hist->GetXaxis()->SetTitle("Vz");
          names[step - 1].push_back(name);
          histVz.push_back(hist); 
        }
      } // end of COORDS
    } // end of sides


    for (const char* COORD1 : COORDS) {
      for (const char* COORD2 : COORDS) {
        // Now we get: <XX> <XY> & <YX> <YY> vs. Centrality
        TString name = TString::Format("hQ%sA_Q%sC_vs_cent%i",COORD1, COORD2, step);
        TProfile* corr = new TProfile(name.Data(), Form("<Q%sA Q%sC> vs centrality",COORD1, COORD2), 90, 0, 90); 
        corrHist[step].push_back(corr); 
      }
    }

  } // end of sum over steps

  // Make the QA Plots to keep track of process! 

    // tree->Print("");
    Int_t nentries = (Int_t) tree->GetEntries();

    // Here set filters/selections for tree: isSelected and vtxz
    //.......
    printf("      Tree has %d entries\n\n", nentries);

    int   runnumber; 
    float cent; 
    float vx; 
    float vy; 
    float vz; 
    float qxA; 
    float qxC; 
    float qyA; 
    float qyC; 
    int   iteration; 
    int   step; 
      
    tree->SetBranchAddress("fRunnumber", &runnumber);
    tree->SetBranchAddress("fCent", &cent);
    tree->SetBranchAddress("fVx", &vx);
    tree->SetBranchAddress("fVy", &vy);
    tree->SetBranchAddress("fVz", &vz);
    tree->SetBranchAddress("fQXA", &qxA);
    tree->SetBranchAddress("fQXC", &qyA);
    tree->SetBranchAddress("fQYA", &qxC);
    tree->SetBranchAddress("fQYC", &qyC);
    tree->SetBranchAddress("fIteration", &iteration);
    tree->SetBranchAddress("fStep", &step);

    int it_out = 0; 

    for(Int_t iev=0; iev<nentries; iev++){
    // So the loop is done for each entry of the tree. 
    tree->GetEntry(iev);  
    // fill calibration thnsparse 
    if(iev==0) it_out = iteration; 

    // if(vx>vx_max || vx<vx_min || vy>vy_max || vy<vy_min || vz>vz_max || vz<vz_min){
    //   continue;
    // }

    hist4D[0]->Fill(runnumber, cent, vx, vy, vz, qxA); 
    hist4D[1]->Fill(runnumber, cent, vx, vy, vz, qyA); 
    hist4D[2]->Fill(runnumber, cent, vx, vy, vz, qxC); 
    hist4D[3]->Fill(runnumber, cent, vx, vy, vz, qyC); 

    corrHist[0][0]->Fill(cent, qxA*qxC); 
    corrHist[0][1]->Fill(cent, qxA*qyC); 
    corrHist[0][2]->Fill(cent, qyA*qxC); 
    corrHist[0][3]->Fill(cent, qyA*qyC); 

    corrHist1D[0][0]->Fill(vx, qxA);
    corrHist1D[0][1]->Fill(vx, qyA);
    corrHist1D[0][2]->Fill(vx, qxC);
    corrHist1D[0][3]->Fill(vx, qyC);

    corrHist1D[1][0]->Fill(vy, qxA);
    corrHist1D[1][1]->Fill(vy, qyA);
    corrHist1D[1][2]->Fill(vy, qxC);
    corrHist1D[1][3]->Fill(vy, qyC);

    corrHist1D[2][0]->Fill(vz, qxA);
    corrHist1D[2][1]->Fill(vz, qyA);
    corrHist1D[2][2]->Fill(vz, qxC);
    corrHist1D[2][3]->Fill(vz, qyC);

    corrHist1D[3][0]->Fill(cent, qxA);
    corrHist1D[3][1]->Fill(cent, qyA);
    corrHist1D[3][2]->Fill(cent, qxC);
    corrHist1D[3][3]->Fill(cent, qyC);
    }
    writeTList(it_out+1, 1, RUN, hist4D); 
    
    writeTList(it_out+1, 0, RUN, corrHist[0], "outCorrelations", false); 

    writeTList(it_out+1, 0, RUN, corrHist1D[0], "outCorrParams", false); 
    writeTList(it_out+1, 0, RUN, corrHist1D[1], "outCorrParams", false, "UPDATE"); 
    writeTList(it_out+1, 0, RUN, corrHist1D[2], "outCorrParams", false, "UPDATE"); 
    writeTList(it_out+1, 0, RUN, corrHist1D[3], "outCorrParams", false, "UPDATE"); 
    // - - - - - -  - - - - - -  - - - - - -  - - - - - -  - - - - - -  - - - - - -  - - - - - -  - - - - - - 
    printf("calib 1 created \n");

   // - - - - - -  - - - - - -  - - - - - -  - - - - - -  - - - - - -  - - - - - -  - - - - - -  - - - - - - 

    //Now create histos and bools for each function 
    std::vector<bool> in(5,false); 
    std::vector<THnSparseD*> hists1; 
    std::vector<TProfile*> hists2;
    std::vector<TProfile*> hists3;
    std::vector<TProfile*> hists4;
    std::vector<TProfile*> hists5;

    // Clone the structure of the original tree (without copying data)
      for(int step=0; step<5; step++){
        TFile* inFile = TFile::Open(Form("/data/alice/nkoster/alice/Analysis/ZDC/runCent/postProcess/%i/Calibration_constants_it%i_step%i.root", RUN, iteration+1, step+1), "READ"); 
        if(inFile){
          TList* calibList = (TList*)inFile->Get("ccdb_object"); 
          if(step==0){
          for(int i=0; i<4; i++){
            THnSparseD* hist = (THnSparseD*)calibList->FindObject(Form("%s", hist4D[i]->GetName()));
            if(hist){
            hists1.push_back(hist);
            in[step] = true; 
            }
            }
          } else if (step==1){
              for(int i=0; i<4; i++){
                TProfile* hist = (TProfile*)calibList->FindObject(Form("%s", histCent[i]->GetName()));
                if(hist){
                hists2.push_back(hist);
                in[step] = true; 
                }
              }
          } else if (step==2){
              for(int i=0; i<4; i++){
                TProfile* hist = (TProfile*)calibList->FindObject(Form("%s", histVx[i]->GetName()));
                if(hist){
                hists3.push_back(hist);
                in[step] = true; 
                }
              }
          } else if (step==3){
              for(int i=0; i<4; i++){
                TProfile* hist = (TProfile*)calibList->FindObject(Form("%s", histVy[i]->GetName()));
                if(hist){
                hists4.push_back(hist);
                in[step] = true; 
                }
              }
          } else if (step==4){
              for(int i=0; i<4; i++){
                TProfile* hist = (TProfile*)calibList->FindObject(Form("%s", histVz[i]->GetName()));
                if(hist){
                hists5.push_back(hist);
                in[step] = true; 
                }
              }
          } 
          inFile->Close();   
        } 
      }

    TTree *newTree = tree->CloneTree(0);

    int iteration_new;
    float qxA5; 
    float qyA5; 
    float qxC5; 
    float qyC5; 

    newTree->SetBranchAddress("fIteration", &iteration_new); 
    newTree->SetBranchAddress("fQXA", &qxA5);
    newTree->SetBranchAddress("fQXC", &qyA5);
    newTree->SetBranchAddress("fQYA", &qxC5);
    newTree->SetBranchAddress("fQYC", &qyC5);

    int start = (nentries/npart)*part; 
    int end = ((nentries/npart)*(part+1) > nentries)? nentries : (nentries/npart)*(part+1); 
    

    std::cout<< "de tree heeft: "<<nentries<<" entries"; 
    std::cout<<"----------------------> Running from: "<< start << " to "<<end<<std::endl; 

  for(Int_t iev=start; iev<end; iev++){

    if (iev % 10000 == 100) {
        std::cout << "\r[LOADING.....] " 
                  << std::fixed << std::setprecision(2) 
                  << (static_cast<double>(iev) / nentries * 100) 
                  << "%" << std::flush;
      }

      // So the loop is done for each entry of the tree. 
      tree->GetEntry(iev);
      if(iev==0) {it_out = iteration; std::cout<< iteration<<std::endl; }

      // if(vx>vx_max || vx<vx_min || vy>vy_max || vy<vy_min || vz>vz_max || vz<vz_min){
      //   if(in[4]) {
      //     // If outside v ranges, do not use for recentering. 
      //     qxA5 = qxA;
      //     qyA5 = qyA;
      //     qxC5 = qxC;
      //     qyC5 = qyC;
      //     iteration_new = iteration +1;  
      //     newTree->Fill(); 
      //     continue;
      //   }
      // }

      if(in[0]){
        if (iev == 0) std::cout <<  " in 1 is true " << std::endl;

        double corrqxA1 = getCorrection(runnumber, cent, vx, vy, vz, hists1[0])[0];
        double corrqyA1 = getCorrection(runnumber, cent, vx, vy, vz, hists1[1])[0];
        double corrqxC1 = getCorrection(runnumber, cent, vx, vy, vz, hists1[2])[0];
        double corrqyC1 = getCorrection(runnumber, cent, vx, vy, vz, hists1[3])[0];

        double WcorrqxA1 = getCorrection(runnumber, cent, vx, vy, vz, hists1[0])[1];
        double WcorrqyA1 = getCorrection(runnumber, cent, vx, vy, vz, hists1[1])[1];
        double WcorrqxC1 = getCorrection(runnumber, cent, vx, vy, vz, hists1[2])[1];
        double WcorrqyC1 = getCorrection(runnumber, cent, vx, vy, vz, hists1[3])[1];

        // std::cout<<"Correction is:"<<corrqxA1<<" "<<corrqyA1<<" "<<corrqxC1<<" "<<corrqyC1<<" "<<std::endl;

        double qxA1 = qxA - corrqxA1; 
        double qyA1 = qyA - corrqyA1; 
        double qxC1 = qxC - corrqxC1;
        double qyC1 = qyC - corrqyC1; 


        if(!in[1]){
        histCent[0]->Fill(cent, qxA1, 1);//wcorrqxA1); 
        histCent[1]->Fill(cent, qyA1, 1);//wcorrqyA1); 
        histCent[2]->Fill(cent, qxC1, 1);//wcorrqxC1); 
        histCent[3]->Fill(cent, qyC1, 1);//wcorrqyC1); 

        corrHist[1][0]->Fill(cent, qxA1*qxC1); 
        corrHist[1][1]->Fill(cent, qxA1*qyC1); 
        corrHist[1][2]->Fill(cent, qyA1*qxC1); 
        corrHist[1][3]->Fill(cent, qyA1*qyC1); 
        }

        if(in[1]){
          if (iev == 0) std::cout <<  " in 2 is true " << std::endl;

          double corrqxA2 = getCorrection(runnumber, cent, vx, vy, vz, hists2[0])[0];
          double corrqyA2 = getCorrection(runnumber, cent, vx, vy, vz, hists2[1])[0];
          double corrqxC2 = getCorrection(runnumber, cent, vx, vy, vz, hists2[2])[0];
          double corrqyC2 = getCorrection(runnumber, cent, vx, vy, vz, hists2[3])[0];

          double qxA2 = qxA1 - corrqxA2; 
          double qyA2 = qyA1 - corrqyA2; 
          double qxC2 = qxC1 - corrqxC2;
          double qyC2 = qyC1 - corrqyC2; 

          if(!in[2]){
          histVx[0]->Fill(vx, qxA2); 
          histVx[1]->Fill(vx, qyA2); 
          histVx[2]->Fill(vx, qxC2); 
          histVx[3]->Fill(vx, qyC2); 

          corrHist[2][0]->Fill(cent, qxA2*qxC2); 
          corrHist[2][1]->Fill(cent, qxA2*qyC2); 
          corrHist[2][2]->Fill(cent, qyA2*qxC2); 
          corrHist[2][3]->Fill(cent, qyA2*qyC2); 
          }

          if(in[2]){
            if (iev == 0) std::cout <<  " in 3 is true " << std::endl;

            double corrqxA3 = getCorrection(runnumber, cent, vx, vy, vz, hists3[0])[0];
            double corrqyA3 = getCorrection(runnumber, cent, vx, vy, vz, hists3[1])[0];
            double corrqxC3 = getCorrection(runnumber, cent, vx, vy, vz, hists3[2])[0];
            double corrqyC3 = getCorrection(runnumber, cent, vx, vy, vz, hists3[3])[0];

            double qxA3 = qxA2 - corrqxA3; 
            double qyA3 = qyA2 - corrqyA3; 
            double qxC3 = qxC2 - corrqxC3;
            double qyC3 = qyC2 - corrqyC3; 

          if(!in[3]){  
            histVy[0]->Fill(vy, qxA3); 
            histVy[1]->Fill(vy, qyA3); 
            histVy[2]->Fill(vy, qxC3); 
            histVy[3]->Fill(vy, qyC3); 

            corrHist[3][0]->Fill(cent, qxA3*qxC3); 
            corrHist[3][1]->Fill(cent, qxA3*qyC3); 
            corrHist[3][2]->Fill(cent, qyA3*qxC3); 
            corrHist[3][3]->Fill(cent, qyA3*qyC3);}

            if(in[3]){
              if (iev == 0) std::cout <<  " in 4 is true " << std::endl;
              double corrqxA4 = getCorrection(runnumber, cent, vx, vy, vz, hists4[0])[0];
              double corrqyA4 = getCorrection(runnumber, cent, vx, vy, vz, hists4[1])[0];
              double corrqxC4 = getCorrection(runnumber, cent, vx, vy, vz, hists4[2])[0];
              double corrqyC4 = getCorrection(runnumber, cent, vx, vy, vz, hists4[3])[0];

              double qxA4 = qxA3 - corrqxA4; 
              double qyA4 = qyA3 - corrqyA4; 
              double qxC4 = qxC3 - corrqxC4;
              double qyC4 = qyC3 - corrqyC4; 

             if(!in[4]){ 
              histVz[0]->Fill(vz, qxA4); 
              histVz[1]->Fill(vz, qyA4); 
              histVz[2]->Fill(vz, qxC4); 
              histVz[3]->Fill(vz, qyC4); 
            

              corrHist[4][0]->Fill(cent, qxA4*qxC4); 
              corrHist[4][1]->Fill(cent, qxA4*qyC4); 
              corrHist[4][2]->Fill(cent, qyA4*qxC4); 
              corrHist[4][3]->Fill(cent, qyA4*qyC4); }

              if(in[4]){
                if (iev == 0) std::cout <<  " in 5 is true " << std::endl;

                double corrqxA5 = getCorrection(runnumber, cent, vx, vy, vz, hists5[0])[0];
                double corrqyA5 = getCorrection(runnumber, cent, vx, vy, vz, hists5[1])[0];
                double corrqxC5 = getCorrection(runnumber, cent, vx, vy, vz, hists5[2])[0];
                double corrqyC5 = getCorrection(runnumber, cent, vx, vy, vz, hists5[3])[0];

                qxA5 = qxA4 - corrqxA5; 
                qyA5 = qyA4 - corrqyA5; 
                qxC5 = qxC4 - corrqxC5;
                qyC5 = qyC4 - corrqyC5; 

                corrHist[5][0]->Fill(cent, qxA5*qxC5); 
                corrHist[5][1]->Fill(cent, qxA5*qyC5); 
                corrHist[5][2]->Fill(cent, qyA5*qxC5); 
                corrHist[5][3]->Fill(cent, qyA5*qyC5); 

               qxA = qxA5; 
               qyA = qyA5; 
               qxC = qxC5; 
               qyC = qyC5; 

              iteration_new = iteration +1;  

              newTree->Fill(); 

              } //5
            } //4
          } //3
        } //2
      } //1
    } // end of loop over events. 

    // fin->Close(); 

 if(in[4]){
      writeTList(it_out+1, 5, RUN, corrHist[5], "outCorrelations", false);

      TFile *newFile = TFile::Open(Form("AO2D%i.root", iteration+1), "RECREATE");
      TDirectory* dfnew = newFile->mkdir(df_name);
      dfnew->cd();
      newTree->Write("O2spzdc");  // Save the new tree to the new file
      newFile->Close();  // Close the new file

      delete newTree; 

      return 4; 
    } else if(in[3]){
      writeTList(it_out+1, 5, RUN, histVz);
      writeTList(it_out+1, 4, RUN, corrHist[4], "outCorrelations", false);
      return 3; 
    } else if(in[2]){
      writeTList(it_out+1, 4, RUN, histVy);
      writeTList(it_out+1, 3, RUN, corrHist[3], "outCorrelations", false);
      return 2;
    } else  if(in[1]){
      std::cout<< " in 2 --> filled"<<std::endl; 
      writeTList(it_out+1, 3, RUN, histVx);
      writeTList(it_out+1, 2, RUN, corrHist[2], "outCorrelations", false);
      return 1;
    } else if(in[0]){
      std::cout<< " in 1 --> filled"<<std::endl; 
      writeTList(it_out+1, 2, RUN, histCent);
      writeTList(it_out+1, 1, RUN, corrHist[1], "outCorrelations", false);
      return 0; 
    } else {return 0;}

  return 0; 
}

void recentering(const char* infilename = "/dcache/alice/nkoster/PhD/q-vectors/LHC23zzh_pass4_small/step0/544122/AO2D.root", int run = 544122, int npart=100, int part=0){ 

  do_recentering(infilename, run, npart, part); 
}
