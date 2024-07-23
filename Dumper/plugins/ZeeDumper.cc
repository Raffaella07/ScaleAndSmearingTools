// -*- C++ -*-
//
// Package:    Electron_GNN_Regression/ZeeDumper
// Class:      ZeeDumper
//
/**\class ZeeDumper ZeeDumper.cc Electron_GNN_Regression/ZeeDumper/plugins/ZeeDumper.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Rajdeep Mohan Chatterjee
//         Created:  Fri, 21 Feb 2020 11:38:58 GMT
//
//


// system include files
#include <memory>
#include <iostream>
#include "TTree.h"
#include "Math/VectorUtil.h"
#include "TFile.h"
// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"


#include "Geometry/EcalAlgo/interface/EcalPreshowerGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
//#include "Geometry/CaloEventSetup/interface/CaloTopologyRecord.h"
//#include "Geometry/CaloTopology/interface/CaloTopology.h"

#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/Photon.h"

#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/PatCandidates/interface/VIDCutFlowResult.h"

#include "DataFormats/EgammaReco/interface/SuperCluster.h"

#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"

#include "RecoEcal/EgammaCoreTools/interface/EcalClusterTools.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "RecoEcal/EgammaCoreTools/interface/PositionCalc.h"

#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalDetId/interface/ESDetId.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"

#include "ScaleAndSmearingTools/Dumper/interface/eleIDMap.h"
#include "DataFormats/Common/interface/ValueMap.h"

////////////// Trigger stuff ///////////////////////
#include "FWCore/Framework/interface/TriggerNamesService.h"
#include <FWCore/Common/interface/TriggerNames.h>
#include <DataFormats/Common/interface/TriggerResults.h>
////////////////////////////////////////////////////


#define NELE 3
#define initSingleFloat     -999.
#define initSingleInt          0
#define initSingleIntCharge -100
#define initFloat     { initSingleFloat, initSingleFloat, initSingleFloat }
#define initInt       { initSingleInt, initSingleInt, initSingleInt }
#define initIntCharge { initSingleIntCharge, initSingleIntCharge, initSingleIntCharge }
#define PDGID 11

//using reco::TrackCollection;

class ZeeDumper : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
   public:
      explicit ZeeDumper(const edm::ParameterSet&);
      ~ZeeDumper();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      virtual void beginJob() override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;



// Set event specific information
    void TreeSetEventSummaryVar(const edm::Event& iEvent);
    void TreeSetPileupVar(void);

//   clear the vectors 
     void InitNewTree(void);
     void ResetMainTreeVar();

// Helper functions to fill the trees
     void TreeSetDiElectronVar(const pat::Electron& electron1, const pat::Electron& electron2);
     void TreeSetSingleElectronVar(const pat::Electron& electron1, int index);
     float InvMass(double eta1, double eta2, double phi1, double phi2, double energy1, double energy2);
     float GetMustEnergy(const reco::GsfElectron& electron, bool isEB);
     UChar_t GetSeedGain(const DetId& seedDetId, int index);
     UInt_t GetID(const pat::Electron& electron);

     bool isMC_;
     bool isAOD_;
// ----------member data ---------------------------
     TTree * _tree;                   //< output file for standard ntuple
     edm::Timestamp _eventTimeStamp;

     // Variables for Run info.
     UInt_t    _runNumber;     ///< run number
     UShort_t  _lumiBlock;     ///< lumi section
     Long64_t  _eventNumber;   ///< event number
     UInt_t    _eventTime;     ///< unix time of the event
     UShort_t  _nBX;           ///< bunch crossing
     Bool_t    _isTrain;

     Float_t   _mcGenWeight;   ///< weight in generator for MC

     std::vector< std::string >  _HLTNames[1];   ///< List of HLT names
     std::vector<Bool_t>         _HLTResults[1]; ///< 0 = fail, 1=fire
     std::map<std::string, bool> _HLTBits;
     Bool_t                      _HLTfire;       ///< true if pass the triggers indicated by hltPaths in cfg
 

     // pileup
     Float_t  _rho;    ///< _rho fast jet
     UChar_t  _nPV;    ///< nVtx
     UChar_t  _nPU;    ///< number of PU (filled only for MC)


     // electron variables
     UChar_t _gainSeedSC[NELE]    = {9, 9, 9}; 
     UInt_t _eleID[NELE] = initInt;      ///< bit mask for _eleID: 1=fiducial, 2=loose, 6=medium, 14=tight, 16=WP90PU, 48=WP80PU, 112=WP70PU, 128=loose25nsRun2, 384=medium25nsRun2, 896=tight25nsRun2, 1024=loose50nsRun2, 3072=medium50nsRun2, 7168=tight50nsRun2. Selection from https://twiki.cern.ch/twiki/bin/viewauth/CMS/EgammaCutBasedIdentification#Electron_ID_Working_Points
     Short_t  _chargeEle[NELE]    = initIntCharge; ///< -100: no electron, 0: SC or photon, -1 or +1:electron or muon //Char_t is interpreted as char and not as integer
     UChar_t  _recoFlagsEle[NELE] = initInt;       ///< 1=trackerDriven, 2=ecalDriven only, 3=tracker and ecal driven
     Float_t  _etaEle[NELE]       = initFloat;
     Float_t  _phiEle[NELE]       = initFloat;     ///< phi of the electron (electron object)
     Float_t  _R9Ele[NELE]        = initFloat;     ///< e3x3/_rawEnergySCEle

     //ID variables
     std::string _eleID_loose, _eleID_medium, _eleID_tight;

     // SC variables
     Float_t _etaSCEle[NELE]    = initFloat; ///< eta of the SC
     Float_t _phiSCEle[NELE]    = initFloat; ///< phi of the SC
     Float_t _isMustacheSC[NELE]    = initInt;
     Float_t _mustEnergySCEle[NELE]    = initFloat;
     Short_t _xSeedSC[NELE]                    = initInt;    ///< ieta(ix) of the SC seed in EB(EE)
     Short_t _ySeedSC[NELE]                    = initInt;    ///< iphi(iy) of the SC seed in EB(EE)
     Float_t _energyEle[NELE]                  = initFloat;
     Float_t _esEnergySCEle[NELE]              = initFloat;
     Float_t _rawEnergySCEle[NELE]             = initFloat;  ///< SC energy without cluster corrections
     Float_t _energy_5x5SC[NELE]               = initFloat; 
     Float_t _energy_ECAL_ele[NELE]            = initFloat;  ///< ele-tuned regression energy (no E-P combination)	
     Float_t _energy_ECAL_pho[NELE]            = initFloat; 	
     Float_t _invMass = initSingleFloat;
     Float_t _invMass_5x5SC = initSingleFloat;
     Float_t _invMass_ECAL_ele = initSingleFloat;
     Float_t _invMass_ECAL_pho = initSingleFloat;
     Float_t _invMass_rawSC = initSingleFloat;
     Float_t _invMass_rawSC_esSC = initSingleFloat;

     // Gen part 4 vectors
     std::vector<float> Gen_Pt;
     std::vector<float> Gen_Eta;
     std::vector<float> Gen_Phi;
     std::vector<float> Gen_E;



      // -----------------Handles--------------------------
      edm::Handle<reco::VertexCollection> primaryVertexHandle;  	
      edm::Handle<double> rhoHandle;
      edm::Handle<std::vector< PileupSummaryInfo > > PupInfo;
      edm::Handle<edm::View<reco::GenParticle> > genParticlesHandle;
      edm::Handle< GenEventInfoProduct >             _GenEventInfoHandle;
      edm::Handle<EcalRecHitCollection> EBRechitsHandle;
      edm::Handle<EcalRecHitCollection> EERechitsHandle;
      edm::Handle<EcalRecHitCollection> ESRechitsHandle;
      edm::Handle<std::vector<reco::SuperCluster>> EBSuperClustersHandle;
      edm::Handle<std::vector<reco::SuperCluster>> EESuperClustersHandle;
      edm::Handle<std::vector<pat::Electron> > electronsHandle; 
      edm::Handle<edm::ValueMap<float> > energySCElePhoMapHandle;
      edm::Handle<edm::TriggerResults>   _triggerResultsHandle;

      //---------------- Input Tags-----------------------
      edm::EDGetTokenT<GenEventInfoProduct>    _generatorInfoToken;
      edm::EDGetTokenT<reco::VertexCollection> vtxCollectionToken_;
      edm::EDGetTokenT<double> rhoToken_;
      edm::EDGetTokenT<std::vector<PileupSummaryInfo> > pileupInfoToken_;
      std::vector< std::string> hltPaths;
      edm::EDGetTokenT<EcalRecHitCollection> recHitCollectionEBToken_;
      edm::EDGetTokenT<EcalRecHitCollection> recHitCollectionEEToken_;
      edm::EDGetTokenT<EcalRecHitCollection> recHitCollectionESToken_;	
      edm::EDGetTokenT<std::vector<reco::SuperCluster> > EBSuperClustersToken_;
      edm::EDGetTokenT<std::vector<reco::SuperCluster> > EESuperClustersToken_; 
      edm::EDGetTokenT<std::vector<pat::Electron> > electronsToken_;
      edm::EDGetTokenT<edm::View<reco::GenParticle> > genParticlesToken_;
      edm::EDGetTokenT<edm::ValueMap<float> > energySCElePhoMapToken_;
      edm::InputTag triggerResultsTAG;
      edm::EDGetTokenT<edm::TriggerResults>   _triggerResultsToken;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
ZeeDumper::ZeeDumper(const edm::ParameterSet& iConfig):
	_eleID_loose(iConfig.getParameter< std::string>("eleID_loose")),
        _eleID_medium(iConfig.getParameter< std::string>("eleID_medium")),
        _eleID_tight(iConfig.getParameter< std::string>("eleID_tight")),
	_generatorInfoToken(consumes<GenEventInfoProduct>(edm::InputTag("generator"))),
	hltPaths(iConfig.getParameter< std::vector<std::string> >("hltPaths")),
	_triggerResultsToken(consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("triggerResultsCollection")))
{
   //now do what ever initialization is needed
   isMC_                    = iConfig.getParameter<bool>("isMC"); 
   isAOD_                   = iConfig.getParameter<bool>("isAOD"); 
   genParticlesToken_       = consumes<edm::View<reco::GenParticle> >(iConfig.getParameter<edm::InputTag>("genParticles"));
   pileupInfoToken_         = consumes<std::vector<PileupSummaryInfo> >(iConfig.getParameter<edm::InputTag>("pileupInfo"));
   rhoToken_                = consumes<double>(iConfig.getParameter<edm::InputTag>("rhoFastJet"));
   vtxCollectionToken_      = consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertexCollection"));
   recHitCollectionEBToken_ = consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("ecalRecHitsEB"));
   recHitCollectionEEToken_ = consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("ecalRecHitsEE"));
   recHitCollectionESToken_ = consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("ecalRecHitsES"));
   EBSuperClustersToken_    = consumes<reco::SuperClusterCollection>(edm::InputTag("superClustersEB"));
   EESuperClustersToken_    = consumes<reco::SuperClusterCollection>(edm::InputTag("superClustersEE"));
   electronsToken_          = consumes<std::vector<pat::Electron> >(iConfig.getParameter<edm::InputTag>("electrons"));
   energySCElePhoMapToken_  = consumes<edm::ValueMap<float> >(iConfig.getParameter<edm::InputTag>("energySCElePhoMap"));
   usesResource("TFileService");
}


ZeeDumper::~ZeeDumper()
{

   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)




}


//
// member functions
//

// ------------ method called for each event  ------------
void
ZeeDumper::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;
   using namespace std;
   using namespace reco;

   ResetMainTreeVar();

   _chargeEle[0] = initSingleIntCharge;
   _chargeEle[1] = initSingleIntCharge;
   _chargeEle[2] = initSingleIntCharge;

   if(isMC_){
	iEvent.getByToken(_generatorInfoToken, _GenEventInfoHandle);   
   	iEvent.getByToken(pileupInfoToken_, PupInfo);
   	iEvent.getByToken(genParticlesToken_, genParticlesHandle);
   }
   iEvent.getByToken(rhoToken_, rhoHandle);
   iEvent.getByToken(vtxCollectionToken_, primaryVertexHandle);
   iEvent.getByToken(recHitCollectionEBToken_, EBRechitsHandle);
   iEvent.getByToken(recHitCollectionEEToken_, EERechitsHandle);
   iEvent.getByToken(recHitCollectionESToken_, ESRechitsHandle);
   iEvent.getByToken(EBSuperClustersToken_, EBSuperClustersHandle);
   iEvent.getByToken(EESuperClustersToken_, EESuperClustersHandle);

   iEvent.getByToken(electronsToken_, electronsHandle);
   iEvent.getByToken(energySCElePhoMapToken_, energySCElePhoMapHandle);
   iEvent.getByToken(_triggerResultsToken, _triggerResultsHandle);

   TreeSetEventSummaryVar(iEvent);
   TreeSetPileupVar();

// at least one of the triggers
        _HLTfire = false;
        if(!hltPaths.empty()) {
		for(std::vector<std::string>::const_iterator hltPath_itr = hltPaths.begin();
                        hltPath_itr != hltPaths.end();
                        hltPath_itr++) {

			if(hltPath_itr->empty()) continue;
                        std::map<std::string, bool>::const_iterator it = _HLTBits.find(*hltPath_itr);
			if(it != _HLTBits.end()) {
				_HLTfire += it->second;
			}

		}

	}


///////////////////////////Fill Electron/Photon related stuff/////////////////////////////////////////////////////

   bool doFill = false;
   //std::cout<< "nElectrons: " << electronsHandle->size() << std::endl;


   for( pat::ElectronCollection::const_iterator eleIter1 = electronsHandle->begin(); eleIter1 != electronsHandle->end(); eleIter1++) {

        if( eleIter1->pt() < 15. ) continue;
        if(!eleIter1->ecalDrivenSeed()) continue;
	if(eleIter1->superCluster().isNull() && eleIter1->parentSuperCluster().isNull()) continue;

	for(pat::ElectronCollection::const_iterator eleIter2 = eleIter1 + 1; eleIter2 != electronsHandle->end() && doFill == false; eleIter2++) {
             if( eleIter2->pt() < 15. ) continue;		
	     if(!eleIter2->ecalDrivenSeed()) continue;	
	     if(eleIter2->superCluster().isNull() && eleIter2->parentSuperCluster().isNull()) continue;

             double mass = InvMass(eleIter1->eta(),eleIter2->eta(),eleIter1->phi(),eleIter2->phi(),eleIter1->energy(),eleIter2->energy());         
             if(mass < 55 ) continue;
             doFill = true;
             TreeSetDiElectronVar(*eleIter1, *eleIter2);	     
 
	}// loop over eleiter2 ends here
   }// loop over eleiTer1 ends here
        
   _invMass = InvMass(_etaEle[0],_etaEle[1],_phiEle[0],_phiEle[1],_energyEle[0],_energyEle[1]);   
   _invMass_5x5SC = InvMass(_etaEle[0],_etaEle[1],_phiEle[0],_phiEle[1],_energy_5x5SC[0],_energy_5x5SC[1]);   
   _invMass_ECAL_ele = InvMass(_etaEle[0],_etaEle[1],_phiEle[0],_phiEle[1],_energy_ECAL_ele[0],_energy_ECAL_ele[1]);   
   _invMass_ECAL_pho = InvMass(_etaEle[0],_etaEle[1],_phiEle[0],_phiEle[1],_energy_ECAL_pho[0],_energy_ECAL_pho[1]);  
   _invMass_rawSC = InvMass(_etaEle[0],_etaEle[1],_phiEle[0],_phiEle[1],_rawEnergySCEle[0],_rawEnergySCEle[1]);  
   _invMass_rawSC_esSC = InvMass(_etaEle[0],_etaEle[1],_phiEle[0],_phiEle[1],(_rawEnergySCEle[0]+_esEnergySCEle[0]),(_rawEnergySCEle[1]+_esEnergySCEle[1]));  

//////////////////////// Gen Stuff hardcoded for status 1 electrons /////////////////////////////////////
    if(isMC_){
       for(edm::View<GenParticle>::const_iterator part = genParticlesHandle->begin(); part != genParticlesHandle->end(); ++part){
           if( part->status()==1 && abs(part->pdgId())==PDGID ){
                Gen_Pt.push_back(part->pt());
                Gen_Eta.push_back(part->eta());
                Gen_Phi.push_back(part->phi());
                Gen_E.push_back(part->energy());
           }
       }
    } 
////////////////////////////////////////////////////////////////////////////////////////////////////////

   if(doFill) _tree->Fill(); // Write out the events

#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
   ESHandle<SetupData> pSetup;
   iSetup.get<SetupRecord>().get(pSetup);
#endif
}


// ------------ method called once each job just before starting event loop  ------------
void
ZeeDumper::beginJob()
{
        edm::Service<TFileService> fs;
        _tree=fs->make<TTree>("selected", "selected");
	InitNewTree();
}

// ------------ method called once each job just after ending the event loop  ------------
void
ZeeDumper::endJob()
{
}




//Clear tree vectors each time analyze method is called

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
ZeeDumper::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);

  //Specify that only 'tracks' is allowed
  //To use, remove the default given above and uncomment below
  //ParameterSetDescription desc;
  //desc.addUntracked<edm::InputTag>("tracks","ctfWithMaterialTracks");
  //descriptions.addDefault(desc);
}

void ZeeDumper::TreeSetDiElectronVar(const pat::Electron& electron1, const pat::Electron& electron2){
	
	TreeSetSingleElectronVar(electron1, 0);
        TreeSetSingleElectronVar(electron2, 1);

}

void ZeeDumper::TreeSetSingleElectronVar(const pat::Electron& electron, int index){

	if(index < 0) {
                _chargeEle[-index] = -100;
                _etaEle[-index]    = 0;
                _phiEle[-index]    = 0;
                _recoFlagsEle[-index] = -1;
                return;
        }

	assert(electron.ecalDrivenSeed());

	_chargeEle[index] = (Char_t)electron.charge();
	_etaEle[index]    = electron.eta();
	_phiEle[index]    = electron.phi();
	
	if(electron.ecalDrivenSeed()) {
                if(electron.trackerDrivenSeed()) _recoFlagsEle[index] = 3;
                else _recoFlagsEle[index] = 2;
        } else _recoFlagsEle[index] = 1;

	_R9Ele[index] = electron.full5x5_r9();
        _eleID[index] = GetID(electron);
	
        _energyEle[index] = electron.energy(); 
        _energy_5x5SC[index] = electron.full5x5_e5x5();
        _energy_ECAL_ele[index] = electron.correctedEcalEnergy();
        _energy_ECAL_pho[index] = electron.userFloat("energySCElePho");

        //std::cout << "TreeSetSingleElectronVar: " << index << std::endl;
        
        _isMustacheSC[index] = !electron.superCluster().isNonnull();
        assert(_isMustacheSC);
	const reco::SuperClusterRef& sc = _isMustacheSC[index] ?  electron.parentSuperCluster() : electron.superCluster();
	assert(sc.isNonnull()); // at least one of the SCs has to be defined!

        DetId seedDetId = sc->seed()->seed();
        assert(!seedDetId.null());
        bool isEB = (seedDetId.subdetId() == EcalBarrel);
           
	_etaSCEle[index] = sc->eta();
        _phiSCEle[index] = sc->phi();
	_rawEnergySCEle[index] = sc->rawEnergy();
        _esEnergySCEle[index] = sc->preshowerEnergy();  
        _gainSeedSC[index] = GetSeedGain(seedDetId,index);

        if(isEB) {
           EBDetId seedDetIdEcal(seedDetId);
           _xSeedSC[index] = seedDetIdEcal.ieta();
           _ySeedSC[index] = seedDetIdEcal.iphi();
	} else {
           EEDetId seedDetIdEcal(seedDetId);
           _xSeedSC[index] = seedDetIdEcal.ix();
           _ySeedSC[index] = seedDetIdEcal.iy();
	}

        if(_isMustacheSC[index])
           _mustEnergySCEle[index] = GetMustEnergy(electron, isEB);
}



void ZeeDumper::InitNewTree()
{
	std::cout << "[STATUS] InitNewTree" << std::endl;
	if(_tree == NULL) return;
        _tree->Branch("runNumber",     &_runNumber,   "runNumber/i");
        _tree->Branch("lumiBlock",     &_lumiBlock,   "lumiBlock/s");
        _tree->Branch("eventNumber",   &_eventNumber, "eventNumber/l");
        _tree->Branch("eventTime",       &_eventTime,     "eventTime/i");
        _tree->Branch("nBX",           &_nBX,         "nBX/s");
        _tree->Branch("isTrain",        &_isTrain,      "isTrain/B"); 

	_tree->Branch("mcGenWeight",   &_mcGenWeight, "mcGenWeight/F");

	_tree->Branch("HLTfire", &_HLTfire, "HLTfire/B");
        //_tree->Branch("HLTNames",&(_HLTNames[0]));
        //_tree->Branch("HLTResults",&(_HLTResults[0]));

	_tree->Branch("rho", &_rho, "rho/F");
        _tree->Branch("nPV", &_nPV, "nPV/b");
        if(isMC_) _tree->Branch("nPU", &_nPU, "nPU/b");

        _tree->Branch("eleID", _eleID, "eleID[3]/i");
        _tree->Branch("chargeEle",   _chargeEle,    "chargeEle[3]/S");
        _tree->Branch("recoFlagsEle", _recoFlagsEle, "recoFlagsEle[3]/b");
        _tree->Branch("gainSeedSC", _gainSeedSC, "gainSeedSC[3]/b");
        _tree->Branch("etaEle",      _etaEle,       "etaEle[3]/F");
        _tree->Branch("phiEle",      _phiEle,       "phiEle[3]/F");
        _tree->Branch("etaSCEle",      _etaSCEle,       "etaSCEle[3]/F");
        _tree->Branch("phiSCEle",      _phiSCEle,       "phiSCEle[3]/F");
        _tree->Branch("xSeedSC",            _xSeedSC,            "xSeedSC[3]/S");
        _tree->Branch("ySeedSC",            _ySeedSC,            "ySeedSC[3]/S");
        _tree->Branch("R9Ele", _R9Ele, "R9Ele[3]/F");
        _tree->Branch("rawEnergySCEle", _rawEnergySCEle, "rawEnergySCEle[3]/F");
        _tree->Branch("esEnergySCEle", _esEnergySCEle, "esEnergySCEle[3]/F");
        _tree->Branch("energyEle", _energyEle, "energyEle[3]/F");
        _tree->Branch("energy_5x5SC", _energy_5x5SC, "energy_5x5SC[3]/F");
        _tree->Branch("energy_ECAL_ele", _energy_ECAL_ele, "energy_ECAL_ele[3]/F"); ///< correctedEcalEnergy from MINIAOD or mustache regression if rereco
        _tree->Branch("energy_ECAL_pho", _energy_ECAL_pho, "energy_ECAL_pho[3]/F"); 
        _tree->Branch("isMustacheSC", _isMustacheSC, "isMustacheSC[3]/b"); 
        _tree->Branch("mustEnergySCEle", _mustEnergySCEle, "mustEnergySCEle[3]/F");
        _tree->Branch("invMass", &_invMass, "invMass/f");  
        _tree->Branch("invMass_5x5SC", &_invMass_5x5SC, "invMass_5x5SC/f");  
        _tree->Branch("invMass_ECAL_ele", &_invMass_ECAL_ele, "invMass_ECAL_ele/f");  
        _tree->Branch("invMass_ECAL_pho", &_invMass_ECAL_pho, "invMass_ECAL_pho/f");  
        _tree->Branch("invMass_rawSC", &_invMass_rawSC, "invMass_rawSC/f");  
        _tree->Branch("invMass_rawSC_esSC", &_invMass_rawSC_esSC, "invMass_rawSC_esSC/f");   
        if(isMC_){
           _tree->Branch("Gen_Pt" , &Gen_Pt);
           _tree->Branch("Gen_Eta" , &Gen_Eta);
           _tree->Branch("Gen_Phi" , &Gen_Phi);
           _tree->Branch("Gen_E" , &Gen_E);
        }

}


void ZeeDumper::ResetMainTreeVar()
{
	for (int i = 0; i < NELE; ++i) {
		_eleID[i] = initSingleInt;
		_chargeEle[i] = initSingleIntCharge;
		_recoFlagsEle[i] = 0;
                _etaEle[i] = initSingleFloat;
                _phiEle[i] = initSingleFloat;
                _R9Ele[i] = initSingleFloat;
                _gainSeedSC[i] = 9;
  
                _etaSCEle[i] = initSingleFloat;
                _phiSCEle[i] = initSingleFloat;
                _xSeedSC[i] = -999;
                _ySeedSC[i] = -999;
                _rawEnergySCEle[i] = initSingleFloat;
                _esEnergySCEle[i] = initSingleFloat;
                _energyEle[i] = initSingleFloat;
                _energy_5x5SC[i] = initSingleFloat;
                _mustEnergySCEle[i] = initSingleFloat;
                _isMustacheSC[i] = 0;
		_energy_ECAL_ele[i] = initSingleFloat;
                _energy_ECAL_pho[i] = initSingleFloat;
	}

        Gen_Pt.clear();
        Gen_Eta.clear();
        Gen_Phi.clear();
        Gen_E.clear();

}

void ZeeDumper::TreeSetEventSummaryVar(const edm::Event& iEvent)
{
	_eventTimeStamp   =  iEvent.eventAuxiliary().time();
        _eventTime = (UInt_t) _eventTimeStamp.unixTime();
        _runNumber = (UInt_t) iEvent.run();
        _eventNumber = (Long64_t) iEvent.id().event();
        if( (_eventNumber % 10) == 0)
                _isTrain = 0;
        else
                _isTrain = 1;
        _nBX = (UShort_t)  iEvent.bunchCrossing();
        _lumiBlock = (UShort_t) iEvent.luminosityBlock();

	if(!hltPaths.empty()) {
                edm::TriggerNames hltNames = iEvent.triggerNames(*_triggerResultsHandle);
                int hltCount = _triggerResultsHandle->size();
                _HLTNames[0].clear();
                _HLTBits.clear();
                for (int i = 0 ; i < hltCount; ++i) {
                        std::string hltName_str(hltNames.triggerName(i));
                        (_HLTNames[0]).push_back(hltName_str);
                        (_HLTResults[0]).push_back(_triggerResultsHandle->accept(i));
                        _HLTBits.insert(std::pair<std::string, bool>( hltName_str, _triggerResultsHandle->accept(i)));
		}
	}

	return;

}


float ZeeDumper::GetMustEnergy(const reco::GsfElectron& electron, bool isEB){

        if(!isAOD_) return electron.parentSuperCluster()->rawEnergy();
        else {  
	  if(isEB){
             for( reco::SuperClusterCollection::const_iterator iter = EBSuperClustersHandle->begin();
                iter != EBSuperClustersHandle->end();
                iter++) {
	        if( fabs(electron.parentSuperCluster()->rawEnergy() - iter->rawEnergy()) < 1E-6 )
		    return iter->energy();

             }
	  } else{
             for( reco::SuperClusterCollection::const_iterator iter1 = EESuperClustersHandle->begin();
                iter1 != EESuperClustersHandle->end();
                iter1++) {
		if( fabs(electron.parentSuperCluster()->rawEnergy() - iter1->rawEnergy()) < 1E-6 )
                    return iter1->energy();
	     }
          }
        }
	return -999.;

}

UChar_t ZeeDumper::GetSeedGain(const DetId& seedDetId, int index)
{
        const EcalRecHitCollection *recHits = (seedDetId.subdetId() == EcalBarrel) ? EBRechitsHandle.product() : EERechitsHandle.product();
	EcalRecHitCollection::const_iterator seedRecHit = recHits->find(seedDetId) ;
	UChar_t gain = 0;
	if(seedRecHit != recHits->end()) {
           if(seedRecHit->checkFlag(EcalRecHit::kHasSwitchToGain6)) gain |= 0x01;
	   if(seedRecHit->checkFlag(EcalRecHit::kHasSwitchToGain1)) gain |= 0x02;
        }

        return gain;
}

UInt_t ZeeDumper::GetID(const pat::Electron& electron)
{ 
        eleIDMap eleID_map;
        UInt_t eleID = 0;
	for (std::map<std::string, UInt_t>::iterator it = eleID_map.eleIDmap.begin(); it != eleID_map.eleIDmap.end(); ++it) {
//	     std::cout << "eleID is " << it->first << "; isAvailable is " << electron.isElectronIDAvailable(it->first) << std::endl;
	     if(electron.isElectronIDAvailable(it->first)) { //
		if ((bool) electron.electronID(it->first))  eleID |= it->second;//
	     }//
	}
        return eleID;
}

float ZeeDumper::InvMass(double eta1, double eta2, double phi1, double phi2, double energy1, double energy2)
{
        double t1 = TMath::Exp(-eta1);
        double t1q = t1 * t1;
        double t2 = TMath::Exp(-eta2);
        double t2q = t2 * t2;
        double angle = 1 - ( (1 - t1q) * (1 - t2q) + 4 * t1 * t2 * cos(phi1 - phi2)) / ( (1 + t1q) * (1 + t2q) );
        double mass = sqrt(2 * energy1 * energy2 * angle);
        
        return float(mass);
}

void ZeeDumper::TreeSetPileupVar(void)
{
        _rho = *rhoHandle;
        _nPV = -1;
        _nPU = -1;
	_mcGenWeight = -1;

        if(primaryVertexHandle->size() > 0) {
                for(reco::VertexCollection::const_iterator v = primaryVertexHandle->begin(); v != primaryVertexHandle->end(); ++v) {
			_nPV++;
		}
	}

	
        if(isMC_){
	   std::vector<PileupSummaryInfo>::const_iterator PVI;
           for(PVI = PupInfo->begin(); PVI != PupInfo->end(); ++PVI) {
                int BX = PVI->getBunchCrossing();
                if(BX == 0) { // in-time pu
                   _nPU = PVI->getTrueNumInteractions();
                }
           }

	   if(!_GenEventInfoHandle->weights().empty()) {
                        _mcGenWeight = (_GenEventInfoHandle->weights())[0];
                }
        }

	


        return;
	
}


//define this as a plug-in
DEFINE_FWK_MODULE(ZeeDumper);
