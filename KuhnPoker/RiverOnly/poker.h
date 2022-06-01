#include <vector>

struct Card;

enum {
	HIGH_CARD = 0,
	ONE_PAIR,
	TWO_PAIR,
	THREE_OF_A_KIND,
	STRAIGHT,
	FLUSH,
	FULL_HOUSE,
	FOUR_OF_A_KIND,
	STRAIGHT_FLUSH,
	ROYAL_FLUSH,
	N_KIND_HAND,
};
//const char *handName[];
typedef unsigned int uint;
typedef std::pair<Card, Card> HoleCards;
int checkHand(const std::vector<Card> &v);
int checkHand(const std::vector<Card> v, uint &odr);
int checkHandBM(const std::vector<Card> &v);		//	ビットマップを用いた役判定関数
int checkHandBM(const std::vector<Card> &v, uint &odr);		//	ビットマップを用いた役判定関数
void getHandBits(const std::vector<Card> &v, std::vector<bool> &o);		//	役を構成するカードを取得
double calcWinSplitProb(Card c1, Card c2, const std::vector<Card> &comu);
double calcWinSplitProb(Card c1, Card c2, const std::vector<Card> &comu, int np);
double calcWinSplitProbRO(Card c1, Card c2, const std::vector<Card> &comu, int np);
double calcHandStrength(Card c1, Card c2, const std::vector<Card> &comu, int np);
void calcHandStrength(const std::vector<HoleCards> &hc, const std::vector<Card> &comu, std::vector<double> &p);
double calcThreshold(int pot, int call);
void print(const std::vector<Card> v);
void print(const std::vector<Card> v, uint);
uint mlbnM(uint x);
