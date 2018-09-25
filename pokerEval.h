#ifndef POKEREVAL
#define POKEREVAL

#include <stdlib.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>
#include <time.h>
#include <bitset>
#include <iostream>
#include <tgmath.h>
#include <vector>

enum Hand {
    None = -1,
    High_Card,
	Pair,
	Two_Pair,
	Three_of_a_kind,
	Straight,
	Flush,
	Full_house,
	Four_of_a_kind,
	Straight_Flush,
	Royal_Flush
};

enum Suit {
    Heart,
    Spade,
    Club,
    Diamond
};
    


class PokerHand 
{
public:
    PokerHand();
    ~PokerHand();
    void reset();
    // Deal
    void dealHand(int nCards);
    void dealHand(std::string* dealtCards,int nCards);
    void dealCard(std::string card,__m128i hand); 
    //Evaluate
    Hand checkHand(__m128i hand);
    __m128i m_hand;
    //tests
    void HighTest();
    void PairTest();
    void TwoPairTest();
    void ThreeOfAKindTest();
    void StraightTest();
    void FlushTest();
    void FullHouseTest();    
    void FourOfAKindTest();
    void StraightFlushTest();
    void RoyalFlushTest();
    void statCheck(int handSize,float rounds);
    // Helper
    void printResult();
private:
    //deal helper
    uint translate(std::string card);
    //functions
    void checkRSCD(uint8_t nyble, int8_t index, int8_t& streak);
    void checkFlush(int8_t& suit,int8_t index);
    void checkStraightFlush(__m128i hnd, int high);
    //lookups
    void initLookups();
    std::vector<const char*> cardFace;
    std::vector<const char*> winHand;
    // Hand value holders. Init to -1, if found replace with 0-12 card index of high card.
    int8_t m_bHighCard; 
    std::pair<int8_t,int8_t> m_bTwoPair;
    int8_t m_bThreeOfAKind; 
    int8_t m_bFlush; //if flush hold High
    int8_t m_bStraight; 
    int8_t m_bFourOfAKind;
    int8_t m_bStraightFlush;
    //Flush's
    int8_t heart;
    int8_t spade;
    int8_t club;
    int8_t diamond;
    //
    Hand m_winner;
    //
    void initRand();
};

class Utils
{
public:
    Utils();
    ~Utils();
    static void printbits128(std::bitset<128> arr);
    static void printbits128(unsigned long long arr, int h);
    static void printbits128(__m128i a); 
};

#endif