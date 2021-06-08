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
#include <string.h>
#define HIST_LEN 0xFFFFFF
//
// TODO:Student Information
//
const char *studentName = "Bryant Liu";
const char *studentID = "A13458492";
const char *email = "brl072@ucsd.edu";

const char *studentName2 = "Bao Hoang";
const char *studentID2 = "A14722199";
const char *email2 = "bghoang@ucsd.edu";

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

uint32_t tGlobMask;
uint32_t tGlobHist;
uint32_t tPCMask;
uint32_t tLocalHistMask;
uint32_t tGlob[HIST_LEN];
uint32_t tHistLocal[HIST_LEN];
uint32_t tPredictLocal[HIST_LEN];
uint32_t tSelect[HIST_LEN];
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

  //initialize tournament
  tGlobHist = 0;
  tGlobMask = 0;
  tLocalHistMask = 0;
  tPCMask = 0;
  for (int i = 0; i < ghistoryBits; i++)
  {
    tGlobMask <<= 1;
    tGlobMask += 1;
  }
  for (int i = 0; i < lhistoryBits; i++)
  {
    tLocalHistMask <<= 1;
    tLocalHistMask += 1;
  }
  for (int i = 0; i < pcIndexBits; i++)
  {
    tPCMask <<= 1;
    tPCMask += 1;
  }
  for (int i = 0; i < HIST_LEN; i++)
  {
    tGlob[i] = WN;         //Setup global predictor for tournament
    tSelect[i] = 3;        //Setup choice predictor for tournament
    tHistLocal[i] = 0;     //Setup local history for tournament
    tPredictLocal[i] = WN; //Setup local predictions for tournament
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t make_prediction(uint32_t pc)
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
    return tournament(pc);
  case CUSTOM:
    return custom(pc);
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
    train_gshare(pc, outcome);
  case TOURNAMENT:
    train_tournament(pc, outcome);
  case CUSTOM:
    train_custom(pc, outcome);
  default:
    break;
  }
}

// The Gshare predictor is characterized by XORing the global history register with 
// the lower bits (same length as the global history) of the branch's address. 
// This XORed value is then used to index into a 1D BHT of 2-bit predictors.
//need a shift register of the most recent taken or not taken
//combine global history with branch adress

uint8_t gshare(uint32_t pc)
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

uint8_t tournament(uint32_t pc)
{
  uint8_t outcome = NOTTAKEN;
  tGlobHist &= tGlobMask;                                 //tGlobHist is the gloabl history of previous branches
  uint32_t histLocalIdx = (pc & tPCMask);                 //Masked version of the pc and PC mask to index the local history table
  uint32_t tLocalHist = tHistLocal[histLocalIdx];         //indexing the local history table to get the local history of that PC
  uint32_t tLocalPreditIdx = tLocalHistMask & tLocalHist; //getting the index of the prediction of that local history

  if (tSelect[tGlobHist] > WN)
  {
    //use global
    outcome = (tGlob[tGlobHist] > WN) ? TAKEN : NOTTAKEN;
  }
  else
  {
    //use local
    outcome = (tPredictLocal[tLocalPreditIdx] > WN) ? TAKEN : NOTTAKEN;
  }
  return outcome;
}

uint8_t custom(uint32_t pc)
{
  uint8_t outcome = NOTTAKEN;
  tGlobHist &= tGlobMask;                                 //tGlobHist is the gloabl history of previous branches
  uint32_t histLocalIdx = (pc & tPCMask);                 //Masked version of the pc and PC mask to index the local history table
  uint32_t tLocalHist = tHistLocal[histLocalIdx];         //indexing the local history table to get the local history of that PC
  uint32_t tLocalPreditIdx = tLocalHistMask & tLocalHist; //getting the index of the prediction of that local history

  if (tSelect[tGlobHist] > 3)
  {
    //use global
    outcome = (tGlob[tGlobHist] > WN) ? TAKEN : NOTTAKEN;
  }
  else
  {
    //use local
    outcome = (tPredictLocal[tLocalPreditIdx] > WN) ? TAKEN : NOTTAKEN;
  }
  return outcome;
}

//training of gShare
void train_gshare(uint32_t pc, uint8_t outcome)
{
  uint32_t pcMasked = (pc & gMask);      //create a masked version of the PC counter
  uint32_t histMasked = (gHist & gMask); //create a masked version of the history counter
  uint32_t loc = pcMasked ^ histMasked;  //get the location to index into gHistTable

  if (outcome == TAKEN && gHistTable[loc] < ST)
  {
    gHistTable[loc]++; //if outcome is taken, we increase gHistTable[i]
  }
  if (outcome != TAKEN && gHistTable[loc] > SN)
  {
    gHistTable[loc]--; //if outcome is not taken, we decrease gHistTable[i]
  }

  gHist <<= 1;      //push the history down
  gHist |= outcome; //mix with the outcome
}

//training of tournament
void train_tournament(uint32_t pc, uint8_t outcome)
{
  tGlobHist &= tGlobMask;                                 //Masked version of the history counter, anded with the mask
  uint32_t histLocalIdx = (pc & tPCMask);                 //Masked version of the pc and PC mask to index the local history table
  uint32_t tLocalHist = tHistLocal[histLocalIdx];         //indexing the local history table to get the local history of that PC
  uint32_t tLocalPreditIdx = tLocalHistMask & tLocalHist; //getting the index of the prediction of that local history

  int localCorrect = 0;
  if ((outcome == TAKEN && tPredictLocal[tLocalPreditIdx] > WN) || (outcome == NOTTAKEN && tPredictLocal[tLocalPreditIdx] <= WN))
  {
    localCorrect = 1;
  }

  //update selector, only when local differs from global
  if (tGlob[tGlobHist] != tPredictLocal[tLocalPreditIdx])
  {
    if (localCorrect == 1) //if the local one is right?
    {
      if (tSelect[tGlobHist] > SN)
      {
        tSelect[tGlobHist]--;
      }
    }
    else
    {
      if (tSelect[tGlobHist] < ST)
      {
        tSelect[tGlobHist]++;
      }
    }
  }

  //update global history as well as local prediction for that local history
  if (outcome == TAKEN)
  {
    if (tGlob[tGlobHist] != ST)
    {
      tGlob[tGlobHist]++;
    }
    if (tPredictLocal[tLocalPreditIdx] != ST)
    {
      tPredictLocal[tLocalPreditIdx]++;
    }
  }
  else
  {
    if (tGlob[tGlobHist] != SN)
    {
      tGlob[tGlobHist]--;
    }
    if (tPredictLocal[tLocalPreditIdx] != SN)
    {
      tPredictLocal[tLocalPreditIdx]--;
    }
  }

  //update localHist table for this PC
  tHistLocal[histLocalIdx] <<= 1;
  tHistLocal[histLocalIdx] += outcome;
  tHistLocal[histLocalIdx] &= tLocalHistMask;

  //update tGlobHist with new out come
  tGlobHist <<= 1;
  tGlobHist |= outcome;
}

//training of custom
void train_custom(uint32_t pc, uint8_t outcome)
{
  tGlobHist &= tGlobMask;                                 //Masked version of the history counter, anded with the mask
  uint32_t histLocalIdx = (pc & tPCMask);                 //Masked version of the pc and PC mask to index the local history table
  uint32_t tLocalHist = tHistLocal[histLocalIdx];         //indexing the local history table to get the local history of that PC
  uint32_t tLocalPreditIdx = tLocalHistMask & tLocalHist; //getting the index of the prediction of that local history

  int localCorrect = 0;
  if ((outcome == TAKEN && tPredictLocal[tLocalPreditIdx] > WN) || (outcome == NOTTAKEN && tPredictLocal[tLocalPreditIdx] <= WN))
  {
    localCorrect = 1;
  }

  //update selector, only when local differs from global
  if (tGlob[tGlobHist] != tPredictLocal[tLocalPreditIdx])
  {
    if (localCorrect == 1) //if the local one is right?
    {
      if (tSelect[tGlobHist] > 0)
      {
        tSelect[tGlobHist]--;
      }
    }
    else
    {
      if (tSelect[tGlobHist] < 7)
      {
        tSelect[tGlobHist]++;
      }
    }
  }

  //update global history as well as local prediction for that local history
  if (outcome == TAKEN)
  {
    if (tGlob[tGlobHist] != ST)
    {
      tGlob[tGlobHist]++;
    }
    if (tPredictLocal[tLocalPreditIdx] != ST)
    {
      tPredictLocal[tLocalPreditIdx]++;
    }
  }
  else
  {
    if (tGlob[tGlobHist] != SN)
    {
      tGlob[tGlobHist]--;
    }
    if (tPredictLocal[tLocalPreditIdx] != SN)
    {
      tPredictLocal[tLocalPreditIdx]--;
    }
  }

  //update localHist table for this PC
  tHistLocal[histLocalIdx] <<= 1;
  tHistLocal[histLocalIdx] += outcome;
  tHistLocal[histLocalIdx] &= tLocalHistMask;

  //update tGlobHist with new out come
  tGlobHist <<= 1;
  tGlobHist |= outcome;
}