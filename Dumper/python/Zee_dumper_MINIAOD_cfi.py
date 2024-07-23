import FWCore.ParameterSet.Config as cms

eleNewEnergiesProducer = cms.EDProducer("EleNewEnergiesProducer",
    electronCollection = cms.InputTag("slimmedElectrons"),
    photonCollection = cms.InputTag("slimmedPhotons"),
    recHitCollectionEB = cms.InputTag("reducedEgamma","reducedEBRecHits"),
    recHitCollectionEE = cms.InputTag("reducedEgamma","reducedEERecHits"),
    scEnergyCorrectorSemiParm = cms.PSet(
        ecalRecHitsEB = cms.InputTag("reducedEgamma","reducedEBRecHits"),
        ecalRecHitsEE = cms.InputTag("reducedEgamma","reducedEERecHits"),
        regTrainedWithPS = cms.bool(False),
        applySigmaIetaIphiBug = cms.bool(False),
        eRecHitThreshold = cms.double(1), 
        isHLT = cms.bool(False),
        isPhaseII = cms.bool(False),
        regressionKeyEB = cms.string('pfscecal_EBCorrection_offline_v2'),
        regressionKeyEE = cms.string('pfscecal_EECorrection_offline_v2'),
        regressionMaxEB = cms.double(2),
        regressionMaxEE = cms.double(2),
        regressionMinEB = cms.double(0.2),
        regressionMinEE = cms.double(0.2),
        uncertaintyKeyEB = cms.string('pfscecal_EBUncertainty_offline_v2'),
        uncertaintyKeyEE = cms.string('pfscecal_EEUncertainty_offline_v2'),
        uncertaintyMaxEB = cms.double(0.5),
        uncertaintyMaxEE = cms.double(0.5),
        uncertaintyMinEB = cms.double(0.0002),
        uncertaintyMinEE = cms.double(0.0002),
        vertexCollection = cms.InputTag("offlinePrimaryVertices")
    )
)

zeedumper = cms.EDAnalyzer("ZeeDumper",

    isMC              = cms.bool(True), 
    isAOD             = cms.bool(False),  
    pileupInfo        = cms.InputTag("slimmedAddPileupInfo"),
    rhoFastJet        = cms.InputTag("fixedGridRhoFastjetCentral"),
    genParticles      = cms.InputTag("prunedGenParticles"),
    vertexCollection  = cms.InputTag("offlineSlimmedPrimaryVertices"),
    ecalRecHitsEB     = cms.InputTag("reducedEgamma","reducedEBRecHits"), 
    ecalRecHitsEE     = cms.InputTag("reducedEgamma","reducedEERecHits"),
    ecalRecHitsES     = cms.InputTag("reducedEgamma","reducedESRecHits"),
    superClustersEB   = cms.InputTag("reducedEgamma","reducedSuperClusters"),
    superClustersEE   = cms.InputTag("reducedEgamma","reducedSuperClusters"),
    electrons         = cms.InputTag("slimmedECALELFElectrons"),
    energySCElePhoMap = cms.InputTag("eleNewEnergiesProducer","energySCElePho"),
    eleID_loose = cms.string("looseRun2022"),
    eleID_medium= cms.string("mediumRun2022"),
    eleID_tight = cms.string("tightRun2022"),
    hltPaths = cms.vstring(

                ),
    triggerResultsCollection = cms.InputTag('TriggerResults::HLT')
)

