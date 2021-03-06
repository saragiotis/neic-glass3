# initialize.d
# Configuration file for the glass parameters
{
  "Cmd": "Initialize",
  "MaximumNumberOfPicks": 10000,
  "MaximumNumberOfPicksPerSite": 40,
  "MaximumNumberOfCorrelations": 1000,
  "MaximumNumberOfHypos": 500,
  "PickDuplicateWindow": 10,
  "NumberOfNucleationThreads": 15,
  "NumberOfHypoThreads": 15,
  "Params": {
      "NucleationStackThreshold": 5.0, 
      "NucleationDataCountThreshold": 10,
      "AssociationStandardDeviationCutoff": 5.0,
      "PruningStandardDeviationCutoff": 5.0,
      "NonLocatingPhaseCutoffFactor": 3.0,
      "TeleseismicDistanceLimit": 30.0,
      "TeleseismicPhaseCountThreshold": 30,
      "PickAffinityExponentialFactor": 2.5,
      "DistanceCutoffFactor": 5.0,
      "DistanceCutoffRatio": 0.8,
      "DistanceCutoffMinimum": 30.0,
      "HypoProcessCountLimit": 15,
      "HypocenterTimeWindow": 30,
      "HypocenterDistanceWindow": 3
  },
  "DefaultNucleationPhase": {
      "PhaseName": "P",
      "TravFile": "./global_example/P.trv"
  },
  "AssociationPhases": [
      {
          "PhaseName": "P",
          "Assoc": [ 0, 130 ],
          "TravFile": "./global_example/P.trv",
          "UseForLocation": true,
          "PublishPhase": true
      },
      {
          "PhaseName": "S",
          "Assoc": [ 5, 100 ],
          "TravFile": "./global_example/S.trv",
          "UseForLocation": true,
          "PublishPhase": true
      },
      {
          "PhaseName": "PcP",
          "Assoc": [ 0, 45 ],
          "TravFile": "./global_example/PcP.trv",
          "UseForLocation": true,
          "PublishPhase": true
      },
      {
          "PhaseName": "PP",
          "Assoc": [ 80, 175 ],
          "TravFile": "./global_example/PP.trv",
          "UseForLocation": true,
          "PublishPhase": true
      },
      {
          "PhaseName": "PKPab",
          "Assoc": [ 130, 178 ],
          "TravFile": "./global_example/PKPab.trv",
          "UseForLocation": true,
          "PublishPhase": true
      },
      {
          "PhaseName": "PKPdf",
          "Assoc": [ 100, 176 ],
          "TravFile": "./global_example/PKPdf.trv",
          "UseForLocation": true,
          "PublishPhase": true
      },
      {
          "PhaseName": "Pdif",
          "Assoc": [ 95, 135 ],
          "TravFile": "./global_example/Pdif.trv",
          "UseForLocation": false,
          "PublishPhase": false
      },
      {
          "PhaseName": "Sdif",
          "Assoc": [ 95, 135 ],
          "TravFile": "./global_example/Sdif.trv",
          "UseForLocation": false,
          "PublishPhase": false
      },
      {
          "PhaseName": "PKKPab",
          "Assoc": [ 100, 125 ],
          "TravFile": "./global_example/PKKPab.trv",
          "UseForLocation": false,
          "PublishPhase": false
      },
      {
          "PhaseName": "PKKPbc",
          "Assoc": [ 70, 125 ],
          "TravFile": "./global_example/PKKPbc.trv",
          "UseForLocation": false,
          "PublishPhase": false
      },
      {
          "PhaseName": "PKKPdf",
          "Assoc": [ 70, 125 ],
          "TravFile": "./global_example/PKKPdf.trv",
          "UseForLocation": false,
          "PublishPhase": false
      },
      {
          "PhaseName": "P'P'ab",
          "Assoc": [ 45, 75 ],
          "TravFile": "./global_example/P'P'ab.trv",
          "UseForLocation": false,
          "PublishPhase": false
      },
      {
          "PhaseName": "P'P'bc",
          "Assoc": [ 45, 75 ],
          "TravFile": "./global_example/P'P'bc.trv",
          "UseForLocation": false,
          "PublishPhase": false
      },
      {
          "PhaseName": "P'P'df",
          "Assoc": [ 0, 110 ],
          "TravFile": "./global_example/P'P'df.trv",
          "UseForLocation": false,
          "PublishPhase": false
      },
      {
          "PhaseName": "PKiKP",
          "Assoc": [ 55, 118 ],
          "TravFile": "./global_example/PKiKP.trv",
          "UseForLocation": false,
          "PublishPhase": false
      }
  ]
}
# End of initialize.d
