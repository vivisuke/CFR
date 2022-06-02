#include <iostream>
#include <string>
//#include "consoleUtil.h"
#include "Deck.h"

std::string Card::toString() const
{
	if( m_suit < 0 || m_suit >= N_SUIT || m_rank < 0 || m_rank >= N_RANK )
		return std::string("??");
	std::string str(1, "SCHD"[m_suit]);
	str += "23456789TJQKA"[m_rank];
	return str;
}
std::wstring Card::toStringW() const
{
	if( m_suit < 0 || m_suit >= N_SUIT || m_rank < 0 || m_rank >= N_RANK )
		return std::wstring(L"??");
	std::wstring str(1, L"♠♣♥♦"[m_suit]);
	str += L"23456789TJQKA"[m_rank];
	return str;
}
void Card::printW() const
{
#if	0
	std::wstring str = toStringW();
	coutW(str.c_str(), str.size());
	std::cout << " ";
#endif
}
//----------------------------------------------------------------------
Deck::Deck(bool shfl)
{
#if		RAND_SEED
	std::random_device rnd;     // 非決定的な乱数生成器
	m_mt.seed(rnd());		//	random_device の乱数でシードを初期化
#else
	m_mt.seed(0);
#endif
	init(shfl);
}
void Deck::init(bool shfl)
{
	m_nDealt = 0;
	int i = 0;
	for (int s = 0; s < Card::N_SUIT; ++s) {
		for (int r = 0; r < Card::N_RANK; ++r) {
			m_card[i++] = Card(s, r);
		}
	}
	if( shfl )
		shuffle();
}
//	まだ配られていない札をシャフルする
void Deck::shuffle()
{
	for (int i = m_nDealt; i < Card::N_CARD - 1; ++i) {
		//int r = rand() % (Card::N_CARD - i) + i;
		std::uniform_int_distribution<> randCard(0, Card::N_CARD - 1 - i);
		int r = randCard(m_mt) + i;
		if( r != i ) {
			//	i 番目と r 番目のカードを入れ替える
			Card c = m_card[i];
			m_card[i] = m_card[r];
			m_card[r] = c;
		}
	}
}
//	カードを１枚配る（配ったカードを返す）
Card Deck::deal()
{
	if( !nRest() ) return Card(-1, -1);
	return m_card[m_nDealt++];
}
//	指定カードをデッキから取り出す
bool Deck::take(Card c)
{
	for (int i = m_nDealt; i < Card::N_CARD; ++i) {
		if( m_card[i] == c ) {
			m_card[i] = m_card[m_nDealt];
			m_card[m_nDealt] = c;			//	取り出すカードを配布済み最後に移動
			++m_nDealt;
			return true;
		}
	}
	return false;
}
void Deck::print() const
{
	std::cout << "nDealt = " << m_nDealt << "\n";
	for (int i = 0; i < Card::N_CARD; ++i) {
		m_card[i].printW();
		//std::cout << "  ";
		if( i % Card::N_RANK == Card::N_RANK - 1 )
			std::cout << "\n";
	}
}
