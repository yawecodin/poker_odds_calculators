#include <iostream>
#include <stdio.h>
#include <string.h>
using namespace std;

#define MAIN_MODULE
#include "poker_hand.h"

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: best_turn_hand (-unevaluate) filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char parse_error[] = "couldn't parse line %d, card %d: %d\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bUnEvaluate;
  int m;
  int n;
  int retval;
  FILE *fptr;
  int line_no;
  int line_len;
  int cards[NUM_CARDS_AT_TURN];
  HoldemTurnHand board_turn_hand;
  PokerHand best_turn_hand;

  if ((argc < 2) || (argc > 3)) {
    cout << usage << endl;
    return 1;
  }

  bUnEvaluate = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-unevaluate"))
      bUnEvaluate = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    cout << usage << endl;
    return 2;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    m = 0;

    // skip whitespace

    for ( ; m < line_len; m++) {
      if (line[m] != ' ')
        break;
    }

    if (m == line_len) {
      printf(parse_error,line_no,-1,3);
      return 4;
    }

    for (n = 0; n < NUM_CARDS_AT_TURN; n++) {
      retval = card_value_from_card_string(&line[m],&cards[n]);

      if (retval) {
        printf(parse_error,line_no,n,5);
        return 5;
      }

      m += 2;

      if (n < NUM_CARDS_AT_TURN - 1) {
        // skip whitespace

        for ( ; m < line_len; m++) {
          if (line[m] != ' ')
            break;
        }

        if (m == line_len) {
          printf(parse_error,line_no,n,6);
          return 6;
        }
      }
    }

    board_turn_hand.NewCards(cards[0],cards[1],cards[2],
      cards[3],cards[4],cards[5]);

    cout << board_turn_hand << endl;

    best_turn_hand = board_turn_hand.BestPokerHand();

    if (bUnEvaluate)
      best_turn_hand.UnEvaluate();

    cout << "best hand: " << endl << endl;
    cout << best_turn_hand << endl;
  }

  fclose(fptr);

  return 0;
}

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen)
{
  int chara;
  int local_line_len;

  local_line_len = 0;

  for ( ; ; ) {
    chara = fgetc(fptr);

    if (feof(fptr))
      break;

    if (chara == '\n')
      break;

    if (local_line_len < maxllen - 1)
      line[local_line_len++] = (char)chara;
  }

  line[local_line_len] = 0;
  *line_len = local_line_len;
}
