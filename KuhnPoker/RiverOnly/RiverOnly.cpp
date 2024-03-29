﻿#include <vector>
#include <string>
#include <unordered_map>
#include <random>
#include <chrono>
#include <iostream>

#include "Deck.h"
#include "poker.h"

using namespace std;

random_device g_rand;     	// 非決定的な乱数生成器
//mt19937 g_mt(g_rand());     // メルセンヌ・ツイスタの32ビット版
mt19937 g_mt(0);     // メルセンヌ・ツイスタの32ビット版

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

#define		DO_PRINT		0
#define		N_PLAYERS		3
#define		N_COMU_CARDS	5

//#define		N_PLAYOUT		3
//#define		N_PLAYOUT		1000
#define		N_PLAYOUT		(100*1000)
//#define		N_PLAYOUT		(10*1000*1000)

enum {
	ACT_FOLD = 0,
	ACT_CHECK,
	ACT_CALL,
	ACT_RAISE,
	N_ACTIONS,
	RANK_10 = 10,
	RANK_J,
	RANK_Q,
	RANK_K,
	RANK_A,
};

const char *handName[];
const char *action_string[N_ACTIONS] = {
	"Folded", "Checked", "Called", "Raised",
};

string		g_key = "12345";					//	[0] for ランク、[1]... for actions（'C' for Call, 'c' for check）
unordered_map<string, pair<int, int>> g_map;	//	後悔値マップ <first, second>: <FOLD, CALL> or <CHECK, RAISE> 順

//	winRate: 1, 3, 5, 7, 9、1 for 10 +/- 10%
void setup_key(int winRate, const vector<uchar>& hist) {
#if	1
	char b[6];
	b[0] = "TJQKA"[winRate];
	for (int i = 0; i != hist.size(); ++i) {
		b[i+1] = "FcCR"[hist[i]];
	}
	b[hist.size()+1] = '\0';
	g_key = b;
#else
	g_key[0] = "TJQKA"[card - RANK_10];
	for (int i = 0; i != hist.size(); ++i) {
		g_key[i+1] = "FcCR"[hist[i]];
	}
	g_key[hist.size()+1] = '\0';
#endif
	//if( g_key.back() == '5' ) {
	//	cout << g_key << " ???\n";
	//}
}
class Agent {
public:
	virtual string get_name() const = 0;
	//virtual int sel_action(uchar card, int n_actions, bool raised) = 0;
	virtual int sel_action(int winRate, const vector<uchar>& hist, bool raised) = 0;
};
//	ランダムエージェント、ただしチェック可能な状態でフォールドは行わない
class RandomAgent : public Agent {
public:
	string get_name() const { return "RandomAgent"; }
	int sel_action(int winRate, const vector<uchar>& hist, bool raised) {
		if( !raised ) {		//	非レイズ状態
			if( (g_mt() & 1) == 0 )
				return ACT_RAISE;
			else
				return ACT_CHECK;
		} else {			//	レイズされた状態
			if( (g_mt() & 1) == 0 )
				return ACT_CALL;
			else
				return ACT_FOLD;
		}
	}
};

// 常に強気
class BullishAgent : public Agent {
	string get_name() const { return "BullishAgent"; }
	int sel_action(int winRate, const vector<uchar>& hist, bool raised) {
		if( !raised ) {		//	非レイズ状態
			return ACT_RAISE;
		} else {			//	レイズされた状態
			return ACT_CALL;
		}
	}
};
// 常に弱気
class BearishAgent : public Agent {
	string get_name() const { return "BearishAgent"; }
	int sel_action(int winRate, const vector<uchar>& hist, bool raised) {
		if( !raised ) {		//	非レイズ状態
			return ACT_CHECK;
		} else {			//	レイズされた状態
			return ACT_FOLD;
		}
	}
};
class HeuristicAgent : public Agent {
public:
	string get_name() const { return "HeuristicAgent"; }
	int sel_action(int winRate, const vector<uchar>& hist, bool raised) {
		return ACT_FOLD;
	};
};
class CFRAgent : public Agent {
public:
	string get_name() const { return "CFRAgent"; }
	int sel_action(int winRate, const vector<uchar>& hist, bool raised) {
		setup_key(winRate, hist);
#if	1
		auto itr = g_map.find(g_key);
		if( itr == g_map.end() ) {
			g_map[g_key] = pair<int, int>{ 0, 0 };
		}
		const auto &tbl = g_map[g_key];		//	<first, second>: <FOLD, CALL> or <CHECK, RAISE> 順
		if( tbl.first <= 0 ) {
			if( tbl.second <= 0 ) {
				if (!raised) {		//	非レイズ状態
					if ((g_mt() & 1) == 0)
						return ACT_RAISE;
					else
						return ACT_CHECK;
				}
				else {			//	レイズされた状態
					if ((g_mt() & 1) == 0)
						return ACT_CALL;
					else
						return ACT_FOLD;
				}
			} else {
				//	二番目の行動を選択
				if( raised )
					return ACT_CALL;
				else
					return ACT_RAISE;
			}
		} else {
			if( tbl.second <= 0 ) {
				//	最初の行動を選択
				if( raised )
					return ACT_FOLD;
				else
					return ACT_CHECK;
			} else {
				//	tbl 要素の割合で行動を選択
				int r = g_mt() % (tbl.first + tbl.second);
				if( r < tbl.first ) {
					//	最初の行動を選択
					if( raised )
						return ACT_FOLD;
					else
						return ACT_CHECK;
				} else {
					//	二番目の行動を選択
					if( raised )
						return ACT_CALL;
					else
						return ACT_RAISE;
				}
			}
		}
#endif
		//return ACT_FOLD;		//	暫定コード
	}
public:
	//string		m_key = "   ";
	//unordered_map<string, int[2]> m_map;		//	(FOLD, CALL) or (CHECK, RAISE) 順
};
//----------------------------------------------------------------------
//vector<pair<Card, Card>>	g_players_cards;	//	各プレイヤー手札
//vector<Card>	g_comu_cards;					//	コミュニティカード

class RiverOnlyPoker {
public:
	RiverOnlyPoker() {
		//cout << "player1: Random, player2: Optimal\n";
		m_agents[0] = new CFRAgent();			//	CFRプレイヤー
		//m_agents[0] = new BullishAgent();		//	常に強気プレイヤー
		//m_agents[0] = new RandomAgent();		//	ランダムプレイヤー
		//m_agents[0] = new BearishAgent();		//	常に弱気プレイヤー
		m_agents[1] = new CFRAgent();			//	CFRプレイヤー
		//m_agents[1] = new BullishAgent();		//	常に強気プレイヤー
		//m_agents[1] = new RandomAgent();		//	ランダムプレイヤー
		//m_agents[1] = new BearishAgent();		//	常に弱気プレイヤー
		//m_agents[2] = new RandomAgent();		//	ランダムプレイヤー
		m_agents[2] = new CFRAgent();			//	CFRプレイヤー
		//m_agents[2] = new BearishAgent();		//	常に弱気プレイヤー
		//
		m_bML[0] = m_agents[0]->get_name() == "CFRAgent";
		m_bML[1] = m_agents[1]->get_name() == "CFRAgent";
		m_bML[2] = m_agents[2]->get_name() == "CFRAgent";
		//
		cout << "Player1: " << m_agents[0]->get_name() << "\n";
		cout << "Player2: " << m_agents[1]->get_name() << "\n";
		cout << "Player3: " << m_agents[2]->get_name() << "\n";
		//
#if	0
		m_deck.push_back(RANK_10);
		m_deck.push_back(RANK_J);
		m_deck.push_back(RANK_Q);
		m_deck.push_back(RANK_K);
		m_deck.push_back(RANK_A);
#endif
		//
		for (int i = 0; i != N_PLAYERS; ++i) m_sum_ut[i] = 0;
	}
public:
	void shuffle_deck() {
		//shuffle(m_deck.begin(), m_deck.end(), g_mt);
		m_deck.shuffle();
	}
	void print_deck() const {
		//for (int i = 0; i != m_deck.size(); ++i) {
		//	cout << "TJQKA"[m_deck[i] - RANK_10] << " ";
		//}
		cout << "\n";
	}
	void print_chist_actions() {
		for (int i = 0; i != m_hist_actions.size(); ++i) {
			cout << action_string[m_hist_actions[i]] << " ";
		}
		cout << "\n";
	}
	//void swap_agents() {
	//	swap(m_agents[0], m_agents[1]);
	//}
	void playout() {
		m_raised = false;
		m_n_active = N_PLAYERS;		//	フォールドしていないプレイヤー数
		m_hist_actions.clear();
		shuffle_deck();
		m_comu_cards.resize(N_COMU_CARDS);
		for (int i = 0; i != N_COMU_CARDS; ++i) {
			m_comu_cards[i] = m_deck[N_PLAYERS*2 + i];
		}
#if DO_PRINT
		cout << "\n";
		for (int i = 0; i != N_COMU_CARDS; ++i) {
			m_deck[N_PLAYERS*2 + i].print();
			cout << " ";
		}
		cout << "\n";
#endif
		for (int i = 0; i != N_PLAYERS; ++i) {
			m_folded[i] = false;
			m_ut[i] = -1;			//	参加費
			vector<Card> v;
			v.push_back(m_deck[i*2]);
			v.push_back(m_deck[i*2+1]);
			for (int k = 0; k != N_COMU_CARDS; ++k)
				v.push_back(m_deck[N_PLAYERS*2 + k]);
			//m_hand[i] = checkHand(v);
			m_hand[i] = checkHandBM(v, m_handOdr[i]);
			m_winProp[i] = calcWinSplitProbRO(m_deck[i*2], m_deck[i*2+1], m_comu_cards, N_PLAYERS);
			m_winRate[i] = std::min((int)(m_winProp[i] * 5), 4);			//	[0, 4]
#if DO_PRINT
			m_deck[i*2].print();
			cout << " ";
			m_deck[i*2+1].print();
			cout << " ";
			//
			cout << "hand = " << m_hand[i] << " " << handName[m_hand[i]] << "\t" << m_handOdr[i]
					<< "\t" << m_winProp[i]
				<< "\t" << m_winRate[i]
				<< "\n";
#endif
		}
#if DO_PRINT
		cout << "\n";
#endif
		m_pot = N_PLAYERS * 1;
#if	1
		playout_sub(0);
#else
		m_pot = N_PLAYERS;		//	ANTI:1 * N_PLAYERS
		int n_actions = 0;
		int ix = 0;
		vector<uchar> act_hist;
		while( n_active > 1 && m_ut[ix] != -2 ) {
			if( !m_folded[ix] ) {
				auto act = m_agents[ix]->sel_action(m_deck[ix], n_actions, raised);
#if DO_PRINT
				cout << (n_actions+1) << ": " << action_string[act] << "\n";
#endif
				if( act == ACT_FOLD ) {
					m_folded[ix] = true;
					n_active -= 1;
				} else if( act == ACT_RAISE ) {
					m_raised = true;
					m_ut[ix] -= 1;			//	レイズ額は常に１
					m_pot += 1;
				} else if( act == ACT_CALL ) {
					m_ut[ix] -= 1;			//	レイズ額は常に１
					m_pot += 1;
				} else {	//	act == ACT_CHECK
				}
				act_hist.push_back(act);
				++n_actions;
			}
			ix = (ix + 1) % N_PLAYERS;
			if( !raised && ix == 0 )	//	チェックで１周した場合
				break;
		}
		calc_utility(false);
#endif
		//
#if	DO_PRINT
		cout << "ut[] = {";
		for (int i = 0; i != N_PLAYERS; ++i) {
			cout << m_utility[i] << ", ";
		}
		cout << "}\n\n";
#endif
		for (int i = 0; i != N_PLAYERS; ++i) {
			m_sum_ut[i] += m_utility[i];
		}
	}
	void do_action(int ix, int act) {
		if( act == ACT_FOLD ) {
			m_folded[ix] = true;
			--m_n_active;
		} else if( act == ACT_RAISE ) {
			m_raised = true;
			m_ut[ix] -= 1;			//	レイズ額は常に１
			m_pot += 1;
		} else if( act == ACT_CALL ) {
			m_ut[ix] -= 1;			//	レイズ額は常に１
			m_pot += 1;
		}
	}
	void undo_action(int ix, int act) {
		if( act == ACT_FOLD ) {
			m_folded[ix] = false;
			++m_n_active;
		} else if( act == ACT_RAISE ) {
			m_raised = false;
			m_ut[ix] += 1;			//	レイズ額は常に１
			m_pot -= 1;
		} else if( act == ACT_CALL ) {
			m_ut[ix] += 1;			//	レイズ額は常に１
			m_pot -= 1;
		}
	}
	void playout_sub(const int ix, bool bCF = false) {
		if( m_ut[ix] != -2 ) {		//	当該プレイヤーがレイズしていない場合
			//if( !m_folded[ix] ) {
			//	フォールドした人に手番が回ってくることはないので !m_folded[ix] チェックは不要
			auto act = m_agents[ix]->sel_action(m_winRate[ix], m_hist_actions, m_raised);
#if	DO_PRINT
			if( !bCF )
				cout << (m_hist_actions.size()+1) << ": " << action_string[act] << "\n";
#endif
			do_action(ix, act);
			//setup_key(m_deck[ix], m_hist_actions);
			//if (!bCF && m_bML[ix] && g_key == "AcRF" ) {
			//	cout << "g_key = " << g_key << "\n";
			//}
			int nix = (ix + 1) % N_PLAYERS;		//	次のプレイヤー
			if( !m_raised && nix == 0 ) {	//	チェックで１周した場合
				calc_utility(bCF);
			} else if( m_n_active > 1 ) {		//	まだ複数のプレイヤーがいる場合
				m_hist_actions.push_back(act);
				playout_sub(nix, bCF);
				m_hist_actions.pop_back();
			} else {	//	降りていないプレイヤーが一人だけになった場合
				calc_utility(bCF);
			}
			undo_action(ix, act);		//	状態をもとに戻す
			//
			if( !bCF && m_bML[ix] ) {
				int act2;		//	反事実行動
				if( m_raised ) {
					act2 = (ACT_FOLD + ACT_CALL) - act;
				} else {
					act2 = (ACT_CHECK + ACT_RAISE) - act;
				}
				do_action(ix, act2);
				if( !m_raised && nix == 0 ) {	//	チェックで１周した場合
					calc_utility(true);
				} else if( m_n_active > 1 ) {		//	まだ複数のプレイヤーがいる場合
					m_hist_actions.push_back(act2);
					playout_sub(nix, true);
					m_hist_actions.pop_back();
				} else {	//	降りていないプレイヤーが一人だけになった場合
					calc_utility(true);
				}
				undo_action(ix, act2);
				//
				setup_key(m_winRate[ix], m_hist_actions);
				if( g_map.find(g_key) == g_map.end() ) {
					g_map[g_key] = pair<int, int>(0, 0);
				}
				auto &tbl = g_map[g_key];
				if( act == ACT_RAISE || act == ACT_CALL ) {
					tbl.first = max(0, tbl.first + m_CFut[ix] - m_utility[ix]);
				} else {		//	CHECK or FOLD の場合
					tbl.second = max(0, tbl.second + m_CFut[ix] - m_utility[ix]);
				}
#if	0
				if( g_key == "AcRF" ) {
					cout << "act = " << action_string[act] << ", act2 = " << action_string[act2] << "\n";
					cout << "ut = " << m_utility[ix] << ", CFut = " << m_CFut[ix] << "\n";
					cout << "map[" << g_key << "] = " << g_map[g_key].first << ", " << g_map[g_key].second << "\n";
					//if( g_map["AccR"].first > 0 ) {
					//	cout << "???\n";
					//}
					cout << "\n";
					if( act == ACT_FOLD && m_utility[ix] > 0 ) {
						cout << "???\n";
						print_deck();
						print_chist_actions();
						do_action(ix, act);
						m_hist_actions.push_back(act);
						playout_sub(nix);
						m_hist_actions.pop_back();
						undo_action(ix, act);
						cout << "\n";
					}
					if( act2 == ACT_FOLD && m_CFut[ix] > 0 ) {
						cout << "???\n";
						print_deck();
						print_chist_actions();
						do_action(ix, act2);
						m_hist_actions.push_back(act2);
						playout_sub(nix, true);
						m_hist_actions.pop_back();
						undo_action(ix, act2);
						cout << "\n";
					}
				}
#endif
			}
		} else { 		//	レイズで１周してきた場合
			calc_utility(bCF);
		}
	}
	void calc_utility(bool bCF) {	//	pot チップを勝者に
		int*ut = !bCF ? m_utility : m_CFut;
		int mxcd = -1;
		int mxi;
		for (int i = 0; i != N_PLAYERS; ++i) {
			ut[i] = m_ut[i];
			//	undone: 手役が同じ場合の勝者判定
			if( !m_folded[i] && (int)m_handOdr[i] > mxcd ) {
				mxcd = m_handOdr[i];
				mxi = i;
			}
		}
		ut[mxi] += m_pot;
	}
	void print_ave_ut() {
		cout << "ave ut[] = {";
		for (int i = 0; i != N_PLAYERS; ++i) {
			cout << (double)m_sum_ut[i] / N_PLAYOUT << ", ";
		}
		cout << "}\n";
	}
private:
	bool	m_raised;				//	レイズ済みか？
	int		m_n_active;				//	フォールドしていないプレイヤー数
	int		m_pot;					//	ポットにあるチップ数
	Agent	*m_agents[N_PLAYERS];		//	エージェントオブジェクトへのポインタ
	bool	m_bML[N_PLAYERS];			//	学習対応エージェント？
	bool	m_folded[N_PLAYERS];
	int		m_ut[N_PLAYERS];			//	作業用１プレイアウトでの各プレイヤーの効用（利得）
	int		m_CFut[N_PLAYERS];			//	反事実値用１プレイアウトでの各プレイヤーの効用（利得）
	int		m_utility[N_PLAYERS];		//	１プレイアウトでの各プレイヤーの効用（利得）
	int		m_hand[N_PLAYERS];			//	各プレイヤーの手役
	uint	m_handOdr[N_PLAYERS];		//	各プレイヤーの手役サブ情報
	double	m_winProp[N_PLAYERS];		//	各プレイヤー期待勝率
	int		m_winRate[N_PLAYERS];		//	各プレイヤー期待勝率状態圧縮、1 or 3 or 5 or 7 or 9
	
	//vector<pair<Card, Card>>	m_players_cards;	//	各プレイヤー手札
	vector<Card>	m_comu_cards;					//	コミュニティカード
	//	deck[0] for Player1, deck[1] for Player2
	Deck			m_deck;
	//vector<uchar> m_deck;
	vector<uchar> m_hist_actions;					//	実行アクション履歴
	//unordered_map<string, int[N_ACTIONS]> m_map;	//	状態 → 反事実後悔テーブル
	//string		m_key = "   ";
	//unordered_map<string, int[2]> m_map;			//	(FOLD, CALL) or (CHECK, RAISE) 順
public:
	int		m_sum_ut[N_PLAYERS];		//	全プレイアウトでの各プレイヤーの効用（利得）
};
//----------------------------------------------------------------------
int main()
{
	auto start = std::chrono::system_clock::now();      // 計測スタート時刻を保存
	//
	RiverOnlyPoker rop;
	int sum = 0;
	cout << "\nN_PLAYOUT = " << N_PLAYOUT << "\n\n";
	for (int i = 0; i < N_PLAYOUT; ++i) {
		#if DO_PRINT
			cout << "#" << (i+1) << ": ";
		#endif
		rop.playout();
	}
	cout << "g_map.size() = " << g_map.size() << "\n\n";
	vector<string> lst;
	for (auto itr = g_map.begin(); itr != g_map.end(); ++itr)
		lst.push_back((*itr).first);
	sort(lst.begin(), lst.end());
	for (auto itr = lst.begin(); itr != lst.end(); ++itr) {
		const pair<int, int> &tbl = g_map[*itr];
		//cout << (*itr) << ":\t" << sprint(tbl.first, 6) << ", " << sprint(tbl.second, 6);
		cout << "\"" << (*itr) << "\": "; //<< sprint(tbl.first, 6) << ", " << sprint(tbl.second, 6);
		if( tbl.first > 0 || tbl.second > 0 ) {
			auto sum = tbl.first + tbl.second;
			//#cout << "\t(" << sprint(tbl.first * 100 / sum, 3) << "%, " << sprint(tbl.second * 100 / sum, 3) << "%)";
			cout << (double)tbl.first / sum << ",";
		} else {
			cout << "0.5,";
		}
		cout << "\n";
	}
	cout << "\n";
	//
	rop.print_ave_ut();
	//
	auto end = std::chrono::system_clock::now();       // 計測終了時刻を保存
    auto dur = end - start;        // 要した時間を計算
    auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    // 要した時間をミリ秒（1/1000秒）に変換して表示
    std::cout << msec << " milli sec \n";
#if	0
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
#endif
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

/*
"A": 0.971014, "AR": 0.0078125, "ARC": 0.00819672, "ARF": 0, "Ac": 0.0873016, "AcR": 0,
"AcRC": 0, "AcRF": 0, "Acc": 0, "AccR": 0, "AccRC": 0.025641, "AccRF": 0,
"K": 0.379691, "KR": 0, "KRC": 0.0903955, "KRF": 0, "Kc": 0.474006, "KcR": 0,
"KcRC": 0, "KcRF": 0.00606061, "Kcc": 0, "KccR": 0.010989, "KccRC": 0, "KccRF": 0.00925926,
"Q": 0.849412, "QR": 0.352577, "QRC": 1, "QRF": 0.0592885, "Qc": 0.330869, "QcR": 0.022293,
"QcRC": 0.971014, "QcRF": 0, "Qcc": 0.251121, "QccR": 0.0144092, "QccRC": 0.886364, "QccRF": 0.0454545,
"J": 0.965726, "JR": 0.937209, "JRC": 1, "JRF": 0.144981, "Jc": 1, "JcR": 0.991935,
"JcRC": 1, "JcRF": 0.256293, "Jcc": 1, "JccR": 0.375912, "JccRC": 1, "JccRF": 0.0116959,
"T": 0.978845, "TR": 1, "TRC": 1, "TRF": 1, "Tc": 0.822243, "TcR": 1,
"TcRC": 1, "TcRF": 1, "Tcc": 0.678679, "TccR": 0.992424, "TccRC": 1, "TccRF": 0.994318,
*/


