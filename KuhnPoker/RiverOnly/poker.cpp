#include "poker.h"
#include "Deck.h"
#include <vector>
#include <algorithm>
#include <random>
#include <assert.h>

using namespace std;

extern std::mt19937 g_mt;

std::vector<std::vector<Card>> g_vv;
std::vector<Card> g_v;

const char *handName[] = {
	"highCard     ",
	"onePair      ",
	"twoPair      ",
	"threeOfAKind ",
	"straight     ",
	"flush        ",
	"fullHouse    ",
	"fourOfAKind  ",
	"straightFlush",
	"RoyalFlush   ",
};
//	v のサイズは 7以下とする
//	要素はソートされていないものとする
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
//	v のサイズは 7以下とする
//	要素はソートされていないものとする
//	強さを odr に格納するもとする、下位 5*4bit：手の強さ、その上位4ビット：役
int checkHand(const std::vector<Card> v, uint &odr)
{
	odr = 0;
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
	uint bitmap = 0;
	if( scnt[s = 0] >= 5 || scnt[s = 1] >= 5 || scnt[s = 2] >= 5 || scnt[s = 3] >= 5 ) {
		//	フラッシュ確定
		for (uint i = 0; i < v.size(); ++i) {
			const Card c = v[i];
			if( c.m_suit == s ) {
				bitmap |= 1 << (c.m_rank);
			}
		}
		uint mask = 0x1f00;		//	AKQJT
		for (int i = 0; i < 9; ++i, mask>>=1) {
			if( (bitmap & mask) == mask ) {
				//	ストレート・フラッシュの場合は、一番大きいカードランクのみで優劣が決まる
				odr |= (12 - i) + (STRAIGHT_FLUSH << 20);
				return STRAIGHT_FLUSH;
			}
		}
		if( bitmap == 0x100f ) {	//	1 0000 00000 1111 = A5432
			odr |= (5-2) + (STRAIGHT_FLUSH << 20);
			return STRAIGHT_FLUSH;
		}
	} else
		s = -1;
	int threeOfAKindIX = -1;
	int threeOfAKindIX2 = -1;
	int pairIX1 = -1;
	int pairIX2 = -1;
	for (int i = 0; i < 13; ++i) {
		switch( rcnt[i] ) {
			case 4:
				//	同じ数の４カードは無いので、４カードのランクのみで優劣が決まる
				odr |= i + (FOUR_OF_A_KIND << 20);
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
	if( threeOfAKindIX >= 0 && pairIX1 >= 0 ) {
		//	フルハウスの場合は、３カードのランクが優先、ついでペアのランク
		odr = threeOfAKindIX * 16 + pairIX1 + (FULL_HOUSE << 20);
		return FULL_HOUSE;
	}
	//	3カード*2 もフルハウス
	if( threeOfAKindIX >= 0 && threeOfAKindIX2 >= 0 ) {
		odr = (threeOfAKindIX2 <<4) + threeOfAKindIX + (FULL_HOUSE << 20);
		return FULL_HOUSE;
	}
	if( s >= 0 ) {
		uint mask = 0x1000 << 1;		//	A
		int r = 12 + 1;		//	A
		for (int i = 0; i < 5; ++i) {		//	大きい順に５枚取り出す
			do {
				--r;
				mask >>= 1;
				if( !mask ) {
					for (uint i = 0; i < v.size(); ++i) {
						std::cout << v[i].toString() << " ";
					}
					std::cout << "\n";
				}
				assert( mask );
			} while( !(bitmap & mask) );
			odr = (odr << 4) + r;
		}
		odr |= (FLUSH << 20);
		return FLUSH;
	}
	bitmap = 0;
	uint mask = 1;
	for (int i = 0; i < 13; ++i, mask<<=1) {
		if( rcnt[i] != 0 )
			bitmap |= mask;
	}
	mask = 0x1f00;		//	AKQJT
	for (int i = 0; i < 9; ++i, mask>>=1) {
		if( (bitmap & mask) == mask ) {
			odr = (12 - i) + (STRAIGHT << 20);
			return STRAIGHT;
		}
	}
	if( (bitmap & 0x100f) == 0x100f ) {	//	1 0000 00000 1111 = A5432
		odr = (5-2) + (STRAIGHT << 20);
		return STRAIGHT;
	}
	if( threeOfAKindIX >= 0 ) {
		odr = threeOfAKindIX;
		int r = 12 + 1;
		while( --r == threeOfAKindIX || !rcnt[r] ) { }
		odr = (odr << 4) + r;
		while( --r == threeOfAKindIX || !rcnt[r] ) { }
		odr = (odr << 4) + r;
		odr |= (THREE_OF_A_KIND << 20);
		return THREE_OF_A_KIND;
	}
	if( pairIX2 >= 0 ) {
		if( pairIX1 > pairIX2 )
			odr = (pairIX1 << 8) + (pairIX2 << 4);
		else
			odr = (pairIX2 << 8) + (pairIX1 << 4);
		int r = 12;		//	A
		while( !rcnt[r] || r == pairIX1 || r == pairIX2 ) --r;
		odr |= r;
		odr |= (TWO_PAIR << 20);
		return TWO_PAIR;
	}
	if( pairIX1 >= 0 ) {
		odr = pairIX1;
		int r = 12 + 1;		//	A
		while( !rcnt[--r] || r == pairIX1) { }
		odr = (odr << 4) + r;
		while( !rcnt[--r] || r == pairIX1) { }
		odr = (odr << 4) + r;
		while( !rcnt[--r] || r == pairIX1) { }
		odr = (odr << 4) + r;
		odr |= (ONE_PAIR << 20);
		return ONE_PAIR;
	}
	int r = 12 + 1;		//	A
	for (int i = 0; i < 5; ++i) {
		while( !rcnt[--r] ) { }
		odr = (odr << 4) + r;
	}
	odr |= (HIGH_CARD << 20);
	return HIGH_CARD;
}
void print(const std::vector<Card> v)
{
	for (uint i = 0; i < v.size(); ++i) {
		//std::cout << v[i].toString() << " ";
		v[i].printW();
		//std::cout << " ";
	}
	std::cout << "\n";
}
void print(const std::vector<Card> v, uint odr)
{
	for (uint i = 0; i < v.size(); ++i) {
		//std::cout << v[i].toString() << " ";
		v[i].printW();
		//std::cout << " ";
	}
	std::cout << std::hex << odr << std::dec << "\n";
}
void print(const std::vector<Card> v, uint odr, const char *ptr)
{
	for (uint i = 0; i < v.size(); ++i) {
		//std::cout << v[i].toString() << " ";
		v[i].printW();
		//std::cout << " ";
	}
	std::cout << std::hex << odr << std::dec << " " << ptr << "\n";
}
//----------------------------------------------------------------------
//	13bit までのビットカウント
int bitCount(int v)
{
	int cnt = 0;
	int mask = 1 << 12;
	while( mask != 0 ) {
		if( (v & mask) != 0 ) ++cnt;
		mask >>= 1;
	}
	return cnt;
}
//	ストレートの場合は、一番大きなランクを返す
int isStraight(int v)
{
	if( (v & (0x1f<<8)) == (0x1f<<8) )	//	AKQJT
		return Card::RANK_A;
	if( (v & (0x1f<<7)) == (0x1f<<7) )	//	KQJT9
		return Card::RANK_K;
	if( (v & (0x1f<<6)) == (0x1f<<6) )	//	QJT98
		return Card::RANK_Q;
	if( (v & (0x1f<<5)) == (0x1f<<5) )	//	JT987
		return Card::RANK_J;
	if( (v & (0x1f<<4)) == (0x1f<<4) )	//	T9876
		return Card::RANK_10;
	if( (v & (0x1f<<3)) == (0x1f<<3) )	//	98765
		return Card::RANK_9;
	if( (v & (0x1f<<2)) == (0x1f<<2) )	//	87654
		return Card::RANK_8;
	if( (v & (0x1f<<1)) == (0x1f<<1) )	//	76543
		return Card::RANK_7;
	if( (v & (0x1f)) == (0x1f) )	//	65432
		return Card::RANK_6;
	if( (v & 0x100f) == 0x100f )	//	5432A
		return Card::RANK_5;
	else
		return 0;
}
//	ビットマップを用いた役判定関数
//	v のサイズは７以下とする
int checkHandBM(const std::vector<Card> &v)
{
	//	各スートごとのビットマップ
	int spades = 0;
	int clubs = 0;
	int hearts = 0;
	int diamonds = 0;
	//	カードの状態をビットマップ（spades, clubs, hearts, diamonds）に変換
	for (int i = 0; i < (int)v.size(); ++i) {
		const Card c = v[i];
		const int mask = 1 << c.m_rank;
		switch( c.m_suit ) {
			case Card::SPADES:	spades |= mask;	break;
			case Card::CLUBS:	clubs |= mask;	break;
			case Card::HEARTS:	hearts |= mask;	break;
			case Card::DIAMONDS:	diamonds |= mask;	break;
		}
	}
	if( isStraight(spades) ||
		isStraight(clubs) ||
		isStraight(hearts) ||
		isStraight(diamonds) )
	{
		return STRAIGHT_FLUSH;
	}
	//	各ランク毎のビットの数を数える
	const int MASK = (1 << 13) - 1;		//	13bit のマスク
	int r0 = ~(spades |clubs | hearts | diamonds) & MASK;
	int r1 = (spades & ~clubs & ~hearts & ~diamonds) | (~spades & clubs & ~hearts & ~diamonds) |
				(~spades & ~clubs & hearts & ~diamonds) | (~spades & ~clubs & ~hearts & diamonds);
	int r3 = (~spades & clubs & hearts & diamonds) | (spades & ~clubs & hearts & diamonds) |
				(spades & clubs & ~hearts & diamonds) | (spades & clubs & hearts & ~diamonds);
	int r4 = spades & clubs & hearts & diamonds;
	int r2 = ~(r0 | r1 | r3 | r4) & MASK;
	if( r4 != 0 )		//	４枚のランクがあれば ４カード
		return FOUR_OF_A_KIND;
	//	各スートのビットカウントが５以上であればフラッシュ
	int sbc = bitCount(spades);
	if( sbc >= 5 ) return FLUSH;
	int cbc = bitCount(clubs);
	if( cbc >= 5 ) return FLUSH;
	int hbc = bitCount(hearts);
	if( hbc >= 5 ) return FLUSH;
	int dbc = bitCount(diamonds);
	if( dbc >= 5 ) return FLUSH;
	if( isStraight(spades | clubs | hearts | diamonds) )		//	ストレートチェック
		return STRAIGHT;
	if( r3 != 0 && r2 != 0 )		//	フルハウス
		return FULL_HOUSE;
	if( r3 != 0 )		//	３カード
		if( bitCount(r3) == 2 )
			return FULL_HOUSE;
		else
			return THREE_OF_A_KIND;
	if( r2 != 0 ) {
		if( bitCount(r2) > 1 )		//	ペアが２つ以上ある
			return TWO_PAIR;
		else
			return ONE_PAIR;
	}
	return HIGH_CARD;
}
//	一番左ビットのみを返す
//	x は13ビット
int MLB(int x)
{
	if( !x ) return 0;
	if( (x & 0x1f00) != 0 ) {
		if( (x & 0x100) != 0 )
			return 0x100;
		else {
			if( (x & 0xc00) != 0 ) {
				if( (x & 0x800) != 0 )
					return 0x800;
				else
					return 0x400;
			} else {
				if( (x & 0x200) != 0 )
					return 0x200;
				else
					return 0x100;
			}
		}
	} else {
		if( (x & 0xf0) != 0 ) {
			if( (x & 0xc0) != 0 ) {
				if( (x & 0x80) != 0 )
					return 0x80;
				else
					return 0x40;
			} else {
				if( (x & 0x20) != 0 )
					return 0x20;
				else
					return 0x10;
			}
		} else {
			if( (x & 0x0c) != 0 ) {
				if( (x & 0x08) != 0 )
					return 0x08;
				else
					return 0x04;
			} else {
				if( (x & 0x02) != 0 )
					return 0x02;
				else
					return 0x01;
			}
		}
	}
}
//	一番左のビットのビット番号を返す
const char mlbn16Tbl[] = {15, 0, 1, 4, 2, 8, 5, 10, 13, 3, 7, 9, 12, 6, 11, 0};
uint mlbn16(uint x)
{
   //if (x == 0) { return 16; }	//	0 でコールされることもない
   x = x | (x >> 1);
   x = x | (x >> 2);
   x = x | (x >> 4);
   x = x | (x >> 8);
   return mlbn16Tbl[(0x135c * (x - (x >> 1)) >> 12) & 15];
}
const char mlbnTbl[] = {0,1,2,15,29,3,23,16,30,27,4,6,12,24,8,17,31,14,28,22,26,5,11,7,13,21,25,10,20,9,19,18};
uint mlbnM(uint x)
{
   //if (x == 0) { return 33; }	//	0 でコールされることもない
   x = x | (x >> 1);
   x = x | (x >> 2);
   x = x | (x >> 4);
   x = x | (x >> 8);
   //x = x | (x >>16);		//	RANK は16ビットで充分なので、この行は必要ない
   //return ("\0\1\2\xf\x1d\3\x17\x10\x1e\x1b\4\6\xc\x18\x8\x11"
   //             "\x1f\xe\x1c\x16\x1a\5\xb\7\xd\x15\x19\xa\x14\x9\x13\x12")
   //            [0x5763e69U * (x - (x >> 1)) >> 27];
   return mlbnTbl[0x5763e69U * (x - (x >> 1)) >> 27];
}
//	上位からcnt枚を順に並べる
uint topN(uint bits, int cnt)
{
	uint o = 0;;
	//int cnt = 5;
	uint mask = 0x1000;
	uint rk = Card::RANK_A;
	while( --cnt >= 0 ) {
		while( (bits & mask) == 0 ) {
			if( (mask >>= 1) == 0 )
				return o;
			--rk;
			assert( mask != 0 );
		}
		o = (o << 4) + rk;
		mask >>= 1;
		--rk;
	}
	return o;
}
//	上位からcnt枚を選択
void topN(uint bits, int cnt, const std::vector<int> &vs, std::vector<bool> &o)
{
	uint mask = 0x1000;
	uint rk = Card::RANK_A;
	while( --cnt >= 0 ) {
		while( (bits & mask) == 0 ) {
			mask >>= 1;
			--rk;
			assert( mask != 0 );
		}
		o[vs[rk]] = true;
		mask >>= 1;
		--rk;
	}
}
//	上位からcnt枚を選択
void topN(int cnt,
				uint spades, uint clubs, uint hearts, uint diamonds,
				const std::vector<int> &vs,
				const std::vector<int> &vc,
				const std::vector<int> &vh,
				const std::vector<int> &vd,
				std::vector<bool> &o)
{
	uint bits = spades | clubs | hearts | diamonds;
	uint mask = 0x1000;
	uint rk = Card::RANK_A;
	while( cnt >= 0 ) {
		while( (bits & mask) == 0 ) {
			mask >>= 1;
			--rk;
			assert( mask != 0 );
		}
		if( (mask & spades) != 0 ) {
			o[vs[rk]] = true;
			if( !--cnt ) return;
		}
		if( (mask & clubs) != 0 ) {
			o[vc[rk]] = true;
			if( !--cnt ) return;
		}
		if( (mask & hearts) != 0 ) {
			o[vh[rk]] = true;
			if( !--cnt ) return;
		}
		if( (mask & diamonds) != 0 ) {
			o[vd[rk]] = true;
			if( !--cnt ) return;
		}
		mask >>= 1;
		--rk;
	}
}
//	ビットマップを用いた役判定関数
//	v のサイズは７以下とする
//	odr:	[役][c1][c2][c3][c4][c5]		（各４ビット、c1-c5 は右に詰める）	手役の強さ
int checkHandBM(const std::vector<Card> &v, uint &odr)
{
	//	各スートごとのビットマップ
	int spades = 0;
	int clubs = 0;
	int hearts = 0;
	int diamonds = 0;
	//	カードの状態をビットマップ（spades, clubs, hearts, diamonds）に変換
	for (int i = 0; i < (int)v.size(); ++i) {
		const Card c = v[i];
		const int mask = 1 << c.m_rank;
		switch( c.m_suit ) {
			case Card::SPADES:	spades |= mask;	break;
			case Card::CLUBS:	clubs |= mask;	break;
			case Card::HEARTS:	hearts |= mask;	break;
			case Card::DIAMONDS:	diamonds |= mask;	break;
		}
	}
	int rk;
	if( (rk = isStraight(spades)) != 0 ||
		(rk = isStraight(clubs)) != 0 ||
		(rk = isStraight(hearts)) != 0 ||
		(rk = isStraight(diamonds)) != 0 )
	{
		//	ストレート・フラッシュの場合は、一番大きいカードランクのみで優劣が決まる
		odr = (STRAIGHT_FLUSH << 20) | rk;
		return STRAIGHT_FLUSH;
	}
	//	各ランク毎のビットの数を数える
	const int MASK = (1 << 13) - 1;		//	13bit のマスク
	int r0 = ~(spades |clubs | hearts | diamonds) & MASK;
	int r1 = (spades & ~clubs & ~hearts & ~diamonds) | (~spades & clubs & ~hearts & ~diamonds) |
				(~spades & ~clubs & hearts & ~diamonds) | (~spades & ~clubs & ~hearts & diamonds);
	int r3 = (~spades & clubs & hearts & diamonds) | (spades & ~clubs & hearts & diamonds) |
				(spades & clubs & ~hearts & diamonds) | (spades & clubs & hearts & ~diamonds);
	int r4 = spades & clubs & hearts & diamonds;
	int r2 = ~(r0 | r1 | r3 | r4) & MASK;
	if( r4 != 0 ) {	//	４枚のランクがあれば ４カード
		//	４カードが同じ数でぶつかることは無い、が、コミュニティカードが４カードの場合はぶつかる
		odr = (FOUR_OF_A_KIND << 20) + (topN(r4, 1)<<4);
		odr += topN(r3|r2|r1, 1);
		return FOUR_OF_A_KIND;
	}
	//	各スートのビットカウントが５以上であればフラッシュ
	int sbc = bitCount(spades);
	if( sbc >= 5 ) {
		odr = (FLUSH << 20) + topN(spades, 5);
		return FLUSH;
	}
	int cbc = bitCount(clubs);
	if( cbc >= 5 ) {
		odr = (FLUSH << 20) + topN(clubs, 5);
		return FLUSH;
	}
	int hbc = bitCount(hearts);
	if( hbc >= 5 ) {
		odr = (FLUSH << 20) + topN(hearts, 5);
		return FLUSH;
	}
	int dbc = bitCount(diamonds);
	if( dbc >= 5 ) {
		odr = (FLUSH << 20) + topN(diamonds, 5);
		return FLUSH;
	}
	if( rk = isStraight(spades | clubs | hearts | diamonds) ) {		//	ストレートチェック
		odr = (STRAIGHT << 20) + rk;
		return STRAIGHT;
	}
	if( r3 != 0 && r2 != 0 ) {		//	フルハウス
		odr = (FULL_HOUSE << 20) + (topN(r3, 1) << 4) + topN(r2, 1);
		return FULL_HOUSE;
	}
	if( r3 != 0 ) {		//	３カード
		switch( bitCount(r3) ) {
			case 2:
				odr = (FULL_HOUSE << 20) + topN(r3, 2);
				return FULL_HOUSE;
			case 1:
				odr = (THREE_OF_A_KIND << 20) + (topN(r3, 1) << 8) + topN(r1, 2);
				return THREE_OF_A_KIND;
		}
	}
	if( r2 != 0 ) {
		switch( bitCount(r2) ) {
			case 3: {
				odr = (TWO_PAIR << 20) + topN(r2, 3);
				uint t = topN(r1, 1);
				if( t > (odr & 0x0f) )
					odr = (odr & ~0x0f) + t;
				return TWO_PAIR;
			}
			case 2:
				odr = (TWO_PAIR << 20) + (topN(r2, 2) << 4) + topN(r1, 1);
				return TWO_PAIR;
			case 1:
				odr = (ONE_PAIR << 20) + (topN(r2, 1) << 12) + topN(r1, 3);
				return ONE_PAIR;
		}
	}
	odr = (HIGH_CARD << 20) + topN(r1, 5);
	return HIGH_CARD;
}
//	役を構成するカードを取得
void getHandBits(const std::vector<Card> &v, std::vector<bool> &o)
{
	o = std::vector<bool>(v.size(), false);
	//	各スートごとのビットマップ
	int spades = 0;
	int clubs = 0;
	int hearts = 0;
	int diamonds = 0;
	std::vector<int> vs(13, -1);		//	各カードの番号（v のインデックス）
	std::vector<int> vc(13, -1);
	std::vector<int> vh(13, -1);
	std::vector<int> vd(13, -1);
	//	カードの状態をビットマップ（spades, clubs, hearts, diamonds）に変換
	for (int i = 0; i < (int)v.size(); ++i) {
		const Card c = v[i];
		const int mask = 1 << c.m_rank;
		switch( c.m_suit ) {
			case Card::SPADES:		vs[c.m_rank] = i;	spades |= mask;	break;
			case Card::CLUBS:			vc[c.m_rank] = i;	clubs |= mask;	break;
			case Card::HEARTS:			vh[c.m_rank] = i;	hearts |= mask;	break;
			case Card::DIAMONDS:	vd[c.m_rank] = i;	diamonds |= mask;	break;
		}
	}
	int rk;
	if( (rk = isStraight(spades)) != 0 ) {
		for (int i = 0; i < 5; ++i, --rk) {
			if( rk < 0 ) rk = Card::RANK_A;
			o[vs[rk]] = true;
		}
		return;
	}
	if( (rk = isStraight(clubs)) != 0 ) {
		for (int i = 0; i < 5; ++i, --rk) {
			if( rk < 0 ) rk = Card::RANK_A;
			o[vc[rk]] = true;
		}
		return;
	}
	if( (rk = isStraight(hearts)) != 0 ) {
		for (int i = 0; i < 5; ++i, --rk) {
			if( rk < 0 ) rk = Card::RANK_A;
			o[vh[rk]] = true;
		}
		return;
	}
	if( (rk = isStraight(diamonds)) != 0 ) {
		for (int i = 0; i < 5; ++i, --rk) {
			if( rk < 0 ) rk = Card::RANK_A;
			o[vd[rk]] = true;
		}
		return;
	}
	//	各ランク毎のビットの数を数える
	const int MASK = (1 << 13) - 1;		//	13bit のマスク
	int r0 = ~(spades |clubs | hearts | diamonds) & MASK;
	int r1 = (spades & ~clubs & ~hearts & ~diamonds) | (~spades & clubs & ~hearts & ~diamonds) |
				(~spades & ~clubs & hearts & ~diamonds) | (~spades & ~clubs & ~hearts & diamonds);
	int r3 = (~spades & clubs & hearts & diamonds) | (spades & ~clubs & hearts & diamonds) |
				(spades & clubs & ~hearts & diamonds) | (spades & clubs & hearts & ~diamonds);
	int r4 = spades & clubs & hearts & diamonds;
	int r2 = ~(r0 | r1 | r3 | r4) & MASK;
	if( r4 != 0 ) {	//	４枚のランクがあれば ４カード
		//	４カードが同じ数でぶつかることは無い、コミュニティカードで４枚でた場合以外
		int rk = topN(r4, 1);
		o[vs[rk]] = true;
		o[vc[rk]] = true;
		o[vh[rk]] = true;
		o[vd[rk]] = true;
		int r = r1 | r2 | r3;
		topN(1, r & spades, r & clubs, r & hearts, r & diamonds,
					vs, vc, vh, vd,
					o);
		return;
	}
	//	各スートのビットカウントが５以上であればフラッシュ
	if( bitCount(spades) >= 5 ) {
		topN(spades, 5, vs, o);
		return;
	}
	if( bitCount(clubs) >= 5 ) {
		topN(clubs, 5, vc, o);
		return;
	}
	if( bitCount(hearts) >= 5 ) {
		topN(hearts, 5, vh, o);
		return;
	}
	if( bitCount(diamonds) >= 5 ) {
		topN(diamonds, 5, vd, o);
		return;
	}
	if( rk = isStraight(spades | clubs | hearts | diamonds) ) {		//	ストレートチェック
		//odr = (STRAIGHT << 20) + rk;
		uint mask = 1 << rk;
		for (int i = 0; i < 5; ++i, mask>>=1, --rk) {
			if( !mask ) {
				mask = 1 << Card::RANK_A;
				rk = Card::RANK_A;
			}
			if( (spades & mask) != 0 )
				o[vs[rk]] = true;
			else if( (clubs & mask) != 0 )
				o[vc[rk]] = true;
			else if( (hearts & mask) != 0 )
				o[vh[rk]] = true;
			else
				o[vd[rk]] = true;
		}
		return;
	}
	if( r3 != 0 && r2 != 0 ) {		//	フルハウス
		if( (spades & r3) != 0 )
			topN(spades & r3, 1, vs, o);
		if( (clubs & r3) != 0 )
			topN(clubs & r3, 1, vc, o);
		if( (hearts & r3) != 0 )
			topN(hearts & r3, 1, vh, o);
		if( (diamonds & r3) != 0 )
			topN(diamonds & r3, 1, vd, o);
		//	done: r2 が２ビットある場合対応
		uint r = r2 & (r2 - 1);		//	一番右のビットを0に
		if( r != 0 ) r2 = r;
		if( (spades & r2) != 0 )
			topN(spades & r2, 1, vs, o);
		if( (clubs & r2) != 0 )
			topN(clubs & r2, 1, vc, o);
		if( (hearts & r2) != 0 )
			topN(hearts & r2, 1, vh, o);
		if( (diamonds & r2) != 0 )
			topN(diamonds & r2, 1, vd, o);
		return;
	}
	if( r3 != 0 ) {		//	３カード
		switch( bitCount(r3) ) {
			case 2: {		//	3カード*2 の場合
				uint r = r3 & (r3 - 1);	//	一番右のビットを0に
				r3 ^= r;		//	少ない方のビット
				if( (spades & r) != 0 )
					topN(spades & r, 1, vs, o);
				if( (clubs & r) != 0 )
					topN(clubs & r, 1, vc, o);
				if( (hearts & r) != 0 )
					topN(hearts & r, 1, vh, o);
				if( (diamonds & r) != 0 )
					topN(diamonds & r, 1, vd, o);
				int cnt = 0;
				if( (spades & r3) != 0 ) {
					topN(spades & r3, 1, vs, o);
					++cnt;
				}
				if( (clubs & r3) != 0 ) {
					topN(clubs & r3, 1, vc, o);
					++cnt;
				}
				if( cnt < 2 && (hearts & r3) != 0 ) {
					topN(hearts & r3, 1, vh, o);
					++cnt;
				}
				if( cnt < 2 && (diamonds & r3) != 0 )
					topN(diamonds & r3, 1, vd, o);
				return;
			}
			case 1: {		//	純粋な３カードの場合
				if( (spades & r3) != 0 )
					topN(spades & r3, 1, vs, o);
				if( (clubs & r3) != 0 )
					topN(clubs & r3, 1, vc, o);
				if( (hearts & r3) != 0 )
					topN(hearts & r3, 1, vh, o);
				if( (diamonds & r3) != 0 )
					topN(diamonds & r3, 1, vd, o);
				topN(2, r1 & spades, r1 & clubs, r1 & hearts, r1 & diamonds,
						vs, vc, vh, vd,
						o);
				return;
			}
		}
	}
	if( r2 != 0 ) {
		switch( bitCount(r2) ) {
			case 3: {	//	ペア*3
				uint r = r2 & (r2 - 1);		//	一番右のビットを落とす
				r2 ^= r;						//	一番右のビット
				r2 |= r1;
				topN(4, r & spades, r & clubs, r & hearts, r & diamonds,
						vs, vc, vh, vd,
						o);
				topN(1, r2 & spades, r2 & clubs, r2 & hearts, r2 & diamonds,
						vs, vc, vh, vd,
						o);
				return;
			}
			case 2:	{ //	ペア*2
				uint r = r2 & (r2 - 1);		//	一番右のビットを落とす
				r2 ^= r;
				topN(2, r & spades, r & clubs, r & hearts, r & diamonds,
						vs, vc, vh, vd,
						o);
				topN(2, r2 & spades, r2 & clubs, r2 & hearts, r2 & diamonds,
						vs, vc, vh, vd,
						o);
				topN(1, r1 & spades, r1 & clubs, r1 & hearts, r1 & diamonds,
						vs, vc, vh, vd,
						o);
				return;
			}
			case 1:
				topN(2, r2 & spades, r2 & clubs, r2 & hearts, r2 & diamonds,
						vs, vc, vh, vd,
						o);
				topN(3, r1 & spades, r1 & clubs, r1 & hearts, r1 & diamonds,
						vs, vc, vh, vd,
						o);
				return;
		}
	}
	topN(5, r1 & spades, r1 & clubs, r1 & hearts, r1 & diamonds,
			vs, vc, vh, vd,
			o);
	return;
}
//----------------------------------------------------------------------
//	ランダムハンドの相手一人に対する勝率（勝ち or 引き分け）を求める
double calcWinSplitProb(Card c1, Card c2, const std::vector<Card> &comu)
{
	assert( comu.size() <= 5 );
	const int N_LOOP = 10000;
	std::vector<Card> v1(7), v2(7);
	v1[0] = c1;
	v1[1] = c2;
	for (int i = 0; i < (int)comu.size(); ++i)
		v1[i+2] = v2[i+2] = comu[i];
	Deck deck;
	//	take は O(N) の時間がかかるので、ループの前に処理を行っておく
	deck.take(c1);
	deck.take(c2);
	for (int k = 0; k < (int)comu.size(); ++k)
		deck.take(comu[k]);
	int nWinSplit = 0;
	for (int i = 0; i < N_LOOP; ++i) {
		//if( i == 222 ) {
		//	std::cout << 222;
		//}
		deck.setNDealt(2+comu.size());		//	ディール済み枚数
		deck.shuffle();
		v2[0] = deck.deal();
		v2[1] = deck.deal();
		for (int k = (int)comu.size(); k < 5; ++k) {
			v1[k+2] = v2[k+2] = deck.deal();
		}
		uint od1 = 0, od2 = 0;
		checkHand(v1, od1);
		checkHand(v2, od2);
		//print(v1, od1);
		//print(v2, od2);
		if( od1 >= od2 )
			++nWinSplit;
	}
	return (double)nWinSplit / N_LOOP;
}
//	ランダムハンドの相手 np - 1人に対する勝率（勝ち or 引き分け）を求める
double calcWinSplitProb(Card c1, Card c2, const std::vector<Card> &comu, int np)
{
	assert( comu.size() <= 5 );
	const int N_LOOP = 1000;
	//const int N_LOOP = 10000;
	//std::vector<std::vector<Card>> vv;
	g_vv.clear();
	for (int i = 0; i < np; ++i) {
		g_vv.push_back(std::vector<Card>(7));
	}
	g_vv[0][0] = c1;
	g_vv[0][1] = c2;
	for (int k = 0; k < np; ++k) {
		for (int i = 0; i != (int)comu.size(); ++i) {
			g_vv[k][i+2] = comu[i];
		}
	}
	Deck deck;
	//	take は O(N) の時間がかかるので、ループの前に処理を行っておく
	deck.take(c1);
	deck.take(c2);
	for (int k = 0; k != (int)comu.size(); ++k)
		deck.take(comu[k]);
	int nWinSplit = 0;
	const int NL = N_LOOP * 2 / np;
	//const int NL = 10;
	for (int i = 0; i < NL; ++i) {
		deck.setNDealt(2+comu.size());		//	ディール済み枚数
		deck.shuffle();		//	undone: shuffle() は多分時間がかかるので、単にランダムに2枚取り出すようにする？
		for (int j = (int)comu.size(); j < 5; ++j) {
			g_vv[0][j+2] = deck.deal();
		}
		uint odr0 = 0, odr = 0;
		int h = checkHand(g_vv[0], odr0);
		//std::cout << "\n";
		//print(vv[0], odr0, handName[h]);
		//std::vector<uint> odr;
		for (int k = 1; k < np; ++k) {
			g_vv[k][0] = deck.deal();
			g_vv[k][1] = deck.deal();
			for (int j = (int)comu.size(); j < 5; ++j) {
				g_vv[k][j+2] = g_vv[0][j+2];
			}
			h = checkHand(g_vv[k], odr);
			//print(g_vv[k], odr, handName[h]);
			if( odr > odr0 )
				break;
		}
		if( odr0 >= odr ) {
			++nWinSplit;
			//std::cout << "win\n";
		}
	}
	return (double)nWinSplit / NL;
}
//	River 専用、ランダムハンドの相手 np - 1人に対する勝率（勝ち or 引き分け）を求める
double calcWinSplitProbRO(Card c1, Card c2, const std::vector<Card> &comu, int np)
{
	assert( comu.size() == 5 );
	const int N_LOOP = 1000;
	//const int N_LOOP = 10000;
	const int N_DEALT = 7;					//	配布カード数
	const int deck_size = 52 - N_DEALT;		//	未配布カード数
	//std::vector<std::vector<Card>> vv;
	g_v.clear();
	g_v.resize(N_DEALT);
	//for (int i = 0; i < np; ++i) {
	//	g_vv.push_back(std::vector<Card>(7));
	//}
	g_v[0] = c1;
	g_v[1] = c2;
	//for (int k = 0; k < np; ++k) {
		for (int i = 0; i < (int)comu.size(); ++i) {
			g_v[i+2] = comu[i];
		}
	//}
	Deck deck;
	//	take は O(N) の時間がかかるので、ループの前に処理を行っておく
	deck.take(c1);
	deck.take(c2);
	for (int k = 0; k != (int)comu.size(); ++k)
		deck.take(comu[k]);
	uint odr0 = 0, odr = 0;
	int h = checkHand(g_v, odr0);
	//for (int i = 0; i != g_v.size(); ++i) {
	//	g_v[i].print();
	//	cout << " ";
	//}
	//cout << "hand = " << handName[h] << " odr0 = " << odr0 << "\n";
	int nWinSplit = 0;
	const int NL = N_LOOP * 2 / np;
	//const int NL = 10;
	for (int i = 0; i < NL; ++i) {
		//deck.setNDealt(2+comu.size());		//	ディール済み枚数
		//deck.shuffle();		//	undone: shuffle() は多分時間がかかるので、単にランダムに2枚取り出すようにする？
		//for (int j = (int)comu.size(); j < 5; ++j) {
		//	g_vv[0][j+2] = deck.deal();
		//}
		//std::cout << "\n";
		//print(g_vv[0], odr0, handName[h]);
		//std::vector<uint> odr;
		for (int k = 1; k < np; ++k) {
			int rix = g_mt() % deck_size;
			int rix2 = g_mt() % deck_size;
			while( rix == rix2 )
				rix2 = g_mt() % deck_size;
			g_v[0] = deck[rix + N_DEALT];
			g_v[1] = deck[rix2 + N_DEALT];
			//for (int j = 0; j != 5; ++j) {
			//	g_v[j+2] = comu[j];
			//}
			h = checkHand(g_v, odr);
			//for (int i = 0; i != g_v.size(); ++i) {
			//	g_v[i].print();
			//	cout << " ";
			//}
			//cout << "hand = " << handName[h] << " odr = " << odr << "\n";
			//print(g_vv[k], odr, handName[h]);
			if( odr > odr0 )
				break;
		}
		if( odr0 >= odr ) {
			++nWinSplit;
			//std::cout << "won\n";
		} else {
			//std::cout << "lose\n";
		}
	}
	return (double)nWinSplit / NL;
}
//	ランダムハンドの相手 np - 1人に対する勝率（引き分けの場合は、1/人数）を求める
double calcHandStrength(Card c1, Card c2, const std::vector<Card> &comu, int np)
{
	///assert(np>1);
	assert( comu.size() <= 5 );
	//const int N_LOOP = 1000;
	const int N_LOOP = 10000;
	//std::vector<std::vector<Card>> vv;
	g_vv.clear();
	for (int i = 0; i < np; ++i) {
		g_vv.push_back(std::vector<Card>(7));
	}
	g_vv[0][0] = c1;
	g_vv[0][1] = c2;
	for (int k = 0; k < np; ++k) {
		for (int i = 0; i < (int)comu.size(); ++i) {
			g_vv[k][i+2] = comu[i];
		}
	}
	Deck deck;
	//	take は O(N) の時間がかかるので、ループの前に処理を行っておく
	deck.take(c1);
	deck.take(c2);
	for (int k = 0; k < (int)comu.size(); ++k)
		deck.take(comu[k]);
	double hs = 0;		//	handStrength
	const int NL = N_LOOP * 2 / np;
	for (int i = 0; i < NL; ++i) {
		deck.setNDealt(2+comu.size());		//	ディール済み枚数
		deck.shuffle();
		for (int j = (int)comu.size(); j < 5; ++j)
			g_vv[0][j+2] = deck.deal();
		uint odr0 = 0, odr = 0;
		int h = checkHandBM(g_vv[0], odr0);
		int nSplit = 1;		//	（自分も含めた）引き分け人数
		for (int k = 1; k < np; ++k) {
			g_vv[k][0] = deck.deal();
			g_vv[k][1] = deck.deal();
			for (int j = (int)comu.size(); j < 5; ++j)
				g_vv[k][j+2] = g_vv[0][j+2];
			h = checkHandBM(g_vv[k], odr);
			if( odr > odr0 )
				break;
			if( odr == odr0 )		//	引き分けの場合
				++nSplit;
		}
		if( odr0 >= odr )
			hs += 1.0 / nSplit;
	}
	return hs / NL;
}
//	各ホールカードの期待勝率を求め、p に返す
void calcHandStrength(const std::vector<HoleCards> &hc,
									const std::vector<Card> &comu,
									std::vector<double> &p)
{
	const int nPlayer = hc.size();
	p.resize(nPlayer);
	std::vector<Card> v(/*size:*/7);
	for (int i = 0; i < (int)comu.size(); ++i) {
		v[i+2] = comu[i];
	}
	//	take() は重い処理なので、ループ外で行っておく
	Deck deck(/*shuffle:*/true);
	for (int k = 0; k < nPlayer; ++k) {
		deck.take(hc[k].first);
		deck.take(hc[k].second);
	}
	const int N_LOOP = 10000;
	std::vector<double> cnt(nPlayer, 0.0);;
	for (int i = 0; i < N_LOOP; ++i) {
		deck.setNDealt(nPlayer * 2);
		deck.shuffle();
		for (int i = comu.size(); i < 5; ++i) {
			v[i+2] = deck.deal();
		}
		std::vector<std::pair<uint, int>> hvv;
		for (int k = 0; k < nPlayer; ++k) {
			v[0] = hc[k].first;
			v[1] = hc[k].second;
			uint hv;
			checkHandBM(v, hv);
			hvv.push_back(std::pair<uint, int>(hv, k));
		}
		std::sort(hvv.begin(), hvv.end());
		//	undone: スプリットの場合
		std::pair<uint, int> e = hvv[nPlayer-1];
		//std::pair<uint, int> e = hvv.end();
		int k = 1;		//	最も強い人の人数
		while( k < nPlayer && hvv[nPlayer - 1 - k].first == e.first )
			++k;
		if( k == 1 )
			cnt[e.second] += 1;
		else {
			for (int j = 0; j < k; ++j)
				cnt[hvv[nPlayer - 1 - j].second] += 1.0/k;
		}
	}
	for (int i = 0; i < nPlayer; ++i) {
		p[i] = (double)cnt[i] / N_LOOP;
	}
}
//	現状のレイズ・コールを含めたチップ合計が pot の時に、call するかどうかの勝率閾値を計算
//	プレイヤー人数は合計２人とする
double calcThreshold(int pot, int call)
{
	return (double)call / (pot + call);
}
