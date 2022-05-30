#pragma once

#include <iostream>
#include <string>
#include <random>

#define		RAND_SEED		1

//----------------------------------------------------------------------
//	１枚のカードを表す構造体（ジョーカーは除く）
struct Card
{
public:
	enum Suit {
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
	int	nRest() const { return Card::N_CARD - m_nDealt; }
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
	Card	m_card[Card::N_CARD];		//	カード配列
	std::mt19937	m_mt;
};
