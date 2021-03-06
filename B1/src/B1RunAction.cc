//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: B1RunAction.cc 75216 2013-10-29 16:08:11Z gcosmo $
//
/// \file B1RunAction.cc
/// \brief Implementation of the B1RunAction class

#include "B1RunAction.hh"
#include "B1PrimaryGeneratorAction.hh"
#include "B1DetectorConstruction.hh"
#include "B1Run.hh"

#include "G4RunManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

#include <cstdio>
#include <cstdlib>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1RunAction::B1RunAction()
: G4UserRunAction(),
  counter(0)
{ 
  // add new units for dose
  // 
  const G4double milligray = 1.e-3*gray;
  const G4double microgray = 1.e-6*gray;
  const G4double nanogray  = 1.e-9*gray;  
  const G4double picogray  = 1.e-12*gray;
   
  new G4UnitDefinition("milligray", "milliGy" , "Dose", milligray);
  new G4UnitDefinition("microgray", "microGy" , "Dose", microgray);
  new G4UnitDefinition("nanogray" , "nanoGy"  , "Dose", nanogray);
  new G4UnitDefinition("picogray" , "picoGy"  , "Dose", picogray);

  counter = 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1RunAction::~B1RunAction()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4Run* B1RunAction::GenerateRun()
{
  return new B1Run; 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B1RunAction::BeginOfRunAction(const G4Run*)
{ 
  //inform the runManager to save random number seed
  G4RunManager::GetRunManager()->SetRandomNumberStore(false);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B1RunAction::EndOfRunAction(const G4Run* run)
{
  G4int nofEvents = run->GetNumberOfEvent();
  if (nofEvents == 0) return;
  
  const B1Run* b1Run = static_cast<const B1Run*>(run);

  // Compute dose
  //
  G4double edep  = b1Run->GetEdep();
  G4double edep2 = b1Run->GetEdep2();
  G4double rms = edep2 - edep * edep / nofEvents;
  if (rms > 0.) rms = std::sqrt(rms); else rms = 0.;

  const B1DetectorConstruction* detectorConstruction
   = static_cast<const B1DetectorConstruction*>
     (G4RunManager::GetRunManager()->GetUserDetectorConstruction());

  //G4double mass = detectorConstruction->GetScoringVolume()->GetMass();
  //G4double dose = edep/mass;
  //G4double rmsDose = rms/mass;

  G4LogicalVolume** volumes = detectorConstruction->GetScoringVolumes();
  G4int volumesCount = 16;//(G4int)(sizeof(*volumes) / sizeof(G4LogicalVolume*));

  G4double masses[volumesCount];
  for(int i = 0; i < volumesCount; i++)
  {
	 masses[i] = volumes[i]->GetMass();
  }

  G4double doses[volumesCount];
  for(int i = 0; i < volumesCount; i++)
  {
	  doses[i] = edep / masses[i];
  }

  G4double rmsDoses[volumesCount];
  for(int i = 0; i < volumesCount; i++)
  {
	  rmsDoses[i] = rms / masses[i];
  }

  // Getting masses of the solids
  G4LogicalVolume** solids_volumes = detectorConstruction->GetSolidVolumes();

  G4double solids_masses[volumesCount];
  for(int i = 0; i < volumesCount; i++)
  {
	  solids_masses[i] = solids_volumes[i]->GetMass();
  }

  // Run conditions
  //  note: There is no primary generator action object for "master"
  //        run manager for multi-threaded mode.
  const B1PrimaryGeneratorAction* generatorAction
   = static_cast<const B1PrimaryGeneratorAction*>
     (G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction());
  G4String runCondition;
  if (generatorAction)
  {
    const G4ParticleGun* particleGun = generatorAction->GetParticleGun();
    runCondition += particleGun->GetParticleDefinition()->GetParticleName();
    runCondition += " of ";
    G4double particleEnergy = particleGun->GetParticleEnergy();
    runCondition += G4BestUnit(particleEnergy,"Energy");
  }
        
  // Print
  //  
  if (IsMaster()) {
    G4cout
     << "\n--------------------End of Global Run-----------------------";
  }
  else {
    G4cout
     << "\n--------------------End of Local Run------------------------";
  }

  //File operations
  //FILE* output = fopen("/media/handziuk/BreakJunctionsData/results.txt", "a");
  FILE* output = fopen("ConsoleOutputData.txt", "a");
  FILE* outputToPlot = fopen("ResultsToPlot.txt", "a");

  G4cout
  	       << "\n The run consists of " << nofEvents << " "<< runCondition
  	       << "\n Dose in scoring volume : "
  	       << G4BestUnit(doses[0],"Dose") << " +- " << G4BestUnit(rmsDoses[0],"Dose")
  	       << "The number of volumes is: "
  	       << volumesCount
  	       << "\n------------------------------------------------------------\n"
  	       << G4endl;

  fprintf(output,
  			  "\n The run consists of %d %s \n Dose in scoring volume : %s +- %s The number of volumes is: %d \n------------------------------------------------------------\n",
  			  static_cast<long long int>(nofEvents),
  			  runCondition.c_str(),
  			  G4String(G4BestUnit(doses[0],"Dose")).c_str(),
  			  G4String(G4BestUnit(rmsDoses[0],"Dose")).c_str(),
  			  static_cast<int>(volumesCount));

  int j = counter++;
  fprintf(outputToPlot, "%d\t%s\t%s +- %s\r\n", j, G4String(G4BestUnit(solids_masses[j],"Mass")).c_str(), G4String(G4BestUnit(doses[0],"Dose")).c_str(), G4String(G4BestUnit(rmsDoses[0],"Dose")).c_str());

  fflush(output);
  fflush(outputToPlot);

  fclose(output);
  fclose(outputToPlot);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
