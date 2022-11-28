#include <iostream>
#include <string>
#include <random>

using namespace std;

random_device g_rand;     	// 非決定的な乱数生成器
mt19937 g_mt(g_rand());     // メルセンヌ・ツイスタの32ビット版
//mt19937 g_mt(0);     // メルセンヌ・ツイスタの32ビット版

enum {			//	アクション：グー・チョキ・パー
	SG_DIST = 5,		//	スタートからゴールまでの距離
	GOO = 0,
	CHOKI,
	PAR,
	N_ACTIONS,
	GOO_GAIN = 1,		//	グーで勝った場合の移動距離
	CHOKI_GAIN = 2,		//	チョキで勝った場合の移動距離
	PAR_GAIN = 2,		//	パーで勝った場合の移動距離
	N_PLAYOUT = 1000*1000,		//	プレイアウト回数
};

//int g_gain[] = {GOO_GAIN, CHOKI_GAIN, PAR_GAIN};	//	各行動で勝利した場合の移動距離
const int g_gain[N_ACTIONS][N_ACTIONS] = {		//	[A1][A2] の場合のP1から見た利得
	{0, GOO_GAIN, -PAR_GAIN},			//	P1：グーの場合
	{-GOO_GAIN, 0, CHOKI_GAIN},			//	P1：チョキの場合
	{PAR_GAIN, -CHOKI_GAIN, 0},			//	P1：パーの場合
};
const char *g_act_str[] = {"GOO", "CHOKI", "PAR"};
int g_sum_regrets[SG_DIST][SG_DIST][N_ACTIONS];		//	各状態・行動ごとの後悔値累積、[P1距離-1][P2距離-1][行動]
int g_win_count[SG_DIST][SG_DIST];					//	各距離ごとのP1勝利回数、[P1距離-1][P2距離-1]

bool random_play_out(int p1_dist = SG_DIST, int p2_dist = SG_DIST) {
	//int p1_dist = SG_DIST;	//	P1距離
	//int p2_dist = SG_DIST;	//	P2距離
	while( p1_dist > 0 && p2_dist > 0 ) {
		//cout << p1_dist << ", " << p2_dist << ": ";
		int a1 = g_mt() % N_ACTIONS;
		int a2 = g_mt() % N_ACTIONS;
		//cout << "GCP"[a1] << ", " << "GCP"[a2] << "\n";
		int g = g_gain[a1][a2];
		if( g > 0 )			//	P1が勝利
			p1_dist -= g;
		else if( g < 0 )	//	P2が勝利
			p2_dist += g;
	}
	//cout << p1_dist << ", " << p2_dist << ": ";
	//if( p1_dist <= 0 )
	//	cout << "p1 won.\n";
	//else
	//	cout << "p2 won.\n";
	return p1_dist <= 0;			//	p1 勝利
}
//	最初の移動だけ行い、その後は g_win_count[][] を参照して勝ち負けを決める
bool random_play_out_DP(int p1_dist = SG_DIST, int p2_dist = SG_DIST) {
	int g = 0;
	while( g == 0 ) {
		int a1 = g_mt() % N_ACTIONS;
		int a2 = g_mt() % N_ACTIONS;
		//cout << "GCP"[a1] << ", " << "GCP"[a2] << "\n";
		g = g_gain[a1][a2];
	}
	if( g > 0 ) {			//	P1が勝利
		if( (p1_dist -= g) <= 0 )
			return true;
	} else if( g < 0 ) {	//	P2が勝利
		if( (p2_dist += g) <= 0 )
			return false;
	}
	return true;
}
void init_win_count(bool b50 = true) {		//	b50: p1 == p2 を 50% に設定
	for(int i = 0; i != SG_DIST; ++i) {
		for(int k = 0; k != SG_DIST; ++k) {
			if( b50 && i == k )
				g_win_count[i][k] = N_PLAYOUT / 2;
			else
				g_win_count[i][k] = 0;
		}
	}
}
string to_string4(double x) {
	auto txt = to_string(x);
	if( txt.size() < 4 ) {
		while( txt.size() < 4 ) txt = ' ' + txt;
	} else if( txt.size() > 4 ) {
		txt = txt.substr(0, 4);
	}
	return txt;
}
void print_win_count() {
	cout << "   ";
	for(int p1 = 1; p1 <= SG_DIST; ++p1) {
		cout << p1 << ":     ";
	}
	cout << " → P1\n";
	for(int p2 = 1; p2 <= SG_DIST; ++p2) {
		cout << p2 << ": ";
		for(int p1 = 1; p1 <= SG_DIST; ++p1) {
			cout << to_string4(((double)g_win_count[p1-1][p2-1] / N_PLAYOUT)*100) << "%, ";
		}
		cout << "\n";
	}
	cout << "\n";
}
void calc_win_count_random() {		//	各距離ごとのP1勝利回数計算、ランダム vs ランダムプレイヤー
#if 1
	init_win_count(false);
	for(int p1 = 1; p1 <= SG_DIST; ++p1) {
		for(int p2 = 1; p2 <= p1; ++p2) {
			int wcnt = 0;
			for(int i = 0; i != N_PLAYOUT; ++i) {
				if( random_play_out(p1, p2) )
					++wcnt;
			}
			g_win_count[p1-1][p2-1] = wcnt;
			if( p1 != p2 )
				g_win_count[p2-1][p1-1] = N_PLAYOUT - wcnt;
		}
	}
#else
	init_win_count();
	for(int p1 = 1; p1 != SG_DIST; ++p1) {
		for(int p2 = 0; p2 != p1; ++p2) {
			int wcnt = 0;
			for(int i = 0; i != N_PLAYOUT; ++i) {
				if( random_play_out(p1+1, p2+1) )
					++wcnt;
			}
			g_win_count[p1][p2] = wcnt;
			g_win_count[p2][p1] = N_PLAYOUT - wcnt;
		}
	}
#endif
	print_win_count();
}
void calc_win_count_random_DP() {		//	各距離ごとのP1勝利回数計算、ランダム vs ランダムプレイヤー
	init_win_count(false);
	for(int p1 = 1; p1 <= SG_DIST; ++p1) {
		for(int p2 = 1; p2 <= p1; ++p2) {
			int wcnt = 0;
			for(int i = 0; i != N_PLAYOUT; ++i) {
				if( random_play_out_DP(p1, p2) )
					++wcnt;
			}
			g_win_count[p1-1][p2-1] = wcnt;
			if( p1 != p2 )
				g_win_count[p2-1][p1-1] = N_PLAYOUT - wcnt;
		}
	}

	print_win_count();
}

int main()
{
	//random_play_out();
	//
	//init_win_count();
	//print_win_count();
	//
	calc_win_count_random();
	//
    std::cout << "\nOK\n";
}
