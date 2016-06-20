#include <iostream>
#include <fstream>
#include <iomanip>
#include <utility>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include "TROOT.h"
#include "TMath.h"
#include "TLorentzVector.h"
#include "TLorentzRotation.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TString.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TProfile.h"
#include "TGraphErrors.h"
#include "TRandom.h"
#include <ZZMatrixElement/MELA/interface/Mela.h>


using namespace std;
using namespace RooFit;


/*
GENERAL COMMENTS
- ALL MES ARE TRANSFORMED TO LOG10(ME).
*/

/* SPECIFIC COMMENT: NONE */
void get_PAvgProfile_JHUGen_JJVBF_HSMHiggs_7or8TeV(int sqrts=8){
  int erg_tev=sqrts;
  float mPOLE=125.;
  TString TREE_NAME = "SelectedTree";
  bool writeFinalTree=false;

  TVar::VerbosityLevel verbosity = TVar::ERROR;
  Mela mela(erg_tev, mPOLE, verbosity);

  short NJets30;
  std::vector<double>* JetPt=0;
  std::vector<double>* JetEta=0;
  std::vector<double>* JetPhi=0;
  std::vector<double>* JetMass=0;
  std::vector<double> myJetPt;
  std::vector<double> myJetEta;
  std::vector<double> myJetPhi;
  std::vector<double> myJetMass;
  TBranch* bJetPt=0;
  TBranch* bJetEta=0;
  TBranch* bJetPhi=0;
  TBranch* bJetMass=0;
  float jetptetaphimass[2][4];

  float mesq_conserveDifermMass=0;
  float mesq_jetPtoEScale=0;
  float mzz = 126.;
  float m1 = 91.471450;
  float m2 = 12.139782;
  float h1 = 0.2682896;
  float h2 = 0.1679779;
  float phi = 1.5969792;
  float hs = -0.727181;
  float phi1 = 1.8828257;
  float ZZPt, ZZPhi, ZZEta;
  int LepID[4]={ 13, -13, 11, -11 };

  TString strchannel[3]={ "4mu", "4e", "2mu2e" };
  TString cinput_main;
  if (sqrts==8) cinput_main = "/scratch0/hep/ianderso/CJLST/140519/PRODFSR_8TeV";
  else if (sqrts==7) cinput_main = "/scratch0/hep/ianderso/CJLST/140519/PRODFSR";
  else return;
  const int nSamples = 42;
  TString strSamples[nSamples]={
    //"HZZ4lTree_VBF0P_H125.6.root",
    "HZZ4lTree_VBFH116.root",
    "HZZ4lTree_VBFH117.root",
    "HZZ4lTree_VBFH118.root",
    "HZZ4lTree_VBFH119.root",
    "HZZ4lTree_VBFH120.root",
    "HZZ4lTree_VBFH121.root",
    "HZZ4lTree_VBFH122.root",
    "HZZ4lTree_VBFH123.root",
    "HZZ4lTree_VBFH124.root",
    "HZZ4lTree_VBFH125.root",
    "HZZ4lTree_VBFH126.root",
    "HZZ4lTree_VBFH127.root",
    "HZZ4lTree_VBFH128.root",
    "HZZ4lTree_VBFH129.root",
    "HZZ4lTree_VBFH130.root",
    "HZZ4lTree_VBFH135.root",
    "HZZ4lTree_VBFH140.root",
    "HZZ4lTree_VBFH145.root",
    "HZZ4lTree_VBFH150.root",
    "HZZ4lTree_VBFH160.root",
    "HZZ4lTree_VBFH170.root",
    "HZZ4lTree_VBFH180.root",
    "HZZ4lTree_VBFH190.root",
    "HZZ4lTree_powheg15VBFH200.root",
    "HZZ4lTree_powheg15VBFH225.root",
    "HZZ4lTree_powheg15VBFH250.root",
    "HZZ4lTree_powheg15VBFH275.root",
    "HZZ4lTree_powheg15VBFH300.root",
    "HZZ4lTree_powheg15VBFH350.root",
    "HZZ4lTree_powheg15VBFH400.root",
    "HZZ4lTree_powheg15VBFH450.root",
    "HZZ4lTree_powheg15VBFH500.root",
    "HZZ4lTree_powheg15VBFH550.root",
    "HZZ4lTree_powheg15VBFH600.root",
    "HZZ4lTree_powheg15VBFH650.root",
    "HZZ4lTree_powheg15VBFH700.root",
    "HZZ4lTree_powheg15VBFH750.root",
    "HZZ4lTree_powheg15VBFH800.root",
    "HZZ4lTree_powheg15VBFH850.root",
    "HZZ4lTree_powheg15VBFH900.root",
    "HZZ4lTree_powheg15VBFH950.root",
    "HZZ4lTree_powheg15VBFH1000.root"
  };

  TChain* tree = new TChain(TREE_NAME, "");
  for (int ic=0; ic<3; ic++){
    for (int is=0; is<nSamples; is++) tree->Add(Form("%s/%s/%s", cinput_main.Data(), (strchannel[ic]).Data(), (strSamples[is]).Data()));
  }
  tree->SetBranchAddress("NJets30", &NJets30);
  tree->SetBranchAddress("JetPt", &JetPt, &bJetPt);
  tree->SetBranchAddress("JetEta", &JetEta, &bJetEta);
  tree->SetBranchAddress("JetPhi", &JetPhi, &bJetPhi);
  tree->SetBranchAddress("JetMass", &JetMass, &bJetMass);
  tree->SetBranchAddress("ZZMass", &mzz);
  tree->SetBranchAddress("ZZPt", &ZZPt);
  tree->SetBranchAddress("ZZEta", &ZZEta);
  tree->SetBranchAddress("ZZPhi", &ZZPhi);
  tree->SetBranchAddress("Z1Mass", &m1);
  tree->SetBranchAddress("Z2Mass", &m2);
  tree->SetBranchAddress("helcosthetaZ1", &h1);
  tree->SetBranchAddress("helcosthetaZ2", &h2);
  tree->SetBranchAddress("helphi", &phi);
  tree->SetBranchAddress("costhetastar", &hs);
  tree->SetBranchAddress("phistarZ1", &phi1);

  const int nTotalEntries = tree->GetEntries();
  cout << "Ntotalentries = " << nTotalEntries << endl;

  TTree* tmptree = new TTree("IntermediateTree", "");
  tmptree->Branch("ZZMass", &mzz);
  tmptree->Branch("ZZPt", &ZZPt);
  tmptree->Branch("ZZEta", &ZZEta);
  tmptree->Branch("ZZPhi", &ZZPhi);
  tmptree->Branch("Z1Mass", &m1);
  tmptree->Branch("Z2Mass", &m2);
  tmptree->Branch("helcosthetaZ1", &h1);
  tmptree->Branch("helcosthetaZ2", &h2);
  tmptree->Branch("helphi", &phi);
  tmptree->Branch("costhetastar", &hs);
  tmptree->Branch("phistarZ1", &phi1);
  tmptree->Branch("NJets30", &NJets30);
  tmptree->Branch("jet1pt", &(jetptetaphimass[0][0]));
  tmptree->Branch("jet2pt", &(jetptetaphimass[1][0]));
  tmptree->Branch("jet1eta", &(jetptetaphimass[0][1]));
  tmptree->Branch("jet2eta", &(jetptetaphimass[1][1]));
  tmptree->Branch("jet1phi", &(jetptetaphimass[0][2]));
  tmptree->Branch("jet2phi", &(jetptetaphimass[1][2]));
  tmptree->Branch("jet1mass", &(jetptetaphimass[0][3]));
  tmptree->Branch("jet2mass", &(jetptetaphimass[1][3]));

  TRandom3 randomthrow(1234567);
  double portion_to_keep = 1;
  if (nTotalEntries>1000000) portion_to_keep = 9.95e5/tree->GetEntries("NJets30>=2");
  for (int ev = 0; ev < nTotalEntries; ev++){
    tree->GetEntry(ev);
    if (NJets30>=2){
      for (int ij=0; ij<2; ij++){
        jetptetaphimass[ij][0]=JetPt->at(ij);
        jetptetaphimass[ij][1]=JetEta->at(ij);
        jetptetaphimass[ij][2]=JetPhi->at(ij);
        jetptetaphimass[ij][3]=JetMass->at(ij);
      }
      double rndnum = randomthrow.Uniform();
      if (rndnum<=portion_to_keep) tmptree->Fill();
    }
  }

  const int nEntries = tmptree->GetEntries();
  if (nEntries>=1000000){
    cerr << "TMath::Sort will experience problems. Aborting!" << endl;
    delete tmptree;
    delete tree;
    assert(0);
  }
  int* index = new int[nEntries];
  tmptree->Draw("ZZMass", "", "goff");
  TMath::Sort(nEntries, tmptree->GetV1(), index, false);

  tmptree->GetEntry(index[0]);
  float firstVal=mzz;
  tmptree->GetEntry(index[nEntries-1]);
  float lastVal=mzz;
  float infimum = (float)((int)firstVal); infimum -= (float)(((int)infimum)%10);
  float supremum = (float)((int)(lastVal+0.5)); supremum += (float)(10-((int)supremum)%10);
  cout << "Nentries = " << nEntries << " | mzz = " << firstVal << " - " << lastVal << "(" << infimum << ", " << supremum << ")" << endl;

  int nbins=0;
  int divisor=11000;
  while (nbins<50){
    if (divisor>1000) divisor -= 1000;
    else if (divisor>100) divisor -= 100;
    else break;
    nbins=nEntries/divisor+1;
  }
  cout << "nbins=" << nbins << endl;
  if (nbins<3) cerr << "Not enough bins!" << endl;
  float* binning = new float[nbins+1];
  binning[0]=infimum;
  binning[nbins]=supremum;
  int ev_stepsize = nEntries/nbins;
  cout << "Event step size: " << ev_stepsize << endl;
  cout << "Boundary (" << 0 << ") = " << binning[0] << endl;
  for (int ix=1; ix<nbins; ix++){
    int ev = index[ix*ev_stepsize];
    tmptree->GetEntry(ev);
    float bhigh = mzz;
    ev = index[ix*ev_stepsize-1];
    float blow = mzz;
    binning[ix]=(bhigh+blow)*0.5;
    cout << "Boundary (" << ix << ")= " << binning[ix] << " [event " << ev << ", step " << ix*ev_stepsize << "]" << endl;
  }
  cout << "Boundary (" << nbins << ") = " << binning[nbins] << endl;
  delete[] index;

  TFile* foutput = new TFile(Form("pAvg_JHUGen_JJVBF_HSMHiggs_%iTeV.root", sqrts), "recreate");

  TProfile* hvar = new TProfile("candMass", "", nbins, binning); hvar->Sumw2();
  TProfile* hmesq_conserveDifermMass = new TProfile("P_ConserveDifermionMass", "", nbins, binning); hmesq_conserveDifermMass->Sumw2();
  TProfile* hmesq_jetPtoEScale = new TProfile("P_MomentumToEnergy", "", nbins, binning); hmesq_jetPtoEScale->Sumw2();

  
  TTree* newtree=0;
  if (writeFinalTree){
    newtree = new TTree("FinalTree", "");
    newtree->Branch("mesq_conserveDifermMass", &mesq_conserveDifermMass);
    newtree->Branch("mesq_jetPtoEScale", &mesq_jetPtoEScale);
    newtree->Branch("ZZMass", &mzz);
  }  

  mela.setCandidateDecayMode(TVar::CandidateDecay_ZZ);

  for (int ev = 0; ev < nEntries; ev++){
    tmptree->GetEntry(ev); // No need for ordering anymore
    if (ev%10000==0) cout << "Doing event " << ev << endl;

    TLorentzVector jet[2], higgs;
    for (int ij=0; ij<2; ij++) jet[ij].SetPtEtaPhiM(jetptetaphimass[ij][0], jetptetaphimass[ij][1], jetptetaphimass[ij][2], jetptetaphimass[ij][3]);
    higgs.SetPtEtaPhiM(ZZPt, ZZEta, ZZPhi, mzz);
    TVector3 boostH = higgs.BoostVector();

    SimpleParticleCollection_t associated;
    associated.push_back(SimpleParticle_t(0, jet[0]));
    associated.push_back(SimpleParticle_t(0, jet[1]));

    TLorentzVector pDaughters[4];
    std::vector<TLorentzVector> daus = mela.calculate4Momentum(mzz, m1, m2, acos(hs), acos(h1), acos(h2), phi1, phi);
    for (int ip=0; ip<min(4, (int)daus.size()); ip++){ pDaughters[ip]=daus.at(ip); pDaughters[ip].Boost(boostH); }
    SimpleParticleCollection_t daughters;
    for (unsigned int idau=0; idau<4; idau++) daughters.push_back(SimpleParticle_t(LepID[idau], pDaughters[idau]));
    mela.setInputEvent(&daughters, &associated, (SimpleParticleCollection_t*)0, false);

    bool doFill=true;
    mela.setProcess(TVar::HSMHiggs, TVar::JHUGen, TVar::JJVBF);
    TUtil::setJetMassScheme(TVar::ConserveDifermionMass);

    mela.computeProdP(mesq_conserveDifermMass, false);
    mesq_conserveDifermMass = log10(mesq_conserveDifermMass);
    if (isnan(mesq_conserveDifermMass) || isinf(mesq_conserveDifermMass)) doFill=false;

    TUtil::setJetMassScheme(TVar::MomentumToEnergy);
    mela.computeProdP(mesq_jetPtoEScale, false);
    mesq_jetPtoEScale = log10(mesq_jetPtoEScale);
    if (isnan(mesq_jetPtoEScale) || isinf(mesq_jetPtoEScale)) doFill=false;

    if (doFill){
      hmesq_conserveDifermMass->Fill(mzz, mesq_conserveDifermMass);
      hmesq_jetPtoEScale->Fill(mzz, mesq_jetPtoEScale);
      hvar->Fill(mzz, mzz);
    }

    if (writeFinalTree) newtree->Fill();
    mela.resetInputEvent();
  }

  double* xexyey[2][4];
  for (int inorm=0; inorm<2; inorm++){
    for (int ix=0; ix<4; ix++) xexyey[inorm][ix] = new double[nbins];
    for (int bin=0; bin<nbins; bin++){
      xexyey[inorm][0][bin] = hvar->GetBinContent(bin+1);
      xexyey[inorm][1][bin] = hvar->GetBinError(bin+1);

      if (inorm==0) cout << "Bin " << bin << " x-center: " << xexyey[inorm][0][bin] << " +- " << xexyey[inorm][1][bin] << endl;

      if (inorm==0){
        xexyey[inorm][2][bin] = hmesq_conserveDifermMass->GetBinContent(bin+1);
        xexyey[inorm][3][bin] = hmesq_conserveDifermMass->GetBinError(bin+1);
      }
      else{
        xexyey[inorm][2][bin] = hmesq_jetPtoEScale->GetBinContent(bin+1);
        xexyey[inorm][3][bin] = hmesq_jetPtoEScale->GetBinError(bin+1);
      }
    }
  }

  for (int inorm=0; inorm<2; inorm++){
    TGraphErrors* tg = new TGraphErrors(nbins, xexyey[inorm][0], xexyey[inorm][2], xexyey[inorm][1], xexyey[inorm][3]);
    if (inorm==0) tg->SetName("tg_P_ConserveDifermionMass");
    else tg->SetName("tg_P_MomentumToEnergy");
    foutput->WriteTObject(tg);
    delete tg;
  }

  for (int inorm=0; inorm<2; inorm++){
    for (int ix=0; ix<4; ix++) delete[] xexyey[inorm][ix];
  }
  foutput->WriteTObject(hmesq_jetPtoEScale);
  foutput->WriteTObject(hmesq_conserveDifermMass);
  foutput->WriteTObject(hvar);
  if (writeFinalTree) foutput->WriteTObject(newtree);
  if (writeFinalTree) delete newtree;
  delete hmesq_conserveDifermMass;
  delete hmesq_jetPtoEScale;
  delete hvar;
  foutput->Close();
  delete[] binning;
  delete tmptree;
  delete tree;
}
/*
Function
[0]*exp(-x/[1])*(1+[2]*exp(-pow(x/[3],2)))
with parameters
0.0187935
489.335
0.0870576
256.215
fits well.
*/


/* SPECIFIC COMMENT: NONE */
void get_PAvgProfile_JHUGen_JVBF_HSMHiggs_7or8TeV(int sqrts=8){
  int erg_tev=sqrts;
  float mPOLE=125.;
  TString TREE_NAME = "SelectedTree";
  bool writeFinalTree=false;

  TVar::VerbosityLevel verbosity = TVar::ERROR;
  Mela mela(erg_tev, mPOLE, verbosity);

  short NJets30;
  std::vector<double>* JetPt=0;
  std::vector<double>* JetEta=0;
  std::vector<double>* JetPhi=0;
  std::vector<double>* JetMass=0;
  std::vector<double> myJetPt;
  std::vector<double> myJetEta;
  std::vector<double> myJetPhi;
  std::vector<double> myJetMass;
  TBranch* bJetPt=0;
  TBranch* bJetEta=0;
  TBranch* bJetPhi=0;
  TBranch* bJetMass=0;
  float jetptetaphimass[2][4]={ { 0 } };

  float mesq_conserveDifermMass=0;
  float mesq_jetPtoEScale=0;
  float mesqaux_conserveDifermMass=0;
  float mesqaux_jetPtoEScale=0;
  float mzz = 126.;
  float m1 = 91.471450;
  float m2 = 12.139782;
  float h1 = 0.2682896;
  float h2 = 0.1679779;
  float phi = 1.5969792;
  float hs = -0.727181;
  float phi1 = 1.8828257;
  float ZZPt, ZZPhi, ZZEta;
  int LepID[4]={ 13, -13, 11, -11 };

  TString strchannel[3]={ "4mu", "4e", "2mu2e" };
  TString cinput_main;
  if (sqrts==8) cinput_main = "/scratch0/hep/ianderso/CJLST/140519/PRODFSR_8TeV";
  else if (sqrts==7) cinput_main = "/scratch0/hep/ianderso/CJLST/140519/PRODFSR";
  else return;
  const int nSamples = 42;
  TString strSamples[nSamples]={
    //"HZZ4lTree_VBF0P_H125.6.root",
    "HZZ4lTree_VBFH116.root",
    "HZZ4lTree_VBFH117.root",
    "HZZ4lTree_VBFH118.root",
    "HZZ4lTree_VBFH119.root",
    "HZZ4lTree_VBFH120.root",
    "HZZ4lTree_VBFH121.root",
    "HZZ4lTree_VBFH122.root",
    "HZZ4lTree_VBFH123.root",
    "HZZ4lTree_VBFH124.root",
    "HZZ4lTree_VBFH125.root",
    "HZZ4lTree_VBFH126.root",
    "HZZ4lTree_VBFH127.root",
    "HZZ4lTree_VBFH128.root",
    "HZZ4lTree_VBFH129.root",
    "HZZ4lTree_VBFH130.root",
    "HZZ4lTree_VBFH135.root",
    "HZZ4lTree_VBFH140.root",
    "HZZ4lTree_VBFH145.root",
    "HZZ4lTree_VBFH150.root",
    "HZZ4lTree_VBFH160.root",
    "HZZ4lTree_VBFH170.root",
    "HZZ4lTree_VBFH180.root",
    "HZZ4lTree_VBFH190.root",
    "HZZ4lTree_powheg15VBFH200.root",
    "HZZ4lTree_powheg15VBFH225.root",
    "HZZ4lTree_powheg15VBFH250.root",
    "HZZ4lTree_powheg15VBFH275.root",
    "HZZ4lTree_powheg15VBFH300.root",
    "HZZ4lTree_powheg15VBFH350.root",
    "HZZ4lTree_powheg15VBFH400.root",
    "HZZ4lTree_powheg15VBFH450.root",
    "HZZ4lTree_powheg15VBFH500.root",
    "HZZ4lTree_powheg15VBFH550.root",
    "HZZ4lTree_powheg15VBFH600.root",
    "HZZ4lTree_powheg15VBFH650.root",
    "HZZ4lTree_powheg15VBFH700.root",
    "HZZ4lTree_powheg15VBFH750.root",
    "HZZ4lTree_powheg15VBFH800.root",
    "HZZ4lTree_powheg15VBFH850.root",
    "HZZ4lTree_powheg15VBFH900.root",
    "HZZ4lTree_powheg15VBFH950.root",
    "HZZ4lTree_powheg15VBFH1000.root"
  };

  TChain* tree = new TChain(TREE_NAME, "");
  for (int ic=0; ic<3; ic++){
    for (int is=0; is<nSamples; is++) tree->Add(Form("%s/%s/%s", cinput_main.Data(), (strchannel[ic]).Data(), (strSamples[is]).Data()));
  }
  tree->SetBranchAddress("NJets30", &NJets30);
  tree->SetBranchAddress("JetPt", &JetPt, &bJetPt);
  tree->SetBranchAddress("JetEta", &JetEta, &bJetEta);
  tree->SetBranchAddress("JetPhi", &JetPhi, &bJetPhi);
  tree->SetBranchAddress("JetMass", &JetMass, &bJetMass);
  tree->SetBranchAddress("ZZMass", &mzz);
  tree->SetBranchAddress("ZZPt", &ZZPt);
  tree->SetBranchAddress("ZZEta", &ZZEta);
  tree->SetBranchAddress("ZZPhi", &ZZPhi);
  tree->SetBranchAddress("Z1Mass", &m1);
  tree->SetBranchAddress("Z2Mass", &m2);
  tree->SetBranchAddress("helcosthetaZ1", &h1);
  tree->SetBranchAddress("helcosthetaZ2", &h2);
  tree->SetBranchAddress("helphi", &phi);
  tree->SetBranchAddress("costhetastar", &hs);
  tree->SetBranchAddress("phistarZ1", &phi1);

  const int nTotalEntries = tree->GetEntries();
  cout << "Ntotalentries = " << nTotalEntries << endl;

  TTree* tmptree = new TTree("IntermediateTree", "");
  tmptree->Branch("ZZMass", &mzz);
  tmptree->Branch("ZZPt", &ZZPt);
  tmptree->Branch("ZZEta", &ZZEta);
  tmptree->Branch("ZZPhi", &ZZPhi);
  tmptree->Branch("Z1Mass", &m1);
  tmptree->Branch("Z2Mass", &m2);
  tmptree->Branch("helcosthetaZ1", &h1);
  tmptree->Branch("helcosthetaZ2", &h2);
  tmptree->Branch("helphi", &phi);
  tmptree->Branch("costhetastar", &hs);
  tmptree->Branch("phistarZ1", &phi1);
  tmptree->Branch("NJets30", &NJets30);
  tmptree->Branch("jet1pt", &(jetptetaphimass[0][0]));
  tmptree->Branch("jet1eta", &(jetptetaphimass[0][1]));
  tmptree->Branch("jet1phi", &(jetptetaphimass[0][2]));
  tmptree->Branch("jet1mass", &(jetptetaphimass[0][3]));

  TRandom3 randomthrow(1234567);
  double portion_to_keep = 1;
  if (nTotalEntries>1000000) portion_to_keep = 9.95e5/tree->GetEntries("NJets30==1");
  for (int ev = 0; ev < nTotalEntries; ev++){
    tree->GetEntry(ev);
    if (NJets30==1){
      for (int ij=0; ij<1; ij++){
        jetptetaphimass[ij][0]=JetPt->at(ij);
        jetptetaphimass[ij][1]=JetEta->at(ij);
        jetptetaphimass[ij][2]=JetPhi->at(ij);
        jetptetaphimass[ij][3]=JetMass->at(ij);
      }
      double rndnum = randomthrow.Uniform();
      if (rndnum<=portion_to_keep) tmptree->Fill();
    }
  }

  const int nEntries = tmptree->GetEntries();
  if (nEntries>=1000000){
    cerr << "TMath::Sort will experience problems. Aborting!" << endl;
    delete tmptree;
    delete tree;
    assert(0);
  }
  int* index = new int[nEntries];
  tmptree->Draw("ZZMass", "", "goff");
  TMath::Sort(nEntries, tmptree->GetV1(), index, false);

  tmptree->GetEntry(index[0]);
  float firstVal=mzz;
  tmptree->GetEntry(index[nEntries-1]);
  float lastVal=mzz;
  float infimum = (float)((int)firstVal); infimum -= (float)(((int)infimum)%10);
  float supremum = (float)((int)(lastVal+0.5)); supremum += (float)(10-((int)supremum)%10);
  cout << "Nentries = " << nEntries << " | mzz = " << firstVal << " - " << lastVal << "(" << infimum << ", " << supremum << ")" << endl;

  int nbins=0;
  int divisor=11000;
  while (nbins<50){
    if (divisor>1000) divisor -= 1000;
    else if (divisor>100) divisor -= 100;
    else break;
    nbins=nEntries/divisor+1;
  }
  cout << "nbins=" << nbins << endl;
  if (nbins<3) cerr << "Not enough bins!" << endl;
  float* binning = new float[nbins+1];
  binning[0]=infimum;
  binning[nbins]=supremum;
  int ev_stepsize = nEntries/nbins;
  cout << "Event step size: " << ev_stepsize << endl;
  cout << "Boundary (" << 0 << ") = " << binning[0] << endl;
  for (int ix=1; ix<nbins; ix++){
    int ev = index[ix*ev_stepsize];
    tmptree->GetEntry(ev);
    float bhigh = mzz;
    ev = index[ix*ev_stepsize-1];
    float blow = mzz;
    binning[ix]=(bhigh+blow)*0.5;
    cout << "Boundary (" << ix << ")= " << binning[ix] << " [event " << ev << ", step " << ix*ev_stepsize << "]" << endl;
  }
  cout << "Boundary (" << nbins << ") = " << binning[nbins] << endl;
  delete[] index;

  TFile* foutput = new TFile(Form("pAvg_JHUGen_JVBF_HSMHiggs_%iTeV.root", sqrts), "recreate");

  TProfile* hvar = new TProfile("candMass", "", nbins, binning); hvar->Sumw2();
  TProfile* hmesq_conserveDifermMass = new TProfile("P_ConserveDifermionMass", "", nbins, binning); hmesq_conserveDifermMass->Sumw2();
  TProfile* hmesq_jetPtoEScale = new TProfile("P_MomentumToEnergy", "", nbins, binning); hmesq_jetPtoEScale->Sumw2();
  TProfile* hmesqaux_conserveDifermMass = new TProfile("PAux_ConserveDifermionMass", "", nbins, binning); hmesqaux_conserveDifermMass->Sumw2();
  TProfile* hmesqaux_jetPtoEScale = new TProfile("PAux_MomentumToEnergy", "", nbins, binning); hmesqaux_jetPtoEScale->Sumw2();


  TTree* newtree=0;
  if (writeFinalTree){
    newtree = new TTree("FinalTree", "");
    newtree->Branch("mesq_conserveDifermMass", &mesq_conserveDifermMass);
    newtree->Branch("mesq_jetPtoEScale", &mesq_jetPtoEScale);
    newtree->Branch("mesqaux_conserveDifermMass", &mesqaux_conserveDifermMass);
    newtree->Branch("mesqaux_jetPtoEScale", &mesqaux_jetPtoEScale);
    newtree->Branch("ZZMass", &mzz);
  }

  mela.setCandidateDecayMode(TVar::CandidateDecay_ZZ);

  for (int ev = 0; ev < nEntries; ev++){
    tmptree->GetEntry(ev); // No need for ordering anymore
    if (ev%10000==0) cout << "Doing event " << ev << endl;

    TLorentzVector jet[2], higgs;
    for (int ij=0; ij<1; ij++) jet[ij].SetPtEtaPhiM(jetptetaphimass[ij][0], jetptetaphimass[ij][1], jetptetaphimass[ij][2], jetptetaphimass[ij][3]);
    higgs.SetPtEtaPhiM(ZZPt, ZZEta, ZZPhi, mzz);
    TVector3 boostH = higgs.BoostVector();

    SimpleParticleCollection_t associated;
    associated.push_back(SimpleParticle_t(0, jet[0]));

    TLorentzVector pDaughters[4];
    std::vector<TLorentzVector> daus = mela.calculate4Momentum(mzz, m1, m2, acos(hs), acos(h1), acos(h2), phi1, phi);
    for (int ip=0; ip<min(4, (int)daus.size()); ip++){ pDaughters[ip]=daus.at(ip); pDaughters[ip].Boost(boostH); }
    SimpleParticleCollection_t daughters;
    for (unsigned int idau=0; idau<4; idau++) daughters.push_back(SimpleParticle_t(LepID[idau], pDaughters[idau]));
    mela.setInputEvent(&daughters, &associated, (SimpleParticleCollection_t*)0, false);

    hvar->Fill(mzz, mzz);

    mela.setProcess(TVar::HSMHiggs, TVar::JHUGen, TVar::JJVBF);
    TUtil::setJetMassScheme(TVar::ConserveDifermionMass);
    mela.computeProdP(mesq_conserveDifermMass, false);
    mela.get_PAux(mesqaux_conserveDifermMass);
    mesqaux_conserveDifermMass *= mesq_conserveDifermMass;
    mesq_conserveDifermMass = log10(mesq_conserveDifermMass);
    mesqaux_conserveDifermMass = log10(mesqaux_conserveDifermMass);
    hmesq_conserveDifermMass->Fill(mzz, mesq_conserveDifermMass);
    hmesqaux_conserveDifermMass->Fill(mzz, mesqaux_conserveDifermMass);

    TUtil::setJetMassScheme(TVar::MomentumToEnergy);
    mela.computeProdP(mesq_jetPtoEScale, false);
    mela.get_PAux(mesqaux_jetPtoEScale);
    mesqaux_jetPtoEScale *= mesq_jetPtoEScale;
    mesq_jetPtoEScale = log10(mesq_jetPtoEScale);
    mesqaux_jetPtoEScale = log10(mesqaux_jetPtoEScale);
    hmesq_jetPtoEScale->Fill(mzz, mesq_jetPtoEScale);
    hmesqaux_jetPtoEScale->Fill(mzz, mesqaux_jetPtoEScale);

    if (writeFinalTree) newtree->Fill();
    mela.resetInputEvent();
  }

  double* xexyey[4][4];
  for (int inorm=0; inorm<4; inorm++){
    for (int ix=0; ix<4; ix++) xexyey[inorm][ix] = new double[nbins];
    for (int bin=0; bin<nbins; bin++){
      xexyey[inorm][0][bin] = hvar->GetBinContent(bin+1);
      xexyey[inorm][1][bin] = hvar->GetBinError(bin+1);

      if (inorm==0) cout << "Bin " << bin << " x-center: " << xexyey[inorm][0][bin] << " +- " << xexyey[inorm][1][bin] << endl;

      if (inorm==0){
        xexyey[inorm][2][bin] = hmesq_conserveDifermMass->GetBinContent(bin+1);
        xexyey[inorm][3][bin] = hmesq_conserveDifermMass->GetBinError(bin+1);
      }
      else if(inorm==1){
        xexyey[inorm][2][bin] = hmesq_jetPtoEScale->GetBinContent(bin+1);
        xexyey[inorm][3][bin] = hmesq_jetPtoEScale->GetBinError(bin+1);
      }
      else if (inorm==2){
        xexyey[inorm][2][bin] = hmesqaux_conserveDifermMass->GetBinContent(bin+1);
        xexyey[inorm][3][bin] = hmesqaux_conserveDifermMass->GetBinError(bin+1);
      }
      else{
        xexyey[inorm][2][bin] = hmesqaux_jetPtoEScale->GetBinContent(bin+1);
        xexyey[inorm][3][bin] = hmesqaux_jetPtoEScale->GetBinError(bin+1);
      }
    }
  }

  for (int inorm=0; inorm<4; inorm++){
    TGraphErrors* tg = new TGraphErrors(nbins, xexyey[inorm][0], xexyey[inorm][2], xexyey[inorm][1], xexyey[inorm][3]);
    if (inorm==0) tg->SetName("tg_P_ConserveDifermionMass");
    else if (inorm==1) tg->SetName("tg_P_MomentumToEnergy");
    else if (inorm==2) tg->SetName("tg_PAux_ConserveDifermionMass");
    else tg->SetName("tg_PAux_MomentumToEnergy");
    foutput->WriteTObject(tg);
    delete tg;
  }

  for (int inorm=0; inorm<2; inorm++){
    for (int ix=0; ix<4; ix++) delete[] xexyey[inorm][ix];
  }
  foutput->WriteTObject(hmesqaux_jetPtoEScale);
  foutput->WriteTObject(hmesqaux_conserveDifermMass);
  foutput->WriteTObject(hmesq_jetPtoEScale);
  foutput->WriteTObject(hmesq_conserveDifermMass);
  foutput->WriteTObject(hvar);
  if (writeFinalTree) foutput->WriteTObject(newtree);
  if (writeFinalTree) delete newtree;
  delete hmesqaux_conserveDifermMass;
  delete hmesqaux_jetPtoEScale;
  delete hmesq_conserveDifermMass;
  delete hmesq_jetPtoEScale;
  delete hvar;
  foutput->Close();
  delete[] binning;
  delete tmptree;
  delete tree;
}


/*
SPECIFIC COMMENT: OUTPUT ME DIVIDED BY 
- ALPHAS(MZ)**4 TO REMAIN INDEPENDENT OF PDF CHOICE TO FIRST APPROXIMATION
*/
void get_PAvgProfile_JHUGen_JJQCD_HSMHiggs_7or8TeV(int sqrts=8){
  int erg_tev=sqrts;
  float mPOLE=125.;
  TString TREE_NAME = "SelectedTree";
  bool writeFinalTree=false;

  TVar::VerbosityLevel verbosity = TVar::ERROR;
  Mela mela(erg_tev, mPOLE, verbosity);

  short NJets30;
  std::vector<double>* JetPt=0;
  std::vector<double>* JetEta=0;
  std::vector<double>* JetPhi=0;
  std::vector<double>* JetMass=0;
  std::vector<double> myJetPt;
  std::vector<double> myJetEta;
  std::vector<double> myJetPhi;
  std::vector<double> myJetMass;
  TBranch* bJetPt=0;
  TBranch* bJetEta=0;
  TBranch* bJetPhi=0;
  TBranch* bJetMass=0;
  float jetptetaphimass[2][4];

  float mesq_conserveDifermMass=0;
  float mesq_jetPtoEScale=0;
  float mzz = 126.;
  float m1 = 91.471450;
  float m2 = 12.139782;
  float h1 = 0.2682896;
  float h2 = 0.1679779;
  float phi = 1.5969792;
  float hs = -0.727181;
  float phi1 = 1.8828257;
  float ZZPt, ZZPhi, ZZEta;
  int LepID[4]={ 13, -13, 11, -11 };

  TString strchannel[3]={ "4mu", "4e", "2mu2e" };
  TString cinput_main;
  if (sqrts==8) cinput_main = "/scratch0/hep/ianderso/CJLST/140519/PRODFSR_8TeV";
  else if (sqrts==7) cinput_main = "/scratch0/hep/ianderso/CJLST/140519/PRODFSR";
  else return;
  const int nSamples = 37;
  TString strSamples[nSamples]={
    "HZZ4lTree_minloH90.root",
    "HZZ4lTree_minloH95.root",
    "HZZ4lTree_minloH100.root",
    "HZZ4lTree_minloH105.root",
    "HZZ4lTree_minloH110.root",
    "HZZ4lTree_minloH115.root",
    "HZZ4lTree_minloH120.root",
    "HZZ4lTree_minloH124.root",
    "HZZ4lTree_minloH125.root",
    "HZZ4lTree_minloH126.root",
    "HZZ4lTree_minloH130.root",
    "HZZ4lTree_minloH135.root",
    "HZZ4lTree_minloH140.root",
    "HZZ4lTree_minloH145.root",
    "HZZ4lTree_minloH150.root",
    "HZZ4lTree_minloH155.root",
    "HZZ4lTree_minloH160.root",
    "HZZ4lTree_minloH170.root",
    "HZZ4lTree_minloH180.root",
    "HZZ4lTree_minloH190.root",
    "HZZ4lTree_minloH200.root",
    "HZZ4lTree_minloH250.root",
    "HZZ4lTree_minloH300.root",
    "HZZ4lTree_minloH350.root",
    "HZZ4lTree_minloH400.root",
    "HZZ4lTree_minloH450.root",
    "HZZ4lTree_minloH500.root",
    "HZZ4lTree_minloH550.root",
    "HZZ4lTree_minloH600.root",
    "HZZ4lTree_minloH650.root",
    "HZZ4lTree_minloH700.root",
    "HZZ4lTree_minloH750.root",
    "HZZ4lTree_minloH800.root",
    "HZZ4lTree_minloH850.root",
    "HZZ4lTree_minloH900.root",
    "HZZ4lTree_minloH950.root",
    "HZZ4lTree_minloH1000.root"
  };

  TChain* tree = new TChain(TREE_NAME, "");
  for (int ic=0; ic<3; ic++){
    for (int is=0; is<nSamples; is++) tree->Add(Form("%s/%s/%s", cinput_main.Data(), (strchannel[ic]).Data(), (strSamples[is]).Data()));
  }
  tree->SetBranchAddress("NJets30", &NJets30);
  tree->SetBranchAddress("JetPt", &JetPt, &bJetPt);
  tree->SetBranchAddress("JetEta", &JetEta, &bJetEta);
  tree->SetBranchAddress("JetPhi", &JetPhi, &bJetPhi);
  tree->SetBranchAddress("JetMass", &JetMass, &bJetMass);
  tree->SetBranchAddress("ZZMass", &mzz);
  tree->SetBranchAddress("ZZPt", &ZZPt);
  tree->SetBranchAddress("ZZEta", &ZZEta);
  tree->SetBranchAddress("ZZPhi", &ZZPhi);
  tree->SetBranchAddress("Z1Mass", &m1);
  tree->SetBranchAddress("Z2Mass", &m2);
  tree->SetBranchAddress("helcosthetaZ1", &h1);
  tree->SetBranchAddress("helcosthetaZ2", &h2);
  tree->SetBranchAddress("helphi", &phi);
  tree->SetBranchAddress("costhetastar", &hs);
  tree->SetBranchAddress("phistarZ1", &phi1);

  const int nTotalEntries = tree->GetEntries();
  cout << "Ntotalentries = " << nTotalEntries << endl;

  TTree* tmptree = new TTree("IntermediateTree", "");
  tmptree->Branch("ZZMass", &mzz);
  tmptree->Branch("ZZPt", &ZZPt);
  tmptree->Branch("ZZEta", &ZZEta);
  tmptree->Branch("ZZPhi", &ZZPhi);
  tmptree->Branch("Z1Mass", &m1);
  tmptree->Branch("Z2Mass", &m2);
  tmptree->Branch("helcosthetaZ1", &h1);
  tmptree->Branch("helcosthetaZ2", &h2);
  tmptree->Branch("helphi", &phi);
  tmptree->Branch("costhetastar", &hs);
  tmptree->Branch("phistarZ1", &phi1);
  tmptree->Branch("NJets30", &NJets30);
  tmptree->Branch("jet1pt", &(jetptetaphimass[0][0]));
  tmptree->Branch("jet2pt", &(jetptetaphimass[1][0]));
  tmptree->Branch("jet1eta", &(jetptetaphimass[0][1]));
  tmptree->Branch("jet2eta", &(jetptetaphimass[1][1]));
  tmptree->Branch("jet1phi", &(jetptetaphimass[0][2]));
  tmptree->Branch("jet2phi", &(jetptetaphimass[1][2]));
  tmptree->Branch("jet1mass", &(jetptetaphimass[0][3]));
  tmptree->Branch("jet2mass", &(jetptetaphimass[1][3]));

  TRandom3 randomthrow(1234567);
  double portion_to_keep = 1;
  if (nTotalEntries>1000000) portion_to_keep = 9.95e5/tree->GetEntries("NJets30>=2");
  for (int ev = 0; ev < nTotalEntries; ev++){
    tree->GetEntry(ev);
    if (NJets30>=2){
      for (int ij=0; ij<2; ij++){
        jetptetaphimass[ij][0]=JetPt->at(ij);
        jetptetaphimass[ij][1]=JetEta->at(ij);
        jetptetaphimass[ij][2]=JetPhi->at(ij);
        jetptetaphimass[ij][3]=JetMass->at(ij);
      }
      double rndnum = randomthrow.Uniform();
      if (rndnum<=portion_to_keep) tmptree->Fill();
    }
  }

  const int nEntries = tmptree->GetEntries();
  if (nEntries>=1000000){
    cerr << "TMath::Sort will experience problems. Aborting!" << endl;
    delete tmptree;
    delete tree;
    assert(0);
  }
  int* index = new int[nEntries];
  tmptree->Draw("ZZMass", "", "goff");
  TMath::Sort(nEntries, tmptree->GetV1(), index, false);

  tmptree->GetEntry(index[0]);
  float firstVal=mzz;
  tmptree->GetEntry(index[nEntries-1]);
  float lastVal=mzz;
  float infimum = (float)((int)firstVal); infimum -= (float)(((int)infimum)%10);
  float supremum = (float)((int)(lastVal+0.5)); supremum += (float)(10-((int)supremum)%10);
  cout << "Nentries = " << nEntries << " | mzz = " << firstVal << " - " << lastVal << "(" << infimum << ", " << supremum << ")" << endl;

  int nbins=0;
  int divisor=11000;
  while (nbins<50){
    if (divisor>1000) divisor -= 1000;
    else if (divisor>100) divisor -= 100;
    else break;
    nbins=nEntries/divisor+1;
  }
  cout << "nbins=" << nbins << endl;
  if (nbins<3) cerr << "Not enough bins!" << endl;
  float* binning = new float[nbins+1];
  binning[0]=infimum;
  binning[nbins]=supremum;
  int ev_stepsize = nEntries/nbins;
  cout << "Event step size: " << ev_stepsize << endl;
  cout << "Boundary (" << 0 << ") = " << binning[0] << endl;
  for (int ix=1; ix<nbins; ix++){
    int ev = index[ix*ev_stepsize];
    tmptree->GetEntry(ev);
    float bhigh = mzz;
    ev = index[ix*ev_stepsize-1];
    float blow = mzz;
    binning[ix]=(bhigh+blow)*0.5;
    cout << "Boundary (" << ix << ")= " << binning[ix] << " [event " << ev << ", step " << ix*ev_stepsize << "]" << endl;
  }
  cout << "Boundary (" << nbins << ") = " << binning[nbins] << endl;
  delete[] index;

  TFile* foutput = new TFile(Form("pAvg_JHUGen_JJQCD_HSMHiggs_%iTeV.root", sqrts), "recreate");

  TProfile* hvar = new TProfile("candMass", "", nbins, binning); hvar->Sumw2();
  TProfile* hmesq_conserveDifermMass = new TProfile("P_ConserveDifermionMass", "", nbins, binning); hmesq_conserveDifermMass->Sumw2();
  TProfile* hmesq_jetPtoEScale = new TProfile("P_MomentumToEnergy", "", nbins, binning); hmesq_jetPtoEScale->Sumw2();

  TTree* newtree=0;
  if (writeFinalTree){
    newtree = new TTree("FinalTree", "");
    newtree->Branch("mesq_conserveDifermMass", &mesq_conserveDifermMass);
    newtree->Branch("mesq_jetPtoEScale", &mesq_jetPtoEScale);
    newtree->Branch("ZZMass", &mzz);
  }

  mela.setCandidateDecayMode(TVar::CandidateDecay_ZZ);

  for (int ev = 0; ev < nEntries; ev++){
    tmptree->GetEntry(ev); // No need for ordering anymore
    if (ev%10000==0) cout << "Doing event " << ev << endl;

    TLorentzVector jet[2], higgs;
    for (int ij=0; ij<2; ij++) jet[ij].SetPtEtaPhiM(jetptetaphimass[ij][0], jetptetaphimass[ij][1], jetptetaphimass[ij][2], jetptetaphimass[ij][3]);
    higgs.SetPtEtaPhiM(ZZPt, ZZEta, ZZPhi, mzz);
    TVector3 boostH = higgs.BoostVector();

    SimpleParticleCollection_t associated;
    associated.push_back(SimpleParticle_t(0, jet[0]));
    associated.push_back(SimpleParticle_t(0, jet[1]));

    TLorentzVector pDaughters[4];
    std::vector<TLorentzVector> daus = mela.calculate4Momentum(mzz, m1, m2, acos(hs), acos(h1), acos(h2), phi1, phi);
    for (int ip=0; ip<min(4, (int)daus.size()); ip++){ pDaughters[ip]=daus.at(ip); pDaughters[ip].Boost(boostH); }
    SimpleParticleCollection_t daughters;
    for (unsigned int idau=0; idau<4; idau++) daughters.push_back(SimpleParticle_t(LepID[idau], pDaughters[idau]));
    mela.setInputEvent(&daughters, &associated, (SimpleParticleCollection_t*)0, false);

    double alphasVal;
    bool doFill=true;
    mela.setProcess(TVar::HSMHiggs, TVar::JHUGen, TVar::JJQCD);

    TUtil::setJetMassScheme(TVar::ConserveDifermionMass);
    mela.computeProdP(mesq_conserveDifermMass, false);
    alphasVal = mela.getIORecord()->getAlphaSatMZ();
    mesq_conserveDifermMass /= pow(alphasVal, 4);
    mesq_conserveDifermMass = log10(mesq_conserveDifermMass);
    if (isnan(mesq_conserveDifermMass) || isinf(mesq_conserveDifermMass)) doFill=false;

    TUtil::setJetMassScheme(TVar::MomentumToEnergy);
    mela.computeProdP(mesq_jetPtoEScale, false);
    alphasVal = mela.getIORecord()->getAlphaSatMZ();
    mesq_jetPtoEScale /= pow(alphasVal, 4);
    mesq_jetPtoEScale = log10(mesq_jetPtoEScale);
    if (isnan(mesq_jetPtoEScale) || isinf(mesq_jetPtoEScale)) doFill=false;

    if (doFill){
      hmesq_conserveDifermMass->Fill(mzz, mesq_conserveDifermMass);
      hmesq_jetPtoEScale->Fill(mzz, mesq_jetPtoEScale);
      hvar->Fill(mzz, mzz);
    }

    if (writeFinalTree) newtree->Fill();
    mela.resetInputEvent();
  }

  double* xexyey[2][4];
  for (int inorm=0; inorm<2; inorm++){
    for (int ix=0; ix<4; ix++) xexyey[inorm][ix] = new double[nbins];
    for (int bin=0; bin<nbins; bin++){
      xexyey[inorm][0][bin] = hvar->GetBinContent(bin+1);
      xexyey[inorm][1][bin] = hvar->GetBinError(bin+1);

      if (inorm==0) cout << "Bin " << bin << " x-center: " << xexyey[inorm][0][bin] << " +- " << xexyey[inorm][1][bin] << endl;

      if (inorm==0){
        xexyey[inorm][2][bin] = hmesq_conserveDifermMass->GetBinContent(bin+1);
        xexyey[inorm][3][bin] = hmesq_conserveDifermMass->GetBinError(bin+1);
      }
      else{
        xexyey[inorm][2][bin] = hmesq_jetPtoEScale->GetBinContent(bin+1);
        xexyey[inorm][3][bin] = hmesq_jetPtoEScale->GetBinError(bin+1);
      }
    }
  }

  for (int inorm=0; inorm<2; inorm++){
    TGraphErrors* tg = new TGraphErrors(nbins, xexyey[inorm][0], xexyey[inorm][2], xexyey[inorm][1], xexyey[inorm][3]);
    if (inorm==0) tg->SetName("tg_P_ConserveDifermionMass");
    else tg->SetName("tg_P_MomentumToEnergy");
    foutput->WriteTObject(tg);
    delete tg;
  }

  for (int inorm=0; inorm<2; inorm++){
    for (int ix=0; ix<4; ix++) delete[] xexyey[inorm][ix];
  }
  foutput->WriteTObject(hmesq_jetPtoEScale);
  foutput->WriteTObject(hmesq_conserveDifermMass);
  foutput->WriteTObject(hvar);
  if (writeFinalTree) foutput->WriteTObject(newtree);
  if (writeFinalTree) delete newtree;
  delete hmesq_conserveDifermMass;
  delete hmesq_jetPtoEScale;
  delete hvar;
  foutput->Close();
  delete[] binning;
  delete tmptree;
  delete tree;
}
/*
Function
[0]*exp(-x/[1])*(1+[2]*exp(-pow(x/[3],2)) + [4]*exp(-x/[5]))
with parameters
152.197
496.507
0.812036
349.521
9.00697
55.4923
fits well.
*/


/* SPECIFIC COMMENT: OUTPUT ME DIVIDED BY ALPHAS(MZ)**3 TO REMAIN INDEPENDENT OF PDF CHOICE TO FIRST APPROXIMATION */
void get_PAvgProfile_JHUGen_JQCD_HSMHiggs_7or8TeV(int sqrts=8){
  int erg_tev=sqrts;
  float mPOLE=125.;
  TString TREE_NAME = "SelectedTree";
  bool writeFinalTree=false;

  TVar::VerbosityLevel verbosity = TVar::ERROR;
  Mela mela(erg_tev, mPOLE, verbosity);

  short NJets30;
  std::vector<double>* JetPt=0;
  std::vector<double>* JetEta=0;
  std::vector<double>* JetPhi=0;
  std::vector<double>* JetMass=0;
  std::vector<double> myJetPt;
  std::vector<double> myJetEta;
  std::vector<double> myJetPhi;
  std::vector<double> myJetMass;
  TBranch* bJetPt=0;
  TBranch* bJetEta=0;
  TBranch* bJetPhi=0;
  TBranch* bJetMass=0;
  float jetptetaphimass[2][4];

  float mesq_conserveDifermMass=0;
  float mesq_jetPtoEScale=0;
  float mzz = 126.;
  float m1 = 91.471450;
  float m2 = 12.139782;
  float h1 = 0.2682896;
  float h2 = 0.1679779;
  float phi = 1.5969792;
  float hs = -0.727181;
  float phi1 = 1.8828257;
  float ZZPt, ZZPhi, ZZEta;
  int LepID[4]={ 13, -13, 11, -11 };

  TString strchannel[3]={ "4mu", "4e", "2mu2e" };
  TString cinput_main;
  if (sqrts==8) cinput_main = "/scratch0/hep/ianderso/CJLST/140519/PRODFSR_8TeV";
  else if (sqrts==7) cinput_main = "/scratch0/hep/ianderso/CJLST/140519/PRODFSR";
  else return;
  const int nSamples = 37;
  TString strSamples[nSamples]={
    "HZZ4lTree_minloH90.root",
    "HZZ4lTree_minloH95.root",
    "HZZ4lTree_minloH100.root",
    "HZZ4lTree_minloH105.root",
    "HZZ4lTree_minloH110.root",
    "HZZ4lTree_minloH115.root",
    "HZZ4lTree_minloH120.root",
    "HZZ4lTree_minloH124.root",
    "HZZ4lTree_minloH125.root",
    "HZZ4lTree_minloH126.root",
    "HZZ4lTree_minloH130.root",
    "HZZ4lTree_minloH135.root",
    "HZZ4lTree_minloH140.root",
    "HZZ4lTree_minloH145.root",
    "HZZ4lTree_minloH150.root",
    "HZZ4lTree_minloH155.root",
    "HZZ4lTree_minloH160.root",
    "HZZ4lTree_minloH170.root",
    "HZZ4lTree_minloH180.root",
    "HZZ4lTree_minloH190.root",
    "HZZ4lTree_minloH200.root",
    "HZZ4lTree_minloH250.root",
    "HZZ4lTree_minloH300.root",
    "HZZ4lTree_minloH350.root",
    "HZZ4lTree_minloH400.root",
    "HZZ4lTree_minloH450.root",
    "HZZ4lTree_minloH500.root",
    "HZZ4lTree_minloH550.root",
    "HZZ4lTree_minloH600.root",
    "HZZ4lTree_minloH650.root",
    "HZZ4lTree_minloH700.root",
    "HZZ4lTree_minloH750.root",
    "HZZ4lTree_minloH800.root",
    "HZZ4lTree_minloH850.root",
    "HZZ4lTree_minloH900.root",
    "HZZ4lTree_minloH950.root",
    "HZZ4lTree_minloH1000.root"
  };

  TChain* tree = new TChain(TREE_NAME, "");
  for (int ic=0; ic<3; ic++){
    for (int is=0; is<nSamples; is++) tree->Add(Form("%s/%s/%s", cinput_main.Data(), (strchannel[ic]).Data(), (strSamples[is]).Data()));
  }
  tree->SetBranchAddress("NJets30", &NJets30);
  tree->SetBranchAddress("JetPt", &JetPt, &bJetPt);
  tree->SetBranchAddress("JetEta", &JetEta, &bJetEta);
  tree->SetBranchAddress("JetPhi", &JetPhi, &bJetPhi);
  tree->SetBranchAddress("JetMass", &JetMass, &bJetMass);
  tree->SetBranchAddress("ZZMass", &mzz);
  tree->SetBranchAddress("ZZPt", &ZZPt);
  tree->SetBranchAddress("ZZEta", &ZZEta);
  tree->SetBranchAddress("ZZPhi", &ZZPhi);
  tree->SetBranchAddress("Z1Mass", &m1);
  tree->SetBranchAddress("Z2Mass", &m2);
  tree->SetBranchAddress("helcosthetaZ1", &h1);
  tree->SetBranchAddress("helcosthetaZ2", &h2);
  tree->SetBranchAddress("helphi", &phi);
  tree->SetBranchAddress("costhetastar", &hs);
  tree->SetBranchAddress("phistarZ1", &phi1);

  const int nTotalEntries = tree->GetEntries();
  cout << "Ntotalentries = " << nTotalEntries << endl;

  TTree* tmptree = new TTree("IntermediateTree", "");
  tmptree->Branch("ZZMass", &mzz);
  tmptree->Branch("ZZPt", &ZZPt);
  tmptree->Branch("ZZEta", &ZZEta);
  tmptree->Branch("ZZPhi", &ZZPhi);
  tmptree->Branch("Z1Mass", &m1);
  tmptree->Branch("Z2Mass", &m2);
  tmptree->Branch("helcosthetaZ1", &h1);
  tmptree->Branch("helcosthetaZ2", &h2);
  tmptree->Branch("helphi", &phi);
  tmptree->Branch("costhetastar", &hs);
  tmptree->Branch("phistarZ1", &phi1);
  tmptree->Branch("NJets30", &NJets30);
  tmptree->Branch("jet1pt", &(jetptetaphimass[0][0]));
  tmptree->Branch("jet2pt", &(jetptetaphimass[1][0]));
  tmptree->Branch("jet1eta", &(jetptetaphimass[0][1]));
  tmptree->Branch("jet2eta", &(jetptetaphimass[1][1]));
  tmptree->Branch("jet1phi", &(jetptetaphimass[0][2]));
  tmptree->Branch("jet2phi", &(jetptetaphimass[1][2]));
  tmptree->Branch("jet1mass", &(jetptetaphimass[0][3]));
  tmptree->Branch("jet2mass", &(jetptetaphimass[1][3]));

  TRandom3 randomthrow(1234567);
  double portion_to_keep = 1;
  if (nTotalEntries>1000000) portion_to_keep = 9.95e5/tree->GetEntries("NJets30==1");
  for (int ev = 0; ev < nTotalEntries; ev++){
    tree->GetEntry(ev);
    if (NJets30==1){
      for (int ij=0; ij<1; ij++){
        jetptetaphimass[ij][0]=JetPt->at(ij);
        jetptetaphimass[ij][1]=JetEta->at(ij);
        jetptetaphimass[ij][2]=JetPhi->at(ij);
        jetptetaphimass[ij][3]=JetMass->at(ij);
      }
      double rndnum = randomthrow.Uniform();
      if (rndnum<=portion_to_keep) tmptree->Fill();
    }
  }

  const int nEntries = tmptree->GetEntries();
  if (nEntries>=1000000){
    cerr << "TMath::Sort will experience problems. Aborting!" << endl;
    delete tmptree;
    delete tree;
    assert(0);
  }
  int* index = new int[nEntries];
  tmptree->Draw("ZZMass", "", "goff");
  TMath::Sort(nEntries, tmptree->GetV1(), index, false);

  tmptree->GetEntry(index[0]);
  float firstVal=mzz;
  tmptree->GetEntry(index[nEntries-1]);
  float lastVal=mzz;
  float infimum = (float)((int)firstVal); infimum -= (float)(((int)infimum)%10);
  float supremum = (float)((int)(lastVal+0.5)); supremum += (float)(10-((int)supremum)%10);
  cout << "Nentries = " << nEntries << " | mzz = " << firstVal << " - " << lastVal << "(" << infimum << ", " << supremum << ")" << endl;

  int nbins=0;
  int divisor=11000;
  while (nbins<50){
    if (divisor>1000) divisor -= 1000;
    else if (divisor>100) divisor -= 100;
    else break;
    nbins=nEntries/divisor+1;
  }
  cout << "nbins=" << nbins << endl;
  if (nbins<3) cerr << "Not enough bins!" << endl;
  float* binning = new float[nbins+1];
  binning[0]=infimum;
  binning[nbins]=supremum;
  int ev_stepsize = nEntries/nbins;
  cout << "Event step size: " << ev_stepsize << endl;
  cout << "Boundary (" << 0 << ") = " << binning[0] << endl;
  for (int ix=1; ix<nbins; ix++){
    int ev = index[ix*ev_stepsize];
    tmptree->GetEntry(ev);
    float bhigh = mzz;
    ev = index[ix*ev_stepsize-1];
    float blow = mzz;
    binning[ix]=(bhigh+blow)*0.5;
    cout << "Boundary (" << ix << ")= " << binning[ix] << " [event " << ev << ", step " << ix*ev_stepsize << "]" << endl;
  }
  cout << "Boundary (" << nbins << ") = " << binning[nbins] << endl;
  delete[] index;

  TFile* foutput = new TFile(Form("pAvg_JHUGen_JQCD_HSMHiggs_%iTeV.root", sqrts), "recreate");

  TProfile* hvar = new TProfile("candMass", "", nbins, binning); hvar->Sumw2();
  TProfile* hmesq_conserveDifermMass = new TProfile("P_ConserveDifermionMass", "", nbins, binning); hmesq_conserveDifermMass->Sumw2();
  TProfile* hmesq_jetPtoEScale = new TProfile("P_MomentumToEnergy", "", nbins, binning); hmesq_jetPtoEScale->Sumw2();

  TTree* newtree=0;
  if (writeFinalTree){
    newtree = new TTree("FinalTree", "");
    newtree->Branch("mesq_conserveDifermMass", &mesq_conserveDifermMass);
    newtree->Branch("mesq_jetPtoEScale", &mesq_jetPtoEScale);
    newtree->Branch("ZZMass", &mzz);
  }

  mela.setCandidateDecayMode(TVar::CandidateDecay_ZZ);

  for (int ev = 0; ev < nEntries; ev++){
    tmptree->GetEntry(ev); // No need for ordering anymore
    if (ev%10000==0) cout << "Doing event " << ev << endl;

    TLorentzVector jet, higgs;
    for (int ij=0; ij<1; ij++) jet.SetPtEtaPhiM(jetptetaphimass[ij][0], jetptetaphimass[ij][1], jetptetaphimass[ij][2], jetptetaphimass[ij][3]);
    higgs.SetPtEtaPhiM(ZZPt, ZZEta, ZZPhi, mzz);
    TVector3 boostH = higgs.BoostVector();

    SimpleParticleCollection_t associated;
    associated.push_back(SimpleParticle_t(0, jet));

    TLorentzVector pDaughters[4];
    std::vector<TLorentzVector> daus = mela.calculate4Momentum(mzz, m1, m2, acos(hs), acos(h1), acos(h2), phi1, phi);
    for (int ip=0; ip<min(4, (int)daus.size()); ip++){ pDaughters[ip]=daus.at(ip); pDaughters[ip].Boost(boostH); }
    SimpleParticleCollection_t daughters;
    for (unsigned int idau=0; idau<4; idau++) daughters.push_back(SimpleParticle_t(LepID[idau], pDaughters[idau]));
    mela.setInputEvent(&daughters, &associated, (SimpleParticleCollection_t*)0, false);

    hvar->Fill(mzz, mzz);

    double alphasVal;

    mela.setProcess(TVar::HSMHiggs, TVar::JHUGen, TVar::JQCD);
    TUtil::setJetMassScheme(TVar::ConserveDifermionMass);
    mela.computeProdP(mesq_conserveDifermMass, false);
    alphasVal = mela.getIORecord()->getAlphaSatMZ();
    mesq_conserveDifermMass /= pow(alphasVal, 3);
    mesq_conserveDifermMass = log10(mesq_conserveDifermMass);
    hmesq_conserveDifermMass->Fill(mzz, mesq_conserveDifermMass);

    TUtil::setJetMassScheme(TVar::MomentumToEnergy);
    mela.computeProdP(mesq_jetPtoEScale, false);
    alphasVal = mela.getIORecord()->getAlphaSatMZ();
    mesq_jetPtoEScale /= pow(alphasVal, 3);
    mesq_jetPtoEScale = log10(mesq_jetPtoEScale);
    hmesq_jetPtoEScale->Fill(mzz, mesq_jetPtoEScale);

    if (writeFinalTree) newtree->Fill();
    mela.resetInputEvent();
  }

  double* xexyey[2][4];
  for (int inorm=0; inorm<2; inorm++){
    for (int ix=0; ix<4; ix++) xexyey[inorm][ix] = new double[nbins];
    for (int bin=0; bin<nbins; bin++){
      xexyey[inorm][0][bin] = hvar->GetBinContent(bin+1);
      xexyey[inorm][1][bin] = hvar->GetBinError(bin+1);

      if (inorm==0) cout << "Bin " << bin << " x-center: " << xexyey[inorm][0][bin] << " +- " << xexyey[inorm][1][bin] << endl;

      if (inorm==0){
        xexyey[inorm][2][bin] = hmesq_conserveDifermMass->GetBinContent(bin+1);
        xexyey[inorm][3][bin] = hmesq_conserveDifermMass->GetBinError(bin+1);
      }
      else{
        xexyey[inorm][2][bin] = hmesq_jetPtoEScale->GetBinContent(bin+1);
        xexyey[inorm][3][bin] = hmesq_jetPtoEScale->GetBinError(bin+1);
      }
    }
  }

  for (int inorm=0; inorm<2; inorm++){
    TGraphErrors* tg = new TGraphErrors(nbins, xexyey[inorm][0], xexyey[inorm][2], xexyey[inorm][1], xexyey[inorm][3]);
    if (inorm==0) tg->SetName("tg_P_ConserveDifermionMass");
    else tg->SetName("tg_P_MomentumToEnergy");
    foutput->WriteTObject(tg);
    delete tg;
  }

  for (int inorm=0; inorm<2; inorm++){
    for (int ix=0; ix<4; ix++) delete[] xexyey[inorm][ix];
  }
  foutput->WriteTObject(hmesq_jetPtoEScale);
  foutput->WriteTObject(hmesq_conserveDifermMass);
  foutput->WriteTObject(hvar);
  if (writeFinalTree) foutput->WriteTObject(newtree);
  if (writeFinalTree) delete newtree;
  delete hmesq_conserveDifermMass;
  delete hmesq_jetPtoEScale;
  delete hvar;
  foutput->Close();
  delete[] binning;
  delete tmptree;
  delete tree;
}
/*
Function
[0]*exp(-x/[1])*(1+[2]*exp(-pow(x/[3],2)) + [4]*exp(-x/[5]))
with parameters
196.358
291.176
14.6094
92.3443
13.2622
133.669
fits well.
*/




/*
SPECIFIC COMMENT: OUTPUT ME DIVIDED BY
- ALPHAS(MZ)**2 TO REMAIN INDEPENDENT OF PDF CHOICE TO FIRST APPROXIMATION
- H(1) PROPAGATOR
- (aL1**2+aR1**2)*(aL2**2+aR2**2) TO REMAIN INDEPENDENT OF CHANNEL
*/
void get_PAvgProfile_JHUGen_ZZGG_HSMHiggs(){
  int erg_tev=8;
  float mPOLE=125.;
  TString TREE_NAME = "SelectedTree";
  const bool writeFinalTree=false;

  TVar::VerbosityLevel verbosity = TVar::ERROR;
  Mela mela(erg_tev, mPOLE, verbosity);

  float mesq_conserveDifermMass=0;
  float mzz = 126.;
  float m1 = 91.471450;
  float m2 = 12.139782;
  float h1 = 0.2682896;
  float h2 = 0.1679779;
  float phi = 1.5969792;
  float hs = -0.727181;
  float phi1 = 1.8828257;
  int LepID[4]={ 13, -13, 11, -11 };

  TString strchannel[3]={ "4mu", "4e", "2mu2e" };
  TString cinput_main = "/scratch0/hep/ianderso/CJLST/140519/PRODFSR_8TeV";
  const int nSamples = 37;
  TString strSamples[nSamples]={
    "HZZ4lTree_minloH90.root",
    "HZZ4lTree_minloH95.root",
    "HZZ4lTree_minloH100.root",
    "HZZ4lTree_minloH105.root",
    "HZZ4lTree_minloH110.root",
    "HZZ4lTree_minloH115.root",
    "HZZ4lTree_minloH120.root",
    "HZZ4lTree_minloH124.root",
    "HZZ4lTree_minloH125.root",
    "HZZ4lTree_minloH126.root",
    "HZZ4lTree_minloH130.root",
    "HZZ4lTree_minloH135.root",
    "HZZ4lTree_minloH140.root",
    "HZZ4lTree_minloH145.root",
    "HZZ4lTree_minloH150.root",
    "HZZ4lTree_minloH155.root",
    "HZZ4lTree_minloH160.root",
    "HZZ4lTree_minloH170.root",
    "HZZ4lTree_minloH180.root",
    "HZZ4lTree_minloH190.root",
    "HZZ4lTree_minloH200.root",
    "HZZ4lTree_minloH250.root",
    "HZZ4lTree_minloH300.root",
    "HZZ4lTree_minloH350.root",
    "HZZ4lTree_minloH400.root",
    "HZZ4lTree_minloH450.root",
    "HZZ4lTree_minloH500.root",
    "HZZ4lTree_minloH550.root",
    "HZZ4lTree_minloH600.root",
    "HZZ4lTree_minloH650.root",
    "HZZ4lTree_minloH700.root",
    "HZZ4lTree_minloH750.root",
    "HZZ4lTree_minloH800.root",
    "HZZ4lTree_minloH850.root",
    "HZZ4lTree_minloH900.root",
    "HZZ4lTree_minloH950.root",
    "HZZ4lTree_minloH1000.root"
  };

  TFile* foutput = new TFile("pAvg_JHUGen_ZZGG_HSMHiggs.root", "recreate");

  for (int ic=0; ic<3; ic++){
    gROOT->cd();
    TChain* tree = new TChain(TREE_NAME, "");
    for (int is=0; is<nSamples; is++) tree->Add(Form("%s/%s/%s", cinput_main.Data(), (strchannel[ic]).Data(), (strSamples[is]).Data()));
    tree->SetBranchAddress("ZZMass", &mzz);
    tree->SetBranchAddress("Z1Mass", &m1);
    tree->SetBranchAddress("Z2Mass", &m2);
    tree->SetBranchAddress("helcosthetaZ1", &h1);
    tree->SetBranchAddress("helcosthetaZ2", &h2);
    tree->SetBranchAddress("helphi", &phi);
    tree->SetBranchAddress("costhetastar", &hs);
    tree->SetBranchAddress("phistarZ1", &phi1);

    const int nTotalEntries = tree->GetEntries();
    cout << "Ntotalentries = " << nTotalEntries << endl;

    TTree* tmptree = new TTree("IntermediateTree", "");
    tmptree->Branch("ZZMass", &mzz);
    tmptree->Branch("Z1Mass", &m1);
    tmptree->Branch("Z2Mass", &m2);
    tmptree->Branch("helcosthetaZ1", &h1);
    tmptree->Branch("helcosthetaZ2", &h2);
    tmptree->Branch("helphi", &phi);
    tmptree->Branch("costhetastar", &hs);
    tmptree->Branch("phistarZ1", &phi1);

    TRandom3 randomthrow(1234567);
    double portion_to_keep = 1;
    if (nTotalEntries>1000000) portion_to_keep = 9.95e5/nTotalEntries;
    for (int ev = 0; ev < nTotalEntries; ev++){
      tree->GetEntry(ev);
      double rndnum = randomthrow.Uniform();
      if (rndnum<=portion_to_keep) tmptree->Fill();
    }

    const int nEntries = tmptree->GetEntries();
    if (nEntries>=1000000){
      cerr << "TMath::Sort will experience problems. Aborting!" << endl;
      delete tmptree;
      delete tree;
      continue;
    }
    int* index = new int[nEntries];
    tmptree->Draw("ZZMass", "", "goff");
    TMath::Sort(nEntries, tmptree->GetV1(), index, false);

    tmptree->GetEntry(index[0]);
    float firstVal=mzz;
    tmptree->GetEntry(index[nEntries-1]);
    float lastVal=mzz;
    float infimum = (float)((int)firstVal); infimum -= (float)(((int)infimum)%10);
    float supremum = (float)((int)(lastVal+0.5)); supremum += (float)(10-((int)supremum)%10);
    cout << "Nentries = " << nEntries << " | mzz = " << firstVal << " - " << lastVal << "(" << infimum << ", " << supremum << ")" << endl;

    int nbins=0;
    int divisor=11000;
    while (nbins<50){
      if (divisor>1000) divisor -= 1000;
      else if (divisor>100) divisor -= 100;
      else break;
      nbins=nEntries/divisor+1;
    }
    cout << "nbins=" << nbins << endl;
    if (nbins<3) cerr << "Not enough bins!" << endl;
    float* binning = new float[nbins+1];
    binning[0]=infimum;
    binning[nbins]=supremum;
    int ev_stepsize = nEntries/nbins;
    cout << "Event step size: " << ev_stepsize << endl;
    cout << "Boundary (" << 0 << ") = " << binning[0] << endl;
    for (int ix=1; ix<nbins; ix++){
      int ev = index[ix*ev_stepsize];
      tmptree->GetEntry(ev);
      float bhigh = mzz;
      ev = index[ix*ev_stepsize-1];
      float blow = mzz;
      binning[ix]=(bhigh+blow)*0.5;
      cout << "Boundary (" << ix << ")= " << binning[ix] << " [event " << ev << ", step " << ix*ev_stepsize << "]" << endl;
    }
    cout << "Boundary (" << nbins << ") = " << binning[nbins] << endl;
    delete[] index;

    foutput->cd();
    TProfile* hvar = new TProfile(Form("candMass_%s", strchannel[ic].Data()), "", nbins, binning); hvar->Sumw2();
    TProfile* hmesq_conserveDifermMass = new TProfile(Form("P_ConserveDifermionMass_%s", strchannel[ic].Data()), "", nbins, binning); hmesq_conserveDifermMass->Sumw2();

    TTree* newtree=0;
    if (writeFinalTree){
      newtree = new TTree("FinalTree", "");
      newtree->Branch("mesq_conserveDifermMass", &mesq_conserveDifermMass);
      newtree->Branch("ZZMass", &mzz);
    }

    mela.setCandidateDecayMode(TVar::CandidateDecay_ZZ);
    if (ic==1){
      LepID[0]=11;
      LepID[1]=-11;
    }
    else{
      LepID[0]=13;
      LepID[1]=-13;
    }
    if (ic==0){
      LepID[2]=13;
      LepID[3]=-13;
    }
    else{
      LepID[2]=11;
      LepID[3]=-11;
    }

    for (int ev = 0; ev < nEntries; ev++){
      tmptree->GetEntry(ev); // No need for ordering anymore
      if (ev%10000==0) cout << "Doing event " << ev << endl;

      TLorentzVector pDaughters[4];
      std::vector<TLorentzVector> daus = mela.calculate4Momentum(mzz, m1, m2, acos(hs), acos(h1), acos(h2), phi1, phi);
      for (int ip=0; ip<min(4, (int)daus.size()); ip++){ pDaughters[ip]=daus.at(ip); }
      SimpleParticleCollection_t daughters;
      for (unsigned int idau=0; idau<4; idau++) daughters.push_back(SimpleParticle_t(LepID[idau], pDaughters[idau]));
      mela.setInputEvent(&daughters, (SimpleParticleCollection_t*)0, (SimpleParticleCollection_t*)0, false);

      hvar->Fill(mzz, mzz);

      double alphasVal, propagator, mh, gah;

      mela.setProcess(TVar::HSMHiggs, TVar::JHUGen, TVar::ZZGG);
      TUtil::setLeptonMassScheme(TVar::ConserveDifermionMass);
      mela.computeP(mesq_conserveDifermMass, false);
      alphasVal = mela.getIORecord()->getAlphaSatMZ();
      mela.getIORecord()->getHiggsMassWidth(mh, gah, 0);
      propagator = 1./(pow(pow(mzz, 2)-pow(mh, 2), 2) + pow(mh*gah, 2));
      mesq_conserveDifermMass /= pow(alphasVal, 2);
      mesq_conserveDifermMass /= propagator;
      double aL1, aR1, aL2, aR2;
      mela.getIORecord()->getVDaughterCouplings(aL1, aR1, 0);
      mela.getIORecord()->getVDaughterCouplings(aL2, aR2, 1);
      if (fabs(aL1)>0. || fabs(aR1)>0.) mesq_conserveDifermMass /= pow(aL1, 2)+pow(aR1, 2);
      if (fabs(aL2)>0. || fabs(aR2)>0.) mesq_conserveDifermMass /= pow(aL2, 2)+pow(aR2, 2);
      mesq_conserveDifermMass = log10(mesq_conserveDifermMass);
      hmesq_conserveDifermMass->Fill(mzz, mesq_conserveDifermMass);

      if (writeFinalTree) newtree->Fill();
      mela.resetInputEvent();
    }

    double* xexyey[4];
    for (int ix=0; ix<4; ix++) xexyey[ix] = new double[nbins];
    for (int bin=0; bin<nbins; bin++){
      xexyey[0][bin] = hvar->GetBinContent(bin+1);
      xexyey[1][bin] = hvar->GetBinError(bin+1);

      cout << "Bin " << bin << " x-center: " << xexyey[0][bin] << " +- " << xexyey[1][bin] << endl;
      xexyey[2][bin] = hmesq_conserveDifermMass->GetBinContent(bin+1);
      xexyey[3][bin] = hmesq_conserveDifermMass->GetBinError(bin+1);
    }


    TGraphErrors* tg = new TGraphErrors(nbins, xexyey[0], xexyey[2], xexyey[1], xexyey[3]);
    tg->SetName(Form("tg_%s", hmesq_conserveDifermMass->GetName()));
    foutput->WriteTObject(tg);
    delete tg;

    for (int ix=0; ix<4; ix++) delete[] xexyey[ix];
    foutput->WriteTObject(hmesq_conserveDifermMass);
    foutput->WriteTObject(hvar);
    if (writeFinalTree) foutput->WriteTObject(newtree);
    if (writeFinalTree) delete newtree;
    delete hmesq_conserveDifermMass;
    delete hvar;
    delete[] binning;
    delete tmptree;
    delete tree;
  }
  foutput->Close();
}


/*
SPECIFIC COMMENT: OUTPUT ME DIVIDED BY
- ALPHAS(MZ)**2 TO REMAIN INDEPENDENT OF PDF CHOICE TO FIRST APPROXIMATION
- H(1) PROPAGATOR
- (aL1**2+aR1**2)*(aL2**2+aR2**2) TO REMAIN INDEPENDENT OF CHANNEL
*/
void get_PAvgProfile_MCFM_ZZGG_HSMHiggs(){
  int erg_tev=8;
  float mPOLE=125.;
  TString TREE_NAME = "SelectedTree";
  const bool writeFinalTree=false;

  TVar::VerbosityLevel verbosity = TVar::ERROR;
  Mela mela(erg_tev, mPOLE, verbosity);

  float mesq_conserveDifermMass=0;
  float mzz = 126.;
  float m1 = 91.471450;
  float m2 = 12.139782;
  float h1 = 0.2682896;
  float h2 = 0.1679779;
  float phi = 1.5969792;
  float hs = -0.727181;
  float phi1 = 1.8828257;
  int LepID[4]={ 13, -13, 11, -11 };

  TString strchannel[3]={ "4mu", "4e", "2mu2e" };
  TString cinput_main = "/scratch0/hep/ianderso/CJLST/140519/PRODFSR_8TeV";
  const int nSamples = 37;
  TString strSamples[nSamples]={
    "HZZ4lTree_minloH90.root",
    "HZZ4lTree_minloH95.root",
    "HZZ4lTree_minloH100.root",
    "HZZ4lTree_minloH105.root",
    "HZZ4lTree_minloH110.root",
    "HZZ4lTree_minloH115.root",
    "HZZ4lTree_minloH120.root",
    "HZZ4lTree_minloH124.root",
    "HZZ4lTree_minloH125.root",
    "HZZ4lTree_minloH126.root",
    "HZZ4lTree_minloH130.root",
    "HZZ4lTree_minloH135.root",
    "HZZ4lTree_minloH140.root",
    "HZZ4lTree_minloH145.root",
    "HZZ4lTree_minloH150.root",
    "HZZ4lTree_minloH155.root",
    "HZZ4lTree_minloH160.root",
    "HZZ4lTree_minloH170.root",
    "HZZ4lTree_minloH180.root",
    "HZZ4lTree_minloH190.root",
    "HZZ4lTree_minloH200.root",
    "HZZ4lTree_minloH250.root",
    "HZZ4lTree_minloH300.root",
    "HZZ4lTree_minloH350.root",
    "HZZ4lTree_minloH400.root",
    "HZZ4lTree_minloH450.root",
    "HZZ4lTree_minloH500.root",
    "HZZ4lTree_minloH550.root",
    "HZZ4lTree_minloH600.root",
    "HZZ4lTree_minloH650.root",
    "HZZ4lTree_minloH700.root",
    "HZZ4lTree_minloH750.root",
    "HZZ4lTree_minloH800.root",
    "HZZ4lTree_minloH850.root",
    "HZZ4lTree_minloH900.root",
    "HZZ4lTree_minloH950.root",
    "HZZ4lTree_minloH1000.root"
  };

  TFile* foutput = new TFile("pAvg_MCFM_ZZGG_HSMHiggs.root", "recreate");

  for (int ic=0; ic<3; ic++){
    gROOT->cd();
    TChain* tree = new TChain(TREE_NAME, "");
    for (int is=0; is<nSamples; is++) tree->Add(Form("%s/%s/%s", cinput_main.Data(), (strchannel[ic]).Data(), (strSamples[is]).Data()));
    tree->SetBranchAddress("ZZMass", &mzz);
    tree->SetBranchAddress("Z1Mass", &m1);
    tree->SetBranchAddress("Z2Mass", &m2);
    tree->SetBranchAddress("helcosthetaZ1", &h1);
    tree->SetBranchAddress("helcosthetaZ2", &h2);
    tree->SetBranchAddress("helphi", &phi);
    tree->SetBranchAddress("costhetastar", &hs);
    tree->SetBranchAddress("phistarZ1", &phi1);

    const int nTotalEntries = tree->GetEntries();
    cout << "Ntotalentries = " << nTotalEntries << endl;

    TTree* tmptree = new TTree("IntermediateTree", "");
    tmptree->Branch("ZZMass", &mzz);
    tmptree->Branch("Z1Mass", &m1);
    tmptree->Branch("Z2Mass", &m2);
    tmptree->Branch("helcosthetaZ1", &h1);
    tmptree->Branch("helcosthetaZ2", &h2);
    tmptree->Branch("helphi", &phi);
    tmptree->Branch("costhetastar", &hs);
    tmptree->Branch("phistarZ1", &phi1);

    TRandom3 randomthrow(1234567);
    double portion_to_keep = 1;
    if (nTotalEntries>1000000) portion_to_keep = 9.95e5/nTotalEntries;
    for (int ev = 0; ev < nTotalEntries; ev++){
      tree->GetEntry(ev);
      double rndnum = randomthrow.Uniform();
      if (rndnum<=portion_to_keep) tmptree->Fill();
    }

    const int nEntries = tmptree->GetEntries();
    if (nEntries>=1000000){
      cerr << "TMath::Sort will experience problems. Aborting!" << endl;
      delete tmptree;
      delete tree;
      continue;
    }
    int* index = new int[nEntries];
    tmptree->Draw("ZZMass", "", "goff");
    TMath::Sort(nEntries, tmptree->GetV1(), index, false);

    tmptree->GetEntry(index[0]);
    float firstVal=mzz;
    tmptree->GetEntry(index[nEntries-1]);
    float lastVal=mzz;
    float infimum = (float)((int)firstVal); infimum -= (float)(((int)infimum)%10);
    float supremum = (float)((int)(lastVal+0.5)); supremum += (float)(10-((int)supremum)%10);
    cout << "Nentries = " << nEntries << " | mzz = " << firstVal << " - " << lastVal << "(" << infimum << ", " << supremum << ")" << endl;

    int nbins=0;
    int divisor=11000;
    while (nbins<50){
      if (divisor>1000) divisor -= 1000;
      else if (divisor>100) divisor -= 100;
      else break;
      nbins=nEntries/divisor+1;
    }
    cout << "nbins=" << nbins << endl;
    if (nbins<3) cerr << "Not enough bins!" << endl;
    float* binning = new float[nbins+1];
    binning[0]=infimum;
    binning[nbins]=supremum;
    int ev_stepsize = nEntries/nbins;
    cout << "Event step size: " << ev_stepsize << endl;
    cout << "Boundary (" << 0 << ") = " << binning[0] << endl;
    for (int ix=1; ix<nbins; ix++){
      int ev = index[ix*ev_stepsize];
      tmptree->GetEntry(ev);
      float bhigh = mzz;
      ev = index[ix*ev_stepsize-1];
      float blow = mzz;
      binning[ix]=(bhigh+blow)*0.5;
      cout << "Boundary (" << ix << ")= " << binning[ix] << " [event " << ev << ", step " << ix*ev_stepsize << "]" << endl;
    }
    cout << "Boundary (" << nbins << ") = " << binning[nbins] << endl;
    delete[] index;

    foutput->cd();
    TProfile* hvar = new TProfile(Form("candMass_%s", strchannel[ic].Data()), "", nbins, binning); hvar->Sumw2();
    TProfile* hmesq_conserveDifermMass = new TProfile(Form("P_ConserveDifermionMass_%s", strchannel[ic].Data()), "", nbins, binning); hmesq_conserveDifermMass->Sumw2();

    TTree* newtree=0;
    if (writeFinalTree){
      newtree = new TTree("FinalTree", "");
      newtree->Branch("mesq_conserveDifermMass", &mesq_conserveDifermMass);
      newtree->Branch("ZZMass", &mzz);
    }

    mela.setCandidateDecayMode(TVar::CandidateDecay_ZZ);
    if (ic==1){
      LepID[0]=11;
      LepID[1]=-11;
    }
    else{
      LepID[0]=13;
      LepID[1]=-13;
    }
    if (ic==0){
      LepID[2]=13;
      LepID[3]=-13;
    }
    else{
      LepID[2]=11;
      LepID[3]=-11;
    }

    for (int ev = 0; ev < nEntries; ev++){
      tmptree->GetEntry(ev); // No need for ordering anymore
      if (ev%10000==0) cout << "Doing event " << ev << endl;

      TLorentzVector pDaughters[4];
      std::vector<TLorentzVector> daus = mela.calculate4Momentum(mzz, m1, m2, acos(hs), acos(h1), acos(h2), phi1, phi);
      for (int ip=0; ip<min(4, (int)daus.size()); ip++){ pDaughters[ip]=daus.at(ip); }
      SimpleParticleCollection_t daughters;
      for (unsigned int idau=0; idau<4; idau++) daughters.push_back(SimpleParticle_t(LepID[idau], pDaughters[idau]));
      mela.setInputEvent(&daughters, (SimpleParticleCollection_t*)0, (SimpleParticleCollection_t*)0, false);

      hvar->Fill(mzz, mzz);

      double alphasVal, propagator, mh, gah;

      mela.setProcess(TVar::HSMHiggs, TVar::MCFM, TVar::ZZGG);
      TUtil::setLeptonMassScheme(TVar::ConserveDifermionMass);
      mela.computeP(mesq_conserveDifermMass, false);
      alphasVal = mela.getIORecord()->getAlphaSatMZ();
      mela.getIORecord()->getHiggsMassWidth(mh, gah, 0);
      propagator = 1./(pow(pow(mzz, 2)-pow(mh, 2), 2) + pow(mh*gah, 2));
      mesq_conserveDifermMass /= pow(alphasVal, 2);
      mesq_conserveDifermMass /= propagator;
      double aL1, aR1, aL2, aR2;
      mela.getIORecord()->getVDaughterCouplings(aL1, aR1, 0);
      mela.getIORecord()->getVDaughterCouplings(aL2, aR2, 1);
      if (fabs(aL1)>0. || fabs(aR1)>0.) mesq_conserveDifermMass /= pow(aL1, 2)+pow(aR1, 2);
      if (fabs(aL2)>0. || fabs(aR2)>0.) mesq_conserveDifermMass /= pow(aL2, 2)+pow(aR2, 2);
      mesq_conserveDifermMass = log10(mesq_conserveDifermMass);
      hmesq_conserveDifermMass->Fill(mzz, mesq_conserveDifermMass);

      if (writeFinalTree) newtree->Fill();
      mela.resetInputEvent();
    }

    double* xexyey[4];
    for (int ix=0; ix<4; ix++) xexyey[ix] = new double[nbins];
    for (int bin=0; bin<nbins; bin++){
      xexyey[0][bin] = hvar->GetBinContent(bin+1);
      xexyey[1][bin] = hvar->GetBinError(bin+1);

      cout << "Bin " << bin << " x-center: " << xexyey[0][bin] << " +- " << xexyey[1][bin] << endl;
      xexyey[2][bin] = hmesq_conserveDifermMass->GetBinContent(bin+1);
      xexyey[3][bin] = hmesq_conserveDifermMass->GetBinError(bin+1);
    }


    TGraphErrors* tg = new TGraphErrors(nbins, xexyey[0], xexyey[2], xexyey[1], xexyey[3]);
    tg->SetName(Form("tg_%s", hmesq_conserveDifermMass->GetName()));
    foutput->WriteTObject(tg);
    delete tg;

    for (int ix=0; ix<4; ix++) delete[] xexyey[ix];
    foutput->WriteTObject(hmesq_conserveDifermMass);
    foutput->WriteTObject(hvar);
    if (writeFinalTree) foutput->WriteTObject(newtree);
    if (writeFinalTree) delete newtree;
    delete hmesq_conserveDifermMass;
    delete hvar;
    delete[] binning;
    delete tmptree;
    delete tree;
  }
  foutput->Close();
}


/*
SPECIFIC COMMENT: OUTPUT ME DIVIDED BY
- ALPHAS(MZ)**2 TO REMAIN INDEPENDENT OF PDF CHOICE TO FIRST APPROXIMATION
- (aL1**2+aR1**2)*(aL2**2+aR2**2) TO REMAIN INDEPENDENT OF CHANNEL
*/
void get_PAvgProfile_MCFM_ZZGG_bkgZZ(){
  int erg_tev=8;
  float mPOLE=125.;
  TString TREE_NAME = "SelectedTree";
  const bool writeFinalTree=false;

  TVar::VerbosityLevel verbosity = TVar::ERROR;
  Mela mela(erg_tev, mPOLE, verbosity);

  float mesq_conserveDifermMass=0;
  float mzz = 126.;
  float m1 = 91.471450;
  float m2 = 12.139782;
  float h1 = 0.2682896;
  float h2 = 0.1679779;
  float phi = 1.5969792;
  float hs = -0.727181;
  float phi1 = 1.8828257;
  int LepID[4]={ 13, -13, 11, -11 };

  TString strchannel[3]={ "4mu", "4e", "2mu2e" };
  TString cinput_main = "/scratch0/hep/ianderso/CJLST/140519/PRODFSR_8TeV";
  const int nSamples = 7;
  TString strSamples[nSamples]={
    "HZZ4lTree_ggTo4mu_Contin-MCFM67.root",
    "HZZ4lTree_ggTo4e_Contin-MCFM67.root",
    "HZZ4lTree_ggTo2e2mu_Contin-MCFM67.root",
    "HZZ4lTree_ggTo4l_Continuum.root",
    "HZZ4lTree_ggTo2l2l_Continuum.root",
    "HZZ4lTree_ggZZ4l.root",
    "HZZ4lTree_ggZZ2l2l.root"
  };

  TFile* foutput = new TFile("pAvg_MCFM_ZZGG_bkgZZ.root", "recreate");

  for (int ic=0; ic<3; ic++){
    gROOT->cd();
    TChain* tree = new TChain(TREE_NAME, "");
    for (int is=0; is<nSamples; is++) tree->Add(Form("%s/%s/%s", cinput_main.Data(), (strchannel[ic]).Data(), (strSamples[is]).Data()));
    tree->SetBranchAddress("ZZMass", &mzz);
    tree->SetBranchAddress("Z1Mass", &m1);
    tree->SetBranchAddress("Z2Mass", &m2);
    tree->SetBranchAddress("helcosthetaZ1", &h1);
    tree->SetBranchAddress("helcosthetaZ2", &h2);
    tree->SetBranchAddress("helphi", &phi);
    tree->SetBranchAddress("costhetastar", &hs);
    tree->SetBranchAddress("phistarZ1", &phi1);

    const int nTotalEntries = tree->GetEntries();
    cout << "Ntotalentries = " << nTotalEntries << endl;

    TTree* tmptree = new TTree("IntermediateTree", "");
    tmptree->Branch("ZZMass", &mzz);
    tmptree->Branch("Z1Mass", &m1);
    tmptree->Branch("Z2Mass", &m2);
    tmptree->Branch("helcosthetaZ1", &h1);
    tmptree->Branch("helcosthetaZ2", &h2);
    tmptree->Branch("helphi", &phi);
    tmptree->Branch("costhetastar", &hs);
    tmptree->Branch("phistarZ1", &phi1);

    TRandom3 randomthrow(1234567);
    double portion_to_keep = 1;
    if (nTotalEntries>1000000) portion_to_keep = 9.95e5/nTotalEntries;
    for (int ev = 0; ev < nTotalEntries; ev++){
      tree->GetEntry(ev);
      double rndnum = randomthrow.Uniform();
      if (rndnum<=portion_to_keep) tmptree->Fill();
    }

    const int nEntries = tmptree->GetEntries();
    if (nEntries>=1000000){
      cerr << "TMath::Sort will experience problems. Aborting!" << endl;
      delete tmptree;
      delete tree;
      continue;
    }
    int* index = new int[nEntries];
    tmptree->Draw("ZZMass", "", "goff");
    TMath::Sort(nEntries, tmptree->GetV1(), index, false);

    tmptree->GetEntry(index[0]);
    float firstVal=mzz;
    tmptree->GetEntry(index[nEntries-1]);
    float lastVal=mzz;
    float infimum = (float)((int)firstVal); infimum -= (float)(((int)infimum)%10);
    float supremum = (float)((int)(lastVal+0.5)); supremum += (float)(10-((int)supremum)%10);
    cout << "Nentries = " << nEntries << " | mzz = " << firstVal << " - " << lastVal << "(" << infimum << ", " << supremum << ")" << endl;

    int nbins=0;
    int divisor=11000;
    while (nbins<50){
      if (divisor>1000) divisor -= 1000;
      else if (divisor>100) divisor -= 100;
      else break;
      nbins=nEntries/divisor+1;
    }
    cout << "nbins=" << nbins << endl;
    if (nbins<3) cerr << "Not enough bins!" << endl;
    float* binning = new float[nbins+1];
    binning[0]=infimum;
    binning[nbins]=supremum;
    int ev_stepsize = nEntries/nbins;
    cout << "Event step size: " << ev_stepsize << endl;
    cout << "Boundary (" << 0 << ") = " << binning[0] << endl;
    for (int ix=1; ix<nbins; ix++){
      int ev = index[ix*ev_stepsize];
      tmptree->GetEntry(ev);
      float bhigh = mzz;
      ev = index[ix*ev_stepsize-1];
      float blow = mzz;
      binning[ix]=(bhigh+blow)*0.5;
      cout << "Boundary (" << ix << ")= " << binning[ix] << " [event " << ev << ", step " << ix*ev_stepsize << "]" << endl;
    }
    cout << "Boundary (" << nbins << ") = " << binning[nbins] << endl;
    delete[] index;

    foutput->cd();
    TProfile* hvar = new TProfile(Form("candMass_%s", strchannel[ic].Data()), "", nbins, binning); hvar->Sumw2();
    TProfile* hmesq_conserveDifermMass = new TProfile(Form("P_ConserveDifermionMass_%s", strchannel[ic].Data()), "", nbins, binning); hmesq_conserveDifermMass->Sumw2();

    TTree* newtree=0;
    if (writeFinalTree){
      newtree = new TTree("FinalTree", "");
      newtree->Branch("mesq_conserveDifermMass", &mesq_conserveDifermMass);
      newtree->Branch("ZZMass", &mzz);
    }

    mela.setCandidateDecayMode(TVar::CandidateDecay_ZZ);
    if (ic==1){
      LepID[0]=11;
      LepID[1]=-11;
    }
    else{
      LepID[0]=13;
      LepID[1]=-13;
    }
    if (ic==0){
      LepID[2]=13;
      LepID[3]=-13;
    }
    else{
      LepID[2]=11;
      LepID[3]=-11;
    }

    for (int ev = 0; ev < nEntries; ev++){
      tmptree->GetEntry(ev); // No need for ordering anymore
      if (ev%10000==0) cout << "Doing event " << ev << endl;

      TLorentzVector pDaughters[4];
      std::vector<TLorentzVector> daus = mela.calculate4Momentum(mzz, m1, m2, acos(hs), acos(h1), acos(h2), phi1, phi);
      for (int ip=0; ip<min(4, (int)daus.size()); ip++){ pDaughters[ip]=daus.at(ip); }
      SimpleParticleCollection_t daughters;
      for (unsigned int idau=0; idau<4; idau++) daughters.push_back(SimpleParticle_t(LepID[idau], pDaughters[idau]));
      mela.setInputEvent(&daughters, (SimpleParticleCollection_t*)0, (SimpleParticleCollection_t*)0, false);

      double alphasVal;
      bool doFill=true;
      mela.setProcess(TVar::bkgZZ, TVar::MCFM, TVar::ZZGG);

      TUtil::setLeptonMassScheme(TVar::ConserveDifermionMass);
      mela.computeP(mesq_conserveDifermMass, false);
      alphasVal = mela.getIORecord()->getAlphaSatMZ();
      mesq_conserveDifermMass /= pow(alphasVal, 2);
      double aL1, aR1, aL2, aR2;
      mela.getIORecord()->getVDaughterCouplings(aL1, aR1, 0);
      mela.getIORecord()->getVDaughterCouplings(aL2, aR2, 1);
      if (fabs(aL1)>0. || fabs(aR1)>0.) mesq_conserveDifermMass /= pow(aL1, 2)+pow(aR1, 2);
      if (fabs(aL2)>0. || fabs(aR2)>0.) mesq_conserveDifermMass /= pow(aL2, 2)+pow(aR2, 2);
      mesq_conserveDifermMass = log10(mesq_conserveDifermMass);
      if (isnan(mesq_conserveDifermMass) || isinf(mesq_conserveDifermMass)) doFill=false;

      if (doFill){
        hmesq_conserveDifermMass->Fill(mzz, mesq_conserveDifermMass);
        hvar->Fill(mzz, mzz);
      }

      if (writeFinalTree) newtree->Fill();
      mela.resetInputEvent();
    }

    double* xexyey[4];
    for (int ix=0; ix<4; ix++) xexyey[ix] = new double[nbins];
    for (int bin=0; bin<nbins; bin++){
      xexyey[0][bin] = hvar->GetBinContent(bin+1);
      xexyey[1][bin] = hvar->GetBinError(bin+1);

      cout << "Bin " << bin << " x-center: " << xexyey[0][bin] << " +- " << xexyey[1][bin] << endl;
      xexyey[2][bin] = hmesq_conserveDifermMass->GetBinContent(bin+1);
      xexyey[3][bin] = hmesq_conserveDifermMass->GetBinError(bin+1);
    }


    TGraphErrors* tg = new TGraphErrors(nbins, xexyey[0], xexyey[2], xexyey[1], xexyey[3]);
    tg->SetName(Form("tg_%s", hmesq_conserveDifermMass->GetName()));
    foutput->WriteTObject(tg);
    delete tg;

    for (int ix=0; ix<4; ix++) delete[] xexyey[ix];
    foutput->WriteTObject(hmesq_conserveDifermMass);
    foutput->WriteTObject(hvar);
    if (writeFinalTree) foutput->WriteTObject(newtree);
    if (writeFinalTree) delete newtree;
    delete hmesq_conserveDifermMass;
    delete hvar;
    delete[] binning;
    delete tmptree;
    delete tree;
  }
  foutput->Close();
}

/*
SPECIFIC COMMENT: OUTPUT ME DIVIDED BY
- (aL1**2+aR1**2)*(aL2**2+aR2**2) TO REMAIN INDEPENDENT OF CHANNEL
*/
void get_PAvgProfile_MCFM_ZZQQB_bkgZZ(){
  int erg_tev=8;
  float mPOLE=125.;
  TString TREE_NAME = "SelectedTree";
  const bool writeFinalTree=false;

  TVar::VerbosityLevel verbosity = TVar::ERROR;
  Mela mela(erg_tev, mPOLE, verbosity);

  float mesq_conserveDifermMass=0;
  float mzz = 126.;
  float m1 = 91.471450;
  float m2 = 12.139782;
  float h1 = 0.2682896;
  float h2 = 0.1679779;
  float phi = 1.5969792;
  float hs = -0.727181;
  float phi1 = 1.8828257;
  int LepID[4]={ 13, -13, 11, -11 };

  TString strchannel[3]={ "4mu", "4e", "2mu2e" };
  TString cinput_main = "/scratch0/hep/ianderso/CJLST/140519/PRODFSR_8TeV";
  TString cinput_main2 = "/scratch0/hep/ianderso/CJLST/140519/PRODFSR";
  const int nSamples = 6;
  TString strSamples[nSamples]={
    "HZZ4lTree_ZZTo4mu.root",
    "HZZ4lTree_ZZTo4e.root",
    "HZZ4lTree_ZZTo2e2mu.root",
    "HZZ4lTree_ZZTo4tau.root",
    "HZZ4lTree_ZZTo2mu2tau.root",
    "HZZ4lTree_ZZTo2e2tau.root"
  };

  TFile* foutput = new TFile("pAvg_MCFM_ZZQQB_bkgZZ.root", "recreate");

  for (int ic=0; ic<3; ic++){
    gROOT->cd();
    TChain* tree = new TChain(TREE_NAME, "");
    for (int is=0; is<nSamples; is++){
      tree->Add(Form("%s/%s/%s", cinput_main2.Data(), (strchannel[ic]).Data(), (strSamples[is]).Data()));
      tree->Add(Form("%s/%s/%s", cinput_main.Data(), (strchannel[ic]).Data(), (strSamples[is]).Data()));
    }
    tree->SetBranchAddress("ZZMass", &mzz);
    tree->SetBranchAddress("Z1Mass", &m1);
    tree->SetBranchAddress("Z2Mass", &m2);
    tree->SetBranchAddress("helcosthetaZ1", &h1);
    tree->SetBranchAddress("helcosthetaZ2", &h2);
    tree->SetBranchAddress("helphi", &phi);
    tree->SetBranchAddress("costhetastar", &hs);
    tree->SetBranchAddress("phistarZ1", &phi1);

    const int nTotalEntries = tree->GetEntries();
    cout << "Ntotalentries = " << nTotalEntries << endl;

    TTree* tmptree = new TTree("IntermediateTree", "");
    tmptree->Branch("ZZMass", &mzz);
    tmptree->Branch("Z1Mass", &m1);
    tmptree->Branch("Z2Mass", &m2);
    tmptree->Branch("helcosthetaZ1", &h1);
    tmptree->Branch("helcosthetaZ2", &h2);
    tmptree->Branch("helphi", &phi);
    tmptree->Branch("costhetastar", &hs);
    tmptree->Branch("phistarZ1", &phi1);

    TRandom3 randomthrow(1234567);
    double portion_to_keep = 1;
    if (nTotalEntries>1000000) portion_to_keep = 9.95e5/nTotalEntries;
    for (int ev = 0; ev < nTotalEntries; ev++){
      tree->GetEntry(ev);
      double rndnum = randomthrow.Uniform();
      if (rndnum<=portion_to_keep) tmptree->Fill();
    }

    const int nEntries = tmptree->GetEntries();
    if (nEntries>=1000000){
      cerr << "TMath::Sort will experience problems. Aborting!" << endl;
      delete tmptree;
      delete tree;
      continue;
    }
    int* index = new int[nEntries];
    tmptree->Draw("ZZMass", "", "goff");
    TMath::Sort(nEntries, tmptree->GetV1(), index, false);

    tmptree->GetEntry(index[0]);
    float firstVal=mzz;
    tmptree->GetEntry(index[nEntries-1]);
    float lastVal=mzz;
    float infimum = (float)((int)firstVal); infimum -= (float)(((int)infimum)%10);
    float supremum = (float)((int)(lastVal+0.5)); supremum += (float)(10-((int)supremum)%10);
    cout << "Nentries = " << nEntries << " | mzz = " << firstVal << " - " << lastVal << "(" << infimum << ", " << supremum << ")" << endl;

    int nbins=0;
    int divisor=11000;
    while (nbins<50){
      if (divisor>1000) divisor -= 1000;
      else if (divisor>100) divisor -= 100;
      else break;
      nbins=nEntries/divisor+1;
    }
    cout << "nbins=" << nbins << endl;
    if (nbins<3) cerr << "Not enough bins!" << endl;
    float* binning = new float[nbins+1];
    binning[0]=infimum;
    binning[nbins]=supremum;
    int ev_stepsize = nEntries/nbins;
    cout << "Event step size: " << ev_stepsize << endl;
    cout << "Boundary (" << 0 << ") = " << binning[0] << endl;
    for (int ix=1; ix<nbins; ix++){
      int ev = index[ix*ev_stepsize];
      tmptree->GetEntry(ev);
      float bhigh = mzz;
      ev = index[ix*ev_stepsize-1];
      float blow = mzz;
      binning[ix]=(bhigh+blow)*0.5;
      cout << "Boundary (" << ix << ")= " << binning[ix] << " [event " << ev << ", step " << ix*ev_stepsize << "]" << endl;
    }
    cout << "Boundary (" << nbins << ") = " << binning[nbins] << endl;
    delete[] index;

    foutput->cd();
    TProfile* hvar = new TProfile(Form("candMass_%s", strchannel[ic].Data()), "", nbins, binning); hvar->Sumw2();
    TProfile* hmesq_conserveDifermMass = new TProfile(Form("P_ConserveDifermionMass_%s", strchannel[ic].Data()), "", nbins, binning); hmesq_conserveDifermMass->Sumw2();

    TTree* newtree=0;
    if (writeFinalTree){
      newtree = new TTree("FinalTree", "");
      newtree->Branch("mesq_conserveDifermMass", &mesq_conserveDifermMass);
      newtree->Branch("ZZMass", &mzz);
    }

    mela.setCandidateDecayMode(TVar::CandidateDecay_ZZ);
    if (ic==1){
      LepID[0]=11;
      LepID[1]=-11;
    }
    else{
      LepID[0]=13;
      LepID[1]=-13;
    }
    if (ic==0){
      LepID[2]=13;
      LepID[3]=-13;
    }
    else{
      LepID[2]=11;
      LepID[3]=-11;
    }

    for (int ev = 0; ev < nEntries; ev++){
      tmptree->GetEntry(ev); // No need for ordering anymore
      if (ev%10000==0) cout << "Doing event " << ev << endl;

      TLorentzVector pDaughters[4];
      std::vector<TLorentzVector> daus = mela.calculate4Momentum(mzz, m1, m2, acos(hs), acos(h1), acos(h2), phi1, phi);
      for (int ip=0; ip<min(4, (int)daus.size()); ip++){ pDaughters[ip]=daus.at(ip); }
      SimpleParticleCollection_t daughters;
      for (unsigned int idau=0; idau<4; idau++) daughters.push_back(SimpleParticle_t(LepID[idau], pDaughters[idau]));
      mela.setInputEvent(&daughters, (SimpleParticleCollection_t*)0, (SimpleParticleCollection_t*)0, false);

      //double alphasVal;
      bool doFill=true;
      mela.setProcess(TVar::bkgZZ, TVar::MCFM, TVar::ZZQQB);

      TUtil::setLeptonMassScheme(TVar::ConserveDifermionMass);
      mela.computeP(mesq_conserveDifermMass, false);
      double aL1, aR1, aL2, aR2;
      mela.getIORecord()->getVDaughterCouplings(aL1, aR1, 0);
      mela.getIORecord()->getVDaughterCouplings(aL2, aR2, 1);
      if (fabs(aL1)>0. || fabs(aR1)>0.) mesq_conserveDifermMass /= pow(aL1, 2)+pow(aR1, 2);
      if (fabs(aL2)>0. || fabs(aR2)>0.) mesq_conserveDifermMass /= pow(aL2, 2)+pow(aR2, 2);
      mesq_conserveDifermMass = log10(mesq_conserveDifermMass);
      if (isnan(mesq_conserveDifermMass) || isinf(mesq_conserveDifermMass)) doFill=false;

      if (doFill){
        hmesq_conserveDifermMass->Fill(mzz, mesq_conserveDifermMass);
        hvar->Fill(mzz, mzz);
      }

      if (writeFinalTree) newtree->Fill();
      mela.resetInputEvent();
    }

    double* xexyey[4];
    for (int ix=0; ix<4; ix++) xexyey[ix] = new double[nbins];
    for (int bin=0; bin<nbins; bin++){
      xexyey[0][bin] = hvar->GetBinContent(bin+1);
      xexyey[1][bin] = hvar->GetBinError(bin+1);

      cout << "Bin " << bin << " x-center: " << xexyey[0][bin] << " +- " << xexyey[1][bin] << endl;
      xexyey[2][bin] = hmesq_conserveDifermMass->GetBinContent(bin+1);
      xexyey[3][bin] = hmesq_conserveDifermMass->GetBinError(bin+1);
    }


    TGraphErrors* tg = new TGraphErrors(nbins, xexyey[0], xexyey[2], xexyey[1], xexyey[3]);
    tg->SetName(Form("tg_%s", hmesq_conserveDifermMass->GetName()));
    foutput->WriteTObject(tg);
    delete tg;

    for (int ix=0; ix<4; ix++) delete[] xexyey[ix];
    foutput->WriteTObject(hmesq_conserveDifermMass);
    foutput->WriteTObject(hvar);
    if (writeFinalTree) foutput->WriteTObject(newtree);
    if (writeFinalTree) delete newtree;
    delete hmesq_conserveDifermMass;
    delete hvar;
    delete[] binning;
    delete tmptree;
    delete tree;
  }
  foutput->Close();
}


/* SPECIFIC COMMENT: THIS FUNCTION TAKES A TGRAPHERRORS AND MODIFIES IT DIRECTLY. THE OPTIONAL STD::VECTOR IS FOR FIXING <P> FOR CERTAIN X-VALUES. */
void regularizeSlice(TGraphErrors* tgSlice, std::vector<double>* fixedX=0, double omitbelow=0.){
  unsigned int nbins_slice = tgSlice->GetN();
  double* xexyey_slice[4]={
    tgSlice->GetX(),
    tgSlice->GetEX(),
    tgSlice->GetY(),
    tgSlice->GetEY()
  };

  double* xexyey_linear[4];
  for (unsigned int ix=0; ix<4; ix++){
    xexyey_linear[ix] = new double[nbins_slice];
    for (unsigned int iy=0; iy<nbins_slice; iy++){
      if (ix<2) xexyey_linear[ix][iy] = xexyey_slice[ix][iy];
      else if (ix==3) xexyey_linear[ix][iy] = exp(xexyey_slice[ix][iy]);
      else xexyey_linear[ix][iy] = xexyey_slice[ix][iy]*xexyey_linear[ix-1][iy];
    }
  }
  TGraphErrors* tgSlice_linear = new TGraphErrors(nbins_slice, xexyey_linear[0], xexyey_linear[2], xexyey_linear[1], xexyey_linear[3]);
  double integral_in=tgSlice_linear->Integral();
  delete tgSlice_linear;
  for (unsigned int ix=0; ix<4; ix++) delete[] xexyey_linear[ix];


  double* xexyey_mod[4];
  for (unsigned int ix=0; ix<4; ix++){
    xexyey_mod[ix] = new double[nbins_slice];
    for (unsigned int iy=0; iy<nbins_slice; iy++) xexyey_mod[ix][iy] = xexyey_slice[ix][iy];
  }
  unsigned int bin_first = 2, bin_last = nbins_slice-1;

  std::vector<int> fixedBins;
  if (fixedX!=0){
    for (unsigned int ifx=0; ifx<fixedX->size(); ifx++){
      double requestedVal = fixedX->at(ifx);
      double distance=1e15;
      int bin_to_fix=-1;
      for (unsigned int bin=0; bin<nbins_slice; bin++){ if (distance>fabs(xexyey_mod[0][bin]-requestedVal)){ bin_to_fix = bin; distance = fabs(xexyey_mod[0][bin]-requestedVal); } }
      if (bin_to_fix>=0) fixedBins.push_back(bin_to_fix);
      cout << "Requested to fix bin " << bin_to_fix << endl;
    }
  }
  if (omitbelow>0.){
    for (unsigned int bin=0; bin<nbins_slice; bin++){
      if (xexyey_mod[0][bin]<omitbelow) fixedBins.push_back(bin);
      cout << "Requested to fix bin " << bin << endl;
    }
  }

  double* xx_second;
  double* yy_second;

  unsigned int nIter = 1000;
  for (unsigned int it=0; it<nIter; it++){
    double threshold = 0.01;
    for (unsigned int binIt = bin_first; binIt<=bin_last; binIt++){
      bool doFix=false;
      for (unsigned int ifx=0; ifx<fixedBins.size(); ifx++){
        if ((int)(binIt-1)==fixedBins.at(ifx)){ doFix=true; /*cout << "Iteration " << it << " is fixing bin " << (binIt-1) << endl; */break; }
      }
      if (doFix) continue;

      int ctr = 0;
      int nbins_second = nbins_slice-1;
      xx_second = new double[nbins_second];
      yy_second = new double[nbins_second];
      for (unsigned int bin = 1; bin<=nbins_slice; bin++){
        if (bin==binIt) continue;
        xx_second[ctr] = xexyey_mod[0][bin-1];
        yy_second[ctr] = xexyey_mod[2][bin-1];
        ctr++;
      }

      TGraph* interpolator = new TGraph(nbins_second, xx_second, yy_second);
      double derivative_first = (yy_second[1]-yy_second[0])/(xx_second[1]-xx_second[0]);
      double derivative_last = (yy_second[nbins_second-1]-yy_second[nbins_second-2])/(xx_second[nbins_second-1]-xx_second[nbins_second-2]);
      TSpline3* spline = new TSpline3("spline", interpolator, "b1e1", derivative_first, derivative_last);

      double center = xexyey_mod[0][binIt-1];
      double val = spline->Eval(center);
      if (fabs(xexyey_mod[2][binIt-1]-val)>threshold*val) xexyey_mod[2][binIt-1]=val;

      delete spline;
      delete interpolator;

      delete[] yy_second;
      delete[] xx_second;
    }
  }

  for (unsigned int ix=0; ix<4; ix++){
    xexyey_linear[ix] = new double[nbins_slice];
    for (unsigned int iy=0; iy<nbins_slice; iy++){
      if (ix<2) xexyey_linear[ix][iy] = xexyey_mod[ix][iy];
      else if (ix==3) xexyey_linear[ix][iy] = exp(xexyey_mod[ix][iy]);
      else xexyey_linear[ix][iy] = xexyey_mod[ix][iy]*xexyey_linear[ix-1][iy];
    }
  }
  tgSlice_linear = new TGraphErrors(nbins_slice, xexyey_linear[0], xexyey_linear[2], xexyey_linear[1], xexyey_linear[3]);
  double integral_out=tgSlice_linear->Integral();
  delete tgSlice_linear;
  for (unsigned int ix=0; ix<4; ix++) delete[] xexyey_linear[ix];

  double scale = integral_out / integral_in;
  for (unsigned int iy=0; iy<nbins_slice; iy++){
    xexyey_slice[2][iy] = xexyey_mod[2][iy]*scale;
    xexyey_slice[3][iy] *= xexyey_mod[2][iy]/xexyey_slice[2][iy];
  }

  for (unsigned int ix=0; ix<4; ix++) delete[] xexyey_mod[ix];
}


/* SPECIFIC COMMENT: NONE */
void produce_PAvgSmooth_JHUGen_JJVBF_HSMHiggs_7or8TeV(int sqrts=8){
  TFile* finput = new TFile(Form("pAvg_JHUGen_JJVBF_HSMHiggs_%iTeV.root", sqrts), "read");
  TFile* foutput = new TFile(Form("pAvgSmooth_JHUGen_JJVBF_HSMHiggs_%iTeV.root", sqrts), "recreate");
  const unsigned int ngraphs=2;
  TString strtg[ngraphs]={
    "tg_P_ConserveDifermionMass",
    "tg_P_MomentumToEnergy"
  };

  for (unsigned int ig=0; ig<ngraphs; ig++){
    finput->cd();
    TGraphErrors* tg = 0;
    tg = (TGraphErrors*)finput->Get(strtg[ig]);
    if (tg==0){ cerr << strtg[ig] << " does not exist." << endl; continue; }
    foutput->cd();
    foutput->WriteTObject(tg);
    tg->SetName(Form("%s_Smooth", tg->GetName()));
    regularizeSlice(tg);
    foutput->WriteTObject(tg);
  }
  foutput->Close();
  finput->Close();
}

/* SPECIFIC COMMENT: NONE */
void produce_PAvgSmooth_JHUGen_JJQCD_HSMHiggs_7or8TeV(int sqrts=8){
  TFile* finput = new TFile(Form("pAvg_JHUGen_JJQCD_HSMHiggs_%iTeV.root", sqrts), "read");
  TFile* foutput = new TFile(Form("pAvgSmooth_JHUGen_JJQCD_HSMHiggs_%iTeV.root", sqrts), "recreate");
  const unsigned int ngraphs=2;
  TString strtg[ngraphs]={
    "tg_P_ConserveDifermionMass",
    "tg_P_MomentumToEnergy"
  };

  for (unsigned int ig=0; ig<ngraphs; ig++){
    finput->cd();
    TGraphErrors* tg = 0;
    tg = (TGraphErrors*)finput->Get(strtg[ig]);
    if (tg==0){ cerr << strtg[ig] << " does not exist." << endl; continue; }
    foutput->cd();
    foutput->WriteTObject(tg);
    tg->SetName(Form("%s_Smooth", tg->GetName()));
    regularizeSlice(tg);
    foutput->WriteTObject(tg);
  }
  foutput->Close();
  finput->Close();
}

/* SPECIFIC COMMENT: NONE */
void produce_PAvgSmooth_JHUGen_JQCD_HSMHiggs_7or8TeV(int sqrts=8){
  TFile* finput = new TFile(Form("pAvg_JHUGen_JQCD_HSMHiggs_%iTeV.root", sqrts), "read");
  TFile* foutput = new TFile(Form("pAvgSmooth_JHUGen_JQCD_HSMHiggs_%iTeV.root", sqrts), "recreate");
  const unsigned int ngraphs=2;
  TString strtg[ngraphs]={
    "tg_P_ConserveDifermionMass",
    "tg_P_MomentumToEnergy"
  };

  for (unsigned int ig=0; ig<ngraphs; ig++){
    finput->cd();
    TGraphErrors* tg = 0;
    tg = (TGraphErrors*)finput->Get(strtg[ig]);
    if (tg==0){ cerr << strtg[ig] << " does not exist." << endl; continue; }
    foutput->cd();
    foutput->WriteTObject(tg);
    tg->SetName(Form("%s_Smooth", tg->GetName()));
    regularizeSlice(tg);
    foutput->WriteTObject(tg);
  }
  foutput->Close();
  finput->Close();
}

/* SPECIFIC COMMENT: SOME BINS ARE FIXED TO GET A MORE REPRESENTATIVE SMOOTHING */
void produce_get_PAvgSmooth_JHUGen_ZZGG_HSMHiggs(){
  TFile* finput = new TFile("pAvg_JHUGen_ZZGG_HSMHiggs.root", "read");
  TFile* foutput = new TFile("pAvgSmooth_JHUGen_ZZGG_HSMHiggs.root", "recreate");
  const unsigned int ngraphs=3;
  TString strtg[ngraphs]={
    "tg_P_ConserveDifermionMass_4mu",
    "tg_P_ConserveDifermionMass_4e",
    "tg_P_ConserveDifermionMass_2mu2e"
  };

  for (unsigned int ig=0; ig<ngraphs; ig++){
    finput->cd();
    TGraphErrors* tg = 0;
    tg = (TGraphErrors*)finput->Get(strtg[ig]);
    if (tg==0){ cerr << strtg[ig] << " does not exist." << endl; continue; }
    foutput->cd();
    foutput->WriteTObject(tg);
    tg->SetName(Form("%s_Smooth", tg->GetName()));
    std::vector<double> fixedX;
    if (ig==0){
      fixedX.push_back(106.);
      fixedX.push_back(123.);
    }
    else if (ig==1){
      fixedX.push_back(108.);
      fixedX.push_back(128.);
    }
    else if (ig==2){
      fixedX.push_back(106.);
      fixedX.push_back(128.);
    }
    regularizeSlice(tg, &fixedX);
    foutput->WriteTObject(tg);
  }
  foutput->Close();
  finput->Close();
}

/* SPECIFIC COMMENT: SOME BINS ARE FIXED TO GET A MORE REPRESENTATIVE SMOOTHING */
void produce_get_PAvgSmooth_MCFM_ZZGG_HSMHiggs(){
  TFile* finput = new TFile("pAvg_MCFM_ZZGG_HSMHiggs.root", "read");
  TFile* foutput = new TFile("pAvgSmooth_MCFM_ZZGG_HSMHiggs.root", "recreate");
  const unsigned int ngraphs=3;
  TString strtg[ngraphs]={
    "tg_P_ConserveDifermionMass_4mu",
    "tg_P_ConserveDifermionMass_4e",
    "tg_P_ConserveDifermionMass_2mu2e"
  };

  for (unsigned int ig=0; ig<ngraphs; ig++){
    finput->cd();
    TGraphErrors* tg = 0;
    tg = (TGraphErrors*)finput->Get(strtg[ig]);
    if (tg==0){ cerr << strtg[ig] << " does not exist." << endl; continue; }
    foutput->cd();
    foutput->WriteTObject(tg);
    tg->SetName(Form("%s_Smooth", tg->GetName()));
    std::vector<double> fixedX;
    if (ig==0){
      fixedX.push_back(106.);
      fixedX.push_back(123.);
      fixedX.push_back(170.);
      fixedX.push_back(187.);
      fixedX.push_back(198.);
      fixedX.push_back(250.);
      fixedX.push_back(305.);
    }
    else if (ig==1){
      fixedX.push_back(108.);
      fixedX.push_back(128.);
      fixedX.push_back(165.);
      fixedX.push_back(185.);
      fixedX.push_back(195.);
      fixedX.push_back(250.);
      fixedX.push_back(300.);
    }
    else if (ig==2){
      fixedX.push_back(106.);
      fixedX.push_back(128.);
      fixedX.push_back(170.);
      fixedX.push_back(183.);
      //fixedX.push_back(188.);
      fixedX.push_back(198.);
      //fixedX.push_back(252.);
    }
    regularizeSlice(tg, &fixedX);
    foutput->WriteTObject(tg);
  }
  foutput->Close();
  finput->Close();
}

/* SPECIFIC COMMENT: SOME BINS ARE FIXED TO GET A MORE REPRESENTATIVE SMOOTHING */
void produce_get_PAvgSmooth_MCFM_ZZGG_bkgZZ(){
  TFile* finput = new TFile("pAvg_MCFM_ZZGG_bkgZZ.root", "read");
  TFile* foutput = new TFile("pAvgSmooth_MCFM_ZZGG_bkgZZ.root", "recreate");
  const unsigned int ngraphs=3;
  TString strtg[ngraphs]={
    "tg_P_ConserveDifermionMass_4mu",
    "tg_P_ConserveDifermionMass_4e",
    "tg_P_ConserveDifermionMass_2mu2e"
  };

  for (unsigned int ig=0; ig<ngraphs; ig++){
    finput->cd();
    TGraphErrors* tg = 0;
    tg = (TGraphErrors*)finput->Get(strtg[ig]);
    if (tg==0){ cerr << strtg[ig] << " does not exist." << endl; continue; }
    foutput->cd();
    foutput->WriteTObject(tg);
    tg->SetName(Form("%s_Smooth", tg->GetName()));
    std::vector<double> fixedX;
    if (ig==0){
      fixedX.push_back(130.);
      fixedX.push_back(140.);
      fixedX.push_back(156.);
      fixedX.push_back(170.);
      fixedX.push_back(175.);
      fixedX.push_back(183.);
      fixedX.push_back(186.);
      fixedX.push_back(191.);
      fixedX.push_back(195.);
      fixedX.push_back(197.);
      fixedX.push_back(204.);
    }
    else if (ig==1){
      fixedX.push_back(140.);
      fixedX.push_back(160.);
      fixedX.push_back(175.);
      fixedX.push_back(183.);
      fixedX.push_back(186.);
      fixedX.push_back(193.);
      fixedX.push_back(199.);
      fixedX.push_back(206.);
      fixedX.push_back(217.);
    }
    else if (ig==2){
      fixedX.push_back(150.);
      fixedX.push_back(160.);
      fixedX.push_back(170.);
      fixedX.push_back(175.);
      fixedX.push_back(183.);
      fixedX.push_back(185.);
      fixedX.push_back(187.5);
      fixedX.push_back(193.);
      fixedX.push_back(199.);
    }
    regularizeSlice(tg, &fixedX);
    foutput->WriteTObject(tg);
  }
  foutput->Close();
  finput->Close();
}

/* SPECIFIC COMMENT: SOME BINS ARE FIXED TO GET A MORE REPRESENTATIVE SMOOTHING */
void produce_get_PAvgSmooth_MCFM_ZZQQB_bkgZZ(){
  TFile* finput = new TFile("pAvg_MCFM_ZZQQB_bkgZZ.root", "read");
  TFile* foutput = new TFile("pAvgSmooth_MCFM_ZZQQB_bkgZZ.root", "recreate");
  const unsigned int ngraphs=3;
  TString strtg[ngraphs]={
    "tg_P_ConserveDifermionMass_4mu",
    "tg_P_ConserveDifermionMass_4e",
    "tg_P_ConserveDifermionMass_2mu2e"
  };

  for (unsigned int ig=0; ig<ngraphs; ig++){
    finput->cd();
    TGraphErrors* tg = 0;
    tg = (TGraphErrors*)finput->Get(strtg[ig]);
    if (tg==0){ cerr << strtg[ig] << " does not exist." << endl; continue; }
    foutput->cd();
    foutput->WriteTObject(tg);
    tg->SetName(Form("%s_Smooth", tg->GetName()));
    std::vector<double> fixedX;

    if (ig==0){
      fixedX.push_back(196.);
      fixedX.push_back(209.);
      fixedX.push_back(230.);
      fixedX.push_back(300.);
      fixedX.push_back(360.);
      fixedX.push_back(450.);
    }
    else if (ig==1){
      fixedX.push_back(195.);
      fixedX.push_back(204.);
      fixedX.push_back(213.);
    }
    else if (ig==2){
      fixedX.push_back(201.);
      fixedX.push_back(212.);
      fixedX.push_back(230.);
      fixedX.push_back(480.);
    }

    double omitbelow;
    if (ig==0){
      omitbelow=192.;
    }
    else if (ig==1){
      omitbelow=189.;
    }
    else if (ig==2){
      omitbelow=192.;
    }
    regularizeSlice(tg, &fixedX, omitbelow);
    foutput->WriteTObject(tg);
  }
  foutput->Close();
  finput->Close();
}



