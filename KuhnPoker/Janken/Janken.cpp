#include <iostream>
#include <string>
#include <random>

#define		NO_STATE		1

using namespace std;

random_device g_rand;     	// 非決定的な乱数生成器
mt19937 g_mt(g_rand());     // メルセンヌ・ツイスタの32ビット版
//mt19937 g_mt(0);     // メルセンヌ・ツイスタの32ビット版

enum {			//	アクション：グー・チョキ・パー
	GOO = 0,
	CHOKI,
	PAR,
	N_ACTIONS,
};

#if	NO_STATE
int sum_AI_regrets[N_ACTIONS];
#else
int sum_AI_regrets[N_ACTIONS*N_ACTIONS*N_ACTIONS];		//	直前の手の状態ごと
#endif
//int sum_enemy_regrets[N_ACTIONS] = { 100, 0, 0 };	//	GOO 100%
int sum_enemy_regrets[N_ACTIONS] = { 50, 50, 0 };	//	GOO, CHOKI：50%, 50%
//int sum_enemy_regrets[N_ACTIONS] = { 80, 20, 0 };	//	GOO, CHOKI：80%, 20%
//int sum_enemy_regrets[N_ACTIONS] = { 90, 10, 0 };	//	GOO, CHOKI：90%, 10%
//int sum_enemy_regrets[N_ACTIONS] = { 40, 30, 30 };	//	GOO, CHOKI, PAR：40%, 30%, 30%

const char *GCP_str[] = {"GOO", "CHOKI", "PAR"};
string to_GCP(int h) { return GCP_str[h]; }

int decide_hand(int sum_regrets[]) {		//	sum_regrets[] を参照し、手を決める
	auto sum = sum_regrets[GOO] + sum_regrets[CHOKI] + sum_regrets[PAR];
	if( sum == 0 ) return g_mt() % N_ACTIONS;
	int r = g_mt() % sum;
	if( r < sum_regrets[GOO] )
		return GOO;
	if( r < sum_regrets[GOO] + sum_regrets[CHOKI] )
		return CHOKI;
	else
		return PAR;
}
int decide_AI_hand() { return decide_hand(sum_AI_regrets); }
int decide_AI_hand(int pp1, int pp2) {
	return decide_hand(&sum_AI_regrets[(pp1*N_ACTIONS + pp2)*N_ACTIONS]);
}
int decide_enemy_hand() { return decide_hand(sum_enemy_regrets); }
int do_judge(int p1, int p2) {      //  p1 からみた勝利判定、勝ち:+1、引き分け：0、負け：-1
	if (p1 < p2) return -do_judge(p2, p1);
	//  p1 > p2
	if (p1 == p2)
		return 0;     //  引き分け
	if (p1 - p2 == 2)
		return 1;		//	p1 の勝ち
	else
		return -1;		//	p1 の負け
}
void do_train(int N_ITR) {		//	N_ITR回学習
	int prev_p1 = PAR;		//	直前手
	int prev_p2 = PAR;		//	直前手
	int sum_ut = 0;
	for(int i = 0; i != N_ITR; ++i) {
#if NO_STATE
		int p1 = decide_AI_hand();
		//int p2 = decide_enemy_hand();
		int p2 = i % N_ACTIONS;
#else
		int p1 = decide_AI_hand(prev_p1, prev_p2);
#if 1
		int p2 = i % N_ACTIONS;
#else
		int p2;
		if( i == 0 )
			p2 = g_mt() % N_ACTIONS;
		else {
			if( (p2 = prev_p1 - 1) < 0 )
				p2 += N_ACTIONS;
		}
#endif
#endif
		int jdg = do_judge(p1, p2);
		sum_ut += jdg;
		string txt = "#" + to_string(i+1);
		while( txt.size() < 4 ) txt = ' ' + txt;
		cout << txt << ": ";
		cout << to_GCP(p1) << "\t" << to_GCP(p2) << "\t" << jdg << "\n";
		//	CFR 学習
		for(int h = GOO; h != N_ACTIONS; ++h) {
			if( h != p1 ) {
				int jdg2 = do_judge(h, p2);
#if NO_STATE
				sum_AI_regrets[h] = max(0, sum_AI_regrets[h] + jdg2 - jdg);
#else
				int ix = h + (prev_p1 * N_ACTIONS + prev_p2) * N_ACTIONS;
				sum_AI_regrets[ix] = max(0, sum_AI_regrets[ix] + jdg2 - jdg);
#endif
			}
		}
		prev_p1 = p1;
		prev_p2 = p2;
	}
	cout << "avg = " << (double)sum_ut / N_ITR << "\n";
}
void print_sum_regrets(int sum_regrets[], int N) {
	cout << "G:C:P = ";
	for (int i = 0; i != N; ++i) {
		cout << sum_regrets[i];
		if( i+1 != N) cout << ":";
	}
	cout << "\n\n";
}
int main()
{
#if 0
	for (int i = 0; i != 10; ++i) {
		int p1 = g_mt() % N_ACTIONS;
		int p2 = g_mt() % N_ACTIONS;
		cout << to_GCP(p1) << " " << to_GCP(p2) << " " << do_judge(p1, p2) << "\n";
	}
#endif
	cout << "enemy: ";
	print_sum_regrets(sum_enemy_regrets, N_ACTIONS);
	do_train(20);
	cout << "AI: ";
	print_sum_regrets(sum_AI_regrets, sizeof(sum_AI_regrets)/sizeof(sum_AI_regrets[0]));
	std::cout << "\nOK.\n";
}
