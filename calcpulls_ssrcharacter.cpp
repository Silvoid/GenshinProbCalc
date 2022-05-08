#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <mpir.h>

#include "calcpulls.h"

// The documentation I, the programmer, read to make use of the "MPIR (Multiple Precision Integers and Rationals)" library that I decided to use.
// http://www.mpir.org/mpir-3.0.0.pdf

// Reference(s) used for calculating probability.
// https://www.hoyolab.com/article/497840

// This is for calculating the probabilities for five-star characters in Genshin Impact.
// For this case, Genshin Impact five-star characters got labeled as "SSR"s.
void GNSN_WProbCalc::CalcSSRCharacter()
{
  if((initialized & 1) == 1)
    return;

  // Use a default precision of at least 256 bits for the next MPF variables to be initialized.
  mpf_set_default_prec(256);

  // Generic variables.
  mpf_t gA, gB, gC;
  mpf_init(gA);
  mpf_init(gB);
  mpf_init(gC);

  // ----- #
  // Source probability.
  // Probability per pull count to pull any five-star.
  // ----- #

  // Setup specific values.
  mpf_set_d(gA, 6.0);
  mpf_set_d(gB, 1000.0);
  mpf_div(gA, gA, gB);   // 6 / 1000 = 0.006 (0.6%).
                         // - Default probability for acquisition of a five-star per pull.
  mpf_set_d(gC, 60.0);
  mpf_div(gB, gC, gB); // 60 / 1000 = 0.06 (6%).
                       // - Increment of probability for acquisition of a five-star per pull during "soft pity".

  for(int pullCount = 0; pullCount < 90; pullCount++)
  {
    // Get and set the location to store the calculated probability.
    mpf_t& tarMemAdd = this->ProbSrc_SSRChar[pullCount];
    mpf_init(tarMemAdd); mpf_set_d(tarMemAdd, 0.0);

    // Get and set the probability for any five-star to occur on this pull count.
    if(pullCount == 89)
    {
      // Guaranteed for a five-star to occur.
      mpf_set_d(tarMemAdd, 1.0);
    }
    else if(pullCount > 72)
    {
      // Soft pity.
      // While not the last pull before a 100% probability, and is after 72 (the 73rd pull),
      // - consider it as "soft pity" where each pull after the 0th has an increased probability.
      // Intended hard-coded limit for the increase of probability through soft pity:
      // - (88 - 72) * 0.06 + 0.006 equals 0.966, which is less than 100%.
      // - So, no need to adjust the check for setting the probability to 100%.
      mpf_set_d(tarMemAdd, pullCount - 72); // Get the number of pulls done in soft pity.
      mpf_mul(tarMemAdd, tarMemAdd, gB);    // Multiply it with the base increment value.
      mpf_add(tarMemAdd, tarMemAdd, gA);    // Add the original rate for acquisition of any five star.
    }
    else
    {
      // The base rate for acqusition of any five star.
      mpf_set(tarMemAdd, gA);
    }
  }

  // ----- #
  // Source distribution of probabilities.
  // ----- #

  // Setup specific values.
  mpf_set_d(gA, 1.0); // Remainining population, where 100% is yet to acquire a five-star.

  // Calculate the probabilities for which pull count a five-star will specifically occur on.
  for(int pullCount = 0; pullCount < 90; pullCount++)
  {
    // Get and set the percentage who acquired a five-star.
    mpf_t& tarMemAdd = this->ProbSrcDist_SSRChar[pullCount];
    mpf_init(tarMemAdd);
    mpf_mul(tarMemAdd, this->ProbSrc_SSRChar[pullCount], gA);

    // Subtract the percentage who acquired a five-star from the remaining population.
    mpf_sub(gA, gA, tarMemAdd);
  }

  // ----- #
  // Base probability.
  // Probability per pull count to pull the specific event-wish featured five-star.
  // ----- #

  // Setup specific values.
  mpf_set_d(gA, 0.5); // The probability for both winning and losing a 50/50.

  // Initialize relevant memory.
  for(int conLevel = 0; conLevel < 7; conLevel++)
  {
    int maxPullsForCon = (conLevel + 1) * 180;
    this->ProbPL_SSRChar[conLevel] = new mpf_t[maxPullsForCon];
    for(int pullCount = 0; pullCount < maxPullsForCon; pullCount++)
    {
      mpf_init(this->ProbPL_SSRChar[conLevel][pullCount]);
      mpf_set_d(this->ProbPL_SSRChar[conLevel][pullCount], 0.0);
    }
  }

  // The first copy.
  // Calculate the probabilities for which pull count the first copy of a specific event-wish featured five-star could occur on.
  // Calculate the probabilities for which pull count the first five-star could occur on.
  for(int pullCountA = 0; pullCountA < 90; pullCountA++)
  {
    mpf_t& pSrcDistA = this->ProbSrcDist_SSRChar[pullCountA];
    // Calculate the probabilities for which pull count the second five-star, which is the guaranteed (after "losing the 50/50") five-star, could occur on.
    for(int pullCountB = 0; pullCountB < 90; pullCountB++)
    {
      mpf_t& tarMemAdd = this->ProbPL_SSRChar[0][pullCountA + pullCountB + 1];
      mpf_t& pSrcDistB = this->ProbSrcDist_SSRChar[pullCountB];
      mpf_mul(gB, gA, pSrcDistA);        // Get the probability for the first five-star to have occured and became a failed 50/50.
      mpf_mul(gB, gB, pSrcDistB);        // Set the probability for this guaranteed five-star to occur on this pull count.
      mpf_add(tarMemAdd, tarMemAdd, gB); // Add the probability to storage.
    }

    // Add the probability for this pull count to have yielded a won 50/50 to storage.
    mpf_t& tarMemAdd = this->ProbPL_SSRChar[0][pullCountA];
    mpf_mul(gB, gA, this->ProbSrcDist_SSRChar[pullCountA]);
    mpf_add(tarMemAdd, tarMemAdd, gB);
  }

  // The duplicates.
  // Calculate the probabilities for which pull count each constellation level could occur on.
  for(int conLevel = 1; conLevel < 7; conLevel++)
  {
    // Iterate through the possible pull counts for the previous constellation level.
    for(int pullCountA = 0; pullCountA < conLevel * 180; pullCountA++)
    {
      // Iterate through the possible pull counts for the target constellation level.
      for(int pullCountB = 0; pullCountB < 180; pullCountB++)
      {
        // Set the probability for (1) this copy to have occured on (2) this pull count after (3) the pull count for the previous copy.
        mpf_mul(gA,                                      // Generic storage variable.
          this->ProbPL_SSRChar[0][pullCountB],           // Probability for the specific five-star to occur on pull count B.
          this->ProbPL_SSRChar[conLevel - 1][pullCountA] // Probability for the previous specific five-star to have occured on pull count A.
        );

        // Add the probability to storage.
        mpf_t& tarMemAdd = this->ProbPL_SSRChar[conLevel][pullCountA + pullCountB + 1];
        mpf_add(tarMemAdd, tarMemAdd, gA);
      }
    }
  }

  initialized = (initialized | 1);

  // Clean memory of temporary variables.
  mpf_clear(gA);
  mpf_clear(gB);
  mpf_clear(gC);
}