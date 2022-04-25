#include <iostream>
#include <fstream>
#include <iomanip>
#include "calcpulls.h"

void GNSN_WProbCalc::Initialize()
{
}

void GNSN_WProbCalc::OutputDebug()
{
  // Output some information for characters.
  std::ofstream ofs;
  if((initialized & 1) == 1)
  {
    ofs.open("GNSN_WProbCalc - Debug - Character Probabilities.txt", std::ofstream::out | std::ofstream::trunc);
    for(int pullCount = 0; pullCount < 90; pullCount++)
    {
      ofs << pullCount
        << std::fixed << std::setprecision(24)
        << "\t" << this->ProbSrc_SSRChar[pullCount] << "\n";
    }
    ofs << "\n\n\n";
    for(int pullCount = 0; pullCount < 90; pullCount++)
    {
      ofs << pullCount
        << std::fixed << std::setprecision(24)
        << "\t" << this->ProbSrcDist_SSRChar[pullCount] << "\n";
    }
    ofs.close();
  }


  // Output some information for weapons.
  if((initialized & 2) == 2)
  {
    ofs.open("GNSN_WProbCalc - Debug - Weapon Probabilities.txt", std::ofstream::out | std::ofstream::trunc);
    for(int pullCount = 0; pullCount < 80; pullCount++)
    {
      ofs << pullCount
        << std::fixed << std::setprecision(24)
        << "\t" << this->ProbSrc_SSRWeap[pullCount] << "\n";
    }
    ofs << "\n\n\n";
    for(int pullCount = 0; pullCount < 80; pullCount++)
    {
      ofs << pullCount
        << std::fixed << std::setprecision(24)
        << "\t" << this->ProbSrcDist_SSRWeap[pullCount] << "\n";
    }
    ofs.close();
  }
}

void GNSN_WProbCalc::OutputResults()
{
  // Output results for characters.
  std::ofstream ofs;
  if((initialized & 1) == 1)
  {
    ofs.open("GNSN_WProbCalc - Results - SSR Character Probabilities.txt", std::ofstream::out | std::ofstream::trunc);
    for(int pullCount = 0; pullCount < 1260; pullCount++)
    {
      ofs << (pullCount + 1);
      for(int conLevel = 0; conLevel < 7; conLevel++)
      {
        ofs << "\t";
        if(pullCount < (conLevel + 1) * 180)
        {
          ofs
            << std::fixed
            << std::setprecision(24)
            << this->ProbPL_SSRChar[conLevel][pullCount];
        }
      }
      ofs << "\n";
    }
    ofs.close();
  }


  // Output results for weapons.
  if((initialized & 2) == 2)
  {
    ofs.open("GNSN_WProbCalc - Results - SSR Weapon Probabilities.txt", std::ofstream::out | std::ofstream::trunc);
    for(int pullCount = 0; pullCount < 1200; pullCount++)
    {
      ofs << (pullCount + 1);
      for(int refineLevel = 0; refineLevel < 5; refineLevel++)
      {
        ofs << "\t";
        if(pullCount < (refineLevel + 1) * 240)
        {
          ofs
            << std::fixed
            << std::setprecision(24)
            << this->ProbPL_SSRWeap[refineLevel][pullCount];
        }
      }
      ofs << "\n";
    }
    ofs.close();
  }

  if((initialized & 4) == 4)
  {
    ofs.open("GNSN_WProbCalc - Results - SSR Pair Probabilities.txt", std::ofstream::out | std::ofstream::trunc);
    for(int pullCount = 0; pullCount < 2460; pullCount++)
    {
      ofs << (pullCount + 1);
      for(int conLevel = 0; conLevel < 7; conLevel++)
      {
        for(int refineLevel = 0; refineLevel < 5; refineLevel++)
        {
          ofs << "\t";
          if(pullCount < (conLevel + 1) * 180 + (refineLevel + 1) * 240)
          {
            ofs
              << std::fixed
              << std::setprecision(24)
              << this->ProbPL_SSRPair[conLevel][refineLevel][pullCount];
          }
        }
      }
      ofs << "\n";
    }
    ofs.close();
  }
}

void GNSN_WProbCalc::Clean()
{
  // Clean memory for character probabilities.
  if((initialized & 1) == 1)
  {
    for(int i = 0; i < 90; i++)
    {
      mpf_clear(this->ProbSrc_SSRChar[i]);
      mpf_clear(this->ProbSrcDist_SSRChar[i]);
    }
    for(int a = 0; a < 7; a++)
    {
      for(int b = 0; b < (a + 1) * 180; b++)
      {
        mpf_clear(this->ProbPL_SSRChar[a][b]);
      }
      delete[] this->ProbPL_SSRChar[a];
    }

    initialized = initialized ^ 1;
  }

  // Clean memory for weapon probabilities.
  if((initialized & 2) == 2)
  {
    for(int i = 0; i < 80; i++)
    {
      mpf_clear(this->ProbSrc_SSRWeap[i]);
      mpf_clear(this->ProbSrcDist_SSRWeap[i]);
    }
    for(int a = 0; a < 5; a++)
    {
      for(int b = 0; b < (a + 1) * 160; b++)
      {
        mpf_clear(this->ProbPL_SSRWeap[a][b]);
      }
      delete[] this->ProbPL_SSRWeap[a];
    }

    initialized = initialized ^ 2;
  }

  // Clean memory for probabilities of combined character and weapon duplicate levels.
  if((initialized & 4) == 4)
  {
    for(int conLevel = 0; conLevel < 7; conLevel++)
    {
      for(int refLevel = 0; refLevel < 5; refLevel++)
      {
        int maxPulls = (conLevel + 1) * 180 + (refLevel + 1) * 240;
        for(int pullCount = 0; pullCount < maxPulls; pullCount++)
        {
          mpf_clear(this->ProbPL_SSRPair[conLevel][refLevel][pullCount]);
        }
        delete[] this->ProbPL_SSRPair[conLevel][refLevel];
      }
    }

    initialized = initialized ^ 4;
  }

  initialized = 0;
}