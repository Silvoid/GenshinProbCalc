#pragma once
#include <mpir.h>

class GNSN_WProbCalc
{
private:
  int initialized = 0;

private:
  // ---- #
  // Source probability for acqusition of any SSR per pull count.
  // These should store the probability to acquire a five-star when landing on a specific pull count.
  // ---- #

  mpf_t ProbSrc_SSRChar[90];
  mpf_t ProbSrc_SSRWeap[80];



  // ---- #
  // Distribution for acquisition of any SSR between pull counts.
  // These should store the probabilities for which pull count a five-star could occur on.
  // ---- #

  mpf_t ProbSrcDist_SSRChar[90];
  mpf_t ProbSrcDist_SSRWeap[80];



  // ---- #
  // Probability per level of an SSR per pull count.
  // ---- #

  // A pointer to memory for the probabilities for seven copies,
  // --- each copy needing at most 180 pulls more than the last.
  mpf_t* ProbPL_SSRChar[7];

  // A pointer to memory for the probabilities for five refinements,
  // --- each copy needing 240 pulls more than the last.
  mpf_t* ProbPL_SSRWeap[5];

  // A pointer to memory for the probabilities for each variation of duplicate levels for character and weapons.
  mpf_t* ProbPL_SSRPair[7][5]; 



public:
  // Deprecated.
  void Initialize();

  void CalcSSRCharacter();
  void CalcSSRWeapon();
  void CalcSSRPair();
  void OutputDebug();
  void OutputResults();
  void Clean();



public:
  ~GNSN_WProbCalc()
  {
    Clean();
  }
};