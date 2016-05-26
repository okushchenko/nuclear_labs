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
// $Id: exampleB1.cc 75216 2013-10-29 16:08:11Z gcosmo $
//
/// \file exampleB1.cc
/// \brief Main program of the B1 example

#include "B1DetectorConstruction.hh"
#include "B1ActionInitialization.hh"

#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#else
#include "G4RunManager.hh"
#endif

#include "G4UImanager.hh"
#include "QBBC.hh"

#ifdef G4VIS_USE
#include "G4VisExecutive.hh"
#endif

#ifdef G4UI_USE
#include "G4UIExecutive.hh"
#endif

#include "Randomize.hh"

#include "G4SystemOfUnits.hh"

#include <cmath>
#include <ctime>
#include <cstdio>
#include <cstdlib>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc,char** argv)
{
	//Removing old data
	if(FILE *ConsoleOutputData = fopen("ConsoleOutputData.txt", "r"))
		std::remove("ConsoleOutputData.txt");
	if(FILE *ResultsToPlot = fopen("ResultsToPlot.txt", "r"))
			std::remove("ResultsToPlot.txt");

	// Set initial parameters to the file
	FILE* parameters_file = fopen("GunPositionParameters.txt", "w");

	fprintf(parameters_file, "%f\t%f\t%f\t%f", 0.0f, 100.0f, 0.0f, 100.0f);
	fflush(parameters_file);
	fclose(parameters_file);

  // Choose the Random engine
  //
  G4Random::setTheEngine(new CLHEP::RanecuEngine);
  
  // Construct the default run manager
  //
#ifdef G4MULTITHREADED
  G4MTRunManager* runManager = new G4MTRunManager;
#else
  G4RunManager* runManager = new G4RunManager;
#endif

  // Set mandatory initialization classes
  //
  // Detector construction
  runManager->SetUserInitialization(new B1DetectorConstruction());

  // Physics list
  G4VModularPhysicsList* physicsList = new QBBC;
  physicsList->SetVerboseLevel(1);
  runManager->SetUserInitialization(physicsList);
    
  // User action initialization
  runManager->SetUserInitialization(new B1ActionInitialization());

  // Initialize G4 kernel
  //
  runManager->Initialize();
  
#ifdef G4VIS_USE
  // Initialize visualization
  G4VisManager* visManager = new G4VisExecutive;
  // G4VisExecutive can take a verbosity argument - see /vis/verbose guidance.
  // G4VisManager* visManager = new G4VisExecutive("Quiet");
  visManager->Initialize();
#endif

  // Get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  /*if (argc == 2) {
    // batch mode
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    UImanager->ApplyCommand(command+fileName);
  }*/
  // Super measurements
  if (argc == 2)
	{
	  // Envelope size
	  G4double env_sizeXY = 100.0;

	  G4int shapesArraySize = 4;

	  const G4double min_size_x = -0.4 * env_sizeXY, max_size_x = 0.4 * env_sizeXY;
	  const G4double min_size_y = -0.4 * env_sizeXY, max_size_y = 0.4 * env_sizeXY;

	  G4double size_x;
	  G4double size_y;

	  size_x = min_size_x;
	  size_y = min_size_y;

	  //Setting random values generator
	  CLHEP::RanecuEngine theEngine;

	  double mean = 0.0, standardDeviation = 0.05;

	  float user_input = atof(argv[1]);

	  for (int i = 0; i < shapesArraySize; i++)
	  {
		 for(int j = 0; j < shapesArraySize; j++)
		 {
			 // Changing position for each run to calculate dose in
			 // every existing volume
			 parameters_file = fopen("GunPositionParameters.txt", "w");
			 fprintf(parameters_file, "%f\t%f\t%f\t%f", static_cast<float>(size_x), 1.0f, static_cast<float>(size_y), 1.0f);
			 fflush(parameters_file);
			 fclose(parameters_file);

			 theEngine.setSeed(time(0));
			 //Executing run for current position
			 int forRandom = static_cast<int>(user_input * 50000.0f / 30.0f);
			 int numGamma = std::abs(forRandom - 0.2 * forRandom * std::fabs(CLHEP::RandGauss::shoot(&theEngine, mean, standardDeviation)));
			 runManager->BeamOn(numGamma);

			 size_y += env_sizeXY / shapesArraySize;
			 }
		 size_x += env_sizeXY / shapesArraySize;
		 size_y = min_size_y;
	  }
	}
  else {
    // interactive mode : define UI session
#ifdef G4UI_USE
    G4UIExecutive* ui = new G4UIExecutive(argc, argv);
#ifdef G4VIS_USE
    UImanager->ApplyCommand("/control/execute init_vis.mac");
#else
    UImanager->ApplyCommand("/control/execute init.mac");
#endif
    ui->SessionStart();
    delete ui;
#endif
  }

  // Job termination
  // Free the store: user actions, physics_list and detector_description are
  // owned and deleted by the run manager, so they should not be deleted 
  // in the main() program !
  
#ifdef G4VIS_USE
  delete visManager;
#endif
  delete runManager;

  return 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....
