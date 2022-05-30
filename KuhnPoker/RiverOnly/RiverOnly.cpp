#include <vector>
#include <string>
#include <random>
#include <iostream>

typedef unsigned int uint;
typedef unsigned short ushort;

enum {
	SPADES = 0,
	CLUBS,
	HEARTS,
	DIAMONDS,
	N_SUIT,
	RANK_BLANK = -1,
	RANK_2 = 0,
	RANK_3,
	RANK_4,
	RANK_5,
	RANK_6,
	RANK_7,
	RANK_8,
	RANK_9,
	RANK_10,
	RANK_J,
	RANK_Q,
	RANK_K,
	RANK_A,
	N_RANK,
	N_CARD = N_SUIT * N_RANK,			//	全カード枚数
};
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
//	１枚のカードを表す構造体（ジョーカーは除く）
struct Card
{
public:
	Card(char suit = 0, char rank = -1) : m_suit(suit), m_rank(rank) {}

public:
	void	print() const { std::cout << toString(); }
	void	printW() const;
	std::string	toString() const;
	std::wstring	toStringW() const;
	int		numRank() const { return m_rank + 2; }		//	A は 14 を返す
	bool	operator==(const Card &x) const { return m_suit == x.m_suit && m_rank == x.m_rank; }

public:
	char	m_suit;
	char	m_rank;		//	0: 2,… 7: 9, 8:10, 9:J, 10:Q, 11:K, 12:A
};
//----------------------------------------------------------------------
//	一組（５２枚）のカードを表すクラス
class Deck
{
public:
	Deck(bool shfl = true);
	~Deck() {}

public:
	void	print() const;
	int	nDealt() const { return m_nDealt; }
	int	nRest() const { return N_CARD - m_nDealt; }
	Card	operator[](int ix) const { return m_card[ix]; }

public:
	void	init(bool shfl = true);
	void	setNDealt(int n) { m_nDealt = n; };
	void	shuffle();
	Card	deal();			//	カードを１枚配る（配ったカードを返す）
	bool take(Card);		//	指定カードをデックから取り出す
	//void	setMtSeed(int s);

private:
	int	m_nDealt;				//	既に配られた枚数（m_card の最初の m_nDealt 枚が配られている）
	Card	m_card[N_CARD];		//	カード配列
	std::mt19937	m_mt;
};
//----------------------------------------------------------------------
const char *handName[] = {
	"highCard",
	"onePair",
	"twoPair",
	"threeOfAKind",
	"straight",
	"flush",
	"fullHouse",
	"fourOfAKind",
	"straightFlush",
	"RoyalFlush",
};
int checkHand(const std::vector<Card> &v)
{
	std::vector<int> rcnt(13, 0);
	std::vector<int> scnt(4, 0);
	//int rcnt[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	//int scnt[4] = {0, 0, 0, 0};
	for (uint i = 0; i < v.size(); ++i) {
		const Card c = v[i];
		rcnt[c.m_rank] += 1;
		scnt[c.m_suit] += 1;
	}
	int s = -1;
	if( scnt[s = 0] >= 5 || scnt[s = 1] >= 5 || scnt[s = 2] >= 5 || scnt[s = 3] >= 5 ) {
		//	フラッシュ確定
		uint bitmap = 0;
		for (uint i = 0; i < v.size(); ++i) {
			const Card c = v[i];
			if( c.m_suit == s )
				bitmap |= 1 << (c.m_rank);
		}
		uint mask = 0x1f00;		//	AKQJT
		for (int i = 0; i < 9; ++i, mask>>=1) {
			if( (bitmap & mask) == mask ) {
				return STRAIGHT_FLUSH;
			}
		}
		if( bitmap == 0x100f )	//	1 0000 00000 1111 = A5432
			return STRAIGHT_FLUSH;
	} else
		s = -1;
	int threeOfAKindIX = -1;
	int threeOfAKindIX2 = -1;
	int pairIX1 = -1;
	int pairIX2 = -1;
	for (int i = 0; i < 13; ++i) {
		switch( rcnt[i] ) {
			case 4:
				return FOUR_OF_A_KIND;
			case 3:
				if( threeOfAKindIX < 0 )
					threeOfAKindIX = i;
				else
					threeOfAKindIX2 = i;
				break;
			case 2:
				pairIX2 = pairIX1;
				pairIX1 = i;
				break;
		}
	}
	//	3カード*2 もフルハウス
	if( threeOfAKindIX >= 0 && (pairIX1 >= 0 || threeOfAKindIX2 >= 0) )
		return FULL_HOUSE;
	if( s >= 0 )
		return FLUSH;
	uint bitmap = 0;
	uint mask = 1;
	for (int i = 0; i < 13; ++i, mask<<=1) {
		if( rcnt[i] != 0 )
			bitmap |= mask;
	}
	mask = 0x1f00;		//	AKQJT
	for (int i = 0; i < 9; ++i, mask>>=1) {
		if( (bitmap & mask) == mask )
			return STRAIGHT;
	}
	if( (bitmap & 0x100f) == 0x100f )		//	5432A
		return STRAIGHT;
	//if( bitmap == 0x100f )	//	1 0000 00000 1111 = A5432
	//	return STRAIGHT;
	if( threeOfAKindIX >= 0 )
		return THREE_OF_A_KIND;
	if( pairIX2 >= 0 )
		return TWO_PAIR;
	if( pairIX1 >= 0 )
		return ONE_PAIR;
	return HIGH_CARD;
}

int main()
{
    std::cout << "\nOK.\n";
}
