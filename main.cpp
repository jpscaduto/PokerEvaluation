#include "pokerEval.h"

int main(){
    PokerHand jp = PokerHand();
	std::cout << "Start Tests::\n";
	jp.HighTest();
	jp.PairTest();
	jp.TwoPairTest();
	jp.ThreeOfAKindTest();
	jp.StraightTest();
	jp.FlushTest();
	jp.FullHouseTest();
	jp.FourOfAKindTest();
	jp.StraightFlushTest();
	jp.RoyalFlushTest();
	// End Tests
	std::cout << "End Test, Start Stats::\n\n";
	jp.statCheck(7,1000000);
}	