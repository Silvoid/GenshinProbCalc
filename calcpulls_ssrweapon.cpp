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

// Descriptions of un-optimized code you may find here:
//   (1) Doesn't immediately stop calculating for a permutation with a zero percent probability.
//   (2) Extra memory allocations.

double getNumber(const mpf_t& target)
{
  return mpf_get_d(target);
}

// This is for calculating the probabilities for five-star weapons in Genshin Impact.
// For this case, Genshin Impact five-star weapons got labeled as "SSR"s.
void GNSN_WProbCalc::CalcSSRWeapon()
{
  if((initialized & 2) == 2)
    return;

  // Use a default precision of at least 256 bits for the next MPF variables to be initialized.
  mpf_set_default_prec(256);
  

  // Generic variables.
  mpf_t gA, gB, gC, gD, gE, gF, gG, gH;
  mpf_init(gA);
  mpf_init(gB);
  mpf_init(gC);
  mpf_init(gD);
  mpf_init(gE);
  mpf_init(gF);
  mpf_init(gG);
  mpf_init(gH);

  // ----- #
  // Source probability.
  // Probability per pull count to pull any five-star.
  // ----- #
  
  // Setup specific values.
  mpf_set_d(gA, 7.0);
  mpf_set_d(gB, 1000.0);
  mpf_div(gA, gA, gB);   // 7 / 1000 = 0.007 (0.7%).
                         // - Default probability for acquisition of a five-star per pull.
  mpf_set_d(gC, 70.0);
  mpf_div(gB, gC, gB); // 70 / 1000 = 0.07 (7%).
                       // - Increment of probability for acquisition of a five-star per pull during "soft pity".

  for(int pullCount = 0; pullCount < 80; pullCount++)
  {
    // Get and set the location to store the calculated probability.
    mpf_t& tarMemAdd = this->ProbSrc_SSRWeap[pullCount];
    mpf_init(tarMemAdd); mpf_set_d(tarMemAdd, 0.0);

    // Get and set the probability for any five-star to occur on this pull count.
    if(pullCount > 75)
    {
      // Guaranteed for a five-star to occur.
      mpf_set_d(tarMemAdd, 1.0);
    }
    else if(pullCount > 61)
    {
      // Soft pity.
      // While not the last pull before a 100% probability, and is after 61 (the 62nd pull),
      // - consider it as "soft pity" where each pull after the 0th has an increased probability.
      // Intended hard-coded limit for the increase of probability through soft pity:
      // - (78 - 61) * 0.07 + 0.007 equals 1.127, which is more than 100%.
      // - | (x - 61) * 0.07 + 0.007 = 1
      // - |                       x = (1 - 0.007) / 0.07 + 61
      // - |                       x = ~75.19
      // - After the 75th iteration (76th pull), the chance is greater than 100%. 
      mpf_set_d(tarMemAdd, pullCount - 61); // Get the number of pulls done in soft pity.
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
  for(int pullCount = 0; pullCount < 80; pullCount++)
  {
    // Get and set the percentage who acquired a five-star.
    mpf_t& tarMemAdd = this->ProbSrcDist_SSRWeap[pullCount];
    mpf_init(tarMemAdd);
    mpf_mul(tarMemAdd, this->ProbSrc_SSRWeap[pullCount], gA);

    // Subtract the percentage who acquired a five-star from the remaining population.
    mpf_sub(gA, gA, tarMemAdd);
  }

  // ----- #
  // Base probability.
  // Probability per pull count to pull the specific event-wish featured five-star.
  // ----- #

  // Setup specific values.
  mpf_set_d(gA, 0.5);  // Store in "gA", the probability for both winning and losing a 50/50.
  mpf_set_d(gB, 3);    // Numerator.
  mpf_set_d(gC, 4);    // Denominator.
  mpf_div(gB, gB, gC); // Store in "gB", 3 / 4 = 75% chance to get one of the two featured five-star.
  mpf_mul(gC, gA, gB); // Store in "gC", 0.75 * 0.5 = 37.5% chance to be the specific five-star.
                       // - Coincidentally, it's also 37.5% chance to be one of the featured five-star, but not be the specific five-star.


  // TODO: These variables are currently conflicting with old code.
  mpf_mul(gD,          // Probability for second five-star to be the specific five-star when the first was a featured five-star.
    gB,                // - 75% probability for first to be a featured five-star.
    gA);               // - 50% probability for the first to not be the specific five-star.
  mpf_mul(gD,          // 
    gD,                // 
    gC);               // - 37.5% probability for the second to be the specific five-star.
  mpf_set_d(gE, 1.0);  //
  mpf_sub(gE, gE, gB); // 25% chance to not be one of the two featured five-star.
  mpf_mul(gE,          // Probability for second five-star to be the specific five-star when the second was not a featured five-star.
    gE,                // - 25% probability for first to not be a featured five-star.
    gA);               // - 50% probability for second to be the specific five-star.

  // Probability for second five-star to be the specific five-star.
  mpf_add(gD, gD, gE); // 0.75 * 0.5 * 0.375 + 0.25 * 0.5 = 0.265625 (26.5625%) 

  // Get the probability for the second five-star to not be the specific five-star.
  mpf_set_d(gF, 1.0);  //
  mpf_sub(gF, gF, gC); // 1 - 0.375 = 0.625 (62.5%)
                       // - probability for the second five-star to not be the specific five-star after the first was not a featured five-star.
  mpf_mul(gG,          //
    gB,                // 75% probability for the first to be a featured five-star.
    gA);               // 50% probability for the first to not be the specific five-star.
  mpf_mul(gG,          //
    gG,                //
    gF);               // 62.5% probability for the second to not be the specific five-star.
  mpf_add(gE, gE, gG); // Probability for the second five-star to not be the specific five-star.

  // Initialize relevant memory.
  for(int refineLevel = 0; refineLevel < 5; refineLevel++)
  {
    int maxPullsForRefine = (refineLevel + 1) * 240;
    this->ProbPL_SSRWeap[refineLevel] = new mpf_t[maxPullsForRefine];
    for(int pullCount = 0; pullCount < maxPullsForRefine; pullCount++)
    {
      mpf_init(this->ProbPL_SSRWeap[refineLevel][pullCount]);
      mpf_set_d(this->ProbPL_SSRWeap[refineLevel][pullCount], 0.0);
    }
  }

  // The first copy.
  // Calculate the probabilities for which pull count the first copy of a specific event-wish featured five-star could occur on.
  // Calculate the probabilities for which pull count the first five-star could occur on.
  for(int pullCountA = 0; pullCountA < 80; pullCountA++)
  {
    mpf_t& pSrcDistA = this->ProbSrcDist_SSRWeap[pullCountA];
    // Calculate the probabilities for which pull count the second five-star could occur on.
    for(int pullCountB = 0; pullCountB < 80; pullCountB++)
    {
      mpf_t& pSrcDistB = this->ProbSrcDist_SSRWeap[pullCountB];

      // For the third five-star.
      // Probability for the second five-star to not be the specific five-star.
      mpf_mul(gF, gE, pSrcDistA); // - With probability for the first to occur.
      mpf_mul(gF, gF, pSrcDistB); // - With probability for the second to occur.

      // Calculate the probabilities for which pull count the third five-star could occur on.
      for(int pullCountC = 0; pullCountC < 80; pullCountC++)
      {
        mpf_t& tarMemAdd = this->ProbPL_SSRWeap[0][pullCountA + pullCountB + pullCountC + 2];
        mpf_mul(gH,                               // Store in "gH", the product of the following:
          gF,                                     // - (1) Probability for the second five-star to have occurred and for there yet to be the specific five-star.
          this->ProbSrcDist_SSRWeap[pullCountC]); // - (2) Probability for this third five-star to occur.
        mpf_add(tarMemAdd, tarMemAdd, gH);        // Add the probability to storage.
      }

      // Add the probability for this pull count to be the specific five-star to storage.
      mpf_t& tarMemAdd = this->ProbPL_SSRWeap[0][pullCountA + pullCountB + 1];
      // Probability for the second five-star to be the specific five-star after both
      // - (1) the first five-star failed to be a featured five-star, and
      // - (2) the first five-star was a featured five-star but not the specific five-star...
      mpf_mul(gF, gD, pSrcDistA); // - With probability for the first to occur.
      mpf_mul(gF, gF, pSrcDistB); // - With probability for the second to occur.
      mpf_add(tarMemAdd, tarMemAdd, gF);
    }

    // Add the probability for this pull count to be the specific five-star.
    mpf_t& tarMemAdd = this->ProbPL_SSRWeap[0][pullCountA];
    mpf_mul(gF, gC, pSrcDistA);
    mpf_add(tarMemAdd, tarMemAdd, gF);
  }

  // The duplicates.
  // Calculate the probabilities for which pull count each refinement rank could occur on.
  for(int refineLevel = 1; refineLevel < 5; refineLevel++)
  {
    // Iterate through the possible pull counts for the previous constellation level.
    for(int pullCountA = 0; pullCountA < refineLevel * 240; pullCountA++)
    {
      // Iterate through the possible pull counts for the target constellation level.
      for(int pullCountB = 0; pullCountB < 240; pullCountB++)
      {
        // Set the probability for (1) this copy to have occured on (2) this pull count after (3) the pull count for the previous copy.
        mpf_mul(gA,                                         // Generic storage variable.
          this->ProbPL_SSRWeap[0][pullCountB],              // Probability for the specific five-star to occur on pull count B.
          this->ProbPL_SSRWeap[refineLevel - 1][pullCountA] // Probability for the previous specific five-star to have occured on pull count A.
        );

        // Add the probability to storage.
        mpf_t& tarMemAdd = this->ProbPL_SSRWeap[refineLevel][pullCountA + pullCountB + 1];
        mpf_add(tarMemAdd, tarMemAdd, gA);
      }
    }
  }

  initialized = (initialized | 2);

  // Clean memory of temporary variables.
  mpf_clear(gA);
  mpf_clear(gB);
  mpf_clear(gC);
  mpf_clear(gD);
  mpf_clear(gE);
  mpf_clear(gF);
  mpf_clear(gG);
  mpf_clear(gH);
}
