/************** MELA interface to MCFM/JHUGen-MELA *************

Notes:
1) Each specific type of computeP* function comes with its wrapper for common use.
   Removing these wrappers from Mela will not introduce any bugs, but
   it might affect packages that depend on it (eg. MEMCalculators).
2) ...

Please adhere to the following coding conventions:
1) Never put return statements in the middle of the computeP* functions unless it is a wrapper.
   Functions calling the Mela::ZZME member have to reset the couplings, so an abrupt termination
   does not reset the couplings properly.
2) ...

***************************************************************/

#include <vector>
#include <string>
#include <cstdio>
#include <cmath>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "Mela.h"
#include "newZZMatrixElement.h"
#include "VectorPdfFactory.h"
#include "TensorPdfFactory.h"
#include "RooqqZZ_JHU_ZgammaZZ_fast.h"
#include "RooqqZZ_JHU.h"
#include "SuperMELA.h"

#include "RooMsgService.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TGraph.h"
#include "TSpline.h"
#include "TString.h"


using namespace std;
using namespace RooFit;

Mela::Mela(
  double LHCsqrts_,
  double mh_,
  TVar::VerbosityLevel verbosity_
  ) :
  LHCsqrts(LHCsqrts_),
  myVerbosity_(verbosity_),
  ZZME(0),
  auxiliaryProb(0.),
  melaCand(0)
{
  if (myVerbosity_>=TVar::DEBUG) cout << "Start Mela constructor" << endl;
  //setRemoveLeptonMasses(false); // Use Run 1 scheme for not removing fermion masses
  setRemoveLeptonMasses(true); // Use Run 2 scheme for removing fermion masses to compute MEs that expect massless fermions properly

  const double maxSqrts = 8.;

  // Create symlinks to the required files, if these are not already present (do nothing otherwise)
  if (myVerbosity_>=TVar::DEBUG) cout << "Create symlinks to the required files if these are not already present:" << endl;

#ifdef _melapkgpathstr_
  const string MELAPKGPATH = _melapkgpathstr_;
  if (myVerbosity_>=TVar::DEBUG)  cout << "\t- MELA package path: " << MELAPKGPATH << endl;
#else
  cout << "MELA package path is undefined! Please modify the makefle or the makefile-equivalent!" << endl;
  assert(0);
#endif

  const string mcfmWarning = MELAPKGPATH + "data/ffwarn.dat"; symlink(mcfmWarning.c_str(), "ffwarn.dat");
  const string mcfm_brsm_o = MELAPKGPATH + "data/br.sm1"; symlink(mcfm_brsm_o.c_str(), "br.sm1");
  const string mcfm_brsm_t = MELAPKGPATH + "data/br.sm2"; symlink(mcfm_brsm_t.c_str(), "br.sm2");
  const string mcfmInput1 = MELAPKGPATH + "data/input.DAT"; symlink(mcfmInput1.c_str(), "input.DAT");
  const string mcfmInput2 = MELAPKGPATH + "data/process.DAT"; symlink(mcfmInput2.c_str(), "process.DAT");
  if (myVerbosity_>=TVar::DEBUG) cout << "\t- MCFM symlinks are done" << endl;
  mkdir("Pdfdata", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  const string mcfmInput3 = MELAPKGPATH + "data/Pdfdata/cteq6l1.tbl"; symlink(mcfmInput3.c_str(), "Pdfdata/cteq6l1.tbl");
  const string mcfmInput4 = MELAPKGPATH + "data/Pdfdata/cteq6l.tbl"; symlink(mcfmInput4.c_str(), "Pdfdata/cteq6l.tbl");
  if (myVerbosity_>=TVar::DEBUG) cout << "\t- PDF symlinks are done" << endl;

  if (myVerbosity_>=TVar::DEBUG) cout << "Create variables used in anaMELA" << endl;
  mzz_rrv = new RooRealVar("mzz", "m_{ZZ}", mh_, 0., 1000.);
  z1mass_rrv = new RooRealVar("z1mass", "m_{Z1}", 0., 160.);
  z2mass_rrv = new RooRealVar("z2mass", "m_{Z2}", 0., 200.);
  costhetastar_rrv = new RooRealVar("costhetastar", "cos#theta^{*}", -1., 1.);
  costheta1_rrv = new RooRealVar("costheta1", "cos#theta_{1}", -1., 1.);
  costheta2_rrv = new RooRealVar("costheta2", "cos#theta_{2}", -1., 1.);
  phi_rrv= new RooRealVar("phi", "#Phi", -TMath::Pi(), TMath::Pi());
  phi1_rrv= new RooRealVar("phi1", "#Phi_{1}", -TMath::Pi(), TMath::Pi());
  Y_rrv = new RooRealVar("Yzz", "#Y_{ZZ}", 0, -4, 4);
  upFrac_rrv = new RooRealVar("upFrac", "fraction up-quarks", .5, 0., 1.);

  RooSpinZero::modelMeasurables measurables_;
  measurables_.h1 = costheta1_rrv;
  measurables_.h2 = costheta2_rrv;
  measurables_.Phi = phi_rrv;
  measurables_.m1 = z1mass_rrv;
  measurables_.m2 = z2mass_rrv;
  measurables_.m12 = mzz_rrv;
  measurables_.hs = costhetastar_rrv;
  measurables_.Phi1 = phi1_rrv;
  measurables_.Y = Y_rrv;

  if (myVerbosity_>=TVar::DEBUG) cout << "Create anaMELA PDF factories" << endl;
  ggSpin0Model = new ScalarPdfFactory_ggH(measurables_, false, RooSpin::kVdecayType_Zll, RooSpin::kVdecayType_Zll); // RooSpin::kVdecayType_Zll,RooSpin::kVdecayType_Zll==ZZ
  spin1Model = new VectorPdfFactory(z1mass_rrv, z2mass_rrv, costhetastar_rrv, costheta1_rrv, costheta2_rrv, phi_rrv, phi1_rrv, mzz_rrv);
  spin2Model = new TensorPdfFactory_HVV(measurables_, RooSpin::kVdecayType_Zll, RooSpin::kVdecayType_Zll);
  qqZZmodel = new RooqqZZ_JHU_ZgammaZZ_fast("qqZZmodel", "qqZZmodel", *z1mass_rrv, *z2mass_rrv, *costheta1_rrv, *costheta2_rrv, *phi_rrv, *costhetastar_rrv, *phi1_rrv, *mzz_rrv, *upFrac_rrv);

  if (myVerbosity_>=TVar::DEBUG) cout << "Paths for newZZMatrixElement" << endl;
  const string path_HiggsWidthFile = MELAPKGPATH + "data/HiggsTotalWidth_YR3.txt";
  if (myVerbosity_>=TVar::DEBUG) cout << "\t- Cross section/width file: " << path_HiggsWidthFile << endl;
  const string path_nnpdf = MELAPKGPATH + "data/Pdfdata/NNPDF30_lo_as_0130.LHgrid";
  char path_nnpdf_c[] = "Pdfdata/NNPDF30_lo_as_0130.LHgrid";
  int pdfmember = 0;
  if (myVerbosity_>=TVar::DEBUG) cout << "\t- Linking NNPDF path " << path_nnpdf.c_str() << " -> " << path_nnpdf_c << endl;
  symlink(path_nnpdf.c_str(), path_nnpdf_c);
  if (myVerbosity_>=TVar::DEBUG) cout << "Start newZZMatrixElement" << endl;
  ZZME = new newZZMatrixElement(path_nnpdf_c, pdfmember, path_HiggsWidthFile.substr(0, path_HiggsWidthFile.length()-23).c_str(), 1000.*LHCsqrts/2., myVerbosity_);
  if (myVerbosity_>=TVar::DEBUG) cout << "Set newZZMatrixElement masses" << endl;
  setMelaPrimaryHiggsMass(mh_);
  setMelaHiggsMass(mh_, 0); setMelaHiggsMass(-1., 1);
  setMelaHiggsWidth(-1., 0); setMelaHiggsWidth(0., 1);
  setMelaLeptonInterference(TVar::DefaultLeptonInterf);
  setCandidateDecayMode(TVar::CandidateDecay_ZZ); // Default decay mode is ZZ at the start


  /***** CONSTANTS FOR MATRIX ELEMENTS *****/
  getPConstantHandles();

  /***** SuperMELA *****/
  // Deactivate generation messages
  RooMsgService::instance().getStream(1).removeTopic(NumIntegration);
  RooMsgService::instance().setStreamStatus(1, kFALSE);
  RooMsgService::instance().setStreamStatus(0, kFALSE);// silence also the error messages, but should really be looked at.

  myRandomNumber=new TRandom3(35797);
  if (myVerbosity_>=TVar::DEBUG) cout << "Start superMELA" << endl;
  int superMELA_LHCsqrts = LHCsqrts;
  if (superMELA_LHCsqrts > maxSqrts) superMELA_LHCsqrts = maxSqrts;
  super = new SuperMELA(mh_, "4mu", superMELA_LHCsqrts); // preliminary intialization, we adjust the flavor later
  char cardpath[500];
  sprintf(cardpath, "data/CombinationInputs/SM_inputs_%dTeV/inputs_4mu.txt", superMELA_LHCsqrts);
  string cardfile = MELAPKGPATH + cardpath;
  super->SetPathToCards(cardfile.substr(0, cardfile.length()-14).c_str());
  super->SetVerbosity((myVerbosity_>=TVar::DEBUG));
  super->init();

  reset_SelfDCouplings();
  if (myVerbosity_>=TVar::DEBUG) cout << "End Mela constructor" << endl;
}

Mela::~Mela(){
  if (myVerbosity_>=TVar::DEBUG) cout << "Begin Mela destructor" << endl;

  //setRemoveLeptonMasses(false); // Use Run 1 scheme for not removing lepton masses. Notice the switch itself is defined as an extern, so it has to be set to default value at the destructor!
  setRemoveLeptonMasses(true); // Use Run 2 scheme for removing lepton masses. Notice the switch itself is defined as an extern, so it has to be set to default value at the destructor!

  // Delete the derived RooFit objects first...
  delete ggSpin0Model;
  delete spin1Model;
  delete spin2Model;
  delete qqZZmodel;
  // ...then delete the observables.
  delete mzz_rrv;
  delete z1mass_rrv; 
  delete z2mass_rrv; 
  delete costhetastar_rrv;
  delete costheta1_rrv;
  delete costheta2_rrv;
  delete phi_rrv;
  delete phi1_rrv;
  delete Y_rrv;
  delete upFrac_rrv;

  delete ZZME;
  delete super;
  delete myRandomNumber;

  // Delete ME constant handles
  deletePConstantHandles();

  if (myVerbosity_>=TVar::DEBUG) cout << "End Mela destructor" << endl;
}

// Set-functions
void Mela::setProcess(TVar::Process myModel, TVar::MatrixElement myME, TVar::Production myProduction){
  myME_ = myME;
  myProduction_ = myProduction;
  // In case s-channel processes are passed for JHUGen ME, flip them back to JHUGen-specific productions.
  if (myME_==TVar::JHUGen){
    if (myProduction_==TVar::Had_ZH_S) myProduction_=TVar::Had_ZH;
    else if (myProduction_==TVar::Had_WH_S) myProduction_=TVar::Had_WH;
    else if (myProduction_==TVar::Lep_ZH_S) myProduction_=TVar::Lep_ZH;
    else if (myProduction_==TVar::Lep_WH_S) myProduction_=TVar::Lep_WH;
    else if (myProduction_==TVar::JJVBF_S) myProduction_=TVar::JJVBF;
    else if (myProduction_==TVar::JJQCD_S) myProduction_=TVar::JJQCD;
  }
  myModel_ = myModel;
  if (ZZME!=0) ZZME->set_Process(myModel_, myME_, myProduction_);
}
void Mela::setVerbosity(TVar::VerbosityLevel verbosity_){ myVerbosity_=verbosity_; if (ZZME!=0) ZZME->set_Verbosity(myVerbosity_); if (super!=0) super->SetVerbosity((myVerbosity_>=TVar::DEBUG)); }
// Should be called per-event
void Mela::setMelaPrimaryHiggsMass(double myHiggsMass){ ZZME->set_PrimaryHiggsMass(myHiggsMass); }
void Mela::setMelaHiggsMass(double myHiggsMass, int index){ ZZME->set_mHiggs(myHiggsMass, index); }
void Mela::setMelaHiggsWidth(double myHiggsWidth, int index){ ZZME->set_wHiggs(myHiggsWidth, index); }
void Mela::setMelaHiggsMassWidth(double myHiggsMass, double myHiggsWidth, int index){ ZZME->set_mHiggs_wHiggs(myHiggsMass, myHiggsWidth, index); }
void Mela::setMelaLeptonInterference(TVar::LeptonInterference myLepInterf){ myLepInterf_=myLepInterf; ZZME->set_LeptonInterference(myLepInterf); }
void Mela::setCandidateDecayMode(TVar::CandidateDecayMode mode){ ZZME->set_CandidateDecayMode(mode); }
void Mela::setCurrentCandidateFromIndex(unsigned int icand){ ZZME->set_CurrentCandidateFromIndex(icand); }
void Mela::setCurrentCandidate(MELACandidate* cand){ ZZME->set_CurrentCandidate(cand); }
void Mela::setInputEvent(
  SimpleParticleCollection_t* pDaughters,
  SimpleParticleCollection_t* pAssociated,
  SimpleParticleCollection_t* pMothers,
  bool isGen
  ){
  ZZME->set_InputEvent(
    pDaughters,
    pAssociated,
    pMothers,
    isGen
    );
}
void Mela::resetInputEvent(){ ZZME->reset_InputEvent(); }
void Mela::setTempCandidate(
  SimpleParticleCollection_t* pDaughters,
  SimpleParticleCollection_t* pAssociated,
  SimpleParticleCollection_t* pMothers,
  bool isGen
  ){ ZZME->set_TempCandidate(pDaughters, pAssociated, pMothers); }
void Mela::appendTopCandidate(SimpleParticleCollection_t* TopDaughters){ ZZME->append_TopCandidate(TopDaughters); }


// Notice that this only sets the members of MELA, not TEvtProb. TEvtProb resets itself.
void Mela::reset_SelfDCouplings(){
  // We have a lot of them, now even more!

  //****Spin-0****//
  differentiate_HWW_HZZ=false;
  // Loop over the number of supported resonances
  for (int jh=0; jh<(int)nSupportedHiggses; jh++){
    for (int im=0; im<2; im++){
      for (int ic=0; ic<SIZE_HGG; ic++) selfDHggcoupl[jh][ic][im]=0;
      for (int ic=0; ic<SIZE_HGG; ic++) selfDHg4g4coupl[jh][ic][im]=0;

      for (int ic=0; ic<SIZE_HQQ; ic++) selfDHqqcoupl[jh][ic][im]=0;
      for (int ic=0; ic<SIZE_HQQ; ic++) selfDHbbcoupl[jh][ic][im]=0;
      for (int ic=0; ic<SIZE_HQQ; ic++) selfDHttcoupl[jh][ic][im]=0;
      for (int ic=0; ic<SIZE_HQQ; ic++) selfDHb4b4coupl[jh][ic][im]=0;
      for (int ic=0; ic<SIZE_HQQ; ic++) selfDHt4t4coupl[jh][ic][im]=0;

      for (int ic=0; ic<SIZE_HVV; ic++){
        selfDHzzcoupl[jh][ic][im] = 0;
        selfDHwwcoupl[jh][ic][im] = 0;
      }
    }
    for (int ik=0; ik<SIZE_HVV_CQSQ; ik++){
      selfDHzzCLambda_qsq[jh][ik]=0;
      selfDHwwCLambda_qsq[jh][ik]=0;
      for (int ic=0; ic<SIZE_HVV_LAMBDAQSQ; ic++){ // These default values do not matter as long as the c's are 0.
        selfDHzzLambda_qsq[jh][ic][ik] = 100.;
        selfDHwwLambda_qsq[jh][ic][ik] = 100.;
      }
    }
  }

  //****Spin-1****//
  for (int im=0; im<2; im++){
    for (int ic=0; ic<SIZE_ZVV; ic++) selfDZvvcoupl[ic][im] = 0;
    for (int ic=0; ic<SIZE_ZQQ; ic++) selfDZqqcoupl[ic][im] = 0;
  }

  //****Spin-2****//
  for (int im=0; im<2; im++){
    for (int ic=0; ic<SIZE_GVV; ic++) selfDGvvcoupl[ic][im] = 0;
    for (int ic=0; ic<SIZE_GGG; ic++) selfDGggcoupl[ic][im] = 0;
    for (int ic=0; ic<SIZE_GQQ; ic++) selfDGqqcoupl[ic][im] = 0;
  }

  // Did I tell you that we have a lot of them?
}
void Mela::resetMass(double inmass, int ipart){ ZZME->reset_Mass(inmass, ipart); }
void Mela::resetWidth(double inwidth, int ipart){ ZZME->reset_Width(inwidth, ipart); }
void Mela::resetQuarkMasses(){ ZZME->reset_QuarkMasses(); }
void Mela::resetMCFM_EWKParameters(double ext_Gf, double ext_aemmz, double ext_mW, double ext_mZ, double ext_xW, int ext_ewscheme){
  ZZME->reset_MCFM_EWKParameters(ext_Gf, ext_aemmz, ext_mW, ext_mZ, ext_xW, ext_ewscheme);
}
void Mela::setRemoveLeptonMasses(bool MasslessLeptonSwitch){ TUtil::applyLeptonMassCorrection(MasslessLeptonSwitch); }
void Mela::setRemoveJetMasses(bool MasslessLeptonSwitch){ TUtil::applyJetMassCorrection(MasslessLeptonSwitch); }
void Mela::setRenFacScaleMode(TVar::EventScaleScheme renormalizationSch, TVar::EventScaleScheme factorizationSch, double ren_sf, double fac_sf){
  ZZME->set_RenFacScaleMode(renormalizationSch, factorizationSch, ren_sf, fac_sf);
}
std::vector<TLorentzVector> Mela::calculate4Momentum(double Mx, double M1, double M2, double theta, double theta1, double theta2, double Phi1, double Phi){
	return ZZME->Calculate4Momentum(Mx, M1, M2, theta, theta1, theta2, Phi1, Phi);
}



// Full parton-by-parton ME record
MelaIO* Mela::getIORecord(){ return ZZME->get_IORecord(); }
// Candidate functions
MELACandidate* Mela::getCurrentCandidate(){ return ZZME->get_CurrentCandidate(); }
int Mela::getCurrentCandidateIndex(){ return ZZME->get_CurrentCandidateIndex(); }
int Mela::getNCandidates(){ return ZZME->get_NCandidates(); }
std::vector<MELATopCandidate*>* Mela::getTopCandidateCollection(){ return ZZME->get_TopCandidateCollection(); }
void Mela::reset_CandRef(){ melaCand=0; }


// SuperProb
void Mela::getPAux(float& prob){ prob = auxiliaryProb; }
void Mela::reset_PAux(){ auxiliaryProb=1.; } // SuperProb reset

// Angle computation script of Mela to convert MELACandidates to m1, m2 etc.
void Mela::computeDecayAngles(
  float& qH,
  float& m1,
  float& m2,
  float& costheta1,
  float& costheta2,
  float& Phi,
  float& costhetastar,
  float& Phi1
  ){
  qH=0; m1=0; m2=0; costheta1=0; costheta2=0; Phi=0; costhetastar=0; Phi1=0;

  if (melaCand==0) melaCand = getCurrentCandidate();
  if (melaCand!=0){
    TLorentzVector nullVector(0, 0, 0, 0);

    qH = melaCand->m();
    m1 = melaCand->getSortedV(0)->m();
    m2 = melaCand->getSortedV(1)->m();

    if (melaCand->getSortedV(0)->getNDaughters()>=1 && melaCand->getSortedV(1)->getNDaughters()>=1){
      MELAParticle* dau[2][2]={ { 0 } };
      for (int vv=0; vv<2; vv++){
        MELAParticle* Vi = melaCand->getSortedV(vv);
        for (int dd=0; dd<Vi->getNDaughters(); dd++) dau[vv][dd] = Vi->getDaughter(dd);
      }
      TUtil::computeAngles(
        (dau[0][0]!=0 ? dau[0][0]->p4 : nullVector), (dau[0][0]!=0 ? dau[0][0]->id : -9000),
        (dau[0][1]!=0 ? dau[0][1]->p4 : nullVector), (dau[0][1]!=0 ? dau[0][1]->id : -9000),
        (dau[1][0]!=0 ? dau[1][0]->p4 : nullVector), (dau[1][0]!=0 ? dau[1][0]->id : -9000),
        (dau[1][1]!=0 ? dau[1][1]->p4 : nullVector), (dau[1][1]!=0 ? dau[1][1]->id : -9000),
        costhetastar, costheta1, costheta2, Phi, Phi1
        );
    }
    // Protect against NaN
    if (!(costhetastar==costhetastar)) costhetastar=0;
    if (!(costheta1==costheta1)) costheta1=0;
    if (!(costheta2==costheta2)) costheta2=0;
    if (!(Phi==Phi)) Phi=0;
    if (!(Phi1==Phi1)) Phi1=0;
  }
  else if (myVerbosity_>=TVar::DEBUG) cerr << "Mela::computeDecayAngles: No possible melaCand in TEvtProb to compute angles." << endl;
}

// Regular probabilities
void Mela::computeP_selfDspin0(
  double selfDHvvcoupl_input[nSupportedHiggses][SIZE_HVV][2],
  float& prob,
  bool useConstant
  ){
  // Don't set these, and you will get 0.
  if (myME_==TVar::JHUGen){
    for (int jh=0; jh<(int)nSupportedHiggses; jh++){
      // JHUGen ME production side is Hqq=1, Hgg=1
      selfDHqqcoupl[jh][0][0] = 1.0;
      selfDHggcoupl[jh][0][0] = 1.0;
    }
  }
  else if (myME_==TVar::MCFM){
    for (int jh=0; jh<(int)nSupportedHiggses; jh++){
      // MCFM ME production side is Htt=Hbb=1
      selfDHttcoupl[jh][0][0] = 1.0;
      selfDHbbcoupl[jh][0][0] = 1.0;
    }
  }
  for (int jh=0; jh<(int)nSupportedHiggses; jh++){
    for (int im=0; im<2; im++){
      for (int ic=0; ic<SIZE_HVV; ic++){
        selfDHzzcoupl[jh][ic][im] = selfDHvvcoupl_input[jh][ic][im];
        selfDHwwcoupl[jh][ic][im] = selfDHvvcoupl_input[jh][ic][im]; // Just for extra protection since differentiate_HWW_HZZ is set to false.
      }
    }
  }

  computeP(
    prob,
    useConstant
    );
}
void Mela::computeP_selfDspin1(
  double selfDZqqcoupl_input[SIZE_ZQQ][2],
  double selfDZvvcoupl_input[SIZE_ZVV][2],
  float& prob,
  bool useConstant
  ){
  for (int im=0; im<2; im++){
    for (int ic=0; ic<SIZE_ZQQ; ic++) selfDZqqcoupl[ic][im] = selfDZqqcoupl_input[ic][im];
    for (int ic=0; ic<SIZE_ZVV; ic++) selfDZvvcoupl[ic][im] = selfDZvvcoupl_input[ic][im];
  }

  computeP(
    prob,
    useConstant
    );
}
void Mela::computeP_selfDspin1(
  double selfDZvvcoupl_input[SIZE_ZVV][2],
  float& prob,
  bool useConstant
  ){
  // Initialize the quark_left_right couplings
  selfDZqqcoupl[0][0]=1.0;
  selfDZqqcoupl[1][0]=1.0;

  for (int im=0; im<2; im++){
    for (int ic=0; ic<SIZE_ZVV; ic++) selfDZvvcoupl[ic][im] = selfDZvvcoupl_input[ic][im];
  }

  computeP(
    prob,
    useConstant
    );
}
void Mela::computeP_selfDspin2(
  double selfDGggcoupl_input[SIZE_GGG][2],
  double selfDGqqcoupl_input[SIZE_GQQ][2],
  double selfDGvvcoupl_input[SIZE_GVV][2],
  float& prob,
  bool useConstant
  ){
  for (int im=0; im<2; im++){
    for (int ic=0; ic<SIZE_GGG; ic++) selfDGggcoupl[ic][im] = selfDGggcoupl_input[ic][im];
    for (int ic=0; ic<SIZE_GQQ; ic++) selfDGqqcoupl[ic][im] = selfDGqqcoupl_input[ic][im];
    for (int ic=0; ic<SIZE_GVV; ic++) selfDGvvcoupl[ic][im] = selfDGvvcoupl_input[ic][im];
  }

  computeP(
    prob,
    useConstant
    );
}
void Mela::computeP_selfDspin2(
  double selfDGggcoupl_input[SIZE_GGG][2],
  double selfDGvvcoupl_input[SIZE_GVV][2],
  float& prob,
  bool useConstant
  ){
  selfDGqqcoupl[0][0]=1.0; // Set these incorrectly, and you see left-right asymmetries in qqG (or nothing)
  selfDGqqcoupl[1][0]=1.0;

  for (int im=0; im<2; im++){
    for (int ic=0; ic<SIZE_GGG; ic++) selfDGggcoupl[ic][im] = selfDGggcoupl_input[ic][im];
    for (int ic=0; ic<SIZE_GVV; ic++) selfDGvvcoupl[ic][im] = selfDGvvcoupl_input[ic][im];
  }

  computeP(
    prob,
    useConstant
    );
}
void Mela::computeP(
  float& prob,
  bool useConstant
  ){
  if (myVerbosity_>=TVar::DEBUG) cout << "Mela: Begin computeP" << endl;
  reset_PAux();

  melaCand = getCurrentCandidate();
  if (melaCand!=0){
    TLorentzVector nullVector(0, 0, 0, 0);
    float mZZ=0, mZ1=0, mZ2=0, costheta1=0, costheta2=0, Phi=0, costhetastar=0, Phi1=0;

    if (myME_ == TVar::ANALYTICAL){
      computeDecayAngles(
        mZZ, mZ1, mZ2,
        costheta1, costheta2, Phi,
        costhetastar, Phi1
        );
      costhetastar_rrv->setVal(costhetastar);
      costheta1_rrv->setVal(costheta1);
      costheta2_rrv->setVal(costheta2);
      phi_rrv->setVal(Phi);
      phi1_rrv->setVal(Phi1);
      z1mass_rrv->setVal(mZ1);
      z2mass_rrv->setVal(mZ2);
      mzz_rrv->setVal(mZZ);
      Y_rrv->setConstant(true); // Just to avoid integrating over this variable unnecessarily

      bool computeAnaMELA = configureAnalyticalPDFs();
      if (computeAnaMELA){
        if (myProduction_==TVar::ZZINDEPENDENT){
          RooAbsPdf* integral = (RooAbsPdf*)pdf->createIntegral(RooArgSet(*costhetastar_rrv, *phi1_rrv));
          prob = integral->getVal();
          delete integral;
        }
        else prob = pdf->getVal();
      }
      else if (myVerbosity_>=TVar::ERROR) cerr << "Mela::computeP: The specified anaMELA configuration is not valid!" << endl;

      Y_rrv->setConstant(false);
    }
    else if (myME_ == TVar::JHUGen || myME_ == TVar::MCFM){
      if (!(myME_ == TVar::MCFM  && myProduction_ == TVar::ZZINDEPENDENT &&  (myModel_ == TVar::bkgZZ || myModel_ == TVar::bkgWW || myModel_ == TVar::bkgZGamma))){
        if (myME_ == TVar::MCFM || myModel_ == TVar::SelfDefine_spin0) ZZME->set_SpinZeroCouplings(
          selfDHggcoupl,
          selfDHg4g4coupl,
          selfDHqqcoupl,
          selfDHbbcoupl,
          selfDHttcoupl,
          selfDHb4b4coupl,
          selfDHt4t4coupl,
          selfDHzzcoupl,
          selfDHwwcoupl,
          selfDHzzLambda_qsq,
          selfDHwwLambda_qsq,
          selfDHzzCLambda_qsq,
          selfDHwwCLambda_qsq,
          differentiate_HWW_HZZ
          );
        else if (myModel_ == TVar::SelfDefine_spin1) ZZME->set_SpinOneCouplings(selfDZqqcoupl, selfDZvvcoupl);
        else if (myModel_ == TVar::SelfDefine_spin2) ZZME->set_SpinTwoCouplings(selfDGqqcoupl, selfDGggcoupl, selfDGvvcoupl);
        ZZME->computeXS(prob);
      }
      else{
        computeDecayAngles(
          mZZ, mZ1, mZ2,
          costheta1, costheta2, Phi,
          costhetastar, Phi1
          );

        if (myVerbosity_>=TVar::DEBUG){ // Notify first
          cout << "Mela::computeP: Condition (myME_ == TVar::MCFM  && myProduction_ == TVar::ZZINDEPENDENT &&  myModel_ == TVar::bkgZZ/WW/ZGamma/ZJJ)." << endl;
          vector<TLorentzVector> pDauVec = calculate4Momentum(mZZ, mZ1, mZ1, acos(costhetastar), acos(costheta1), acos(costheta2), Phi1, Phi);
          cout
            << "\tOriginal mZZ=" << mZZ << " "
            << "m1=" << mZ1 << " "
            << "m2=" << mZ2 << " "
            << "h1=" << costheta1 << " "
            << "h2=" << costheta2 << " "
            << "Phi=" << Phi << " "
            << "hs=" << costhetastar << " "
            << "Phi1=" << Phi1 << endl;
          cout << "\tfor daughters:" << endl;
          for (int iv=0; iv<2; iv++){
            for (int idau=0; idau<min(2, melaCand->getSortedV(iv)->getNDaughters()); idau++){
              cout
                << "id=" << melaCand->getSortedV(iv)->getDaughter(idau)->id << " "
                << "x=" << pDauVec.at(2*iv+idau).X() << " "
                << "y=" << pDauVec.at(2*iv+idau).Y() << " "
                << "z=" << pDauVec.at(2*iv+idau).Z() << " "
                << "t=" << pDauVec.at(2*iv+idau).T() << endl;
            }
          }

        }

        prob = 0;
        int gridsize_hs = 5;
        double hs_min = 0; //-1.;
        double hs_max = 1;
        double hs_step = (hs_max - hs_min) / double(gridsize_hs);

        int gridsize_phi1 = 5;
        double phi1_min = 0; //-TMath::Pi();
        double phi1_max = TMath::Pi();
        double phi1_step = (phi1_max - phi1_min) / double(gridsize_phi1);

        for (int i_hs = 0; i_hs < gridsize_hs + 1; i_hs++) {
          double hs_val = hs_min + i_hs * hs_step;
          for (int i_phi1 = 0; i_phi1 < gridsize_phi1 +1; i_phi1++) {
            double phi1_val = phi1_min + i_phi1 * phi1_step;
            float temp_prob=0;

            // Get identical 4-vectors
            SimpleParticleCollection_t daughters;
            vector<TLorentzVector> pDauVec = calculate4Momentum(mZZ, mZ1, mZ2, acos(hs_val), acos(costheta1), acos(costheta2), phi1_val, Phi);
            for (int iv=0; iv<2; iv++){
              for (int idau=0; idau<min(2, melaCand->getSortedV(iv)->getNDaughters()); idau++){
                SimpleParticle_t tmpPair(melaCand->getSortedV(iv)->getDaughter(idau)->id, pDauVec.at(2*iv+idau));
                daughters.push_back(tmpPair);
              }
            }
            if (myVerbosity_>=TVar::DEBUG){ // Summarize the integrated particles
              cout << "Mela::computeP: hs, Phi1 are now " << hs_val << " " << phi1_val << endl;
              for (unsigned int idau=0; idau<daughters.size(); idau++){
                cout << "Dau " << idau << " "
                  << "id=" << daughters.at(idau).first << " "
                  << "x=" << daughters.at(idau).second.X() << " "
                  << "y=" << daughters.at(idau).second.Y() << " "
                  << "z=" << daughters.at(idau).second.Z() << " "
                  << "t=" << daughters.at(idau).second.T() << endl;
              }
            }
            vector<MELAParticle*> partList_tmp;
            vector<MELACandidate*> candList_tmp;
            MELACandidate* cand_tmp = TUtil::ConvertVectorFormat(
              &daughters,
              0,
              0,
              false,
              &partList_tmp,
              &candList_tmp
              );
            if (myVerbosity_>=TVar::ERROR && cand_tmp==0) cerr << "Mela::computeP: Failed to construct temporary candidate!" << endl;
            setCurrentCandidate(cand_tmp);
            if (myVerbosity_>=TVar::DEBUG && cand_tmp!=0){ cout << "Mela::computeP: ZZINDEPENDENT calculation produces candidate:" << endl; TUtil::PrintCandidateSummary(cand_tmp); }
            // calculate the ME
            ZZME->computeXS(temp_prob);
            // Delete the temporary particles
            for (unsigned int ic=0; ic<candList_tmp.size(); ic++){ if (candList_tmp.at(ic)!=0) delete candList_tmp.at(ic); } // Only one candidate should really be here
            for (unsigned int ip=0; ip<partList_tmp.size(); ip++){ if (partList_tmp.at(ip)!=0) delete partList_tmp.at(ip); }
            setCurrentCandidate(melaCand);
            prob += temp_prob;
          }
        }
        prob =  prob / float((gridsize_hs + 1) * (gridsize_phi1 +1));
      }
    }

    if (useConstant) computeConstant(prob);
  }

  reset_SelfDCouplings();
  reset_CandRef();
  if (myVerbosity_>=TVar::DEBUG) cout << "Mela: End computeP" << endl;
}


void Mela::computeD_CP(
  TVar::MatrixElement myME,
  TVar::Process myType,
  float& prob
  ){
  if (myVerbosity_>=TVar::DEBUG) cout << "Mela: Begin computeD_CP" << endl;
  double coupl_mix[nSupportedHiggses][SIZE_HVV][2] ={ { { 0 } } };
  double coupl_1[nSupportedHiggses][SIZE_HVV][2] ={ { { 0 } } };
  double coupl_2[nSupportedHiggses][SIZE_HVV][2] ={ { { 0 } } };

  switch (myType){
  case TVar::D_g1g4:
    coupl_mix[0][0][0] =1.;
    coupl_mix[0][3][0] =2.521;
    coupl_1[0][0][0] =1.;
    coupl_2[0][3][0] =2.521;
    break;
  case TVar::D_g1g4_pi_2:
    coupl_mix[0][0][0] =1.;
    coupl_mix[0][3][1] =2.521;
    coupl_1[0][0][0] =1.;
    coupl_2[0][3][1] =2.521;
    break;
  case TVar::D_g1g2:
    coupl_mix[0][0][0] =1.;
    coupl_mix[0][1][0] = 1.638;
    coupl_1[0][0][0] =1.;
    coupl_2[0][1][0] = 1.638;
    break;
  case TVar::D_g1g2_pi_2:
    coupl_mix[0][0][0] =1.;
    coupl_mix[0][1][1] = 1.638;
    coupl_1[0][0][0] =1.;
    coupl_2[0][1][1] = 1.638;
    break;
  case TVar::D_g1g1prime2:
    coupl_mix[0][0][0] =1.;
    coupl_mix[0][11][0] = 12046.01;
    coupl_1[0][0][0] =1.;
    coupl_2[0][11][0] = 12046.01;
    break;
  case TVar::D_zzzg:
    coupl_mix[0][0][0] =1.;
    coupl_mix[0][4][0] = 0.0688;
    coupl_1[0][0][0] =1.;
    coupl_2[0][4][0] = 0.0688;
    break;
  case TVar::D_zzgg:
    coupl_mix[0][0][0] =1.;
    coupl_mix[0][7][0] = -0.0898;
    coupl_1[0][0][0] =1.;
    coupl_2[0][7][0] = -0.0898;
    break;
  case TVar::D_zzzg_PS:
    coupl_mix[0][0][0] =1.;
    coupl_mix[0][6][0] = 0.0855;
    coupl_1[0][0][0] =1.;
    coupl_2[0][6][0] = 0.0855;
    break;
  case TVar::D_zzgg_PS:
    coupl_mix[0][0][0] =1.;
    coupl_mix[0][9][0] = -0.0907;
    coupl_1[0][0][0] =1.;
    coupl_2[0][9][0] = -0.0907;
    break;
  case TVar::D_zzzg_g1prime2:
    coupl_mix[0][0][0] =1.;
    coupl_mix[0][30][0] = -7591.914;
    coupl_1[0][0][0] =1.;
    coupl_2[0][30][0] = -7591.914;
    break;
  case TVar::D_zzzg_g1prime2_pi_2:
    coupl_mix[0][0][0] =1.;
    coupl_mix[0][30][1] = -7591.914;
    coupl_1[0][0][0] =1.;
    coupl_2[0][30][1] = -7591.914;
    break;
  default:
    cout <<"Error: Not supported!"<<endl;
  }

  float pMix, p1, p2;
  setProcess(TVar::SelfDefine_spin0, myME, TVar::ZZGG);
  computeP_selfDspin0(coupl_mix, pMix, true);
  computeP_selfDspin0(coupl_1, p1, true);
  computeP_selfDspin0(coupl_2, p2, true);
  prob = pMix- p1- p2;
  if (myVerbosity_>=TVar::DEBUG) cout << "Mela: End computeD_CP" << endl;
}


void Mela::computeProdDecP(
  double selfDHvvcoupl_input[nSupportedHiggses][SIZE_HVV][2],
  double selfDHwwcoupl_input[nSupportedHiggses][SIZE_HVV][2],
  float& prob,
  bool useConstant
  ){
  for (int jh=0; jh<(int)nSupportedHiggses; jh++){
    for (int im=0; im<2; im++){
      for (int ic=0; ic<SIZE_HVV; ic++){
        selfDHzzcoupl[jh][ic][im] = selfDHvvcoupl_input[jh][ic][im];
        selfDHwwcoupl[jh][ic][im] = selfDHwwcoupl_input[jh][ic][im]; // Just for extra protection since differentiate_HWW_HZZ is set to false.
      }
    }
  }
  computeProdDecP(
    prob,
    useConstant
    );
}
void Mela::computeProdDecP(
  float& prob,
  bool useConstant
  ){
  if (myVerbosity_>=TVar::DEBUG) cout << "Mela: Begin computeProdDecP" << endl;
  reset_PAux();
  melaCand = getCurrentCandidate();

  bool hasFailed = false;
  if (myME_ != TVar::MCFM){
    cout << "Mela::computeProdDecP ME is not supported for ME " << myME_ << endl;
    hasFailed = true;
  }
  if (
    !(
    myProduction_==TVar::Had_WH || myProduction_==TVar::Had_ZH
    || myProduction_==TVar::Had_WH_S || myProduction_==TVar::Had_ZH_S
    || myProduction_==TVar::Had_WH_TU || myProduction_==TVar::Had_ZH_TU
    || myProduction_==TVar::Lep_ZH || myProduction_==TVar::Lep_WH
    || myProduction_==TVar::Lep_ZH_S || myProduction_==TVar::Lep_WH_S
    || myProduction_==TVar::Lep_ZH_TU || myProduction_==TVar::Lep_WH_TU
    || myProduction_==TVar::JJVBF || myProduction_==TVar::JJEW || myProduction_==TVar::JJEWQCD || myProduction_==TVar::JJQCD
    || myProduction_==TVar::JJVBF_S || myProduction_==TVar::JJEW_S || myProduction_==TVar::JJEWQCD_S || myProduction_==TVar::JJQCD_S
    || myProduction_==TVar::JJVBF_TU || myProduction_==TVar::JJEW_TU || myProduction_==TVar::JJEWQCD_TU || myProduction_==TVar::JJQCD_TU
    )
    ){
    cout << "Mela::computeProdDecP production mode is not supported for production " << myProduction_ << endl;
    hasFailed = true;
  }
  if (melaCand==0) hasFailed=true;
  if (hasFailed) prob=0;
  else{
    ZZME->set_SpinZeroCouplings(
      selfDHggcoupl,
      selfDHg4g4coupl,
      selfDHqqcoupl,
      selfDHbbcoupl,
      selfDHttcoupl,
      selfDHb4b4coupl,
      selfDHt4t4coupl,
      selfDHzzcoupl,
      selfDHwwcoupl,
      selfDHzzLambda_qsq,
      selfDHwwLambda_qsq,
      selfDHzzCLambda_qsq,
      selfDHwwCLambda_qsq,
      differentiate_HWW_HZZ
      );
    ZZME->computeProdXS_VVHVV(prob);
    if (useConstant) computeConstant(prob);
  }

  reset_SelfDCouplings();
  reset_CandRef();
  if (myVerbosity_>=TVar::DEBUG) cout << "Mela: End computeProdDecP" << endl;
}


void Mela::computeProdP(
  double selfDHggcoupl_input[SIZE_HGG][2],
  double selfDHvvcoupl_input[nSupportedHiggses][SIZE_HVV][2],
  double selfDHwwcoupl_input[nSupportedHiggses][SIZE_HVV][2],
  float& prob,
  bool useConstant
  ){
  for (int im=0; im<2; im++){ for (int ic=0; ic<SIZE_HGG; ic++) selfDHggcoupl[0][ic][im] = selfDHggcoupl_input[ic][im]; }
  for (int jh=0; jh<(int)nSupportedHiggses; jh++){
    for (int im=0; im<2; im++){
      for (int ic=0; ic<SIZE_HVV; ic++){
        selfDHzzcoupl[jh][ic][im] = selfDHvvcoupl_input[jh][ic][im];
        selfDHwwcoupl[jh][ic][im] = selfDHwwcoupl_input[jh][ic][im]; // Just for extra protection since differentiate_HWW_HZZ is set to false.
      }
    }
  }
  computeProdP(
    prob,
    useConstant
    );
}
void Mela::computeProdP(
  float& prob,
  bool useConstant
  ){
  if (myVerbosity_>=TVar::DEBUG) cout << "Mela: Begin computeProdP" << endl;
  if (myProduction_ == TVar::ttH || myProduction_ == TVar::bbH) computeProdP_ttH(prob, 2, 0, useConstant);
  else if (myProduction_ == TVar::Lep_ZH || myProduction_ == TVar::Lep_WH || myProduction_ == TVar::Had_ZH || myProduction_ == TVar::Had_WH || myProduction_ == TVar::GammaH) computeProdP_VH(prob, false, useConstant);
  else{
    reset_PAux();

    melaCand = getCurrentCandidate();
    if (melaCand!=0){
      TLorentzVector nullFourVector(0, 0, 0, 0);
      bool isJet2Fake = false;
      MELACandidate* candOriginal = melaCand;

      unsigned int firstJetIndex=0;
      TLorentzVector jet1, higgs;
      TLorentzVector jet1massless(0, 0, 0, 0);
      TLorentzVector jet2massless(0, 0, 0, 0);
      higgs=melaCand->p4;
      if (myProduction_ == TVar::JJQCD || myProduction_ == TVar::JJVBF){
        int njets=0;
        for (int ip=0; ip<melaCand->getNAssociatedJets(); ip++){
          if (melaCand->getAssociatedJet(ip)->passSelection){
            njets++;
            if (njets==1){
              firstJetIndex = ip;
              jet1 = melaCand->getAssociatedJet(ip)->p4;
            }
          }
        }
        if (njets==1){
          TUtil::scaleMomentumToEnergy(jet1, jet1massless);
          TUtil::computeFakeJet(jet1massless, higgs, jet2massless);
          isJet2Fake=true;

          // Scale jet2massless pz if necessary
          const double threshold = 1000.*LHCsqrts/2.;
          TLorentzVector pTotal = higgs+jet1massless+jet2massless;
          double sysZ = pTotal.Z();
          if (fabs(sysZ)>threshold){
            double maxpz2 = threshold - higgs.Z() - jet1massless.Z();
            if (fabs(maxpz2)>0.){
              double ratio = jet2massless.Z()/maxpz2;
              double absp=sqrt(pow(jet2massless.Pt(), 2)+pow(jet2massless.Z()*ratio, 2));
              if (myVerbosity_>=TVar::INFO) cout << "Mela::computeProdP, isJet2Fake=true case: Rescaling pz of fake jet by " << ratio << " and energy = " << absp << "." << endl;
              jet2massless.SetXYZT(jet2massless.X(), jet2massless.Y(), jet2massless.Z()*ratio, absp);
            }
            else{
              if (myVerbosity_>=TVar::INFO) cout << "Mela::computeProdP, isJet2Fake=true case: Unable to rescaling pz of fake jet since max(|pz|)<0. Setting to 0 with appropriate energy = pT = " << jet2massless.Pt() << "." << endl;
              jet2massless.SetXYZT(jet2massless.X(), jet2massless.Y(), 0., jet2massless.Pt());
            }
          }
        }
      }

      if (isJet2Fake){ // Do the integration
        MELACandidate* candCopy = melaCand->shallowCopy();
        MELAParticle* firstJet = candCopy->getAssociatedJet(firstJetIndex);
        firstJet->p4.SetXYZT(jet1massless.X(), jet1massless.Y(), jet1massless.Z(), jet1massless.T()); // Re-assign momenta of the first jet. Be careful, it changes candOriginal as well!
        MELAParticle fakeJet(0, jet2massless); // jet2massless is the unknown jet
        candCopy->addAssociatedJets(&fakeJet);
        setCurrentCandidate(candCopy);

        if (myModel_ == TVar::SelfDefine_spin0) ZZME->set_SpinZeroCouplings(
          selfDHggcoupl,
          selfDHg4g4coupl,
          selfDHqqcoupl,
          selfDHbbcoupl,
          selfDHttcoupl,
          selfDHb4b4coupl,
          selfDHt4t4coupl,
          selfDHzzcoupl,
          selfDHwwcoupl,
          selfDHzzLambda_qsq,
          selfDHwwLambda_qsq,
          selfDHzzCLambda_qsq,
          selfDHwwCLambda_qsq,
          differentiate_HWW_HZZ
          );
        ZZME->computeProdXS_JJH(prob); // Higgs + 2 jets: SBF or WBF main probability

        int nGrid=11;
        std::vector<double> etaArray;
        std::vector<double> pArray;
        double eta_max = 10;
        if (jet2massless.Pt()>0.) eta_max = max(eta_max, 1.2*fabs(jet2massless.Eta()));
        double eta_min = -eta_max;

        for (int iter=0; iter<nGrid; iter++){
          float prob_temp=0;

          double jet2temp_eta = ((double)iter)*(eta_max-eta_min) / (((double)nGrid) - 1.) + eta_min;
          etaArray.push_back(jet2temp_eta);
          double jet2temp_sinh_eta = TMath::SinH(jet2temp_eta);
          double jet2temp_pz = jet2massless.Pt()*jet2temp_sinh_eta;
          fakeJet.p4.SetZ(jet2temp_pz);
          fakeJet.p4.SetX(jet2massless.X()); fakeJet.p4.SetY(jet2massless.Y()); fakeJet.p4.SetT(fakeJet.p4.P());

          // Skip case with invalid pz
          const double threshold = 1000.*LHCsqrts/2.;
          TLorentzVector pTotal = higgs+jet1massless+fakeJet.p4;
          double sys = (pTotal.T()+fabs(pTotal.Z()))/2.;
          if (fabs(sys)<threshold){
            if (myModel_ == TVar::SelfDefine_spin0) ZZME->set_SpinZeroCouplings(
              selfDHggcoupl,
              selfDHg4g4coupl,
              selfDHqqcoupl,
              selfDHbbcoupl,
              selfDHttcoupl,
              selfDHb4b4coupl,
              selfDHt4t4coupl,
              selfDHzzcoupl,
              selfDHwwcoupl,
              selfDHzzLambda_qsq,
              selfDHwwLambda_qsq,
              selfDHzzCLambda_qsq,
              selfDHwwCLambda_qsq,
              differentiate_HWW_HZZ
              );
            ZZME->computeProdXS_JJH(prob_temp);
          }
          pArray.push_back((double)prob_temp);
        }

        double* xGrid;
        double* yGrid;
        const double grid_precision = 0.15;
        int ctr_iter=0;
        for (int iG=0; iG<nGrid-1; iG++){ // For each spacing, first compare the average of end points to spline value
          if (pArray[iG]==pArray[iG+1]) continue;
          if (etaArray[iG]==etaArray[iG+1]) continue;

          ctr_iter++;

          xGrid = new double[nGrid];
          yGrid = new double[nGrid];
          for (int iter=0; iter<nGrid; iter++){ // Fill the arrays
            xGrid[iter] = (double)etaArray[iter];
            yGrid[iter] = (double)pArray[iter];
          }

          TGraph* interpolator = new TGraph(nGrid, xGrid, yGrid);
          double derivative_first = (yGrid[1]-yGrid[0])/(xGrid[1]-xGrid[0]);
          double derivative_last = (yGrid[nGrid-1]-yGrid[nGrid-2])/(xGrid[nGrid-1]-xGrid[nGrid-2]);
          TSpline3* spline = new TSpline3("spline", interpolator, "b1e1", derivative_first, derivative_last);
          double x_middle = (xGrid[iG]+xGrid[iG+1])*0.5;
          double y_middle = (yGrid[iG]+yGrid[iG+1])*0.5;
          double y_sp = spline->Eval(x_middle);
          if (y_sp<0) y_sp = 0;

          std::vector<double>::iterator gridIt;

          if (fabs(y_sp-y_middle)<grid_precision*fabs(y_middle) || fabs(xGrid[iG+1]-xGrid[iG])<1e-3){
            gridIt = pArray.begin()+iG+1;
            pArray.insert(gridIt, y_sp);
            gridIt = etaArray.begin()+iG+1;
            etaArray.insert(gridIt, x_middle);
            iG++; // Pass to next bin
          }
          else{
            float prob_temp=0;

            double jet2temp_eta = x_middle;
            gridIt = etaArray.begin()+iG+1;
            etaArray.insert(gridIt, x_middle);
            double jet2temp_sinh_eta = TMath::SinH(jet2temp_eta);
            double jet2temp_pz = jet2massless.Pt()*jet2temp_sinh_eta;
            fakeJet.p4.SetZ(jet2temp_pz);
            fakeJet.p4.SetX(jet2massless.X()); fakeJet.p4.SetY(jet2massless.Y()); fakeJet.p4.SetT(fakeJet.p4.P());

            // Skip case with invalid pz
            const double threshold = 1000.*LHCsqrts/2.;
            TLorentzVector pTotal = higgs+jet1massless+fakeJet.p4;
            double sys = (pTotal.T()+fabs(pTotal.Z()))/2.;
            if (fabs(sys)<threshold){
              if (myModel_ == TVar::SelfDefine_spin0) ZZME->set_SpinZeroCouplings(
                selfDHggcoupl,
                selfDHg4g4coupl,
                selfDHqqcoupl,
                selfDHbbcoupl,
                selfDHttcoupl,
                selfDHb4b4coupl,
                selfDHt4t4coupl,
                selfDHzzcoupl,
                selfDHwwcoupl,
                selfDHzzLambda_qsq,
                selfDHwwLambda_qsq,
                selfDHzzCLambda_qsq,
                selfDHwwCLambda_qsq,
                differentiate_HWW_HZZ
                );
              ZZME->computeProdXS_JJH(prob_temp);
            }
            gridIt = pArray.begin()+iG+1;
            pArray.insert(gridIt, (double)prob_temp);
            iG--; // Do not pass to next bin, repeat until precision is achieved.
          }
          nGrid++;

          delete spline;
          delete interpolator;
          delete xGrid;
          delete yGrid;
        }

        if (myVerbosity_>=TVar::DEBUG) cout << "Mela::computeProdP: Number of iterations for JVBF eta integration: " << ctr_iter << endl;

        auxiliaryProb = 0;
        int iGFirst=0, iGLast=nGrid-1;
        for (int iG=1; iG<nGrid; iG++){
          if (pArray[iG]>0 && pArray[iG-1]==0){
            iGFirst = iG-1;
            break;
          }
        }
        for (int iG=nGrid-2; iG>=0; iG--){
          if (pArray[iG]>0 && pArray[iG+1]==0){
            iGLast = iG+1;
            break;
          }
        }
        double dEtaGrid = etaArray[iGLast] - etaArray[iGFirst];
        for (int iG=iGFirst; iG<iGLast-1; iG++){
          double dEta = etaArray[iG+1] - etaArray[iG];
          double sumProb = pArray[iG]+pArray[iG+1];
          sumProb *= 0.5;
          dEta = dEta/dEtaGrid;
          double addProb = sumProb*dEta;
          auxiliaryProb += (float)addProb;
        }

        firstJet->p4.SetXYZT(jet1.X(), jet1.Y(), jet1.Z(), jet1.T()); // Re-assign momenta of the first jet to original. Be careful, it changes candOriginal as well!
        delete candCopy; // Delete the shallow copy
        setCurrentCandidate(candOriginal);
        melaCand = getCurrentCandidate();
        if (myVerbosity_>=TVar::DEBUG){
          if (melaCand!=candOriginal) cerr << "Mela::computeProdP: melaCand!=candOriginal at the end of the fake jet scenario!" << endl;
        }

        if (fabs(prob)>0) auxiliaryProb /= prob;
      }
      else{
        if (myProduction_ == TVar::JJQCD || myProduction_ == TVar::JJVBF){
          if (myModel_ == TVar::SelfDefine_spin0) ZZME->set_SpinZeroCouplings(
            selfDHggcoupl,
            selfDHg4g4coupl,
            selfDHqqcoupl,
            selfDHbbcoupl,
            selfDHttcoupl,
            selfDHb4b4coupl,
            selfDHt4t4coupl,
            selfDHzzcoupl,
            selfDHwwcoupl,
            selfDHzzLambda_qsq,
            selfDHwwLambda_qsq,
            selfDHzzCLambda_qsq,
            selfDHwwCLambda_qsq,
            differentiate_HWW_HZZ
            );
          ZZME->computeProdXS_JJH(prob); // Higgs + 2 jets: SBF or WBF
        }
        else if (myProduction_ == TVar::JQCD){
          // No anomalous couplings are implemented in HJ
          ZZME->computeProdXS_JH(prob); // Higgs + 1 jet; only SM is supported for now.
        }
      }
      if (useConstant) computeConstant(prob);
    }

    reset_SelfDCouplings();
    reset_CandRef();
  }
  if (myVerbosity_>=TVar::DEBUG) cout << "Mela: End computeProdP" << endl;
}


void Mela::computeProdP_VH(
  double selfDHvvcoupl_input[nSupportedHiggses][SIZE_HVV][2],
  float& prob,
  bool includeHiggsDecay,
  bool useConstant
  ){
  // Dedicated function for VH ME
  selfDHggcoupl[0][0][0] = 1.0; // Don't set this, and you might get 0 in the future for ggVH.
  for (int jh=0; jh<(int)nSupportedHiggses; jh++){
    for (int im=0; im<2; im++){
      for (int ic=0; ic<SIZE_HVV; ic++){
        selfDHzzcoupl[jh][ic][im] = selfDHvvcoupl_input[jh][ic][im];
        selfDHwwcoupl[jh][ic][im] = selfDHvvcoupl_input[jh][ic][im]; // Just for extra protection since differentiate_HWW_HZZ is set to false.
      }
    }
  }

  computeProdP_VH(
    prob,
    includeHiggsDecay,
    useConstant
    );
}
void Mela::computeProdP_VH(
  float& prob,
  bool includeHiggsDecay,
  bool useConstant
  ){
  if (myVerbosity_>=TVar::DEBUG) cout << "Mela: Begin computeProdP_VH" << endl;
  reset_PAux();

  melaCand = getCurrentCandidate();
  if (melaCand!=0){
    if (myProduction_ == TVar::Lep_ZH || myProduction_ == TVar::Lep_WH || myProduction_ == TVar::Had_ZH || myProduction_ == TVar::Had_WH || myProduction_ == TVar::GammaH){
      if (myModel_ == TVar::SelfDefine_spin0) ZZME->set_SpinZeroCouplings(
        selfDHggcoupl,
        selfDHg4g4coupl,
        selfDHqqcoupl,
        selfDHbbcoupl,
        selfDHttcoupl,
        selfDHb4b4coupl,
        selfDHt4t4coupl,
        selfDHzzcoupl,
        selfDHwwcoupl,
        selfDHzzLambda_qsq,
        selfDHwwLambda_qsq,
        selfDHzzCLambda_qsq,
        selfDHwwCLambda_qsq,
        differentiate_HWW_HZZ
        );
      ZZME->computeProdXS_VH(prob, includeHiggsDecay); // VH

      if (useConstant) computeConstant(prob);
    }
  }

  reset_SelfDCouplings();
  reset_CandRef();
  if (myVerbosity_>=TVar::DEBUG) cout << "Mela: End computeProdP_VH" << endl;
}


void Mela::computeProdP_ttH(
  float& prob,
  int topProcess,
  int topDecay,
  bool useConstant
  ){
  if (myVerbosity_>=TVar::DEBUG) cout << "Mela: Begin computeProdP_ttH" << endl;
  reset_PAux();

  melaCand = getCurrentCandidate();
  if (melaCand!=0){
    if (myModel_ == TVar::SelfDefine_spin0) ZZME->set_SpinZeroCouplings(
      selfDHggcoupl,
      selfDHg4g4coupl,
      selfDHqqcoupl,
      selfDHbbcoupl,
      selfDHttcoupl,
      selfDHb4b4coupl,
      selfDHt4t4coupl,
      selfDHzzcoupl,
      selfDHwwcoupl,
      selfDHzzLambda_qsq,
      selfDHwwLambda_qsq,
      selfDHzzCLambda_qsq,
      selfDHwwCLambda_qsq,
      differentiate_HWW_HZZ
      );
    ZZME->computeProdXS_ttH(prob,topProcess, topDecay);
    if (useConstant) computeConstant(prob);
  }

  reset_SelfDCouplings();
  reset_CandRef();
  if (myVerbosity_>=TVar::DEBUG) cout << "Mela: End computeProdP_ttH" << endl;
}

void Mela::getXPropagator(TVar::ResonancePropagatorScheme scheme, float& prop){
  prop=0.;
  melaCand = getCurrentCandidate();
  if (melaCand!=0) ZZME->get_XPropagator(scheme, prop);
  reset_CandRef();
}


void Mela::compute4FermionWeight(float& w){ // Lepton interference using JHUGen
  reset_PAux();

  melaCand = getCurrentCandidate();
  if (melaCand!=0){
    bool hasFailed=false;
    int id_original[2][2];
    for (int iv=0; iv<2; iv++){
      MELAParticle* Vi = melaCand->getSortedV(iv);
      int ndau=Vi->getNDaughters();
      if (ndau!=2 || !(PDGHelpers::isAZBoson(Vi->id) || PDGHelpers::isAPhoton(Vi->id))){ w=1; hasFailed=true; break; } // Veto WW, ZG, GG
      for (int ivd=0; ivd<2; ivd++) id_original[iv][ivd]=Vi->getDaughter(ivd)->id;
    }
    if (
      !PDGHelpers::isALepton(id_original[0][0])
      ||
      !PDGHelpers::isALepton(id_original[0][1])
      ||
      !PDGHelpers::isALepton(id_original[1][0])
      ||
      !PDGHelpers::isALepton(id_original[1][1])
      ){
      if (myVerbosity_>=TVar::ERROR) cerr << "Mela::computeWeight: Function is not implemented for decay states other than 4l/2l2l." << endl;
      w=0;
      hasFailed=true;
    }
    /*
    if (
    (id_original[0][0]==0 && id_original[0][1]==0)
    ||
    (id_original[1][0]==0 && id_original[1][1]==0)
    ){ // Return 1 if any pairs of quarks are unknown
    w=1;
    hasFailed=true;
    }
    */

    if (!hasFailed){
      float dXsec_HZZ_JHU, dXsec_HZZ_JHU_interf; // temporary prob

      // Calculate dXsec ratio by directly modifying the candidate
      computeP(dXsec_HZZ_JHU, false);
      for (int ivd=0; ivd<2; ivd++) melaCand->getSortedV(1)->getDaughter(ivd)->id=id_original[0][0]*(1-2*ivd);
      computeP(dXsec_HZZ_JHU_interf, false);
      for (int ivd=0; ivd<2; ivd++) melaCand->getSortedV(1)->getDaughter(ivd)->id=id_original[1][ivd];

      w = dXsec_HZZ_JHU_interf / dXsec_HZZ_JHU;
      // protect against anomalously large weights
      if (w>5.) w=25./w;
    }
  }

  reset_SelfDCouplings();
  reset_CandRef();
}


void Mela::computePM4l(TVar::SuperMelaSyst syst, float& prob){
  reset_PAux();
  prob=-99;

  melaCand = getCurrentCandidate();
  if (melaCand!=0){
    bool hasFailed=false;
    int id_original[2][2];
    for (int iv=0; iv<2; iv++){
      MELAParticle* Vi = melaCand->getSortedV(iv);
      int ndau=Vi->getNDaughters();
      if (ndau!=2 || !(PDGHelpers::isAZBoson(Vi->id) || PDGHelpers::isAPhoton(Vi->id))){ hasFailed=true; break; } // Veto WW, ZG, GG
      for (int ivd=0; ivd<2; ivd++) id_original[iv][ivd]=Vi->getDaughter(ivd)->id;
    }

    if (!hasFailed){
      if (abs(id_original[0][0])==11 && abs(id_original[1][0])==11 && abs(id_original[0][1])==11 && abs(id_original[1][1])==11) super->SetDecayChannel("4e");
      else if (abs(id_original[0][0])==13 && abs(id_original[1][0])==13 && abs(id_original[0][1])==13 && abs(id_original[1][1])==13) super->SetDecayChannel("4mu");
      else if (
        (abs(id_original[0][0])==11 && abs(id_original[0][1])==11 && abs(id_original[1][0])==13 && abs(id_original[1][1])==13)
        ||
        (abs(id_original[0][0])==13 && abs(id_original[0][1])==13 && abs(id_original[1][0])==11 && abs(id_original[1][1])==11)
        ) super->SetDecayChannel("2e2mu");
      else{ if (myVerbosity_>=TVar::ERROR) cerr << "Mela::computePM4l: SuperMELA is currently not implemented for decay states other than 4e. 4mu, 2e2mu." << endl; hasFailed=true; }
    }

    if (!hasFailed){
      double mZZ = melaCand->m();
      // currently only supported signal is ggH(0+), only supported background is summed paramterization
      if (syst == TVar::SMSyst_None){
        std::pair<double, double> m4lP = super->M4lProb(mZZ);
        if (myModel_ == TVar::HSMHiggs) prob = m4lP.first;
        else if (myModel_ == TVar::bkgZZ) prob = m4lP.second;
      }
      else{
        //systematics for p(m4l)
        float mZZtmp=mZZ;
        float meanErr=float(super->GetSigShapeSystematic("meanCB"));
        float sigmaErr=float(super->GetSigShapeSystematic("sigmaCB"));
        float sigmaCB=float(super->GetSigShapeParameter("sigmaCB"));
        if (syst == TVar::SMSyst_ScaleUp) mZZtmp = mZZ*(1.0+meanErr);
        else if (syst == TVar::SMSyst_ScaleDown) mZZtmp = mZZ*(1.0-meanErr);
        else if (syst == TVar::SMSyst_ResUp || syst ==  TVar::SMSyst_ResDown) mZZtmp= myRandomNumber->Gaus(mZZ, sigmaErr*sigmaCB);

        std::pair<double, double> m4lP = super->M4lProb(mZZtmp);
        if (myModel_ == TVar::HSMHiggs) prob = m4lP.first;
        else if (myModel_ == TVar::bkgZZ) prob = m4lP.second;
      }
    }
  }

  reset_SelfDCouplings();
  reset_CandRef();
}


void Mela::constructDggr(
  float bkg_VAMCFM_noscale,
  float ggzz_VAMCFM_noscale,
  float ggHZZ_prob_pure_noscale,
  float ggHZZ_prob_int_noscale,
  float widthScale,
  float& myDggr
  ){
  float total_sig_ME = (widthScale * ggHZZ_prob_pure_noscale + sqrt(widthScale) * ggHZZ_prob_int_noscale + ggzz_VAMCFM_noscale);
  float total_bkg_ME = bkg_VAMCFM_noscale;
  float kd_denominator = (total_sig_ME+total_bkg_ME);
  if (kd_denominator>0.) myDggr = total_sig_ME/(total_sig_ME+total_bkg_ME);
  else myDggr=-99.;
}
void Mela::computeD_gg(
  TVar::MatrixElement myME,
  TVar::Process myType,
  float& prob
  ){
  prob=-99;
  if (myME != TVar::MCFM || myType != TVar::D_gg10){
    cout << "Only support MCFM and D_gg10"<<endl;
    return;
  }

  melaCand = getCurrentCandidate();
  if (melaCand!=0){
    float bkg_VAMCFM, ggzz_VAMCFM_noscale, ggHZZ_prob_pure_noscale, ggHZZ_prob_int_noscale, bkgHZZ_prob_noscale;
    float ggScale=0;
    setProcess(TVar::bkgZZ, myME, TVar::ZZGG); computeP(ggzz_VAMCFM_noscale, false);
    setProcess(TVar::HSMHiggs, myME, TVar::ZZGG); computeP(ggHZZ_prob_pure_noscale, false);
    setProcess(TVar::bkgZZ_SMHiggs, myME, TVar::ZZGG); computeP(bkgHZZ_prob_noscale, false); setConstant(); getConstant(ggScale);
    if (ggScale>0.){
      bkgHZZ_prob_noscale /= ggScale;
      ggHZZ_prob_pure_noscale /= ggScale;
      ggzz_VAMCFM_noscale /= ggScale;
    }
    ggHZZ_prob_int_noscale = bkgHZZ_prob_noscale - ggHZZ_prob_pure_noscale -  ggzz_VAMCFM_noscale;

    setProcess(TVar::bkgZZ, myME, TVar::ZZQQB); computeP(bkg_VAMCFM, true);

    constructDggr(bkg_VAMCFM, ggzz_VAMCFM_noscale, ggHZZ_prob_pure_noscale, ggHZZ_prob_int_noscale, 10., prob); // Use 10 for Dgg10
  }

  reset_SelfDCouplings();
  reset_CandRef();
}


bool Mela::configureAnalyticalPDFs(){
  // 
  // Configure the analytical calculations 
  // 
  bool noPass=false;

  if (myModel_==TVar::bkgZZ)  pdf = qqZZmodel;
  else if (myProduction_ == TVar::JJQCD || myProduction_ == TVar::JJVBF);
  else if (myProduction_ == TVar::Lep_ZH || myProduction_ == TVar::Lep_WH || myProduction_ == TVar::Had_ZH || myProduction_ == TVar::Had_WH || myProduction_ == TVar::GammaH);
  else if (
    myModel_ == TVar::HSMHiggs
    || myModel_ == TVar::H0minus || myModel_ == TVar::D_g1g4 || myModel_ == TVar::D_g1g4_pi_2
    || myModel_ == TVar::H0hplus || myModel_ == TVar::D_g1g2 || myModel_ == TVar::D_g1g2_pi_2
    || myModel_ == TVar::H0_g1prime2 || myModel_ == TVar::D_g1g1prime2
    || myModel_ == TVar::SelfDefine_spin0
    ){
    pdf = (RooAbsPdf*)ggSpin0Model->getPDF();
    ggSpin0Model->makeParamsConst(false);
    ggSpin0Model->resetHypotheses();

    // Add the hypotheses with best-guess coefficients
    // ZZ/WW
    if (
      myModel_ == TVar::HSMHiggs
      || myModel_ == TVar::D_g1g1prime2
      || myModel_ == TVar::D_g1g2 || myModel_ == TVar::D_g1g2_pi_2
      || myModel_ == TVar::D_g1g4 || myModel_ == TVar::D_g1g4_pi_2
      || myModel_ == TVar::D_zzzg_g1prime2 || myModel_ == TVar::D_zzzg_g1prime2_pi_2 || myModel_ == TVar::D_zzzg || myModel_ == TVar::D_zzzg_PS
      || myModel_ == TVar::D_zzgg || myModel_ == TVar::D_zzgg_PS
      ) ggSpin0Model->addHypothesis(0, 0);
    if (myModel_ == TVar::H0_g1prime2 || myModel_ == TVar::D_g1g1prime2) ggSpin0Model->addHypothesis(0, 2);
    if (myModel_ == TVar::H0hplus || myModel_ == TVar::D_g1g2 || myModel_ == TVar::D_g1g2_pi_2) ggSpin0Model->addHypothesis(1, 0, (myModel_ == TVar::D_g1g2_pi_2 ? TMath::Pi() : 0.));
    if (myModel_ == TVar::H0minus || myModel_ == TVar::D_g1g4 || myModel_ == TVar::D_g1g4_pi_2) ggSpin0Model->addHypothesis(3, 0, (myModel_ == TVar::D_g1g4_pi_2 ? TMath::Pi() : 0.));
    // ZG/ZGs
    if (myModel_ == TVar::H0_Zgsg1prime2 || myModel_ == TVar::D_zzzg_g1prime2 || myModel_ == TVar::D_zzzg_g1prime2_pi_2) ggSpin0Model->addHypothesis(4, 2, (myModel_ == TVar::D_zzzg_g1prime2_pi_2 ? TMath::Pi() : 0.));
    if (myModel_ == TVar::H0_Zgs || myModel_ == TVar::D_zzzg) ggSpin0Model->addHypothesis(5, 0);
    if (myModel_ == TVar::H0_Zgs_PS || myModel_ == TVar::D_zzzg_PS) ggSpin0Model->addHypothesis(7, 0);
    // GG/GGs/GsGs
    if (myModel_ == TVar::H0_gsgs || myModel_ == TVar::D_zzgg) ggSpin0Model->addHypothesis(8, 0);
    if (myModel_ == TVar::H0_gsgs_PS || myModel_ == TVar::D_zzgg_PS) ggSpin0Model->addHypothesis(10, 0);
    // Self-defined spin-0
    if (myModel_ == TVar::SelfDefine_spin0){
      for (int im=0; im<2; im++){
        ((RooRealVar*)ggSpin0Model->couplings.g1List[0][im])->setVal(selfDHzzcoupl[0][0][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g2List[0][im])->setVal(selfDHzzcoupl[0][1][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g3List[0][im])->setVal(selfDHzzcoupl[0][2][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g4List[0][im])->setVal(selfDHzzcoupl[0][3][im]);

        ((RooRealVar*)ggSpin0Model->couplings.gzgs2List[0][im])->setVal(selfDHzzcoupl[0][4][im]);
        ((RooRealVar*)ggSpin0Model->couplings.gzgs3List[0][im])->setVal(selfDHzzcoupl[0][5][im]);
        ((RooRealVar*)ggSpin0Model->couplings.gzgs4List[0][im])->setVal(selfDHzzcoupl[0][6][im]);
        ((RooRealVar*)ggSpin0Model->couplings.ggsgs2List[0][im])->setVal(selfDHzzcoupl[0][7][im]);
        ((RooRealVar*)ggSpin0Model->couplings.ggsgs3List[0][im])->setVal(selfDHzzcoupl[0][8][im]);
        ((RooRealVar*)ggSpin0Model->couplings.ggsgs4List[0][im])->setVal(selfDHzzcoupl[0][9][im]);

        ((RooRealVar*)ggSpin0Model->couplings.g1List[1][im])->setVal(selfDHzzcoupl[0][10][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g1List[2][im])->setVal(selfDHzzcoupl[0][11][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g1List[3][im])->setVal(selfDHzzcoupl[0][12][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g1List[4][im])->setVal(selfDHzzcoupl[0][13][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g1List[5][im])->setVal(selfDHzzcoupl[0][14][im]);

        ((RooRealVar*)ggSpin0Model->couplings.g2List[1][im])->setVal(selfDHzzcoupl[0][15][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g2List[2][im])->setVal(selfDHzzcoupl[0][16][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g2List[3][im])->setVal(selfDHzzcoupl[0][17][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g2List[4][im])->setVal(selfDHzzcoupl[0][18][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g2List[5][im])->setVal(selfDHzzcoupl[0][19][im]);

        ((RooRealVar*)ggSpin0Model->couplings.g3List[1][im])->setVal(selfDHzzcoupl[0][20][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g3List[2][im])->setVal(selfDHzzcoupl[0][21][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g3List[3][im])->setVal(selfDHzzcoupl[0][22][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g3List[4][im])->setVal(selfDHzzcoupl[0][23][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g3List[5][im])->setVal(selfDHzzcoupl[0][24][im]);

        ((RooRealVar*)ggSpin0Model->couplings.g4List[1][im])->setVal(selfDHzzcoupl[0][25][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g4List[2][im])->setVal(selfDHzzcoupl[0][26][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g4List[3][im])->setVal(selfDHzzcoupl[0][27][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g4List[4][im])->setVal(selfDHzzcoupl[0][28][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g4List[5][im])->setVal(selfDHzzcoupl[0][29][im]);

        ((RooRealVar*)ggSpin0Model->couplings.gzgs1List[0][im])->setVal(selfDHzzcoupl[0][30][im]); // Zgs1_prime2

        ((RooRealVar*)ggSpin0Model->couplings.g1List[6][im])->setVal(selfDHzzcoupl[0][31][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g1List[7][im])->setVal(selfDHzzcoupl[0][32][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g2List[6][im])->setVal(selfDHzzcoupl[0][33][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g2List[7][im])->setVal(selfDHzzcoupl[0][34][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g3List[6][im])->setVal(selfDHzzcoupl[0][35][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g3List[7][im])->setVal(selfDHzzcoupl[0][36][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g4List[6][im])->setVal(selfDHzzcoupl[0][37][im]);
        ((RooRealVar*)ggSpin0Model->couplings.g4List[7][im])->setVal(selfDHzzcoupl[0][38][im]);
      }
      for (int qoqtqz=0; qoqtqz<SIZE_HVV_CQSQ; qoqtqz++){ // 0==q1, 1==q2, 2==q12
        ((RooRealVar*)ggSpin0Model->couplings.Lambda_z1qsq[qoqtqz])->setVal(selfDHzzLambda_qsq[0][0][qoqtqz]);
        ((RooRealVar*)ggSpin0Model->couplings.Lambda_z2qsq[qoqtqz])->setVal(selfDHzzLambda_qsq[0][1][qoqtqz]);
        ((RooRealVar*)ggSpin0Model->couplings.Lambda_z3qsq[qoqtqz])->setVal(selfDHzzLambda_qsq[0][2][qoqtqz]);
        ((RooRealVar*)ggSpin0Model->couplings.Lambda_z4qsq[qoqtqz])->setVal(selfDHzzLambda_qsq[0][3][qoqtqz]);
        ((RooRealVar*)ggSpin0Model->couplings.cLambda_qsq[qoqtqz])->setVal(selfDHzzCLambda_qsq[0][qoqtqz]);
      }
    }
    ggSpin0Model->makeParamsConst(true);
  }
  else if (!spin1Model->configure(myModel_)){
    pdf = spin1Model->PDF;
    // Self-defined spin-1
    if (myModel_ == TVar::SelfDefine_spin1){
      for (int i=0; i<SIZE_ZVV; i++){ if (selfDZvvcoupl[i][1]!=0){ if (myVerbosity_>=TVar::ERROR) cerr << "Mela::configureAnalyticalPDFs: MELA does not support complex couplings for spin-1 at the moment! " << endl; noPass=true; break; } }
      if (!noPass){
        spin1Model->g1Val->setVal(selfDZvvcoupl[0][0]);
        spin1Model->g2Val->setVal(selfDZvvcoupl[1][0]);
      }
    }
  }
  else if (
    myModel_ == TVar::H2_g1
    || myModel_ == TVar::H2_g1g5
    || myModel_ == TVar::H2_g2
    || myModel_ == TVar::H2_g3
    || myModel_ == TVar::H2_g4
    || myModel_ == TVar::H2_g5
    || myModel_ == TVar::H2_g6
    || myModel_ == TVar::H2_g7
    || myModel_ == TVar::H2_g8
    || myModel_ == TVar::H2_g9
    || myModel_ == TVar::H2_g10
    || myModel_ == TVar::SelfDefine_spin2
    ){
    pdf = (RooAbsPdf*)spin2Model->getPDF();
    spin2Model->makeParamsConst(false);
    spin2Model->resetHypotheses();
    // Add the hypotheses with best-guess coefficients
    // ZZ/WW
    if (
      myModel_ == TVar::H2_g1
      || myModel_ == TVar::H2_g1g5
      ) spin2Model->addHypothesis(0, 1.);
    if (
      myModel_ == TVar::H2_g1g5
      || myModel_ == TVar::H2_g5
      ) spin2Model->addHypothesis(4, 1.);
    if (myModel_ == TVar::H2_g2) spin2Model->addHypothesis(1, 1.);
    if (myModel_ == TVar::H2_g3) spin2Model->addHypothesis(2, 1.);
    if (myModel_ == TVar::H2_g4) spin2Model->addHypothesis(3, 1.);
    if (myModel_ == TVar::H2_g5) spin2Model->addHypothesis(4, 1.);
    if (myModel_ == TVar::H2_g6) spin2Model->addHypothesis(5, 1.);
    if (myModel_ == TVar::H2_g7) spin2Model->addHypothesis(6, 1.);
    if (myModel_ == TVar::H2_g8) spin2Model->addHypothesis(7, 1.);
    if (myModel_ == TVar::H2_g9) spin2Model->addHypothesis(8, 1.);
    if (myModel_ == TVar::H2_g10) spin2Model->addHypothesis(10, 1.);
    // Self-defined spin-2
    if (myModel_ == TVar::SelfDefine_spin2){
      for (int i=0; i<SIZE_GVV; i++){ if (selfDGvvcoupl[i][1]!=0){ if (myVerbosity_>=TVar::ERROR) cerr << "Mela::configureAnalyticalPDFs: MELA does not support complex couplings for spin-2 at the moment! " << endl; noPass=true; break; } }
      if (!noPass){
        for (int ig=0; ig<SIZE_GVV; ig++){
          for (int im=0; im<2; im++) ((RooRealVar*)spin2Model->couplings.bList[ig][im])->setVal(selfDGvvcoupl[ig][im]);
        }
      }
    }
    if (!noPass){
      if (myProduction_ == TVar::ZZQQB){
        spin2Model->setTensorPolarization(1, 1.);
        spin2Model->setTensorPolarization(2, 0.);
      }
      else{
        if (myModel_ == TVar::SelfDefine_spin2){
          double c1 = 2*selfDGggcoupl[0][0] + 2.*selfDGggcoupl[1][0];
          double c2 = -0.5*selfDGggcoupl[0][0] + selfDGggcoupl[2][0] + 2.*selfDGggcoupl[3][0];
          double c5 = 0./*4*selfDGggcoupl[7][0]*/;
          Double_t fppReal = 1./sqrt(6.) * (c1/4.*2. + 2.*c2);
          Double_t fppImag = 1./sqrt(6.) * c5;
          Double_t fmmReal = 1./sqrt(6.) * (c1/4.*2. + 2.*c2);
          Double_t fmmImag = 1./sqrt(6.)* c5;
          Double_t fmpReal = 1./4.*c1*2.;
          Double_t fmpImag = 0;
          Double_t fpp = fppImag*fppImag + fppReal*fppReal;
          Double_t fmm = fmmImag*fmmImag + fmmReal*fmmReal;
          Double_t fmp = fmpImag*fmpImag + fmpReal*fmpReal;
          spin2Model->setTensorPolarization(1, 0.); // This is wrong in the strict sense of what "SelfDefine_spin2" is.
          spin2Model->setTensorPolarization(2, 2.*fmp/(fmm+fpp+2.*fmp));
        }
        else{
          spin2Model->setTensorPolarization(1, 0.);
          spin2Model->setTensorPolarization(2, 1.);
        }
      }
      spin2Model->makeParamsConst(true);
    }
  }
  else if (myME_ == TVar::ANALYTICAL){
    cout << "Mela::configureAnalyticalPDFs -> ERROR TVar::Process not applicable!!! ME: " << myME_ << ", model: " << myModel_ << endl;
    noPass=true;
  }

  return (!noPass);
}



// Constants to normalize probabilities
void Mela::getConstant(float& prob){ prob = getIORecord()->getMEConst(); }
void Mela::computeConstant(float& prob){
  float pConst=1.;
  setConstant();
  getConstant(pConst);
  prob *= pConst;
}
void Mela::setConstant(){
  float constant = 1;
  if (melaCand==0){ if (myVerbosity_>=TVar::DEBUG) cout << "Mela::getConstant: melaCand==0" << endl; }
  else{
    if ( // Undecayed Higgs MEs from JHUGen
      myME_ == TVar::JHUGen
      &&
      (
      myProduction_ == TVar::JQCD
      ||
      myProduction_ == TVar::JJQCD || myProduction_ == TVar::JJVBF
      ||
      myProduction_ == TVar::Lep_ZH || myProduction_ == TVar::Had_ZH
      ||
      myProduction_ == TVar::Lep_WH || myProduction_ == TVar::Had_WH
      ||
      myProduction_ == TVar::GammaH
      ||
      myProduction_ == TVar::ttH || myProduction_ == TVar::bbH
      )
      ) constant = getConstant_JHUGenUndecayed();
    else if ( // H->4l/2l2l
      melaCand->getSortedV(0)->getNDaughters()==2
      &&
      melaCand->getSortedV(1)->getNDaughters()==2
      &&
      PDGHelpers::isALepton(melaCand->getSortedV(0)->getDaughter(0)->id) && PDGHelpers::isALepton(melaCand->getSortedV(0)->getDaughter(1)->id)
      &&
      PDGHelpers::isALepton(melaCand->getSortedV(1)->getDaughter(0)->id) && PDGHelpers::isALepton(melaCand->getSortedV(1)->getDaughter(1)->id)
      ) constant = getConstant_4l();
    else if ( // H->2l2q
      melaCand->getSortedV(0)->getNDaughters()==2
      &&
      melaCand->getSortedV(1)->getNDaughters()==2
      &&
      (
      (
      PDGHelpers::isALepton(melaCand->getSortedV(0)->getDaughter(0)->id) && PDGHelpers::isALepton(melaCand->getSortedV(0)->getDaughter(1)->id)
      &&
      PDGHelpers::isAJet(melaCand->getSortedV(1)->getDaughter(0)->id) && PDGHelpers::isAJet(melaCand->getSortedV(1)->getDaughter(1)->id)
      )
      ||
      (
      PDGHelpers::isALepton(melaCand->getSortedV(1)->getDaughter(0)->id) && PDGHelpers::isALepton(melaCand->getSortedV(1)->getDaughter(1)->id)
      &&
      PDGHelpers::isAJet(melaCand->getSortedV(0)->getDaughter(0)->id) && PDGHelpers::isAJet(melaCand->getSortedV(0)->getDaughter(1)->id)
      )
      )
      ) constant = getConstant_2l2q();
  }
  if (std::isnan(constant) || std::isinf(constant) || constant<=0.) constant=0;
  else constant=1./constant;
  if (myVerbosity_>=TVar::DEBUG) cout << "Mela::getConstant: Constant is " << constant << endl;
  getIORecord()->setMEConst(constant);
}
float Mela::getConstant_JHUGenUndecayed(){
  float constant = 1;
  if (melaCand==0) return constant;

  MelaPConstant* pchandle=0;
  unsigned int iarray=0;
  double correction=1;

  if (myProduction_ == TVar::JQCD){
    if (TUtil::JetMassScheme == TVar::ConserveDifermionMass) iarray=0; // First element points to the case when the difermion invariant mass is conserved in mass removal scheme
    else if (TUtil::JetMassScheme == TVar::MomentumToEnergy) iarray=1; // Second element points to the case when the 3-momentum vector magnitude is scaled to energy in mass removal scheme
    pchandle = pAvgSmooth_JHUGen_JQCD_HSMHiggs[iarray];
  }
  else if (myProduction_ == TVar::JJQCD){
    if (TUtil::JetMassScheme == TVar::ConserveDifermionMass) iarray=0; // First element points to the case when the difermion invariant mass is conserved in mass removal scheme
    else if (TUtil::JetMassScheme == TVar::MomentumToEnergy) iarray=1; // Second element points to the case when the 3-momentum vector magnitude is scaled to energy in mass removal scheme
    pchandle = pAvgSmooth_JHUGen_JJQCD_HSMHiggs[iarray];
  }
  else if (myProduction_ == TVar::JJVBF){
    if (TUtil::JetMassScheme == TVar::ConserveDifermionMass) iarray=0; // First element points to the case when the difermion invariant mass is conserved in mass removal scheme
    else if (TUtil::JetMassScheme == TVar::MomentumToEnergy) iarray=1; // Second element points to the case when the 3-momentum vector magnitude is scaled to energy in mass removal scheme
    pchandle = pAvgSmooth_JHUGen_JJVBF_HSMHiggs[iarray];
  }
  /*
  else if (myProduction_ == TVar::Lep_ZH || myProduction_ == TVar::Had_ZH)
  else if (myProduction_ == TVar::Lep_WH || myProduction_ == TVar::Had_WH)
  else if (myProduction_ == TVar::GammaH)
  else if (myProduction_ == TVar::ttH)
  else if (myProduction_ == TVar::bbH)
  */
  else return constant;

  constant = pchandle->Eval(getIORecord(), myVerbosity_);
  if (myProduction_==TVar::JJVBF && LHCsqrts==7.){
    // Fitting is not good enough for region starting at ~105 GeV due to poor statistics.
    const double a0=0.67;
    const double a1=22.;
    const double a2=73.;
    double var = melaCand->m();
    if (var>a2) correction = 1.+a0*exp(-pow((var-a2)/a1, 2));
    else correction = 1.+a0; // Smooth by virtue of the correction function itself.
  }
  else if (myProduction_==TVar::JJVBF && LHCsqrts==8.){
    // Fitting is not good enough for region starting at ~105 GeV due to poor statistics.
    const double a0=0.53;
    const double a1=21.;
    const double a2=73.;
    double var = melaCand->m();
    if (var>a2) correction = 1.+a0*exp(-pow((var-a2)/a1, 2));
    else correction = 1.+a0; // Smooth by virtue of the correction function itself.
  }
  else if (myProduction_==TVar::JJVBF && LHCsqrts==13.){
    // Fitting is not good enough for region starting at ~105 GeV due to poor statistics.
    const double a0=0.2;
    const double a1=22.;
    const double a2=73.;
    double var = melaCand->m();
    if (var>a2) correction = 1.+a0*exp(-pow((var-a2)/a1, 2));
    else correction = 1.+a0; // Smooth by virtue of the correction function itself.
  }
  //
  else if (myProduction_==TVar::JJQCD && LHCsqrts==8.){
    // Fitting is not good enough for region starting at ~105 GeV due to poor statistics.
    const double a0=-0.24;
    const double a1=80.;
    const double a2=9.;
    const double a3=0.08;
    const double a4=100.;
    const double a5=20.;
    double var = melaCand->m();
    if (var>a1) correction = 1+a0*exp(-pow((var-a1)/a2, 2))+a3*exp(-pow((var-a4)/a5, 2));
    else correction = 1.+a0+a3*exp(-pow((var-a4)/a5, 2)); // Smooth by virtue of the correction function itself.
  }
  //
  else if (myProduction_==TVar::JQCD && LHCsqrts==7.){
    const double a0=-0.5;
    const double a1=80.;
    const double a2=9.;
    const double a3=-0.35352;
    const double a4=1500.;
    const double a5=268.;
    double var = melaCand->m();
    if (var>a1 && var<a4) correction = 1+a0*exp(-pow((var-a1)/a2, 2))+a3*exp(-pow((var-a4)/a5, 2));
    else if (var>a1) correction = 1+a0*exp(-pow((var-a1)/a2, 2))+a3;
    else correction = 1.+a0+a3*exp(-pow((var-a4)/a5, 2));
  }
  else if (myProduction_==TVar::JQCD && LHCsqrts==8.){
    const double a0=-0.2;
    const double a1=80.;
    const double a2=9.;
    const double a3=-0.0792;
    const double a4=1500.;
    const double a5=615.;
    double var = melaCand->m();
    if (var>a1 && var<a4) correction = 1+a0*exp(-pow((var-a1)/a2, 2))+a3*exp(-pow((var-a4)/a5, 2));
    else if (var>a1) correction = 1+a0*exp(-pow((var-a1)/a2, 2))+a3;
    else correction = 1.+a0+a3*exp(-pow((var-a4)/a5, 2));
  }
  else if (myProduction_==TVar::JQCD && LHCsqrts==13.){
    // 1+[0]*exp(-pow((x-[1])/[2],2))+[3]*exp(-pow((x-[4])/[5],2))
    const double a0=0.15;
    const double a1=320.;
    const double a2=300.;
    const double a3=0.179;
    const double a4=1530.;
    const double a5=212.;
    const double offset=0.8444;
    double var = melaCand->m();
    if (var<a4) correction = offset+a0*exp(-pow((var-a1)/a2, 2))+a3*exp(-pow((var-a4)/a5, 2));
    else correction = offset+a0*exp(-pow((var-a1)/a2, 2))+a3;
  }

  constant *= correction;
  return constant;
}
float Mela::getConstant_4l(){
  float constant = 1;
  if (melaCand==0) return constant;

  const unsigned int nPossibleHandles=2;
  MelaPConstant* pchandle[nPossibleHandles]={ 0 };
  const int idprod =
    abs(melaCand->getSortedV(0)->getDaughter(0)->id)*
    abs(melaCand->getSortedV(0)->getDaughter(1)->id)*
    abs(melaCand->getSortedV(1)->getDaughter(0)->id)*
    abs(melaCand->getSortedV(1)->getDaughter(1)->id);
  const bool is4mu = (idprod==28561);
  const bool is4e = (idprod==14641 || idprod==50625); // Use 4e for 4tau as well (I don't know why you would do this, but anyway
  const bool is2mu2e = (idprod==20449 || idprod==27225 || idprod==38025); // Use 2e2mu for 2e2tau and 2mu2tau as well

  float constant_tmp=0;
  if (myME_ == TVar::JHUGen){
    if (myProduction_ == TVar::ZZGG){
      if (
        myModel_==TVar::HSMHiggs
        ||
        myModel_==TVar::H0minus
        ||
        myModel_==TVar::H0hplus
        ||
        myModel_==TVar::H0_g1prime2
        ||
        myModel_==TVar::H0_Zgsg1prime2
        ||
        myModel_==TVar::H0_Zgs
        ||
        myModel_==TVar::H0_Zgs_PS
        ||
        myModel_==TVar::H0_gsgs
        ||
        myModel_==TVar::H0_gsgs_PS
        ||
        myModel_==TVar::SelfDefine_spin0
        ){
        if (is2mu2e) pchandle[0] = pAvgSmooth_JHUGen_ZZGG_HSMHiggs_2mu2e;
        else if (is4mu) pchandle[0] = pAvgSmooth_JHUGen_ZZGG_HSMHiggs_4mu;
        else if (is4e) pchandle[0] = pAvgSmooth_JHUGen_ZZGG_HSMHiggs_4e;
      }
    }
  }
  else if (myME_ == TVar::MCFM){
    if (myProduction_ == TVar::ZZQQB){
      if (myModel_ == TVar::bkgZZ){
        if (is2mu2e) pchandle[0] = pAvgSmooth_MCFM_ZZQQB_bkgZZ_2mu2e;
        else if (is4mu) pchandle[0] = pAvgSmooth_MCFM_ZZQQB_bkgZZ_4mu;
        else if (is4e) pchandle[0] = pAvgSmooth_MCFM_ZZQQB_bkgZZ_4e;
      }
    }
    else if (myProduction_ == TVar::ZZGG){
      if (myModel_ == TVar::bkgZZ){
        if (is2mu2e) pchandle[0] = pAvgSmooth_MCFM_ZZGG_bkgZZ_2mu2e;
        else if (is4mu) pchandle[0] = pAvgSmooth_MCFM_ZZGG_bkgZZ_4mu;
        else if (is4e) pchandle[0] = pAvgSmooth_MCFM_ZZGG_bkgZZ_4e;
      }
      else if (myModel_ == TVar::HSMHiggs){
        if (is2mu2e) pchandle[0] = pAvgSmooth_MCFM_ZZGG_HSMHiggs_2mu2e;
        else if (is4mu) pchandle[0] = pAvgSmooth_MCFM_ZZGG_HSMHiggs_4mu;
        else if (is4e) pchandle[0] = pAvgSmooth_MCFM_ZZGG_HSMHiggs_4e;
      }
      else if (myModel_ == TVar::bkgZZ_SMHiggs){
        if (is2mu2e){
          pchandle[0] = pAvgSmooth_MCFM_ZZGG_bkgZZ_2mu2e;
          pchandle[1] = pAvgSmooth_MCFM_ZZGG_HSMHiggs_2mu2e;
        }
        else if (is4mu){
          pchandle[0] = pAvgSmooth_MCFM_ZZGG_bkgZZ_4mu;
          pchandle[1] = pAvgSmooth_MCFM_ZZGG_HSMHiggs_4mu;
        }
        else if (is4e){
          pchandle[0] = pAvgSmooth_MCFM_ZZGG_bkgZZ_4e;
          pchandle[1] = pAvgSmooth_MCFM_ZZGG_HSMHiggs_4e;
        }
      }
    }
    else if (myProduction_ == TVar::JJQCD){
      if (myModel_ == TVar::bkgZJets){
        pchandle[0] = pAvgSmooth_MCFM_JJQCD_bkgZJets_2l2q; // Only option at the moment
      }
    }
  }

  bool hasNullHandle=true;
  for (unsigned int ihandle=0; ihandle<nPossibleHandles; ihandle++){ if (pchandle[ihandle]!=0){ constant_tmp += pchandle[ihandle]->Eval(getIORecord(), myVerbosity_); hasNullHandle=false; } }
  if (hasNullHandle) return constant;

  constant = constant_tmp;
  return constant;
}
float Mela::getConstant_2l2q(){
  float constant = 1;
  if (melaCand==0) return constant;

  const unsigned int nPossibleHandles=2;
  MelaPConstant* pchandle[nPossibleHandles]={ 0 };

  float constant_tmp=0;
  // Most constants use the 2e2mu constant. MelaPConstant scales for the left/right couplings itself based on what is recorded into the MelaIO object.
  if (myME_ == TVar::JHUGen){
    if (myProduction_ == TVar::ZZGG){
      if (myModel_ == TVar::HSMHiggs){
        pchandle[0] = pAvgSmooth_JHUGen_ZZGG_HSMHiggs_2mu2e;
      }
    }
  }
  else if (myME_ == TVar::MCFM){
    if (myProduction_ == TVar::ZZQQB){
      if (myModel_ == TVar::bkgZZ){
        pchandle[0] = pAvgSmooth_MCFM_ZZQQB_bkgZZ_2mu2e;
      }
    }
    else if (myProduction_ == TVar::ZZGG){
      if (myModel_ == TVar::bkgZZ){
        pchandle[0] = pAvgSmooth_MCFM_ZZGG_bkgZZ_2mu2e;
      }
      else if (myModel_ == TVar::HSMHiggs){
        pchandle[0] = pAvgSmooth_MCFM_ZZGG_HSMHiggs_2mu2e;
      }
      else if (myModel_ == TVar::bkgZZ_SMHiggs){
        pchandle[0] = pAvgSmooth_MCFM_ZZGG_bkgZZ_2mu2e;
        pchandle[1] = pAvgSmooth_MCFM_ZZGG_HSMHiggs_2mu2e;
      }
    }
    else if (myProduction_ == TVar::JJQCD){
      if (myModel_ == TVar::bkgZJets){
        pchandle[0] = pAvgSmooth_MCFM_JJQCD_bkgZJets_2l2q;
      }
    }
  }

  bool hasNullHandle=true;
  for (unsigned int ihandle=0; ihandle<nPossibleHandles; ihandle++){ if (pchandle[ihandle]!=0){ constant_tmp += pchandle[ihandle]->Eval(getIORecord(), myVerbosity_); hasNullHandle=false; } }
  if (hasNullHandle) return constant;

  constant = constant_tmp;
  return constant;
}
void Mela::getPConstantHandles(){
  if (myVerbosity_>=TVar::DEBUG) cout << "Begin Mela::getPConstantHandles" << endl;

  // Find closest sqrts allowable
  const unsigned int npossiblesqrts=3;
  const double possible_sqrts[npossiblesqrts]={ 7, 8, 13 };
  unsigned int sqrts_index=0;
  double sqrtsdiff = 99.; // Some large number
  for (unsigned isq=0; isq<npossiblesqrts; isq++){
    double diff = fabs(LHCsqrts-possible_sqrts[isq]);
    if (diff<sqrtsdiff){ sqrts_index=isq; sqrtsdiff=diff; }
  }
  const double chsqrts=possible_sqrts[sqrts_index];
  TString strsqrts=Form("%.0f%s", chsqrts, "TeV");

  // Initialize all to 0
  for (unsigned int isch=0; isch<(unsigned int)(TVar::nFermionMassRemovalSchemes-1); isch++){
    pAvgSmooth_JHUGen_JJQCD_HSMHiggs[isch]=0;
    //
    pAvgSmooth_JHUGen_JJVBF_HSMHiggs[isch]=0;
    //
    pAvgSmooth_JHUGen_JQCD_HSMHiggs[isch]=0;
    //
  }
  //
  pAvgSmooth_MCFM_JJQCD_bkgZJets_2l2q=0;
  //
  pAvgSmooth_JHUGen_ZZGG_HSMHiggs_4mu=0;
  pAvgSmooth_JHUGen_ZZGG_HSMHiggs_4e=0;
  pAvgSmooth_JHUGen_ZZGG_HSMHiggs_2mu2e=0;
  //
  pAvgSmooth_MCFM_ZZGG_HSMHiggs_4mu=0;
  pAvgSmooth_MCFM_ZZGG_HSMHiggs_4e=0;
  pAvgSmooth_MCFM_ZZGG_HSMHiggs_2mu2e=0;
  //
  pAvgSmooth_MCFM_ZZGG_bkgZZ_4mu=0;
  pAvgSmooth_MCFM_ZZGG_bkgZZ_4e=0;
  pAvgSmooth_MCFM_ZZGG_bkgZZ_2mu2e=0;
  //
  pAvgSmooth_MCFM_ZZQQB_bkgZZ_4mu=0;
  pAvgSmooth_MCFM_ZZQQB_bkgZZ_4e=0;
  pAvgSmooth_MCFM_ZZQQB_bkgZZ_2mu2e=0;


  TString filename, spname;

  for (unsigned int isch=0; isch<(unsigned int)(TVar::nFermionMassRemovalSchemes-1); isch++){
    filename = Form("pAvgSmooth_JHUGen_JJQCD_HSMHiggs_%s", strsqrts.Data());
    spname = "P_ConserveDifermionMass";
    pAvgSmooth_JHUGen_JJQCD_HSMHiggs[0] = getPConstantHandle(TVar::JHUGen, TVar::JJQCD, TVar::HSMHiggs, filename.Data(), spname.Data());
    spname = "P_MomentumToEnergy";
    pAvgSmooth_JHUGen_JJQCD_HSMHiggs[1] = getPConstantHandle(TVar::JHUGen, TVar::JJQCD, TVar::HSMHiggs, filename.Data(), spname.Data());
    //
    filename = Form("pAvgSmooth_JHUGen_JJVBF_HSMHiggs_%s", strsqrts.Data());
    spname = "P_ConserveDifermionMass";
    pAvgSmooth_JHUGen_JJVBF_HSMHiggs[0] = getPConstantHandle(TVar::JHUGen, TVar::JJVBF, TVar::HSMHiggs, filename.Data(), spname.Data());
    spname = "P_MomentumToEnergy";
    pAvgSmooth_JHUGen_JJVBF_HSMHiggs[1] = getPConstantHandle(TVar::JHUGen, TVar::JJVBF, TVar::HSMHiggs, filename.Data(), spname.Data());
    //
    filename = Form("pAvgSmooth_JHUGen_JQCD_HSMHiggs_%s", strsqrts.Data());
    spname = "P_ConserveDifermionMass";
    pAvgSmooth_JHUGen_JQCD_HSMHiggs[0] = getPConstantHandle(TVar::JHUGen, TVar::JQCD, TVar::HSMHiggs, filename.Data(), spname.Data());
    spname = "P_MomentumToEnergy";
    pAvgSmooth_JHUGen_JQCD_HSMHiggs[1] = getPConstantHandle(TVar::JHUGen, TVar::JQCD, TVar::HSMHiggs, filename.Data(), spname.Data());
  }
  //
  filename = "pAvgSmooth_MCFM_JJQCD_bkgZJets_13TeV_2l2q"; // 13 TeV is a placeholder for all energies.
  spname = "P_ConserveDifermionMass";
  pAvgSmooth_MCFM_JJQCD_bkgZJets_2l2q = getPConstantHandle(TVar::MCFM, TVar::JJQCD, TVar::bkgZJets, filename.Data(), spname.Data());
  //
  filename = "pAvgSmooth_JHUGen_ZZGG_HSMHiggs";
  spname = "P_ConserveDifermionMass_4mu";
  pAvgSmooth_JHUGen_ZZGG_HSMHiggs_4mu = getPConstantHandle(TVar::JHUGen, TVar::ZZGG, TVar::HSMHiggs, filename.Data(), spname.Data());
  spname = "P_ConserveDifermionMass_4e";
  pAvgSmooth_JHUGen_ZZGG_HSMHiggs_4e = getPConstantHandle(TVar::JHUGen, TVar::ZZGG, TVar::HSMHiggs, filename.Data(), spname.Data());
  spname = "P_ConserveDifermionMass_2mu2e";
  pAvgSmooth_JHUGen_ZZGG_HSMHiggs_2mu2e = getPConstantHandle(TVar::JHUGen, TVar::ZZGG, TVar::HSMHiggs, filename.Data(), spname.Data());
  //
  filename = "pAvgSmooth_MCFM_ZZGG_HSMHiggs";
  spname = "P_ConserveDifermionMass_4mu";
  pAvgSmooth_MCFM_ZZGG_HSMHiggs_4mu = getPConstantHandle(TVar::MCFM, TVar::ZZGG, TVar::HSMHiggs, filename.Data(), spname.Data());
  spname = "P_ConserveDifermionMass_4e";
  pAvgSmooth_MCFM_ZZGG_HSMHiggs_4e = getPConstantHandle(TVar::MCFM, TVar::ZZGG, TVar::HSMHiggs, filename.Data(), spname.Data());
  spname = "P_ConserveDifermionMass_2mu2e";
  pAvgSmooth_MCFM_ZZGG_HSMHiggs_2mu2e = getPConstantHandle(TVar::MCFM, TVar::ZZGG, TVar::HSMHiggs, filename.Data(), spname.Data());
  //
  filename = "pAvgSmooth_MCFM_ZZGG_bkgZZ";
  spname = "P_ConserveDifermionMass_4mu";
  pAvgSmooth_MCFM_ZZGG_bkgZZ_4mu = getPConstantHandle(TVar::MCFM, TVar::ZZGG, TVar::bkgZZ, filename.Data(), spname.Data());
  spname = "P_ConserveDifermionMass_4e";
  pAvgSmooth_MCFM_ZZGG_bkgZZ_4e = getPConstantHandle(TVar::MCFM, TVar::ZZGG, TVar::bkgZZ, filename.Data(), spname.Data());
  spname = "P_ConserveDifermionMass_2mu2e";
  pAvgSmooth_MCFM_ZZGG_bkgZZ_2mu2e = getPConstantHandle(TVar::MCFM, TVar::ZZGG, TVar::bkgZZ, filename.Data(), spname.Data());
  //
  filename = "pAvgSmooth_MCFM_ZZQQB_bkgZZ";
  spname = "P_ConserveDifermionMass_4mu";
  pAvgSmooth_MCFM_ZZQQB_bkgZZ_4mu = getPConstantHandle(TVar::MCFM, TVar::ZZQQB, TVar::bkgZZ, filename.Data(), spname.Data());
  spname = "P_ConserveDifermionMass_4e";
  pAvgSmooth_MCFM_ZZQQB_bkgZZ_4e = getPConstantHandle(TVar::MCFM, TVar::ZZQQB, TVar::bkgZZ, filename.Data(), spname.Data());
  spname = "P_ConserveDifermionMass_2mu2e";
  pAvgSmooth_MCFM_ZZQQB_bkgZZ_2mu2e = getPConstantHandle(TVar::MCFM, TVar::ZZQQB, TVar::bkgZZ, filename.Data(), spname.Data());
  //

  if (myVerbosity_>=TVar::DEBUG) cout << "End Mela::getPConstantHandles" << endl;
}
MelaPConstant* Mela::getPConstantHandle(
  TVar::MatrixElement me_,
  TVar::Production prod_,
  TVar::Process proc_,
  const char* relpath,
  const char* spname
  ){
  if (myVerbosity_>=TVar::DEBUG) cout << "Begin Mela::getPConstantHandle" << endl;

  if (myVerbosity_>=TVar::DEBUG) cout << "Mela::getPConstantHandle: relpath and spline name: " << relpath << ", " << spname << endl;
#ifdef _melapkgpathstr_
  const string MELAPKGPATH = _melapkgpathstr_;
#else
  cout << "Mela::getPConstantHandle: MELA package path is undefined! Please modify the makefle or the makefile-equivalent!" << endl;
  assert(0);
#endif
  const string path = MELAPKGPATH + "data/";
  string cfile_fullpath = path;
  cfile_fullpath.append(relpath);
  cfile_fullpath.append(".root");
  if (myVerbosity_>=TVar::DEBUG) cout << "Mela::getPConstantHandle: path and spline name: " << path << ", " << spname << endl;
  if (myVerbosity_>=TVar::DEBUG) cout << "Mela::getPConstantHandle: Full path and spline name: " << cfile_fullpath << ", " << spname << endl;
  MelaPConstant* pchandle = new MelaPConstant(me_, prod_, proc_, cfile_fullpath.c_str(), spname);

  if (myVerbosity_>=TVar::DEBUG) cout << "End Mela::getPConstantHandle" << endl;
  return pchandle;
}
void Mela::deletePConstantHandles(){
  for (unsigned int isch=0; isch<(unsigned int)(TVar::nFermionMassRemovalSchemes-1); isch++){
    if (pAvgSmooth_JHUGen_JJQCD_HSMHiggs[isch]!=0) delete pAvgSmooth_JHUGen_JJQCD_HSMHiggs[isch];
    //
    if (pAvgSmooth_JHUGen_JJVBF_HSMHiggs[isch]!=0) delete pAvgSmooth_JHUGen_JJVBF_HSMHiggs[isch];
    //
    if (pAvgSmooth_JHUGen_JQCD_HSMHiggs[isch]!=0) delete pAvgSmooth_JHUGen_JQCD_HSMHiggs[isch];
    //
  }
  //
  if (pAvgSmooth_MCFM_JJQCD_bkgZJets_2l2q!=0) delete pAvgSmooth_MCFM_JJQCD_bkgZJets_2l2q;
  //
  if (pAvgSmooth_JHUGen_ZZGG_HSMHiggs_4mu!=0) delete pAvgSmooth_JHUGen_ZZGG_HSMHiggs_4mu;
  if (pAvgSmooth_JHUGen_ZZGG_HSMHiggs_4e!=0) delete pAvgSmooth_JHUGen_ZZGG_HSMHiggs_4e;
  if (pAvgSmooth_JHUGen_ZZGG_HSMHiggs_2mu2e!=0) delete pAvgSmooth_JHUGen_ZZGG_HSMHiggs_2mu2e;
  //
  if (pAvgSmooth_MCFM_ZZGG_HSMHiggs_4mu!=0) delete pAvgSmooth_MCFM_ZZGG_HSMHiggs_4mu;
  if (pAvgSmooth_MCFM_ZZGG_HSMHiggs_4e!=0) delete pAvgSmooth_MCFM_ZZGG_HSMHiggs_4e;
  if (pAvgSmooth_MCFM_ZZGG_HSMHiggs_2mu2e!=0) delete pAvgSmooth_MCFM_ZZGG_HSMHiggs_2mu2e;
  //
  if (pAvgSmooth_MCFM_ZZGG_bkgZZ_4mu!=0) delete pAvgSmooth_MCFM_ZZGG_bkgZZ_4mu;
  if (pAvgSmooth_MCFM_ZZGG_bkgZZ_4e!=0) delete pAvgSmooth_MCFM_ZZGG_bkgZZ_4e;
  if (pAvgSmooth_MCFM_ZZGG_bkgZZ_2mu2e!=0) delete pAvgSmooth_MCFM_ZZGG_bkgZZ_2mu2e;
  //
  if (pAvgSmooth_MCFM_ZZQQB_bkgZZ_4mu!=0) delete pAvgSmooth_MCFM_ZZQQB_bkgZZ_4mu;
  if (pAvgSmooth_MCFM_ZZQQB_bkgZZ_4e!=0) delete pAvgSmooth_MCFM_ZZQQB_bkgZZ_4e;
  if (pAvgSmooth_MCFM_ZZQQB_bkgZZ_2mu2e!=0) delete pAvgSmooth_MCFM_ZZQQB_bkgZZ_2mu2e;
  //
}

