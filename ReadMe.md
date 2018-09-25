# Poker Hand Evaluation

## Intro

### The Ranking of hands is as follows:
* Royal Flush
* Straight Flush
* Four of a kind
* Full house
* Flush
* Straight
* 3 of a kind
* Two Pair
* Pair
* High Card

### Bits Orgainized as Follows
* |       H        |       S        |       C        |       D        |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 |  J |  Q |  K |  A |
* |234567890JQKA   |234567890JQKA   |234567890JQKA   |234567890JQKA   |HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|

## Public Methods

Constructor: Initialize Lookup Tables and member vars
* PokerHand()

void reset()
* reset all member vars. 
* can be of use between games 

void dealHand: 
* dealHand(int nCards): deal hand of size n number of cards
* dealHand(std::string* dealtCards, int nCards): Deal hand of n cards specifying the cards using the convention, numberSuit (i.e. 4S, 10H, JD)
* dealCard(std::string card,__m128i hand): deal a single card to an already existing hand

Hand checkHand(__m128i hand):
* check the result of the hand at present time

## Tests

void HighTest()
* check high card winning hands
void PairTest()
* check pair winning hands
void TwoPairTest()
* check two pair winning hands
void ThreeOfAKindTest()
* check three of a kind winning hands
void StraightTest()
* check straight winning hands
void FlushTest()
* check flush winning hands
void FullHouseTest()   
* check full house winning hands
void FourOfAKindTest()
* check four of a kind winning hands
void StraightFlushTest()
* check straight flush winning hands
void RoyalFlushTest()
* check Royal Flush winning hands
void statCheck(int handSize,float rounds)
* check statistical outcome of a handsize of n over i number of rounds

## Helper
void printResult()
* print a hand
