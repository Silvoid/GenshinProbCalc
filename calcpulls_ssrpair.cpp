#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <mpir.h>

#include "calcpulls.h"

void GNSN_WProbCalc::CalcSSRPair()
{
  // Make sure dependencies are there.
  if((initialized & 1) != 1)
  {
    this->CalcSSRCharacter();
  }
  if((initialized & 2) != 2)
  {
    this->CalcSSRWeapon();
  }
  if((initialized & 4) == 4)
  {
    return;
  }

  mpf_set_default_prec(256);

  // Generic variables.
  mpf_t gA;
  mpf_init(gA);

  // Initialize relevant memory.
  for(int conLevel = 0; conLevel < 7; conLevel++)
  {
    for(int refLevel = 0; refLevel < 5; refLevel++)
    {
      int maxPulls = (conLevel + 1) * 180 + (refLevel + 1) * 240;
      mpf_t*& tarMemAdd = this->ProbPL_SSRPair[conLevel][refLevel];
      tarMemAdd = new mpf_t[maxPulls];
      for(int pullCount = 0; pullCount < maxPulls; pullCount++)
      {
        mpf_init(tarMemAdd[pullCount]);
        mpf_set_d(tarMemAdd[pullCount], 0.0);
      }
    }
  }

  // Calculate probabilities.
  for(int conLevel = 0; conLevel < 7; conLevel++)
  {
    for(int refLevel = 0; refLevel < 5; refLevel++)
    {
      for(int pullCountA = 0; pullCountA < (conLevel + 1) * 180; pullCountA++)
      {
        for(int pullCountB = 0; pullCountB < (refLevel + 1) * 240; pullCountB++)
        {
          mpf_mul(gA,
            this->ProbPL_SSRChar[conLevel][pullCountA],
            this->ProbPL_SSRWeap[refLevel][pullCountB]);
          mpf_t& tarMemAdd = this->ProbPL_SSRPair[conLevel][refLevel][pullCountA + pullCountB + 1];
          mpf_add(tarMemAdd, tarMemAdd, gA);
        }
      }
    }
  }

  mpf_clear(gA);
  initialized = initialized | 4;
}