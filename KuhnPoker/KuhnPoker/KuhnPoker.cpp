#include <vector>
#include <string>
#include <random>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std;

random_device g_rand;     	// 非決定的な乱数生成器
mt19937 g_mt(g_rand());     // メルセンヌ・ツイスタの32ビット版
//mt19937 g_mt(0);     // メルセンヌ・ツイスタの32ビット版

#define		DO_PRINT		0
#define		N_PLAYERS		2
//#define		N_PLAYOUT		200
#define		N_PLAYOUT		(1000*1000)

typedef unsigned char uchar;
typedef unsigned int uint;

enum {
	ACT_FOLD = 0,
	ACT_CHECK,
	ACT_CALL,
	ACT_RAISE,
	N_ACTIONS,
	RANK_J = 11,
	RANK_Q,
	RANK_K,
};
enum {
	PLAYER_1 = 0, PLAYER_2,
};

const char *action_string[N_ACTIONS] = {
	"Folded", "Checked", "Called", "Raised",
};

int			g_n_playout = 0;					//	プレイアウト回数
string		g_key = "   ";
unordered_map<string, pair<int, int>> g_map;	//	<first, second>: <FOLD, CALL> or <CHECK, RAISE> 順
//vector<int>	g_cnt_card = {0, 0, 0};
//vector<uchar> g_hist_actions;					//	実行アクション履歴

class Agent {
public:
	virtual string get_name() const = 0;
	virtual int sel_action(uchar card, int n_actions, bool raised) = 0;
	//virtual int sel_action(uchar card, bool raised, const vector<uchar>& hist_actions) = 0;
};
//	ランダムエージェント、ただしチェック可能な状態でフォールドは行わない
class RandomAgent : public Agent {
public:
	string get_name() const { return "RandomAgent"; }
	int sel_action(uchar card, int n_actions, bool raised) {
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
	int sel_action(uchar card, int n_actions, bool raised) {
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
	int sel_action(uchar card, int n_actions, bool raised) {
		if( !raised ) {		//	非レイズ状態
			return ACT_CHECK;
		} else {			//	レイズされた状態
			return ACT_FOLD;
		}
	}
};
// 複合戦略ナッシュ均衡 最適戦略
const int RAND_RANGE = 10000;
class OptimalAgent : public Agent {
	string get_name() const {
		//sstream ss;
		return string("OptimalAgent ") + to_string((int)(m_alpha * 100));
	}
	int sel_action(uchar card, int n_actions, bool raised) {
		int rd = g_mt() % RAND_RANGE;
		switch( n_actions ) {
		case 0:		//	初手
			switch( card ) {
			case RANK_J:
				if( rd < m_alpha * RAND_RANGE )
					return ACT_RAISE;
				return ACT_CHECK;
			case RANK_Q:
				return ACT_CHECK;
			case RANK_K:
				if( rd < m_alpha * RAND_RANGE * 3 )
					return ACT_RAISE;
				return ACT_CHECK;
			}
		case 1:		//	２手目
			switch( card ) {
			case RANK_J:
				if( raised )			//	レイズされた
					return ACT_FOLD;
				if( rd < RAND_RANGE /3 )	//	1/3 の確率でレイズ
					return ACT_RAISE;
				return ACT_CHECK;
			case RANK_Q:
				if( raised ) {
					if( rd < RAND_RANGE /3 )	//	1/3 の確率でコール
						return ACT_CALL;
					return ACT_FOLD;
				}
				return ACT_CHECK;
			case RANK_K:
				if( raised )
					return ACT_CALL;
				return ACT_RAISE;
			}
		case 2:		//	３手目（チェック→レイズ の場合に限る）
			switch( card ) {
			case RANK_J:
				return ACT_FOLD;
			case RANK_Q:
				if( raised ) {
					if( rd < RAND_RANGE * (m_alpha + 1.0/3) )	//	alpha + 1/3 の確率でコール
						return ACT_CALL;
					return ACT_FOLD;
				}
				return ACT_CHECK;
			case RANK_K:
				return ACT_CALL;
			}
		}
		return ACT_FOLD;
	}
public:
	double	m_alpha = 0.3;
};
class CFRAgent : public Agent {
public:
	string get_name() const { return "CFRAgent"; }
	int sel_action(uchar card, int n_actions, bool raised) {
		g_key[0] = "JQK"[card - RANK_J];
		g_key[1] = '0' + n_actions;
		g_key[2] = raised ? 'R' : ' ';
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
	}
public:
};

//--------------------------------------------------------------------------------

class KuhnPoker {
public:
	KuhnPoker() {
		//cout << "player1: Random, player2: Optimal\n";
		m_agents[0] = new CFRAgent();			//	CFRプレイヤー
		//m_agents[0] = new OptimalAgent();		//	最適戦略プレイヤー
		//m_agents[0] = new BullishAgent();		//	常に強気プレイヤー
		//m_agents[0] = new RandomAgent();		//	ランダムプレイヤー
		//m_agents[0] = new BearishAgent();		//	常に弱気プレイヤー
		m_agents[1] = new CFRAgent();			//	CFRプレイヤー
		//m_agents[1] = new OptimalAgent();		//	最適戦略プレイヤー
		//((OptimalAgent*)m_agents[1])->m_alpha = 0.05;
		//m_agents[1] = new BullishAgent();		//	常に強気プレイヤー
		//m_agents[1] = new RandomAgent();		//	ランダムプレイヤー
		//m_agents[1] = new BearishAgent();		//	常に弱気プレイヤー
		//
		m_bML[0] = m_agents[0]->get_name() == "CFRAgent";
		m_bML[1] = m_agents[1]->get_name() == "CFRAgent";
		//
		cout << "Player1: " << m_agents[0]->get_name() << "\n";
		cout << "Player2: " << m_agents[1]->get_name() << "\n\n";
		//
		m_deck.push_back(RANK_J);
		m_deck.push_back(RANK_Q);
		m_deck.push_back(RANK_K);
	}
public:
	void print_deck() const {
		for (int i = 0; i != m_deck.size(); ++i) {
			cout << "JQK"[m_deck[i] - RANK_J] << " ";
		}
		cout << "\n";
	}
	void shuffle_deck() {
		shuffle(m_deck.begin(), m_deck.end(), g_mt);
	}
	void print_hist_actions() {
		for (int i = 0; i != m_hist_actions.size(); ++i) {
			cout << action_string[m_hist_actions[i]] << ", ";
		}
		cout << "\n";
	}
	//void swap_agents() {
	//	swap(m_agents[0], m_agents[1]);
	//}
	int playout() {
		//if( g_n_playout == 103 ) {
		//	cout << "g_n_playout == 103\n";
		//	print_deck();
		//}
		m_raised = false;
		m_hist_actions.clear();
		shuffle_deck();
		#if DO_PRINT
			print_deck();
		#endif
		//act_random(PLAYER_1)
		//m_cnt_card = {0, 0, 0};		//	for J, Q, K カウント
		auto ut = playout_sub(m_deck[0], m_deck[1], 0, false);
		#if	DO_PRINT
			cout << "utility = " << ut << "\n\n";
		#endif
		return ut;
	}
	//	return: 次の手番からみた利得
	int playout_sub(uchar card1, uchar card2, const int n_actions, const bool raised, bool noML = false) {
		int ut = 0;
		int aix = n_actions % N_PLAYERS;
		//int aix = m_hist_actions.size() % 2;
		auto act = m_agents[aix]->sel_action(card1, n_actions, raised);
		#if DO_PRINT
			cout << n_actions + 1 << ": " << action_string[act] << "\n";
		#endif
		m_hist_actions.push_back(act);
		if( act == ACT_FOLD ) {
			ut = -1;
		} else {
			if( n_actions >= 1 && (act == ACT_CHECK || act == ACT_CALL) ) {
				ut = card1 > card2 ? 1 : -1;
				if (act == ACT_CALL)
					ut *= 2;
			} else {
				//if( act == ACT_RAISE ) raised = true;
				ut = -playout_sub(card2, card1, n_actions + 1, act == ACT_RAISE);
			}
		}
		if( !noML && m_bML[n_actions % 2] ) {		//	学習ありの場合
			g_key[0] = "JQK"[card1 - RANK_J];
			g_key[1] = '0' + n_actions;
			g_key[2] = raised ? 'R' : ' ';
#if	0
			if( /*g_key == "Q1R" ||*/ g_key == "Q2R" ) {
				//cout << g_key << "\n";
				if( g_n_playout >= 100 ) {
					g_cnt_card[card2 - RANK_J] += 1;
					if( card2 == RANK_J ) {
						cout << "#" << g_n_playout << ": card2 == RANK_J\n";
						print_deck();
						print_hist_actions();
					}
				}
				//cout << m_cnt_card[0] << ", " << m_cnt_card[1] << ", " << m_cnt_card[2] << "\n";
			}
#endif
			auto &tbl = g_map[g_key];
			int ut2;
			if( raised ) {
				//	FOLD, CALL 順
				if( act == ACT_CALL ) {		//	CALL行動済み → FOLD を試す
					tbl.first = max(0, tbl.first -1 - ut);
				} else {		//	FOLD 行動済み → CALL を試す
					ut2 = card1 > card2 ? 2 : -2;
					tbl.second = max(0, tbl.second + ut2 - ut);
				}
			} else {
				//	CHECK, RAISE 順
				if( act == ACT_RAISE ) {		//	RAISE 行動済み → CHECK を試す
					if( n_actions != 0 ) {		//	CHECK → CHECK の場合
						ut2 = card1 > card2 ? 1 : -1;
					} else {
						ut2 = -playout_sub(card2, card1, n_actions + 1, false, /*noML:*/true);
					}
					tbl.first = max(0, tbl.first + ut2 - ut);
				} else {		//	CHECK 行動済み → RAISE を試す
					ut2 = -playout_sub(card2, card1, n_actions + 1, /*raised:*/true, /*noML:*/true);
					tbl.second = max(0, tbl.second + ut2 - ut);
				}
			}
		}
		m_hist_actions.pop_back();
		return ut;
	}
private:
	bool	m_raised;
	//Agent	*m_agent1;
	//Agent	*m_agent2;
	Agent	*m_agents[2];
	bool	m_bML[2];			//	学習対応？
	
	//	deck[0] for Player1, deck[1] for Player2
	vector<uchar> m_deck;
	vector<uchar> m_hist_actions;					//	実行アクション履歴
//public:
	//vector<int>	m_cnt_card;
};
//vector<uchar> g_deck;

string sprint(int n, int wd) {
	string txt = to_string(n);
	if( txt.size() < wd )
		txt = string(wd - txt.size(), ' ') + txt;
	return txt;
}

//KuhnPoker g_kp;
int main()
{
	KuhnPoker kp;
	//kp.print_deck();
	//kp.shuffle_deck();
	//kp.print_deck();
	int sum = 0;
	//const int N_PLAYOUT = 1000*1000;
	//g_cnt_card = { 0, 0, 0 };
	g_n_playout = 0;
	cout << "N_PLAYOUT = " << N_PLAYOUT << "\n\n";
	for (int i = 0; i < N_PLAYOUT; ) {
		#if DO_PRINT
			cout << "#" << (i+1) << ": ";
		#endif
		++g_n_playout;
		sum += kp.playout();
		if( (++i % (N_PLAYOUT/10)) == 0 ) {
			//cout << "ave(ut) = " << (double)sum / (N_PLAYOUT/10) << "\n";
			sum = 0;
		}
	}
	//cout << g_cnt_card[0] << ", " << g_cnt_card[1] << ", " << g_cnt_card[2] << "\n";
	//cout << "\nave(ut) = " << (double)sum / N_PLAYOUT << "\n";
	//
	cout << "\ng_map.size() = " << g_map.size() << "\n\n";
#if	1
	vector<string> lst;
	for (auto itr = g_map.begin(); itr != g_map.end(); ++itr)
		lst.push_back((*itr).first);
	sort(lst.begin(), lst.end());
	for (auto itr = lst.begin(); itr != lst.end(); ++itr) {
		const pair<int, int> &tbl = g_map[*itr];
		cout << (*itr) << ": " << tbl.first << ", " << tbl.second;
		if( tbl.first > 0 || tbl.second > 0 ) {
			auto sum = tbl.first + tbl.second;
			cout << "\t(" << sprint(tbl.first * 100 / sum, 3) << "%, " << sprint(tbl.second * 100 / sum, 3) << "%)";
		}
		cout << "\n";
	}
#else
	for (auto itr = g_map.begin(); itr != g_map.end(); ++itr) {
		const pair<int, int> &tbl = (*itr).second;
		cout << (*itr).first << ": " << tbl.first << ", " << tbl.second << "\n";
	}
#endif
    std::cout << "\nOK!\n";
}
