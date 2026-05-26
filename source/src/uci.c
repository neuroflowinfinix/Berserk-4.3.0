#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emscripten.h>
#include "board.h"
#include "eval.h"
#include "move.h"
#include "movegen.h"
#include "movepick.h"
#include "noobprobe/noobprobe.h"
#include "perft.h"
#include "pyrrhic/tbprobe.h"
#include "search.h"
#include "thread.h"
#include "transposition.h"
#include "uci.h"
#include "util.h"

#define NAME "Berserk WASM"
#define VERSION "4.3.0"
#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define MOVE_BUFFER 50

extern int failedQueries;
Board board;
ThreadData* threads;
SearchParams searchParameters = {.quit = 0};
int uci_init_done = 0;

void ParseGo(char* in, SearchParams* params, Board* board, ThreadData* threads) {
    in += 3; params->depth = MAX_SEARCH_PLY; params->startTime = GetTimeMS();
    params->timeset = 0; params->stopped = 0; params->quit = 0;
    char* ptrChar = in;
    int perft = 0, movesToGo = 30, moveTime = -1, time = -1, inc = 0, depth = -1;
    if ((ptrChar = strstr(in, "perft")))                        perft      = atoi(ptrChar + 6);
    if ((ptrChar = strstr(in, "binc"))  && board->side == BLACK) inc       = atoi(ptrChar + 5);
    if ((ptrChar = strstr(in, "winc"))  && board->side == WHITE) inc       = atoi(ptrChar + 5);
    if ((ptrChar = strstr(in, "wtime")) && board->side == WHITE) time      = atoi(ptrChar + 6);
    if ((ptrChar = strstr(in, "btime")) && board->side == BLACK) time      = atoi(ptrChar + 6);
    if ((ptrChar = strstr(in, "movestogo")))                     movesToGo = atoi(ptrChar + 10);
    if ((ptrChar = strstr(in, "movetime")))                      moveTime  = atoi(ptrChar + 9);
    if ((ptrChar = strstr(in, "depth")))  depth = min(MAX_SEARCH_PLY - 1, atoi(ptrChar + 6));
    if (perft) { PerftTest(perft, board); return; }
    params->depth = depth;
    if (moveTime != -1) {
        params->timeset    = 1;
        params->timeToSpend = moveTime - MOVE_BUFFER;
        params->endTime    = params->startTime + moveTime - MOVE_BUFFER;
        params->maxTime    = params->startTime + moveTime - MOVE_BUFFER;
    } else if (time != -1) {
        params->timeset    = 1;
        params->maxTime    = params->startTime + (time + inc) / 2 - MOVE_BUFFER;
        int timeToSpend    = time / movesToGo + inc - MOVE_BUFFER;
        params->timeToSpend = timeToSpend;
        params->endTime    = min(params->maxTime, params->startTime + timeToSpend);
    } else { params->timeset = 0; }
    if (depth <= 0) params->depth = MAX_SEARCH_PLY - 1;
    SearchArgs* args = malloc(sizeof(SearchArgs));
    args->board = board; args->params = params; args->threads = threads;
    UCISearch(args);
    free(args);
}

void ParsePosition(char* in, Board* board) {
    in += 9; char* ptrChar = in;
    if (strncmp(in, "startpos", 8) == 0) { ParseFen(START_FEN, board); }
    else {
        ptrChar = strstr(in, "fen");
        if (!ptrChar) ParseFen(START_FEN, board);
        else { ptrChar += 4; ParseFen(ptrChar, board); }
    }
    ptrChar = strstr(in, "moves");
    if (!ptrChar) return;
    ptrChar += 6;
    Move enteredMove;
    while (*ptrChar) {
        enteredMove = ParseMove(ptrChar, board);
        if (!enteredMove) break;
        MakeMove(enteredMove, board);
        while (*ptrChar && *ptrChar != ' ') ptrChar++;
        ptrChar++;
    }
}

void PrintUCIOptions() {
    printf("id name %s %s\n", NAME, VERSION);
    printf("id author Jay Honnold\n");
    printf("uciok\n");
}

EMSCRIPTEN_KEEPALIVE
void ExecCommand(char* in) {
    if (!uci_init_done) {
        ParseFen(START_FEN, &board);
        threads = CreatePool(1);
        uci_init_done = 1;
    }
    if (in[0] == '\n' || in[0] == '\0') return;
    if      (!strncmp(in, "isready",    7)) { printf("readyok\n"); }
    else if (!strncmp(in, "position",   8)) { ParsePosition(in, &board); }
    else if (!strncmp(in, "ucinewgame", 10)) {
        ParsePosition("position startpos\n", &board);
        TTClear(); ResetThreadPool(&board, &searchParameters, threads); failedQueries = 0;
    }
    else if (!strncmp(in, "go",   2)) { ParseGo(in, &searchParameters, &board, threads); }
    else if (!strncmp(in, "stop", 4)) { searchParameters.stopped = 1; }
    else if (!strncmp(in, "uci",  3)) { PrintUCIOptions(); }
    else if (!strncmp(in, "eval", 4)) {
        Score s = Evaluate(&board, &threads[0]); printf("Score: %dcp\n", s);
    }
}
void UCILoop() {}
