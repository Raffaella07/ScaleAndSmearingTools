import FWCore.ParameterSet.Config as cms
import FWCore.Utilities.FileUtils as FileUtils
import FWCore.ParameterSet.VarParsing as VarParsing

process = cms.Process("ZeeDumper")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.StandardSequences.GeometryDB_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
#process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff") # gives deprecated message in 80X but still runs
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.load('Configuration.StandardSequences.EndOfProcess_cff')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag,'130X_mcRun3_2023_realistic_v14','')

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32( -1 ) )
process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32( 1000 )
                                                                       
process.source = cms.Source("PoolSource",
    skipEvents = cms.untracked.uint32(0),                       
    fileNames = cms.untracked.vstring(
        'root://cms-xrd-global.cern.ch//store/mc/Run3Summer23MiniAODv4/DYto2L-4Jets_MLL-50_TuneCP5_13p6TeV_madgraphMLM-pythia8/MINIAODSIM/130X_mcRun3_2023_realistic_v14-v1/70002/a8a27830-f9cc-4e85-8c93-df6864ab0e32.root'
    ),
    secondaryFileNames = cms.untracked.vstring()
) 

######################Activate Run 3 2022 IDs [Might need change to the 2023 recommendation, but none exists so far]##########################################
from PhysicsTools.SelectorUtils.tools.vid_id_tools import *
dataFormat = DataFormat.MiniAOD ## DataFormat.AOD while running on AOD
switchOnVIDElectronIdProducer(process, dataFormat)
my_id_modules = ['RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Winter22_122X_V1_cff']

for idmod in my_id_modules:
    setupAllVIDIdsInModule(process,idmod,setupVIDElectronSelection)
##############################################################################################################################################################

########################## Make Photon regressed energies and the IDs accessible from the electron pointer ########################################### 
process.slimmedECALELFElectrons = cms.EDProducer("PATElectronSlimmer",
    dropBasicClusters = cms.string('0'),
    dropClassifications = cms.string('0'),
    dropCorrections = cms.string('0'),
    dropExtrapolations = cms.string('pt < 5'),
    dropIsolations = cms.string('0'),
    dropPFlowClusters = cms.string('0'),
    dropPreshowerClusters = cms.string('0'),
    dropRecHits = cms.string('0'),
    dropSaturation = cms.string('pt < 5'),
    dropSeedCluster = cms.string('0'),
    dropShapes = cms.string('0'),
    dropSuperCluster = cms.string('0'),
    linkToPackedPFCandidates = cms.bool(False),
    modifierConfig = cms.PSet(

        modifications = cms.VPSet(
                cms.PSet(
                ecalRecHitsEB = cms.InputTag("reducedEgamma","reducedEBRecHits"),
                ecalRecHitsEE = cms.InputTag("reducedEgamma","reducedEERecHits"),
                electron_config = cms.PSet(
                    electronSrc = cms.InputTag("slimmedElectrons"),
                    energySCEleMust = cms.InputTag("eleNewEnergiesProducer","energySCEleMust"),
                    energySCEleMustVar = cms.InputTag("eleNewEnergiesProducer","energySCEleMustVar"),
                    energySCElePho = cms.InputTag("eleNewEnergiesProducer","energySCElePho"),
                    energySCElePhoVar = cms.InputTag("eleNewEnergiesProducer","energySCElePhoVar")
                    ),
                modifierName = cms.string('EGExtraInfoModifierFromFloatValueMaps'),
                photon_config = cms.PSet()
                    ),

                cms.PSet(
                    modifierName = cms.string('EleIDModifierFromBoolValueMaps'),
                    electron_config = cms.PSet(
                    electronSrc = cms.InputTag("slimmedElectrons"),
                    looseRun2022 = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-RunIIIWinter22-V1-loose"),
                    mediumRun2022 = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-RunIIIWinter22-V1-medium"),
                    tightRun2022 = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-RunIIIWinter22-V1-tight")
                    ),
                    photon_config   = cms.PSet( )
                    )
            
            )
    ),

    modifyElectrons = cms.bool(True),
    packedPFCandidates = cms.InputTag("packedPFCandidates"),
    recoToPFMap = cms.InputTag("reducedEgamma","reducedGsfElectronPfCandMap"),
    reducedBarrelRecHitCollection = cms.InputTag("reducedEgamma","reducedEBRecHits"),
    reducedEndcapRecHitCollection = cms.InputTag("reducedEgamma","reducedEERecHits"),
    saveNonZSClusterShapes = cms.string('pt > 5'),
    src = cms.InputTag("slimmedElectrons")

)
#################################################################################################################################

process.load('ScaleAndSmearingTools.Dumper.Zee_dumper_MINIAOD_cfi') # Runs the ele energy producer and sets up the dumper
process.TFileService = cms.Service("TFileService",
    fileName = cms.string("output.root")
)

process.output = cms.OutputModule("PoolOutputModule",
                                   splitLevel = cms.untracked.int32(0),
                                   outputCommands = cms.untracked.vstring("keep *"),
                                   fileName = cms.untracked.string("miniAOD.root")
)


from Geometry.CaloEventSetup.CaloGeometryBuilder_cfi import *
CaloGeometryBuilder.SelectedCalos = ['HCAL', 'ZDC', 'EcalBarrel', 'EcalEndcap', 'EcalPreshower', 'TOWER'] # Why is this needed?

process.dumper_step = cms.Path(process.egmGsfElectronIDSequence+process.eleNewEnergiesProducer+process.slimmedECALELFElectrons+process.zeedumper)
#process.output_step = cms.EndPath(process.output)

process.schedule = cms.Schedule(process.dumper_step)





