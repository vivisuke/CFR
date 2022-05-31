#include <vector>
#include <string>
#include <random>
#include <iostream>

#include "Deck.h"
#include "poker.h"

using namespace std;

typedef unsigned int uint;
typedef unsigned short ushort;

#define		N_PLAYERS		3
#define		N_COMU_CARDS	5

const char *handName[];

vector<pair<Card, Card>>	g_players_cards;	//	各プレイヤー手札
vector<Card>	g_comu_cards;					//	コミュニティカード

int main()
{
	Deck dk;
	dk.shuffle();
	
	for (int i = 0; i != N_PLAYERS; ++i) {
		g_players_cards.push_back(pair<Card, Card>{dk[i*2], dk[i*2+1]});
	}
	
	for (int i = 0; i != N_COMU_CARDS; ++i) {
		g_comu_cards.push_back(dk[N_PLAYERS*2 + i]);
	}
	for (int i = 0; i != N_PLAYERS; ++i) {
		vector<Card> v;
		v.push_back(g_players_cards[i].first);
		v.push_back(g_players_cards[i].second);
		for (int k = 0; k != N_COMU_CARDS; ++k) {
			v.push_back(g_comu_cards[k]);
		}
		for (int k = 0; k != v.size(); ++k) {
			auto cd = v[k];
			cd.print();
			cout << " ";
		}
		auto h = checkHand(v);
		cout << "hand = " << h << " " << handName[h] << "\n";
	}
#if	0
	vector<Card> v;
	for (int i = 0; i != 7; ++i) {
		auto cd = dk[i];
		v.push_back(cd);
		cd.print();
		cout << " ";
	}
	cout << "\n";
	auto h = checkHand(v);
	cout << "hand = " << h << " " << handName[h] << "\n";
#endif
	
    std::cout << "\nOK.\n";
}
