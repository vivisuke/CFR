#include <vector>
#include <string>
#include <random>
#include <unordered_map>
#include <algorithm>
#include <iostream>

using namespace std;

//#define		DO_PRINT		1

typedef unsigned char uchar;

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

random_device g_rand;     	// 非決定的な乱数生成器
mt19937 g_mt(g_rand());     // メルセンヌ・ツイスタの32ビット版

class Agent {
public:
	virtual int sel_action(uchar card, int n_actions, bool raised) = 0;
	//virtual int sel_action(uchar card, bool raised, const vector<uchar>& hist_actions) = 0;
};
//	ランダムエージェント、ただしチェック可能な状態でフォールドは行わない
class RandomAgent : public Agent {
public:
	virtual int sel_action(uchar card, int n_actions, bool raised) {
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
	virtual int sel_action(uchar card, int n_actions, bool raised) {
		if( !raised ) {		//	非レイズ状態
			return ACT_RAISE;
		} else {			//	レイズされた状態
			return ACT_CALL;
		}
	}
};
// 常に弱気
class BearishAgent : public Agent {
	virtual int sel_action(uchar card, int n_actions, bool raised) {
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
	virtual int sel_action(uchar card, int n_actions, bool raised) {
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
	virtual int sel_action(uchar card, int n_actions, bool raised) {
		if( raised )
			return ACT_CALL;
		else
			return ACT_CHECK;
	}
public:
	unordered_map<const string, int[N_ACTIONS]> m_map;
};

//--------------------------------------------------------------------------------

class KuhnPoker {
public:
	KuhnPoker() {
		//cout << "player1: Random, player2: Optimal\n";
		m_agents[0] = new OptimalAgent();		//	最適戦略プレイヤー
		//m_agents[0] = new BullishAgent();		//	常に強気プレイヤー
		//m_agents[0] = new RandomAgent();		//	ランダムプレイヤー
		//m_agents[0] = new BearishAgent();		//	常に弱気プレイヤー
		m_agents[1] = new OptimalAgent();		//	最適戦略プレイヤー
		//m_agents[1] = new BullishAgent();		//	常に強気プレイヤー
		//m_agents[1] = new RandomAgent();		//	ランダムプレイヤー
		//m_agents[1] = new BearishAgent();		//	常に弱気プレイヤー
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
	void swap_agents() {
		swap(m_agents[0], m_agents[1]);
	}
	int playout(bool swapped) {
		m_raised = false;
		m_hist_actions.clear();
		shuffle_deck();
		#ifdef DO_PRINT
			print_deck();
		#endif
		//act_random(PLAYER_1)
		auto ut = playout_sub(m_deck[0], m_deck[1], 0, false);
		#ifdef	DO_PRINT
			if( !swapped )
				cout << "utility = " << ut << "\n\n";
			else
				cout << "utility = " << -ut << "\n\n";
		#endif
		return ut;
	}
	//	return: 次の手番からみた効用
	int playout_sub(uchar card1, uchar card2, int n_actions, bool raised) {
		int ut = 0;
		int aix = m_hist_actions.size() % 2;
		auto act = m_agents[aix]->sel_action(card1, n_actions, raised);
		#ifdef DO_PRINT
			cout << m_hist_actions.size() + 1 << ": " << action_string[act] << "\n";
		#endif
		m_hist_actions.push_back(act);
		if( act == ACT_FOLD ) {
			ut = -1;
		} else {
			if( m_hist_actions.size() >= 2 && (act == ACT_CHECK || act == ACT_CALL) ) {
				ut = card1 > card2 ? 1 : -1;
				if (act == ACT_CALL)
					ut *= 2;
			} else {
				if( act == ACT_RAISE ) raised = true;
				ut = -playout_sub(card2, card1, n_actions + 1, raised);
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
	
	//	deck[0] for Player1, deck[1] for Player2
	vector<uchar> m_deck;
	vector<uchar> m_hist_actions;					//	実行アクション履歴
	unordered_map<string, int[N_ACTIONS]> m_map;	//	状態 → 反事実後悔テーブル
};
//vector<uchar> g_deck;

int main()
{
	KuhnPoker kp;
	//kp.print_deck();
	//kp.shuffle_deck();
	//kp.print_deck();
	int sum = 0;
	const int N_PLAYOUT = 1000*1000;
	cout << "N_PLAYOUT = " << N_PLAYOUT << "\n";
	for (int i = 0; i < N_PLAYOUT; ++i) {
		#ifdef DO_PRINT
			cout << "#" << (i+1) << ": ";
		#endif
		sum += kp.playout(false);
	}
#if	0
	for (int i = 0; i < N_PLAYOUT/2; ++i) {
		#ifdef DO_PRINT
			cout << "#" << (i*2+1) << ": ";
		#endif
		sum += kp.playout(false);
		kp.swap_agents();
		#ifdef DO_PRINT
			cout << "#" << (i*2+2) << ": ";
		#endif
		sum -= kp.playout(true);
		kp.swap_agents();
	}
#endif
	cout << "ave(ut) = " << (double)sum/N_PLAYOUT << "\n";
#if	0
	g_deck.push_back(RANK_J);
	g_deck.push_back(RANK_Q);
	g_deck.push_back(RANK_K);
	//
	for (int k = 0; k < 10; ++k) {
		shuffle(g_deck.begin(), g_deck.end(), g_mt);
		//
		for (int i = 0; i != g_deck.size(); ++i) {
			cout << "JQK"[g_deck[i]] << " ";
		}
		cout << "\n";
	}
#endif
	//
    std::cout << "OK!\n";
}
