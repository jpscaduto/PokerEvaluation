#include "pokerEval.h"
/*
The Ranking of hands is as follows:
	- Royal Flush
	- Straight Flush
	- Four of a kind
	- Full house
	- Flush
	- Straight
	- 3 of a kind
	- Two Pair
	- Pair
	- High Card

Bit Organization is as follows:
    |       H        |       S        |       C        |       D        |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 |  J |  Q |  K |  A |
	|234567890JQKA   |234567890JQKA   |234567890JQKA   |234567890JQKA   |HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|
*/


PokerHand::PokerHand()
{
    initLookups();
	initRand();
	//Hands
	m_hand = _mm_setzero_si128();
	//
	m_bHighCard = -1;
	m_bThreeOfAKind = -1;
	m_bFourOfAKind = -1;
	m_bStraight = -1;
	m_bFlush = -1;
	m_bStraightFlush = -1;//
    m_bTwoPair = std::pair<int8_t,int8_t>(-1,-1);
	m_bStraightFlush = false;
	//
	heart = 0;
	spade = 0;
	club = 0;
	diamond = 0;
	//
	m_winner = None;
}
PokerHand::~PokerHand()
{
}
void PokerHand::reset()
{
	m_hand = _mm_setzero_si128();
	//
	m_bHighCard = -1;
	m_bThreeOfAKind = -1;
	m_bFourOfAKind = -1;
	m_bStraight = -1;
	m_bFlush = -1;
	m_bStraightFlush = -1;//
    m_bTwoPair = std::pair<int8_t,int8_t>(-1,-1);
	m_bStraightFlush = false;
	//
	heart = 0;
	spade = 0;
	club = 0;
	diamond = 0;
	//
	m_winner = None;
}
void PokerHand::initLookups()
{
	cardFace.push_back("2");
	cardFace.push_back("3");
	cardFace.push_back("4");
	cardFace.push_back("5");
	cardFace.push_back("6");
	cardFace.push_back("7");
	cardFace.push_back("8");
	cardFace.push_back("9");
	cardFace.push_back("10");
	cardFace.push_back("J");
	cardFace.push_back("Q");
	cardFace.push_back("K");
	cardFace.push_back("A");
	//
	winHand.push_back("High Card");
	winHand.push_back("Pair");
	winHand.push_back("Two Pair");
	winHand.push_back("Three of a kind");
	winHand.push_back("Straight");
	winHand.push_back("Flush");
	winHand.push_back("Full house");
	winHand.push_back("Four of a kind");
	winHand.push_back("Straight Flush");
	winHand.push_back("Royal Flush");
}
void PokerHand::initRand()
{
	std::srand(time(NULL)); 		//ever-changing seed
}
//Translate card format '4C' into number
uint PokerHand::translate(std::string card)
{
	char st = card.back();
	card.pop_back();
	int number;
	switch(card.at(0)){
		case 65:
			number = 14;
			break;
		case 75:
			number = 13;
			break;
		case 81:
			number = 12;
			break;
		case 74:
			number = 11;
			break;
		default:
			number = std::stoi(card);
	}

	switch(st)
	{
		case 72:
			number = ((number-2)*4)+0;
			break;
		case 83:
			number = ((number-2)*4)+1;
			break;
		case 67:
			number = ((number-2)*4)+2;
			break;
		case 68:
			number = ((number-2)*4)+3;
			break;
		default:
			std::cout << "Error\n";
	}
	return number;
}
/*
Deal Hand: 
	Take n cards and create
	the hand with no duplicates. Seed
	of random generator based off 
	time

	Time Complexity: O(nCards) 
*/
void PokerHand::dealHand(int nCards)
{
	reset();
	std::bitset<128> hand_1 = 0;
	int cards[52] = {0}; 			//Deck of cards
    for(int i=0; i<nCards; i++)
	{
		uint l = std::rand() % 52; 	//Generate random numbers between 1 and 52 (Cards in Deck)
		if(cards[l]==0) 			//No duplicates
		{
			cards[l]=1; 			//card taken from deck
			hand_1.set(64+l,true);		//set the bit of "hand"
			uint h2 = (16*(l%4))+(l/4);//not 52 /4
			hand_1.set(h2,true);
		}
		else
		{
			i--; 					//repeat draw if duplicate
		}
	}
    m_hand = _mm_loadu_si128((__m128i*)&hand_1); //load into SSE buffer
}
void PokerHand::dealHand(std::string* dealtCards,int nCards)
{
	reset();
	for(int i=0; i<nCards; i++)
	{
		dealCard(dealtCards[i],m_hand);
	}
}
void PokerHand::dealCard(std::string card,__m128i hand)
{
	std::bitset<128> hand_1 = 0;
	__m128i cardFld = _mm_setzero_si128();
	uint l = translate(card); 	//Generate random numbers between 1 and 52 (Cards in Deck)
	hand_1.set(64+l,true);	
	uint h2 = (16*(l%4))+floor(l/4);
	hand_1.set(h2,true);
	cardFld = _mm_loadu_si128((__m128i*)&hand_1);
	hand = _mm_or_si128(cardFld,hand);
	//Utils::printbits128(hand);
	reset();
	m_hand=hand;
}
/*
Check Hand: 
	Master function to check   
	the best 5-card play based off the
	n-cards drawn.

	- Time Complexity: O(13) ~ O(1)
	* if flush and straight then add O(max card in flush/straight)
*/
Hand PokerHand::checkHand(__m128i hand) 
{
	m_hand = hand;
	__m128i mask = _mm_set_epi8(0,0,0,0,0,0,0,15,0,0,0,0,0,0,0,0);
	__m128i result = _mm_setzero_si128();
	char* mark = (char*)&result+8; // move to second half of result arr for hscd
	int8_t streak = 0;
	for(int i=0;i<13;i++)
	{
		result = _mm_and_si128(hand,mask);
		checkRSCD(*mark, i, streak); //always updates in increasing value order
		hand = _mm_srli_epi64(hand,4);
	}	
	switch(m_winner){ //Special cases that can be weeded out by previous logic
		case Flush:
			if(m_bStraight != -1)
			{
				int range = (m_bStraight<m_bFlush)?m_bStraight:m_bFlush;
				checkStraightFlush(m_hand,range);
			}
		case Straight:
		case Three_of_a_kind:
			if(m_bTwoPair.first != -1 && m_bThreeOfAKind != -1)
			{
				m_winner = (Full_house>m_winner)?Full_house:m_winner;
			}
			break;
		case Pair:
			if(m_bTwoPair.second != -1)
			{
				m_winner = Two_Pair;
			}
			break;
		default:
			break;
	}
	return m_winner;
}
/*
Has Flush: 
	Check and accumulate state on if there 
	is a flush by accumulating suit count.

	-Time Complexity: O(1) 
*/
void PokerHand::checkFlush(int8_t& suit, int8_t index)
{
	suit++;
	if(suit > 4)
	{
		m_bFlush = index;
		m_winner = (Flush > m_winner)?Flush:m_winner;
	}
}
/*
RSCD: 
	Check the value of a nyble
	to see what is the best hand for
	a given card type.

	**Considered using hash table for this 
	**but the it wouldnt be worth adding 
	**the conditionals With small items, switch 
	**is O(1) as long as compiler supports making
	**it a jump table.

	- Time Complexity: ~O(1)
*/
void PokerHand::checkRSCD(uint8_t nyble, int8_t index,int8_t& streak)
{
	Hand thisC = None;
	switch(nyble)
	{
		case 1:
			checkFlush(heart,index);
			m_bHighCard = index;
			thisC = High_Card;
			break;
		case 2:
			checkFlush(spade,index);
			m_bHighCard = index;
			thisC = High_Card;
			break;
		case 4:
			checkFlush(club,index);
			m_bHighCard = index;
			thisC = High_Card;
			break; 
		case 8:
			checkFlush(diamond,index);
			m_bHighCard = index;
			thisC = High_Card;
			break;
		//============== TWO ====================
		case 3:
			checkFlush(heart,index);
			checkFlush(spade,index);
			m_bTwoPair.second = m_bTwoPair.first;
			m_bTwoPair.first = index;
			thisC = Pair;
			break;
		case 5:
			checkFlush(heart,index);
			checkFlush(club,index);
			m_bTwoPair.second = m_bTwoPair.first;
			m_bTwoPair.first = index;
			thisC = Pair;
			break;
		case 6:
			checkFlush(spade,index);
			checkFlush(club,index);
			m_bTwoPair.second = m_bTwoPair.first;
			m_bTwoPair.first = index;
			thisC = Pair;
			break;
		case 9:
			checkFlush(heart,index);
			checkFlush(diamond,index);
			m_bTwoPair.second = m_bTwoPair.first;
			m_bTwoPair.first = index;
			thisC = Pair;
			break;
		case 10:
			checkFlush(diamond,index);
			checkFlush(spade,index);
			m_bTwoPair.second = m_bTwoPair.first;
			m_bTwoPair.first = index;
			thisC = Pair;
			break;
		case 12:
			checkFlush(diamond,index);
			checkFlush(club,index);
			m_bTwoPair.second = m_bTwoPair.first;
			m_bTwoPair.first = index;
			thisC = Pair;
			break;
		//============== THREE ====================
		case 7:
			checkFlush(heart,index);
			checkFlush(spade,index);
			checkFlush(club,index);
			m_bThreeOfAKind = index;
			thisC = Three_of_a_kind;
			break;
		case 11:
			checkFlush(heart,index);
			checkFlush(spade,index);
			checkFlush(diamond,index);
			m_bThreeOfAKind = index;
			thisC = Three_of_a_kind;
			break;
		case 13:
			checkFlush(heart,index);
			checkFlush(club,index);
			checkFlush(diamond,index);
			m_bThreeOfAKind = index;
			thisC = Three_of_a_kind;			
			break;
		case 14:
			checkFlush(spade,index);
			checkFlush(club,index);
			checkFlush(diamond,index);
			m_bThreeOfAKind = index;
			thisC = Three_of_a_kind;
			break;
		//=============== FOUR =====================
		case 15:
			checkFlush(heart,index);
			checkFlush(spade,index);
			checkFlush(club,index);
			checkFlush(diamond,index);
			m_bFourOfAKind = index;
			thisC = Four_of_a_kind;
			break;
		default: 
			thisC = None;
	}
	//check straight
	streak = (thisC!=None)?streak+1:0;
	if(streak >= 5){
		m_bStraight = index;
		thisC=Straight;
	}
	m_winner = (thisC > m_winner)?thisC:m_winner;
}

/*
Staight Flush: 
	Shift and AND with bitmask.
	This will require n shifts with n being the 
	max card set by the lowest high card of 
	flush and straight. only invoked if both pre-
	conditions met.

	(isolate highest bit set)::EndCardIndex-4 == log2(flush / 31)

	ITERATE ACROSS 8 HIGHCARD STRAIGHTS worst case: O(8)
*/
void PokerHand::checkStraightFlush(__m128i hnd, int high)
{
	unsigned long long straightValue = 0x001F001F001F001F;// 
	__m128i mask = _mm_set_epi64x(0,straightValue);
	__m128i checkS = _mm_setzero_si128();
	//Each suit stack
	uint16_t *ptrDiamond = (uint16_t*)&checkS; //Assign to diamond suit
	uint16_t *ptrClub = (uint16_t*)(ptrDiamond+1); //Assign to Club suit
	uint16_t *ptrSpade = (uint16_t*)(ptrDiamond+2); //Assign to Spade suit
	uint16_t *ptrHeart = (uint16_t*)(ptrDiamond+3); //Assign to heart suit
	//shift mask to check for 5 consec bits
	for(int i=0;i<high-3;i++) //only shift as many as high card
	{
		checkS = _mm_and_si128(mask,hnd);
		//used to calc if 5 bits in a row set during masking
		float endHeart = (log2(*ptrHeart/31.0)+4); 
		float endSpade = (log2(*ptrSpade/31.0)+4);
		float endClub = (log2(*ptrClub/31.0)+4);
		float endDiamond = (log2(*ptrDiamond/31.0)+4);
		// Check if value matches straight Flush
		if(endHeart == (i+4) || endSpade == (i+4) || 
			endClub == (i+4) || endDiamond == (i+4))
		{
			m_bStraightFlush = (i+4);
			Hand type = ((i+4)==12)?Royal_Flush:Straight_Flush;
			m_winner = (type > m_winner)?type:m_winner;
		}
		//shift mask down one bit
		mask = _mm_slli_epi16(mask,1);				
	}
}

// ==================== Tests =======================

void PokerHand::HighTest()
{
	__m128i hand;
	char* ptr = (char*)&hand;
	ptr = ptr+8;
	int cardInd = 0;
	for (int i=0;i<7;i++)
	{
		for(int j=0; j<8;j++)
		{
			hand = _mm_setzero_si128();
			reset();
			*ptr = 1<<j;
			Hand res = checkHand(hand);
			if(j==3){cardInd++;}
			if(cardInd == 13){
				printf("High Card Test Success\n");
				return;
			}
			if(res != High_Card){
				printf("FAILED:: For card %s, value %d, the had was %s\n",cardFace[cardInd],j,winHand[res]);
				Utils::printbits128(hand);
				return;
			}
		}
		cardInd++;
		ptr++;
	}
	reset();
}

void PokerHand::PairTest()
{
	int prs[12] = {3,5,9,6,10,12,48,80,144,160,96,192};
	__m128i hand;
	char* ptr = (char*)&hand;
	ptr = ptr+8;
	int cardInd = 0;
	for (int i=0;i<7;i++)
	{
		for(int j=0; j<12;j++)
		{
			hand = _mm_setzero_si128();
			reset();	
			*ptr = prs[j];
			Hand res = checkHand(hand);
			//printbits128(hand);
			if(j==5){cardInd++;}
			if(cardInd == 13){
				printf("Pair Test Success\n");
				return;
			}
			if(res != Pair){
				printf("FAILED:: For card %s, value %d, the had was %s\n",cardFace[cardInd],j,winHand[res]);
				Utils::printbits128(hand);
				return;
			}
		}
		cardInd++;
		ptr++;
	}
	reset();
}

void PokerHand::TwoPairTest()
{
	int prs[12] = {3,5,9,6,10,12,48,80,144,160,96,192};
	__m128i hand;
	char* ptr = (char*)&hand;
	ptr = ptr+8;
	unsigned long long* hndLLU = (unsigned long long*)&hand;
	hndLLU++;
	unsigned long long mask = 0xFFFFFFFFFFFFF;
	int cardInd = 0;
	for (int i=0;i<7;i++)
	{
		for(int j=0; j<12;j++)
		{
			char* ptr2 = (char*)&hand;
			ptr2=ptr2+8;
			int cardInd2 = 0;
			for (int k=0;k<7;k++)
			{

				for(int l=0; l<12;l++)
				{
					hand = _mm_setzero_si128();	
					reset();
					//*ptr = prs[j];
					if(ptr==ptr2){//same place
						if((j<6 && l>=6) || (j>=6 && l<6)){//first num
							*ptr = prs[j] | prs[l];
						}
						else {//second num
							if(l==6){cardInd2++;}
							continue;
						}
					}
					else{ //different bytes
							*ptr = prs[j];
							*ptr2 = prs[l];
					}
					*hndLLU = *hndLLU & mask;
					if(l==6){
						cardInd2++;
					}
					if(cardInd2 >= 13){break;}
					Hand wn = checkHand(hand);
					if(wn != Two_Pair){
						printf("FAILED:: the test failed for the following two pairs: %d, %d with hand of %s\n",cardInd,cardInd2,winHand[m_winner]);
						Utils::printbits128(hand);
						return;
					}

				}
				cardInd2++;
				ptr2++;
			}
			//printbits128(hand);
			if(j==6){cardInd++;}
			if(cardInd >= 12){//12 because we exclude the duplicate
				printf("Two Pair Test Success!\n");
				reset();
				return;
			}
		}
		cardInd++;
		ptr++;
	}
}

void PokerHand::ThreeOfAKindTest()
{
	int prs[8] = {7,11,13,14,112,176,208,224};
	__m128i hand;	
	char* ptr = (char*)&hand;
	ptr = ptr+8;
	int cardInd = 0;
	for (int i=0;i<7;i++)
	{
		for(int j=0; j<8;j++)
		{
			hand = _mm_setzero_si128();
			reset();		
			*ptr = prs[j];
			Hand res = checkHand(hand);
			//printbits128(hand);
			if(j==3){cardInd++;}
			if(cardInd == 13){
				printf("Three of a kind test Success\n");
				return;
			}
			if(res != Three_of_a_kind){
				printf("FAILED:: For card %s, value %d, the had was %s\n",cardFace[cardInd],j,winHand[res]);
				Utils::printbits128(hand);
				return;
			}
		}
		cardInd++;
		ptr++;
	}
	reset();
}

void PokerHand::StraightTest()
{
	std::string crd;
	std::string suits[4] = {"H","S","C","D"};
	for(int b=0;b<9;b++)
	{
		for(int q=0;q<4;q++)
		{
			int p=q;
			reset();
			std::string cards[5];
			for (int i=0;i<5;i++)
			{
				int j = (b+i+2);
				crd = std::to_string(j) + suits[p];
				p++;
				p = (p<4)?p:0;
				cards[i] = crd;
			}
			dealHand(cards,5);
			Hand wn = checkHand(m_hand);
			if(wn!=Straight){
				printf("FAILED:: For card %s, the had was %s\n",cardFace[m_bStraight],winHand[m_winner]);
				Utils::printbits128(m_hand);
				return;
			}
		}
	}
	reset();
	printf("Straight test success!\n");
}

void PokerHand::FlushTest()
{
	std::string crd;
	std::string suits[4] = {"H","S","C","D"};
	for(int i=0;i<8;i++){
		for(int s=0;s<4;s++)
		{
			int inddd = 0;
			std::string cards[5];
			for(int b=0;b<6;b++)
			{
				int p = (b+i+2);
				if(b==2){b++;}
				crd = std::to_string(p)+suits[s];
				cards[inddd] = crd;
				inddd++;
				
			}
			dealHand(cards,5);
			Hand wn = checkHand(m_hand);
			if(wn!=Flush){
				printf("FAILED:: For card %s, the had was %s\n",cardFace[m_bStraight],winHand[m_winner]);
				Utils::printbits128(m_hand);
				return;
			}
		}
	}
	reset();
	printf("Flush test success!\n");
}

void PokerHand::FullHouseTest()
{
	int prs[12] = {3,5,9,6,10,12,48,80,144,160,96,192};
	int toak[8] = {7,11,13,14,112,176,208,224};
	__m128i hand;
	char* ptr = (char*)&hand;
	ptr = ptr+8;
	unsigned long long* hndLLU = (unsigned long long*)&hand;
	hndLLU++;
	unsigned long long mask = 0xFFFFFFFFFFFFF;
	int cardInd = 0;
	for (int i=0;i<7;i++)
	{
		for(int j=0; j<8;j++)
		{
			char* ptr2 = (char*)&hand;
			ptr2=ptr2+8;
			int cardInd2 = 0;
			for (int k=0;k<7;k++)
			{

				for(int l=0; l<12;l++)
				{
					hand = _mm_setzero_si128();	
					reset();
					//*ptr = prs[j];
					if(ptr==ptr2){//same place
						if((j<4 && l>=6) || (j>=6 && l<6)){//first num
							*ptr = toak[j] | prs[l];
						}
						else {//second num
							if(l==6){cardInd2++;}
							continue;
						}
					}
					else{ //different bytes
							*ptr = toak[j];
							*ptr2 = prs[l];
					}
					*hndLLU = *hndLLU & mask;
					if(l==6){
						cardInd2++;
					}
					if(cardInd2 >= 13){break;}
					Hand wn = checkHand(hand);
					if(wn != Full_house){
						printf("FAILED:: the test failed for the following two pairs: %d, %d with hand of %s\n",cardInd,cardInd2,winHand[m_winner]);
						Utils::printbits128(hand);
						return;
					}

				}
				cardInd2++;
				ptr2++;
			}
			//printbits128(hand);
			if(j==3){cardInd++;}
			if(cardInd >= 12){//12 because we exclude the duplicate
				printf("Full House Test Success!\n");
				reset();
				return;
			}
		}
		cardInd++;
		ptr++;
	}
}

void PokerHand::FourOfAKindTest()
{
	int prs[2] = {15,240};
	__m128i hand;
	char* ptr = (char*)&hand;
	ptr = ptr+8;
	int cardInd = 0;
	for (int i=0;i<7;i++)
	{
		for(int j=0; j<2;j++)
		{
			hand = _mm_setzero_si128();
			reset();			
			*ptr = prs[j];
			Hand res = checkHand(hand);
			//printbits128(hand);
			if(j==0){cardInd++;}
			if(cardInd == 13){
				printf("Four of a kind test Success\n");
				return;
			}
			if(res != Four_of_a_kind){
				printf("FAILED:: For card %s, value %d, the hand was %s\n",cardFace[cardInd],j,winHand[res]);
				Utils::printbits128(hand);
				return;
			}
		}
		cardInd++;
		ptr++;
	}
	reset();
}

void PokerHand::StraightFlushTest()
{
	unsigned long long a = 31;
	unsigned long long b = 69905;
	__m128i hand;
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<8;j++)
		{
			reset();
			hand = _mm_set_epi64x(b,a);
			Hand wn = checkHand(hand);
			if(wn != Straight_Flush){
				printf("FAILED:: Result was %s\n",winHand[wn]);
				Utils::printbits128(hand);
				return;
			}
			a = a<<1;//suit
			b = b<<4;//hscd
		}
		a = a<<8;
		b = 69905<<1;
	}
	reset();
	printf("Straight Flush Test Success!\n");
	return;

}

void PokerHand::RoyalFlushTest()
{
	unsigned long long a = 31<<8;
	unsigned long long b = 0x1111100000000;
	__m128i hand;
	for(int i=0;i<4;i++)
	{
		reset();
		hand = _mm_set_epi64x(b,a);
		Hand wn = checkHand(hand);
		if(wn != Royal_Flush){
			printf("FAILED:: Result was %s\n",winHand[wn]);
			Utils::printbits128(hand);
			return;
		}
		a = a<<16;
		b = b<<1;
	}
	reset();
	printf("Royal Flush Test Success!\n");
	return;

}
// =================== Helper =======================

void PokerHand::printResult()
{
	switch(m_winner)
	{
		case Royal_Flush:
			std::cout << winHand[m_winner] << " High Card of " << cardFace[12] << "\n";
			break;
		case Straight_Flush:
			std::cout << winHand[m_winner] << " High Card of " << cardFace[m_bStraightFlush] << "\n";
			break;
		case Four_of_a_kind:
			std::cout << winHand[m_winner] << " High Card of " << cardFace[m_bFourOfAKind] << "\n";
			break;
		case Full_house:
			std::cout << winHand[m_winner] << " of (3 of a kind, 2 of a kind) " << cardFace[m_bThreeOfAKind] << " & " << cardFace[m_bTwoPair.first] << "\n";
			break;
		case Flush:
			std::cout << winHand[m_winner] << " High Card of " << cardFace[m_bFlush] << "\n";
			break;
		case Straight:
			std::cout << winHand[m_winner] << " High Card of " << cardFace[m_bStraight] << "\n";
			break;
		case Three_of_a_kind:
			std::cout << winHand[m_winner] << " of " << cardFace[m_bThreeOfAKind] << "\n";
			break;
		case Two_Pair:
			std::cout << winHand[m_winner] << " of " << cardFace[m_bTwoPair.first] << " & " << cardFace[m_bTwoPair.second] << "\n";
			break;
		case Pair:
			std::cout << winHand[m_winner] << " of" << cardFace[m_bTwoPair.first] << "\n";
			break;
		case High_Card:
			std::cout << winHand[m_winner] << " of " << cardFace[m_bHighCard] << "\n";
			break;
		default:
			std::cout << "Error" << "\n";
			Utils::printbits128(m_hand);
	}
	Utils::printbits128(m_hand);
}

//==================== Utils ========================
/*
Utilities: 
	To debug and visualize the
	problem.
*/

void Utils::printbits128(std::bitset<128> arr)
{ 
	std::vector<std::string> cardFace;
	cardFace.push_back("2");
	cardFace.push_back("3");
	cardFace.push_back("4");
	cardFace.push_back("5");
	cardFace.push_back("6");
	cardFace.push_back("7");
	cardFace.push_back("8");
	cardFace.push_back("9");
	cardFace.push_back("10");
	cardFace.push_back("J");
	cardFace.push_back("Q");
	cardFace.push_back("K");
	cardFace.push_back("A");
	//=====================================================
	std::string s_arr = arr.to_string();	
	std::cout << "|       H        |       S        |       C        |       D        |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 |  J |  Q |  K |  A |    |    |    |\n" ;
	std::cout << "|234567890JQKA   |234567890JQKA   |234567890JQKA   |234567890JQKA   |HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|    |    |    |\n";

	printf("|");
	for(int i=127;i>64;i--)
	{
		std::cout << s_arr[i];
		if((i)%16==0){printf("|");}

	}
	for(int i=64;i>=0;i--)
	{
		std::cout << s_arr[i];
		if((i)%4==0){printf("|");}

	}
    std::cout << "\n";
}
void Utils::printbits128(unsigned long long arr, int h)
{ 
	if(h ==1){
		std::vector<std::string> cardFace;
		cardFace.push_back("2");
		cardFace.push_back("3");
		cardFace.push_back("4");
		cardFace.push_back("5");
		cardFace.push_back("6");
		cardFace.push_back("7");
		cardFace.push_back("8");
		cardFace.push_back("9");
		cardFace.push_back("10");
		cardFace.push_back("J");
		cardFace.push_back("Q");
		cardFace.push_back("K");
		cardFace.push_back("A");
		//=====================================================
		//for(int i=12;i>=0;i--)
		for(int i = 0;i<13;i++)
		{
			if(i==8)
			{
				std::cout << "| " << cardFace[i%13]<<" ";

			}
			else{
				std::cout << "|  " << cardFace[i%13]<<" ";
			}
		}
		std::cout << "\n|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD";
		std::cout << "\n";
		unsigned long long t = 1;//pow(2,63);
		for(int i=0;i<52;i++)
		{
			if((i-12)%4==0){printf("|");}
			char c = (arr & (t<<i))?'1':'0';
			printf("%c",c);
		}

		printf("\n%llu\n",arr);
	}
	else if(h == 2){
		std::cout << "|        H       |        S       |        C       |        D       " ;
		std::cout << "\n";
		std::cout << "|000AKQJ098765432|000AKQJ098765432|000AKQJ098765432|000AKQJ098765432";
		std::cout << "\n";
	unsigned long long t = pow(2,63);
	for(int i=0;i<64;i++)
	{
		if((i)%16==0){printf("|");}
		char c = (arr & (t>>i))?'1':'0';
		printf("%c",c);
	}
	}
    std::cout << "\n";
}
void Utils::printbits128(__m128i a)
{
	__m128i *arr = &a;
	char* number = ((char*)arr);
	std::cout << "|       H        |       S        |       C        |       D        |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 |  J |  Q |  K |  A |    |    |    |\n" ;
	std::cout << "|234567890JQKA   |234567890JQKA   |234567890JQKA   |234567890JQKA   |HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|HSCD|    |    |    |\n";
	printf("|");
	for(int i=0; i<16; i++){
		for(int j=0;j<8;++j){
			printf("%d", number[i]&1);
			number[i]>>=1;
			if(i>=8 && j==3){printf("|");}
		}
		if(i%2==1 || i>7){printf("|");}
		
	}
	printf("\n");
}
// ================== STATS =========================
void PokerHand::statCheck(int handSize, float rounds)
{
	int bHighCard = 0; 
	int bPair = 0;
    int bTwoPair = 0;
    int bThreeOfAKind = 0; 
    int bFlush = 0; //if flush hold High
    int bStraight = 0; 
	int bFullHouse = 0;
    int bFourOfAKind = 0;
    int bStraightFlush = 0;
	int bRoyalFlush = 0;
	for(int i = 0;i<rounds;i++)
	{
		dealHand(handSize);
		Hand wn = checkHand(m_hand);
		switch(m_winner)
		{
			case Royal_Flush:
				bRoyalFlush++;
				break;
			case Straight_Flush:
				bStraightFlush++;
				break;
			case Four_of_a_kind:
				bFourOfAKind++;
				break;
			case Full_house:
				bFullHouse++;
				break;
			case Flush:
				bFlush++;
				break;
			case Straight:
				bStraight++;
				break;
			case Three_of_a_kind:
				bThreeOfAKind++;
				break;
			case Two_Pair:
				bTwoPair++;
				break;
			case Pair:
				bPair++;
				break;
			case High_Card:
				bHighCard++;
				break;
			default:
				std::cout << "Error" << "\n";
				Utils::printbits128(m_hand);
		}
	}
	printf("High Card: %f\n",(bHighCard/rounds)*100.0);
	printf("Pair: %f\n",(bPair/rounds)*100.0);
	printf("Two Pair: %f\n",(bTwoPair/rounds)*100.0);
	printf("Three of a kind: %f\n",(bThreeOfAKind/rounds)*100.0);
	printf("Straight: %f\n",(bStraight/rounds)*100.0);
	printf("Flush: %f\n",(bFlush/rounds)*100.0);
	printf("Full House: %f\n",(bFullHouse/rounds)*100.0);
	printf("Four of a Kind: %f\n",(bFourOfAKind/rounds)*100.0);
	printf("Straight Flush: %f\n",(bStraightFlush/rounds)*100.0);
	printf("Royal Flush: %f\n",(bRoyalFlush/rounds)*100.0);
}


