// Copyright 2015 Alexandre Vaillancourt
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
// and associated documentation files (the "Software"), to deal in the Software without 
// restriction, including without limitation the rights to use, copy, modify, merge, publish, 
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all copies or 
// substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <unordered_map>
#include <vector>
#include <iostream>
#include <ctime>

#include <boost/random.hpp>


typedef std::unordered_map<unsigned int, unsigned int> Echelles;
typedef std::unordered_map<unsigned int, unsigned int> Serpents;

typedef boost::random::uniform_int_distribution<> RndGen;

boost::random::mt19937 rng;

const Echelles
getEchelles()
{
  Echelles echelles;
  echelles[2]  = 18;
  echelles[8]  = 50;
  echelles[10] = 30;
  echelles[26] = 74;
  echelles[40] = 60;
  echelles[44] = 64;
  echelles[52] = 60;
  echelles[68] = 94;
  echelles[78] = 100;
  echelles[84] = 96;

  return echelles;
}

const Serpents
getSerpents()
{
  Serpents serpents;
  serpents[16] = 6;
  serpents[22] = 20;
  serpents[32] = 7;
  serpents[48] = 28;
  serpents[58] = 24;
  serpents[76] = 56;
  serpents[80] = 42;
  serpents[86] = 36;
  serpents[92] = 72;
  serpents[98] = 62;

  return serpents;
}

static const unsigned int DICE_SIZE = 10u;
static const unsigned int DICE_STREAK_MAX = 10u;
static const unsigned int END_MISS_STREAK_MAX = 20u;
static const unsigned int RUNS = 1000000u;
#define VERBOSE 0
#define VERBOSE_STATS 1

RndGen 
getAGen()
{
  return RndGen(1,DICE_SIZE);
}


int main(void)
{

  RndGen gen = getAGen();
  boost::random::mt19937 engine(time(0));
  const Echelles echelles = getEchelles();
  const Serpents serpents = getSerpents();

  std::vector<unsigned int> laddersHits(100u, 0u);
  std::vector<unsigned int> snakesHits(100u, 0u);
  std::vector<unsigned int> diceHits( DICE_SIZE + DICE_STREAK_MAX, 0u );
  std::vector<unsigned int> endMissStreaks( END_MISS_STREAK_MAX, 0u );
  unsigned int diceRollCount = 0u;
  unsigned int turnCount = 0u;

  for ( unsigned int runIdx = 0u; runIdx < RUNS; ++runIdx )
  {

    unsigned int endMissStreak = 0u;

    unsigned int currentPos = 0;
    while(currentPos != 100u)
    {
#if VERBOSE
      std::cout << "starting turn at " << currentPos << std::endl;
#endif

      const unsigned int initialPosition = currentPos;
      int diceRoll = gen(engine);
      ++diceRollCount;
#if VERBOSE
      std::cout << "rolled " << diceRoll << std::endl;
#endif
      unsigned int maxValueStreak = 0u;
      while( DICE_SIZE == diceRoll )
      {
        ++maxValueStreak;
        currentPos += diceRoll;
        diceRoll = gen(engine);
        ++diceRollCount;
#if VERBOSE
      std::cout << DICE_SIZE << ", rerolled " << diceRoll << std::endl;
#endif
      }

      currentPos += diceRoll;
      diceHits[diceRoll - 1u] += 1;
      if ( 0u != maxValueStreak )
      {
        maxValueStreak = std::min( maxValueStreak, DICE_STREAK_MAX );
        diceHits[DICE_SIZE - 2 + maxValueStreak] += 1; // Store the values of the streak; max dice value stores the single occurence, max dice + 1 stores the 2 occurences in a row, etc..
      }
#if VERBOSE
      std::cout << "currently at " << currentPos << std::endl;
#endif

      const auto echelle = echelles.find(currentPos);
      const auto serpent = serpents.find(currentPos);

      if(echelle != echelles.end())
      {
        currentPos = echelle->second;
        laddersHits[echelle->first] += 1u;
#if VERBOSE
      std::cout << "ladder(" << echelle->first << "), going up at " << currentPos << std::endl;
#endif
      }
      else if (serpent != serpents.end())
      {
        currentPos = serpent->second;
        snakesHits[serpent->first] += 1u;
#if VERBOSE
      std::cout << "snake(" << serpent->first << "), going down at " << currentPos << std::endl;
#endif
      }

      if ( currentPos > 100u )
      {
        currentPos = initialPosition;
        ++endMissStreak;
#if VERBOSE
      std::cout << "went over 100, going back down to " << currentPos << std::endl;
#endif
      }
      else
      {
        if ( endMissStreak != 0u )
        {
          endMissStreak = std::min( endMissStreak, END_MISS_STREAK_MAX );
          endMissStreaks[endMissStreak - 1u] += 1u;
          endMissStreak = 0u;
        }
      }
      ++turnCount;
    }

    if ( runIdx % 1000u == 0u)
      std::cout << runIdx << std::endl;
  }
#if VERBOSE
  std::cout << "game end; turns (" << turnCount << ") diceRolls (" << diceRollCount << ")" << std::endl;
#endif

#if VERBOSE_STATS
  std::ostringstream ossLadders;
  std::ostringstream ossSnakes;
  std::ostringstream ossDiceHits;
  std::ostringstream ossEndMissStreaks;

  const double dRUNS = static_cast<double>( RUNS );

  for ( unsigned int i = 0u; i < 100u; ++i )
  {
    const auto echelle = echelles.find(i);
    const auto serpent = serpents.find(i);
    if(echelle != echelles.end())
    {
      ossLadders << "lad" << (i + 1u) << " " << laddersHits[i] << " " << static_cast<double>(laddersHits[i]) / dRUNS << std::endl;
    }
    else if ( serpent != serpents.end() )
    {
      ossSnakes << "sna" << (i + 1u) << " " << snakesHits[i] << " " << static_cast<double>(snakesHits[i]) / dRUNS << std::endl;
    }
  }
  for ( unsigned int i = 0u; i < (DICE_SIZE + DICE_STREAK_MAX - 1); ++i )
  {
    if ( i < (DICE_SIZE - 1u) )
    { 
      ossDiceHits << "d" << DICE_SIZE << "|" << (i + 1u) << " " << diceHits[i] << " " << static_cast<double>(diceHits[i]) / dRUNS << std::endl;
    }
    else
    { 
      ossDiceHits << "d" << DICE_SIZE << "|" << DICE_SIZE << "x" << (i - DICE_SIZE + 2u) << " " << diceHits[i] << " " << static_cast<double>(diceHits[i]) / dRUNS << std::endl;
    }
  }
  
  for ( unsigned int i = 0u; i < END_MISS_STREAK_MAX; ++i )
  {
    ossEndMissStreaks << "endMissStreak" << ( i + 1u ) << " " << endMissStreaks[i] << " " << static_cast<double>(endMissStreaks[i]) / dRUNS << std::endl;
  }
  
  std::cout <<
    "runs " << RUNS << std::endl << 
    ossLadders.str() <<
    ossSnakes.str() <<
    ossDiceHits.str() <<
    ossEndMissStreaks.str() << 
    "turns " << turnCount << " " << static_cast<double>(turnCount) / dRUNS << std::endl << 
    "diceRolls " << diceRollCount << " " << static_cast<double>(diceRollCount) / dRUNS << 
    std::endl;
#endif

    return 0;
}