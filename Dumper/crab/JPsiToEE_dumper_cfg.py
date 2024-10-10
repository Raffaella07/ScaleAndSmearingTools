from FWCore.ParameterSet.VarParsing import VarParsing
import FWCore.ParameterSet.Config as cms
import FWCore.Utilities.FileUtils as FileUtils


options = VarParsing('python')

options.register('isMC', True,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.bool,
    "Run this on real data"
)
options.register('globalTag', 'NOTSET',
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "Set global tag"
)
options.register('wantSummary', True,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.bool,
    "Run this on real data"
)
options.register('reportEvery', 10,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.int,
    "report every N events"
)
options.register('skip', 0,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.int,
    "skip first N events"
)

options.setDefault('maxEvents', 100)
options.setDefault('tag', '10215')
options.parseArguments()

#check gtags
globaltag = '130X_dataRun3_PromptAnalysis_v1' if not options.isMC else '130X_mcRun3_2023_realistic_v14'
if options._beenSet['globalTag']:
    globaltag = options.globalTag

extension = {False : 'data', True : 'mc'}
outputFileNANO = cms.string('_'.join(['LowPt_ScaleSmear', extension[options.isMC], options.tag])+'.root')
outputFileFEVT = cms.untracked.string('_'.join(['LowPt_ScaleSmear', extension[options.isMC], options.tag])+'.root')



process = cms.Process("ZeeDumper")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.StandardSequences.GeometryDB_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("TrackingTools/TransientTrack/TransientTrackBuilder_cfi")
#process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff") # gives deprecated message in 80X but still runs
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.load('Configuration.StandardSequences.EndOfProcess_cff')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag,'130X_mcRun3_2023_realistic_v14','')

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32( -1 ) )
process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32( 1000 )
                                                                       
process.source = cms.Source("PoolSource",
    skipEvents = cms.untracked.uint32(0),                       
    fileNames = cms.untracked.vstring('root://cms-xrd-global.cern.ch//store/mc/Run3Summer22EEMiniAODv4/BuToKJPsi_JPsiToEE_SoftQCD_TuneCP5_13p6TeV_pythia8-evtgen/MINIAODSIM/130X_mcRun3_2022_realistic_postEE_v6-v2/60000/06e569b0-2f47-497c-87bc-d0aa2e039dc1.root'#
  #  'root://cms-xrd-global.cern.ch//store/data/Run2022F/ParkingDoubleElectronLowMass0/MINIAOD/22Sep2023-v1/2550000/08117b53-bc3e-48b1-9c13-dbf5d8793a01.root'
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
mvaConfigsForEleProducer = cms.VPSet()
from RecoEgamma.ElectronIdentification.Identification.mvaElectronID_Fall17_noIso_V2_cff \
    import mvaEleID_Fall17_noIso_V2_producer_config
from ScaleAndSmearingTools.Dumper.mvaElectronID_BParkRetrain_cff \
    import mvaEleID_BParkRetrain_producer_config
mvaConfigsForEleProducer.append( mvaEleID_Fall17_noIso_V2_producer_config )
mvaConfigsForEleProducer.append( mvaEleID_BParkRetrain_producer_config )
process.electronMVAValueMapProducer = cms.EDProducer(
    'ElectronMVAValueMapProducer',
    src = cms.InputTag("slimmedElectrons"),#,processName=cms.InputTag.skipCurrentProcess()),
    mvaConfigurations = mvaConfigsForEleProducer,
)

process.userIDElectrons = cms.EDProducer(
    'EleIDsFromValueMapProducer',
    pfSrc = cms.InputTag("slimmedElectrons"),#,processName=cms.InputTag.skipCurrentProcess()),
    pfmvaId = cms.InputTag("electronMVAValueMapProducer:ElectronMVAEstimatorRun2BParkRetrainRawValues"),
    pfmvaId_Run2 = cms.InputTag("electronMVAValueMapProducer:ElectronMVAEstimatorRun2Fall17NoIsoV2RawValues"),
)

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
   # pfmvaId = cms.InputTag("electronMVAValueMapProducer:ElectronMVAEstimatorRun2Fall17NoIsoV2RawValues"),
    src = cms.InputTag("userIDElectrons:SelectedElectrons")

)
#################################################################################################################################

process.load('ScaleAndSmearingTools.Dumper.Zee_dumper_MINIAOD_cfi') # Runs the ele energy producer and sets up the dumper
process.TFileService = cms.Service("TFileService",
    fileName = outputFileNANO
)

#process.output = cms.OutputModule("PoolOutputModule",
#                                   splitLevel = cms.untracked.int32(0),
#                                   outputCommands = cms.untracked.vstring("keep *"),
#                                   fileName = cms.untracked.string("miniAOD.root")
#)


from Geometry.CaloEventSetup.CaloGeometryBuilder_cfi import *
CaloGeometryBuilder.SelectedCalos = ['HCAL', 'ZDC', 'EcalBarrel', 'EcalEndcap', 'EcalPreshower', 'TOWER'] # Why is this needed?

paths=['HLT_DoubleEle10_eta1p22_mMax6',
       'HLT_DoubleEle9p5_eta1p22_mMax6',
       'HLT_DoubleEle9_eta1p22_mMax6',
       'HLT_DoubleEle8p5_eta1p22_mMax6',
       'HLT_DoubleEle8_eta1p22_mMax6',
       'HLT_DoubleEle7p5_eta1p22_mMax6',
       'HLT_DoubleEle7_eta1p22_mMax6',
       'HLT_DoubleEle6p5_eta1p22_mMax6',
       'HLT_DoubleEle6_eta1p22_mMax6',
       'HLT_DoubleEle5p5_eta1p22_mMax6',
       'HLT_DoubleEle5_eta1p22_mMax6',
       'HLT_DoubleEle4p5_eta1p22_mMax6',
       'HLT_DoubleEle4_eta1p22_mMax6'
]
paths_OR = " || ".join([ 'path( "{:s}_v*" )'.format(path) for path in paths])

seeds = ['L1_DoubleEG11_er1p2_dR_Max0p6',
         'L1_DoubleEG10p5_er1p2_dR_Max0p6',
         'L1_DoubleEG10_er1p2_dR_Max0p6',
         'L1_DoubleEG9p5_er1p2_dR_Max0p6',
         'L1_DoubleEG9_er1p2_dR_Max0p7',
         'L1_DoubleEG8p5_er1p2_dR_Max0p7',
         'L1_DoubleEG8_er1p2_dR_Max0p7',
         'L1_DoubleEG7p5_er1p2_dR_Max0p7',
         'L1_DoubleEG7_er1p2_dR_Max0p8',
         'L1_DoubleEG6p5_er1p2_dR_Max0p8',
         'L1_DoubleEG6_er1p2_dR_Max0p8',
         'L1_DoubleEG5p5_er1p2_dR_Max0p8',
         'L1_DoubleEG5_er1p2_dR_Max0p9',
         'L1_DoubleEG4p5_er1p2_dR_Max0p9',
         'L1_DoubleEG4_er1p2_dR_Max0p9',
]

# https://github.com/cms-sw/cmssw/blob/master/PhysicsTools/PatAlgos/plugins/PATTriggerObjectStandAloneUnpacker.cc
process.myUnpackedPatTrigger = cms.EDProducer(
    "PATTriggerObjectStandAloneUnpacker",
    patTriggerObjectsStandAlone = cms.InputTag("slimmedPatTrigger"),
    triggerResults = cms.InputTag("TriggerResults::HLT"),
    unpackFilterLabels = cms.bool(True),
)

# https://github.com/cms-sw/cmssw/blob/master/PhysicsTools/PatAlgos/python/triggerLayer1/triggerMatcherExamples_cfi.py
# https://github.com/cms-sw/cmssw/blob/master/PhysicsTools/PatAlgos/plugins/PATTriggerMatcher.cc
process.myTriggerMatches = cms.EDProducer(
    "PATTriggerMatcherDEtaLessByDR", # match by DeltaEta only, best match by DeltaR
    #"PATTriggerMatcherDEtaLessByDEta", # match by DeltaEta only, best match by DeltaEta
    #"PATTriggerMatcherDRDPtLessByR", # match by DeltaR only, best match by DeltaR
    src = cms.InputTag("slimmedECALELFElectrons"),
    matched = cms.InputTag("myUnpackedPatTrigger"),
    matchedCuts = cms.string(paths_OR), # e.g. 'path("HLT_DoubleEle6_eta1p22_mMax6_v*")'
    maxDeltaR = cms.double(2.0),
    maxDeltaEta = cms.double(0.5),
    #maxDPtRel = cms.double(0.5),
    resolveAmbiguities    = cms.bool( True ), # only one match per trigger object
    resolveByMatchQuality = cms.bool( True ), # take best match found per reco object (e.g. by DeltaR)
)

# Electron ID MVA raw values
# https://github.com/cms-sw/cmssw/blob/master/PhysicsTools/PatAlgos/plugins/PATTriggerMatchEmbedder.cc
process.mySlimmedElectronsWithEmbeddedTrigger = cms.EDProducer(
    "PATTriggerMatchElectronEmbedder",
    src = cms.InputTag("slimmedECALELFElectrons"),
    matches = cms.VInputTag('myTriggerMatches'),
)


process.electronTrgSelector = cms.EDProducer(
    "ElectronTriggerSelector",
    electronCollection = cms.InputTag("mySlimmedElectronsWithEmbeddedTrigger"),
    bits = cms.InputTag("TriggerResults","","HLT"),
    prescales = cms.InputTag("patTrigger"),
    objects = cms.InputTag("slimmedPatTrigger"),
    vertexCollection = cms.InputTag("offlineSlimmedPrimaryVertices"),
    maxdR_matching = cms.double(10.), # not used
    dzForCleaning_wrtTrgElectron = cms.double(1.),
    filterElectron = cms.bool(True),
    ptMin = cms.double(2.),
    absEtaMax = cms.double(1.25),
    HLTPaths=cms.vstring(paths),
    L1seeds=cms.vstring(seeds),
)

process.countTrgElectrons = cms.EDFilter(
    "PATCandViewCountFilter",
    minNumber = cms.uint32(1),
    maxNumber = cms.uint32(999999),
    src = cms.InputTag("electronTrgSelector", "trgElectrons"),
)

process.trigEleCustom_step = cms.Path(process.myUnpackedPatTrigger + process.myTriggerMatches +process.mySlimmedElectronsWithEmbeddedTrigger + process.electronTrgSelector + process.countTrgElectrons)
#process.dumper_step = cms.Path(process.egmGsfElectronIDSequence+process.eleNewEnergiesProducer+process.slimmedECALELFElectrons +process.zeedumper)
if not options.isMC:
	process.dumper_step = cms.Path(process.egmGsfElectronIDSequence+process.eleNewEnergiesProducer+process.electronMVAValueMapProducer+process.userIDElectrons+process.slimmedECALELFElectrons+process.myUnpackedPatTrigger + process.myTriggerMatches +process.mySlimmedElectronsWithEmbeddedTrigger + process.electronTrgSelector + process.countTrgElectrons+process.BToKLLdumper)
else:

	process.dumper_step = cms.Path(process.egmGsfElectronIDSequence+process.eleNewEnergiesProducer+process.electronMVAValueMapProducer+process.userIDElectrons+process.slimmedECALELFElectrons+process.myUnpackedPatTrigger + process.myTriggerMatches +process.mySlimmedElectronsWithEmbeddedTrigger + process.electronTrgSelector + process.countTrgElectrons+process.BToKLLmc_dumper)
#process.output_step = cms.EndPath(process.output)

process.schedule = cms.Schedule(process.dumper_step)





