//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <stdint.h>
#include "predictor.h"
#define HIST_LEN 0xFFFFF
//
// TODO:Student Information
//
const char *studentName = "Bryant Liu";
const char *studentID = "A13458492";
const char *email = "brl072@ucsd.edu";

//Gshare hint: http://people.cs.pitt.edu/~childers/CS2410/slides/lect-branch-prediction.pdf
//Gshare hint: https://www.youtube.com/watch?v=avp3bDqCXYM

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = {"Static", "Gshare",
                         "Tournament", "Custom"};

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
uint32_t gMask;
uint32_t gHist;
uint32_t gHistTable[HIST_LEN];
// uint32_t gHistTable[UINTN_MAX];

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //

  // initialize gshare
  gMask = 0;
  gHist = 0;
  for (int i = 0; i < ghistoryBits; i++) //setup the mask
  {
    gMask <<= 1;
    gMask += 1;
  }
  for (int i = 0; i < HIST_LEN; i++)
  {
    gHistTable[i] = WN;
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
  switch (bpType)
  {
  case STATIC:
    return TAKEN;
  case GSHARE:
    return gshare(pc);
  case TOURNAMENT:
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
void train_predictor(uint32_t pc, uint8_t outcome)
{
  // Train a prediction based on the bpType
  switch (bpType)
  {
  case STATIC:
    break;
  case GSHARE:
    train_ghsare(pc, outcome);
  case TOURNAMENT:
  case CUSTOM:
  default:
    break;
  }
}

// The Gshare predictor is characterized by XORing the global history register with the lower bits (same length as the global history) of the branch's address. This XORed value is then used to index into a 1D BHT of 2-bit predictors.
//need a shift register of the most recent taken or not taken
//combine global history with branch adress

uint8_t
gshare(uint32_t pc)
{
  uint32_t pcMasked = (pc & gMask);      //create a masked version of the PC counter
  uint32_t histMasked = (gHist & gMask); //create a masked version of the history counter
  uint32_t loc = pcMasked ^ histMasked;  //get the location to index into gHistTable
  // fprintf(stderr, "loc: %u        ghist: %u\n", loc, gHistTable[loc]);

  //return not taken if less than weakly taken (aka SN and WN)
  if (gHistTable[loc] < WT)
  {
    return NOTTAKEN;
  }
  //return taken otherwise
  return TAKEN;
}

//training of gShare
void train_ghsare(uint32_t pc, uint8_t outcome)
{
  uint32_t pcMasked = (pc & gMask);      //create a masked version of the PC counter
  uint32_t histMasked = (gHist & gMask); //create a masked version of the history counter
  uint32_t loc = pcMasked ^ histMasked;  //get the location to index into gHistTable

  if (outcome)
  {
    gHistTable[loc]++; //if outcome is not taken, we decrease gHist[i]
  }
  else
  {
    gHistTable[loc]--; //if outcome is taken, we increase gHist[i]
  }
  //bound check
  if (gHistTable[loc] > ST)
  {
    gHistTable[loc] = ST;
  } else if (gHistTable[loc] < SN)
  {
    gHistTable[loc] = SN;
  }
}