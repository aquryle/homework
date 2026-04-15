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
#include <string.h>

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
} CanSetting_t;

// CANチャネル構造体
typedef struct {
	int channelNum;
	int settingCount;
	uint32_t baudRate;
	uint8_t samplePoint;
	CanSetting_t settings[MAX_SETTINGS_PER_CH];
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

void parseConfigFromMemory(const char *buffer) {
	const char *p = buffer;
	ChannelConfig_t *currentCh = NULL;
	int lineCount = 0; // エラー表示用に位置を記録
	CanSetting_t *set;
	char *next;
	int i;
	const char *lineStart;
	const char *commentStart;
	const char *lineEnd = p;
	const char *validEnd;
	const char *cursor;
	int chNum;
	const char *equalPos;
	char errorBuf[32];
	size_t len;


	while (*p != '\0') {
		lineCount++;
		lineStart = p;
		commentStart = NULL;

		while (*p != '\n' && *p != '\0') {
			if (*p == '#' && commentStart == NULL) commentStart = p;
			p++;
		}
		lineEnd = p;
		if (*p == '\n') p++;

		validEnd = (commentStart != NULL) ? commentStart : lineEnd;
		cursor = lineStart;
		skip_spaces(&cursor);

		// 空行またはコメント行
		if (cursor >= validEnd) continue;

		// --- 1. セクションヘッダ [chX] ---
		if (*cursor == '[') {
			cursor++;
			if (strncmp(cursor, "ch", 2) == 0) {
				cursor += 2;
				chNum = (int)strtol(cursor, (char**)&cursor, 10);
				if (*cursor == ']') {
					if (g_channelCount < MAX_CHANNELS) {
						currentCh = &g_channels[g_channelCount++];
						currentCh->channelNum = chNum;
						currentCh->settingCount = 0;
						currentCh->baudRate = 0;   // 初期化
						currentCh->samplePoint = 0;
					}
					continue;
				}
			}
			printf("Error: Line %d - Invalid section format\n", lineCount);
			continue;
		}

		if (currentCh == NULL) continue;

		// --- 2. 厳密なキーワード比較 (BaudRate / SamplePoint) ---

		// 行の中に '=' があるか確認（あれば「設定行」とみなしてチェック）
		equalPos = strchr(cursor, '=');
		if (equalPos != NULL && equalPos < validEnd) {
			if (strncmp(cursor, "BaudRate", 8) == 0) {
				cursor = equalPos + 1; // '=' の直後へ
				currentCh->baudRate = (uint32_t)strtoul(cursor, NULL, 10);
			}
			else if (strncmp(cursor, "SamplePoint", 11) == 0) {
				cursor = equalPos + 1;
				currentCh->samplePoint = (uint8_t)strtoul(cursor, NULL, 10);
			}
			else {
				// タイポまたは未知のキーワード
				len = equalPos - cursor;
				if (len > 31) len = 31;
				strncpy(errorBuf, cursor, len);
				errorBuf[len] = '\0';
				printf("Error: Line %d - Unknown keyword '%s'\n", lineCount, errorBuf);
			}
			continue;
		}

		// --- 3. データ行のパース (数字で始まる場合) ---
		if ((*cursor >= '0' && *cursor <= '9') || (*cursor >= 'a' && *cursor <= 'f') || (*cursor >= 'A' && *cursor <= 'F')) {
			if (currentCh->settingCount >= MAX_SETTINGS_PER_CH) {
				printf("Error: Line %d - Too many settings in [ch%d]\n", lineCount, currentCh->channelNum);
				continue;
			}



			set = &currentCh->settings[currentCh->settingCount];
			set->id = (uint32_t)strtoul(cursor, &next, 16);
			cursor = next; skip_comma(&cursor);
			set->xFlag = (uint8_t)strtoul(cursor, &next, 10);
			cursor = next; skip_comma(&cursor);
			set->dlc = (uint8_t)strtoul(cursor, &next, 10);
			cursor = next; skip_comma(&cursor);

			for (i = 0; i < 8; i++) {
				set->data[i] = (uint8_t)strtoul(cursor, &next, 16);
				cursor = next;
			}
			skip_comma(&cursor);
			set->interval = (uint32_t)strtoul(cursor, &next, 10);

			currentCh->settingCount++;
		}
	}
}

void printChannelConfigs(void) {
	printf("\n==============================================================\n");
	printf(" Parsed CAN Configuration\n");
	printf("==============================================================\n");

	if (g_channelCount == 0) {
		printf("No data found.\n");
		return;
	}

	for (int i = 0; i < g_channelCount; i++) {
		ChannelConfig_t *ch = &g_channels[i];
		printf("[ch%d] BaudRate: %u bps | SamplePoint: %d%%\n",
			ch->channelNum, ch->baudRate, ch->samplePoint);
		printf("--------------------------------------------------------------\n");

		for (int j = 0; j < ch->settingCount; j++) {
			CanSetting_t *set = &ch->settings[j];
			printf("  ID:%08X  x:%d  DLC:%d  Data:", set->id, set->xFlag, set->dlc);
			for (int k = 0; k < 8; k++) printf(" %02X", set->data[k]);
			printf("  Int:%dms\n", set->interval);
		}
		printf("\n");
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
