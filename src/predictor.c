//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Mayunk Kulkarni";
const char *studentID   = "A53285335";
const char *email       = "mkulkarn@eng.ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//
//
//TODO: Add your own Branch Predictor data structures here
//
// ----------- GSHARE DATA STRUCTURES and function declarations ----------//
void gshareInit();
void gshareTrainer(uint32_t pc, uint8_t outcome);
uint8_t gsharePredict(uint32_t pc);

uint16_t gshareGHR = 0;
uint16_t gBhtSize = 1;
uint16_t gResultReg;
uint16_t gmask = 0;
uint8_t  gPred;
uint8_t  gBhtMask = 0x03;
uint8_t  gBranchPrediction;
uint8_t  gSNT = 0x00;
uint8_t  gWNT = 0x01;
uint8_t  gWT  = 0x02;
uint8_t  gST  = 0x03;
uint8_t  gBht[1<<16];

// -------------- TOURNAMENT PREDICTOR DATA STRUCTURES and function declarations ---------------- //

void alphaInit();
void alphaTrainer(uint32_t pc, uint8_t outcome);
uint8_t alphaPredict(uint32_t pc);
// using the specification as described in 21264 paper
uint16_t alphaGHR = 0;
uint16_t alphaGSize;
uint16_t alphaPHTSize;
uint16_t alphaLPSize;
uint16_t alphaGMask = 0; // mask for GHR
uint16_t alphaPMask = 0; // mask for LP BHT
uint16_t alphaPHTMask = 0;
uint16_t alphaPHTPointer;
uint16_t alphaGPointer;
uint16_t alphaPHT[65536];
uint8_t  alphaLP[65536];
uint8_t  alphaCP[65536];
uint8_t  alphaGP[65536];
uint8_t  alphaCntrMask = 0x03; // mask for all the counters
uint8_t	 alphaCPrediction;
uint8_t	 alphaLPrediction;
uint8_t	 alphaGPrediction;
uint8_t	 alphaCPred;
uint8_t  alphaLPred;
uint8_t  alphaGPred;
uint8_t  alphaPred;
uint8_t  aSNT = 0x00;
uint8_t  aWNT = 0x01;
uint8_t  aWT  = 0x02;
uint8_t  aST  = 0x03;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  switch (bpType) {
    case STATIC:
    	// do nothing
    case GSHARE:
      gshareInit();
      break;        
    case TOURNAMENT:
      alphaInit();
      break;
    case CUSTOM:
    default:
      break;
  }


}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return gsharePredict(pc);
    case TOURNAMENT:
      return alphaPredict(pc);
    case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  switch (bpType) {
    case STATIC:
    case GSHARE:
      gshareTrainer(pc, outcome);
      break;
    case TOURNAMENT:
      alphaTrainer(pc, outcome);
      break;
    case CUSTOM:
    default:
      break;
  }
}
// ------------------ TOURNAMENT FUNCTIONS ------------------ //
void 
alphaInit()
{
  alphaPHTSize = (1<<(pcIndexBits-1));
  alphaGSize = (1<<(ghistoryBits-1));
  alphaLPSize = (1<<(lhistoryBits-1));
  // initializing all counters as Weakly Not Taken
  for(int i=0; i < alphaPHTSize; i++) {
    alphaPHT[i] = aWNT;
  }

  for(int i=0; i < alphaLPSize; i++) {
    alphaLP[i] = aWNT;
  }
  
  for(int i=0; i < alphaPHTSize; i++) {
    alphaPHT[i] = aWNT;
    alphaLP[i] = aWNT;
  }

  for(int i=0; i < alphaGSize; i++) {
    alphaGP[i] = aWNT;
    alphaCP[i] = aWNT;
  }
// alphaPMask for LP
  for(int i=0; i < lhistoryBits -1; i++) {
    alphaPMask ^= 1;
    alphaPMask <<= 1;
  }
// for PHT patterns
  for(int i=0; i < pcIndexBits -1; i++) {
    alphaPHTMask ^= 1;
    alphaPHTMask <<= 1;
  }
  for(int i=0; i < ghistoryBits -1; i++) {
    alphaGMask ^= 1;
    alphaGMask <<= 1;
  }
}

uint8_t alphaPredict(uint32_t pc)
{
  // get local prediction first
  alphaPHTPointer = pc & alphaPHTMask;
  alphaGPointer = alphaGHR & alphaGMask;
  alphaLPrediction = alphaLP[alphaPHT[alphaPHTPointer] & alphaPMask] & alphaCntrMask;
  alphaGPrediction = alphaGP[alphaGPointer] & alphaCntrMask;
  
  alphaCPrediction = alphaCP[alphaGHR & alphaGMask] & alphaCntrMask;
  
  if(alphaLPrediction == aSNT || alphaLPrediction == aWNT) {
    alphaLPred = 0;
  } else {
    alphaLPred = 1;
  }
  if(alphaGPrediction == aSNT || alphaGPrediction == aWNT) {
    alphaGPred = 0;
  } else {
    alphaGPred = 1;
  }
  // Choice only if predictions differ
  if(alphaCPrediction == aSNT || alphaCPrediction == aWNT) {
    return alphaLPred;
  } else {
    return alphaGPred;
  }
}

void 
alphaTrainer(uint32_t pc, uint8_t outcome)
{   
  // assign GHR
  alphaGHR = alphaGHR >> 1;
  alphaGHR = (alphaGHR | outcome << (ghistoryBits-1)) & alphaGMask;
  // assign Pattern in LP
  alphaPHT[alphaPHTPointer] = alphaPHT[alphaPHTPointer] >> 1;
  alphaPHT[alphaPHTPointer] = (alphaPHT[alphaPHTPointer] | outcome << (lhistoryBits-1)) & alphaPMask;
  // LP Training
  if (alphaLPrediction == aSNT) {
    if(outcome == alphaLPred)
      alphaLP[alphaPHT[alphaPHTPointer]] = aSNT;
    else
      alphaLP[alphaPHT[alphaPHTPointer]] = aWNT;
    } else if (alphaLPrediction == aWNT) {
    if(outcome == alphaLPred)
      alphaLP[alphaPHT[alphaPHTPointer]] = aSNT;
    else
      alphaLP[alphaPHT[alphaPHTPointer]] = aWT;
    } else if (alphaLPrediction == aWT) {
    if(outcome == alphaLPred)
      alphaLP[alphaPHT[alphaPHTPointer]] = aST;
    else
      alphaLP[alphaPHT[alphaPHTPointer]] = aWNT;
    } else {
    if(outcome == alphaLPred)
      alphaLP[alphaPHT[alphaPHTPointer]] = aST;
    else
      alphaLP[alphaPHT[alphaPHTPointer]] = aWT;
    }
    // GP Training
  if (alphaGPrediction == aSNT) {
    if(outcome == alphaGPred)
      alphaGP[alphaGPointer] = aSNT;
    else
      alphaGP[alphaGPointer] = aWNT;
    } else if (alphaGPrediction == aWNT) {
    if(outcome == alphaGPred)
      alphaGP[alphaGPointer] = aSNT;
    else
      alphaGP[alphaGPointer] = aWT;
    } else if (alphaGPrediction == aWT) {
    if(outcome == alphaGPred)
      alphaGP[alphaGPointer] = aST;
    else
      alphaGP[alphaGPointer] = aWNT;
    } else {
    if(outcome == alphaLPred)
      alphaGP[alphaGPointer] = aST;
    else
      alphaGP[alphaGPointer] = aWT;
    } 
  // CP Training
  if(alphaLPred != alphaGPred) {
    if(alphaCPrediction == aSNT) {
      if(outcome == alphaLPred)
        alphaCP[alphaGPointer] = aSNT;
      else
        alphaCP[alphaGPointer] = aWNT;
    } else if(alphaCPrediction == aWNT) {
      if(outcome == alphaLPred)
        alphaCP[alphaGPointer] = aSNT;
      else
        alphaCP[alphaGPointer] = aWT;
    } else if(alphaCPrediction == aWT) {
      if(outcome == alphaLPred)
        alphaCP[alphaGPointer] = aWNT;
      else
        alphaCP[alphaGPointer] = aST;
    } else {
      if(outcome == alphaLPred)
        alphaCP[alphaGPointer] = aWT;
      else
        alphaCP[alphaGPointer] = aST;
    }
  }
}

// ------------------ GSHARE FUNCTIONS ----------------------- //
void
gshareTrainer(uint32_t pc, uint8_t outcome)
{
    if (gBranchPrediction == gSNT) {
	if(outcome == gPred)
          gBht[gResultReg] = gSNT;
	else
	  gBht[gResultReg] = gWNT;
    } else if (gBranchPrediction == gWNT) {
	if(outcome == gPred)
          gBht[gResultReg] = gSNT;
	else
	  gBht[gResultReg] = gWT;
    } else if (gBranchPrediction == gWT) {
 	if(outcome == gPred)
          gBht[gResultReg] = gST;
	else
	  gBht[gResultReg] = gWNT;
    } else {
        if(outcome == gPred)
          gBht[gResultReg] = gST;
	else
	  gBht[gResultReg] = gWT;
    }
    gshareGHR = gshareGHR >> 1;
    gshareGHR = (gshareGHR | outcome << (ghistoryBits-1)) & gmask;
}

void 
gshareInit() 
{
  // making all gBhts as 01 or Weakly NOT TAKENs 
  gBhtSize = 1 << (ghistoryBits - 1);
  for(uint16_t i=0; i<gBhtSize; i++){
    gBht[i] = gWNT;
  }
  // mask generation
  for(int i=0; i<ghistoryBits; i++){
    gmask ^= 0x01;
    gmask = (gmask<<1); // to get correct register sizing for gshare
  }
}

uint8_t
gsharePredict(uint32_t pc)
{
// generate the result of XOR of GHR and gmask and use that as pointer to gBht
      gResultReg = (pc ^ gshareGHR) & gmask;
      gBranchPrediction = gBht[gResultReg] & gBhtMask;
      if (gBranchPrediction == gSNT || gBranchPrediction == gWNT)
      {	
	gPred = 0;
        return NOTTAKEN;
      } else {
	gPred = 1;
        return TAKEN;
      }
}
