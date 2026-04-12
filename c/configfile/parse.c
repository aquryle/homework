/**
 * @file	parse.c
 * @author	your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date	2026-04-11
 *
 * @copyright Copyright (c) 2026
 *
 *
 * CRはスペースやタブと同じように無効文字として扱う。
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_CHANNELS 2
#define MAX_SETTINGS_PER_CH 25

// ユーザ設定構造体
typedef struct {
	uint32_t id;		// CAN ID
	uint8_t xFlag;		// 0: standard id, 1: extended id
	uint8_t dlc;		// DLC
	uint8_t data[8];	// data
	uint32_t interval;	// transmit interval(ms)

	uint64_t nextTick;
} user_config_t;

// CANチャネル構造体
typedef struct {
	int channelNum;
	int settingCount;
	user_config_t settings[MAX_SETTINGS_PER_CH];
} ChannelConfig_t;


ChannelConfig_t g_channels[MAX_CHANNELS];
int g_channelCount = 0;


/// @brief スペースの分だけポインタを進める
/// @param p
static void skip_spaces(const char **p)
{
	while (**p == ' ' || **p == '\t' || **p == '\r') {
		(*p)++;
	}
}

/// @brief カンマの分だけポインタを進める
/// @param p
static void skip_comma(const char **p)
{
	skip_spaces(p);		// カンマまでのスペースをスキップ
	if (**p == ',') {
		(*p)++;
	}
}

/// @brief 配列から設定値にパースする
/// @param buffer 配列
void parseConfigFromMemory(const char *buffer) {
	const char *p = buffer;				// 読み出し先
	ChannelConfig_t *currentCh = NULL;	// 今読んでるチャネル
	const char *lineStart;		// 行の開始位置
	const char *lineEnd;		// 行の終了位置
	const char *commentStart;	// コメントの開始位置
	const char *validEnd;		//
	const char *cursor;			// 今の読み出しポイント
	int chNum;		// チャネル番号
	user_config_t *setting;		// 設定構造体
	char *next;		//
	int i;

	// ヌル文字まで読み出しを続ける
	while (*p != '\0') {
		// 行初期化
		lineStart = p;
		lineEnd = p;
		commentStart = NULL;

		// コメント処理
		while (*p != '\n' && *p != '\0') {
			if (*p == '#' && commentStart == NULL) {
				commentStart = p;
			}
			p++;
		}
		lineEnd = p;
		// lineEndの直前が \r なら、有効なデータの終端を一つ前にずらす
		if (lineEnd > lineStart && *(lineEnd - 1) == '\r') {
			if (commentStart == NULL || commentStart == lineEnd) {
				validEnd = lineEnd - 1;
			}
		}
		if (*p == '\n') p++;

		validEnd = (commentStart != NULL) ? commentStart : lineEnd;
		cursor = lineStart;
		skip_spaces(&cursor);

		if (cursor >= validEnd) continue;

		if (*cursor == '[') {
			cursor++;
			if (cursor[0] == 'c' && cursor[1] == 'h') {
				cursor += 2;
				chNum = strtol(cursor, NULL, 10);

				if (g_channelCount < MAX_CHANNELS) {
					currentCh = &g_channels[g_channelCount++];
					currentCh->channelNum = chNum;
					currentCh->settingCount = 0;
				} else {
					currentCh = NULL;
				}
			}
			continue;
		}

		if (currentCh == NULL || currentCh->settingCount >= MAX_SETTINGS_PER_CH) {
			continue;
		}

		setting = &currentCh->settings[currentCh->settingCount];


		setting->id = (uint32_t)strtoul(cursor, &next, 16);
		cursor = next; skip_comma(&cursor);

		setting->xFlag = (uint8_t)strtoul(cursor, &next, 10);
		cursor = next; skip_comma(&cursor);

		setting->dlc = (uint8_t)strtoul(cursor, &next, 10);
		cursor = next; skip_comma(&cursor);

		for (i = 0; i < 8; i++) {
			if (i < setting->dlc) {
				setting->data[i] = (uint8_t)strtoul(cursor, &next, 16);
				cursor = next;
			} else {
				setting->data[i] = 0;
			}
		}
		skip_comma(&cursor);

		setting->interval = (uint32_t)strtoul(cursor, &next, 10);

		currentCh->settingCount++;
	}
}

void printChannelConfigs(void) {

	for (int i = 0; i < g_channelCount; i++) {
		ChannelConfig_t *ch = &g_channels[i];
		printf("[ch%d] (Total Settings: %d)\n", ch->channelNum, ch->settingCount);

		for (int j = 0; j < ch->settingCount; j++) {
			user_config_t *set = &ch->settings[j];

			// IDは8桁のHex、各種フラグ、ペイロードデータを整形して表示
			printf("  - ID: %08X | xFlag: %d | DLC: %d | Data: ",
				set->id, set->xFlag, set->dlc);

			for (int k = 0; k < 8; k++) {
				printf("%02X ", set->data[k]);
			}

			printf("| Interval: %d ms\n", set->interval);
		}
		printf("--------------------------------------------------------------\n");
	}
}

int main(void) {
	const char *filename = "config.txt";

	// ファイルを開く
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Error: '%s' を開けませんでした。\n", filename);
		printf("実行ファイルと同じディレクトリにファイルが存在するか確認してください。\n");
		return 1;
	}

	// ファイルのサイズを取得する
	fseek(fp, 0, SEEK_END);
	long fileSize = ftell(fp);
	rewind(fp); // ファイルポインタを先頭に戻す

	// ファイル内容を格納するバッファを動的確保 (終端文字 '\0' のために+1)
	char *buffer = (char *)malloc(fileSize + 1);
	if (buffer == NULL) {
		printf("Error: メモリの確保に失敗しました。\n");
		fclose(fp);
		return 1;
	}

	// ファイルの内容をバッファに一括読み込み
	size_t readSize = fread(buffer, 1, fileSize, fp);
	buffer[readSize] = '\0'; // 文字列の終端をセット

	// ファイルはもう不要なので閉じる
	fclose(fp);


	// CM0向けに作ったパース処理を実行 (メモリ上のバッファを渡す)
	parseConfigFromMemory(buffer);

	// 結果の表示
	printChannelConfigs();

	// テスト用バッファの解放
	free(buffer);

	return 0;
}
