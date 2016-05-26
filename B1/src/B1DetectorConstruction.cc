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
// $Id: B1DetectorConstruction.cc 75117 2013-10-28 09:38:37Z gcosmo $
//
/// \file B1DetectorConstruction.cc
/// \brief Implementation of the B1DetectorConstruction class

#include "B1DetectorConstruction.hh"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Tubs.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4Trd.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"

#include <string>
#include <iostream>
#include <fstream>

#define M_Pi 3.14159265358979323846

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1DetectorConstruction::B1DetectorConstruction()
: G4VUserDetectorConstruction(),
  fScoringVolumeArray(0),
  fSolidVolumeArray(0)
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1DetectorConstruction::~B1DetectorConstruction()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* B1DetectorConstruction::Construct()
{  
  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();
  
  // Envelope parameters
  //
  G4double env_sizeXY = 100*cm, env_sizeZ = 100*cm;
  G4Material* env_mat = nist->FindOrBuildMaterial("G4_AIR");
   
  // Option to switch on/off checking of volumes overlaps
  //
  G4bool checkOverlaps = true;

  //     
  // World
  //
  G4double world_sizeXY = 1.2*env_sizeXY;
  G4double world_sizeZ  = 1.2*env_sizeZ;
  G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");
  
  G4Box* solidWorld =    
    new G4Box("World",                       //its name
       0.5*world_sizeXY, 0.5*world_sizeXY, 0.5*world_sizeZ);     //its size
      
  G4LogicalVolume* logicWorld =                         
    new G4LogicalVolume(solidWorld,          //its solid
                        world_mat,           //its material
                        "World");            //its name
                                   
  G4VPhysicalVolume* physWorld = 
    new G4PVPlacement(0,                     //no rotation
                      G4ThreeVector(),       //at (0,0,0)
                      logicWorld,            //its logical volume
                      "World",               //its name
                      0,                     //its mother  volume
                      false,                 //no boolean operation
                      0,                     //copy number
                      checkOverlaps);        //overlaps checking
                     
  //     
  // Envelope
  //  
  G4Box* solidEnv =    
    new G4Box("Envelope",                    //its name
        0.5*env_sizeXY, 0.5*env_sizeXY, 0.5*env_sizeZ); //its size
      
  G4LogicalVolume* logicEnv =                         
    new G4LogicalVolume(solidEnv,            //its solid
                        env_mat,             //its material
                        "Envelope");         //its name
               
  new G4PVPlacement(0,                       //no rotation
                    G4ThreeVector(),         //at (0,0,0)
                    logicEnv,                //its logical volume
                    "Envelope",              //its name
                    logicWorld,              //its mother  volume
                    false,                   //no boolean operation
                    0,                       //copy number
                    checkOverlaps);          //overlaps checking

  //     
  // Detector shape
  //

  // Creating an array of scoring volumes to get all the data simultaneously

  G4double shapeArrayElement_dx = 16*cm;
  G4double shapeArrayElement_dy = 16*cm;
  G4double shapeArrayElement_dz;

  // For creating 4 x 4 matrix of shapes and detectors
  G4int shapesArraySize = 4;



  /*----------Shapes and detector materials definition---------------------*/

  /*
   * Material of shapes before detectors (to change, put the other material instead of "G4_Pb",
   * for example "G4_Al" for aluminum). The list of available materials can be found here:
   * https://geant4.web.cern.ch/geant4/UserDocumentation/UsersGuides/ForApplicationDeveloper/html/apas08.html
   */
  G4Material* shapesArrayMaterial = nist->FindOrBuildMaterial("G4_Pb");

  /*
   * Material of detectors (to change, put the other material instead of "G4_Pb",
   * for example "G4_Al" for aluminum). The list of available materials can be found here:
   * https://geant4.web.cern.ch/geant4/UserDocumentation/UsersGuides/ForApplicationDeveloper/html/apas08.html
   */
  G4Material* shapesDetectorMaterial = nist->FindOrBuildMaterial("G4_Pb");

  /*-----------------------------------------------------------------------*/



  G4ThreeVector* shapesArrayPositions = new G4ThreeVector[shapesArraySize * shapesArraySize];
  G4ThreeVector* detectorsArrayPositions = new G4ThreeVector[shapesArraySize * shapesArraySize];

  const G4double min_size_x = -0.4 * env_sizeXY;
  const G4double min_size_y = -0.4 * env_sizeXY;

  G4double size_x;
  G4double size_y;

  size_x = min_size_x;
  size_y = min_size_y;

  for (int i = 0; i < shapesArraySize; i++)
  {
	  for(int j = 0; j < shapesArraySize; j++)
	  {
		  shapesArrayPositions[i * shapesArraySize + j] = G4ThreeVector(size_x,
				  size_y, 7*cm);

		  detectorsArrayPositions[i * shapesArraySize + j] = G4ThreeVector(size_x,
				  size_y, 17*cm);

		  size_y += env_sizeXY / shapesArraySize;
	  }
	  size_x += env_sizeXY / shapesArraySize;
	  size_y = min_size_y;
  }

  // Creating solids for detection
  G4Box** solidShapesArray = new G4Box*[shapesArraySize * shapesArraySize];

  // Creating detector solids
  G4double detectorLenght_dz = 1*mm;
  G4Tubs** solidDetectorsArray = new G4Tubs*[shapesArraySize * shapesArraySize];

  for(int i = 0; i < shapesArraySize; i++)
    {
  	  for(int j = 0; j < shapesArraySize; j++)
  	  {
  		  std::stringstream boxNumberStream;
  		  boxNumberStream << "Box_" << i << "_" << j;

  		  shapeArrayElement_dz = 1*mm + (i * shapesArraySize + j)*mm;

  		  solidShapesArray[i * shapesArraySize + j] = new G4Box(boxNumberStream.str(),
  				  0.5*shapeArrayElement_dx, 0.5*shapeArrayElement_dy, 0.5*shapeArrayElement_dz);

  		  std::stringstream tubsNumberStream;
  		  tubsNumberStream << "Tube_" << i << "_" << j;

  		  solidDetectorsArray[i * shapesArraySize + j] = new G4Tubs(tubsNumberStream.str(),
  				  0, 2.5*cm,
  				  0.5*detectorLenght_dz,
  				  0, 2*M_Pi);
  	  }
    }

  // Creating logical shapes volumes
  G4LogicalVolume** logicalShapesArray = new G4LogicalVolume*[shapesArraySize * shapesArraySize];
  // Creating logical detectors volumes
  G4LogicalVolume** logicalDetectorsArray = new G4LogicalVolume*[shapesArraySize * shapesArraySize];

  for(int i = 0; i < shapesArraySize; i++)
  {
	  for(int j = 0; j < shapesArraySize; j++)
	  {
		  std::stringstream boxNumberStream;
		  boxNumberStream << "Box_" << i << "_" << j;

		  logicalShapesArray[i * shapesArraySize + j] = new G4LogicalVolume(solidShapesArray[i * shapesArraySize + j], //its solid
				  shapesArrayMaterial, // its material
				  boxNumberStream.str()); // its name

		  std::stringstream tubsNumberStream;
		  tubsNumberStream << "Tube_" << i << "_" << j;

		  logicalDetectorsArray[i * shapesArraySize + j] = new G4LogicalVolume(solidDetectorsArray[i * shapesArraySize + j], //its solid
				  shapesDetectorMaterial, // its material
		  		  tubsNumberStream.str()); // its name
	  }
  }

  // Placing elements
  for(int i = 0; i < shapesArraySize; i++)
  {
	  for(int j = 0; j < shapesArraySize; j++)
	  {
		  std::stringstream boxNumberStream;
		  boxNumberStream << "Box_" << i << "_" << j;

		  new G4PVPlacement(0,                       						//no rotation
				  	  	  	shapesArrayPositions[i * shapesArraySize + j],  //at position
				  	  	  	logicalShapesArray[i * shapesArraySize + j],    //its logical volume
				  	  	  	boxNumberStream.str(),                			//its name
		                    logicEnv,                //its mother  volume
		                    false,                   //no boolean operation
		                    0,                       //copy number
		                    checkOverlaps);          //overlaps checking

		  std::stringstream tubsNumberStream;
		  tubsNumberStream << "Tube_" << i << "_" << j;

		  new G4PVPlacement(0,                       							//no rotation
				  	  	  	detectorsArrayPositions[i * shapesArraySize + j],  	//at position
		  				  	logicalDetectorsArray[i * shapesArraySize + j],    	//its logical volume
		  				  	tubsNumberStream.str(),                				//its name
		  		            logicEnv,                //its mother  volume
		  		            false,                   //no boolean operation
		  		            0,                       //copy number
		  		            checkOverlaps);          //overlaps checking
	  }
  }

  fScoringVolumeArray = logicalDetectorsArray;
  fSolidVolumeArray = logicalShapesArray;

  //
  //always return the physical World
  //
  return physWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......