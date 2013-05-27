#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#define _MAX_PATH 4096
#include <unistd.h>
#endif
using namespace std;

#define MAIN_MODULE
#include "poker_hand.h"

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: fdelta3 (-terse) (-verbose) (-debug) (-handhand)\n"
"  (-skip_folded) (-abbrev) (-skip_zero) (-show_board)\n"
"  (-show_hand) (-show_hand_type) (-saw_flop) (-saw_river) (-only_folded)\n"
"  (-spent_money_on_the_river) (-stealth_two_pair) player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char in_chips[] = " in chips";
#define IN_CHIPS_LEN (sizeof (in_chips) - 1)
static char summary[] = "*** SUMMARY ***";
#define SUMMARY_LEN (sizeof (summary) - 1)
static char flop[] = "*** FLOP *** [";
#define FLOP_LEN (sizeof (flop) - 1)
static char river[] = "*** RIVER *** [";
#define RIVER_LEN (sizeof (river) - 1)
static char street_marker[] = "*** ";
#define STREET_MARKER_LEN (sizeof (street_marker) - 1)
static char posts[] = " posts ";
#define POSTS_LEN (sizeof (posts) - 1)
static char dealt_to[] = "Dealt to ";
#define DEALT_TO_LEN (sizeof (dealt_to) - 1)
static char folds[] = " folds ";
#define FOLDS_LEN (sizeof (folds) - 1)
static char bets[] = " bets ";
#define BETS_LEN (sizeof (bets) - 1)
static char calls[] = " calls ";
#define CALLS_LEN (sizeof (calls) - 1)
static char raises[] = " raises ";
#define RAISES_LEN (sizeof (raises) - 1)
static char uncalled_bet[] = "Uncalled bet (";
#define UNCALLED_BET_LEN (sizeof (uncalled_bet) - 1)
static char collected[] = " collected ";
#define COLLECTED_LEN (sizeof (collected) - 1)

#define NUM_CARDS_IN_SUIT 13

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);
int get_work_amount(char *line,int line_len);

int main(int argc,char **argv)
{
  int m;
  int n;
  int p;
  int q;
  int curr_arg;
  bool bTerse;
  bool bVerbose;
  bool bDebug;
  int player_name_ix;
  int player_name_len;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_len;
  int line_no;
  int dbg_line_no;
  int ix;
  int street;
  int num_street_markers;
  int starting_balance;
  int spent_this_street;
  int spent_this_hand;
  int end_ix;
  int uncalled_bet_amount;
  int collected_from_pot;
  int collected_from_pot_count;
  int ending_balance;
  int delta;
  int file_no;
  int dbg_file_no;
  int num_hands;
  int dbg;
  int work;
  double dwork1;
  double dwork2;
  char hole_cards[6];
  char board_cards[15];
  bool bHandSpecified;
  bool bSkipFolded;
  bool bAbbrev;
  bool bSkipZero;
  bool bShowBoard;
  bool bShowHand;
  bool bShowHandType;
  bool bSawFlop;
  bool bSawRiver;
  bool bOnlyFolded;
  bool bSpentMoneyOnTheRiver;
  bool bStealthTwoPair;
  bool bSuited;
  bool bHaveFlop;
  bool bHaveRiver;
  bool bHaveStealthTwoPair;
  bool bSpentRiverMoney;
  char *hand;
  bool bSkipping;
  int rank_ix1;
  int rank_ix2;
  int cards[NUM_CARDS_IN_HAND];
  int holdem_cards[NUM_CARDS_IN_HOLDEM_POOL];
  HoldemPokerHand holdem_hand;
  PokerHand poker_hand;
  char card_string[3];
  int retval;

  if ((argc < 3) || (argc > 18)) {
    printf(usage);
    return 1;
  }

  bTerse = false;
  bVerbose = false;
  bDebug = false;
  bHandSpecified = false;
  bSkipFolded = false;
  bAbbrev = false;
  bSkipZero = false;
  bShowBoard = false;
  bShowHand = false;
  bShowHandType = false;
  bSawFlop = false;
  bSawRiver = false;
  bOnlyFolded = false;
  bSpentMoneyOnTheRiver = false;
  bStealthTwoPair = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strncmp(argv[curr_arg],"-hand",5)) {
      hand = &argv[curr_arg][5];
      bHandSpecified = true;

      if ((strlen(&argv[curr_arg][5]) == 3) && (argv[curr_arg][7] == 's'))
        bSuited = true;
      else
        bSuited = false;
    }
    else if (!strcmp(argv[curr_arg],"-skip_folded"))
      bSkipFolded = true;
    else if (!strcmp(argv[curr_arg],"-abbrev"))
      bAbbrev = true;
    else if (!strcmp(argv[curr_arg],"-skip_zero"))
      bSkipZero = true;
    else if (!strcmp(argv[curr_arg],"-show_board"))
      bShowBoard = true;
    else if (!strcmp(argv[curr_arg],"-show_hand"))
      bShowHand = true;
    else if (!strcmp(argv[curr_arg],"-show_hand_type"))
      bShowHandType = true;
    else if (!strcmp(argv[curr_arg],"-saw_flop"))
      bSawFlop = true;
    else if (!strcmp(argv[curr_arg],"-saw_river"))
      bSawRiver = true;
    else if (!strcmp(argv[curr_arg],"-only_folded"))
      bOnlyFolded = true;
    else if (!strcmp(argv[curr_arg],"-spent_money_on_the_river"))
      bSpentMoneyOnTheRiver = true;
    else if (!strcmp(argv[curr_arg],"-stealth_two_pair"))
      bStealthTwoPair = true;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  if (bTerse && bVerbose) {
    printf("can't specify both -terse and -verbose\n");
    return 3;
  }

  if (bSawFlop && bSawRiver) {
    printf("can't specify both -saw_flop and -saw_river\n");
    return 4;
  }

  if (bAbbrev && bShowHand) {
    printf("can't specify both -abbrev and -show_hand\n");
    return 5;
  }

  if (bAbbrev && bShowHandType) {
    printf("can't specify both -abbrev and -show_hand_type\n");
    return 6;
  }

  if (bAbbrev && bStealthTwoPair) {
    printf("can't specify both -abbrev and -stealth_two_pair\n");
    return 7;
  }

  if (bSkipFolded && bOnlyFolded) {
    printf("can't specify both -skip_folded and -only_folded\n");
    return 8;
  }

  if (!bSawRiver && bSpentMoneyOnTheRiver) {
    printf("can't specify -spent_money_on_the_river if didn't specify -saw_river\n");
    return 9;
  }

  if (bStealthTwoPair && bHandSpecified) {
    printf("can't specify both -stealth_two_pair and -hand\n");
    return 10;
  }

  if (bStealthTwoPair && !bSawFlop && !bSawRiver) {
    printf("can't specify -stealth_two_pair if didn't specify -saw_flop or -saw_river\n");
    return 11;
  }

  player_name_ix = curr_arg++;
  player_name_len = strlen(argv[player_name_ix]);

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 12;
  }

  ending_balance = -1;

  file_no = 0;
  dbg_file_no = -1;

  if (!bAbbrev)
    hole_cards[5] = 0;
  else
    hole_cards[3] = 0;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    file_no++;

    if (dbg_file_no == file_no)
      dbg = 1;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    line_no = 0;
    bSkipping = false;
    num_hands = 0;
    bHaveFlop = false;
    bHaveRiver = false;
    bSpentRiverMoney = false;
    bHaveStealthTwoPair = false;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (line_no == dbg_line_no)
        dbg = 1;

      if (bDebug)
        printf("line %d %s\n",line_no,line);

      if (Contains(true,
        line,line_len,
        argv[player_name_ix],player_name_len,
        &ix)) {

        if (Contains(true,
          line,line_len,
          in_chips,IN_CHIPS_LEN,
          &ix)) {

          num_hands++;
          bSkipping = false;
          bHaveFlop = false;
          bHaveRiver = false;
          bSpentRiverMoney = false;
          bHaveStealthTwoPair = false;

          street = 0;
          num_street_markers = 0;
          spent_this_street = 0;
          spent_this_hand = 0;
          uncalled_bet_amount = 0;
          collected_from_pot = 0;
          collected_from_pot_count = 0;

          line[ix] = 0;

          for (ix--; (ix >= 0) && (line[ix] != '('); ix--)
            ;

          sscanf(&line[ix+1],"%d",&starting_balance);

          if (bDebug)
            printf("line %d starting_balance = %d\n",line_no,starting_balance);

          continue;
        }
        else if (bSkipping)
          continue;
        else if (Contains(true,
          line,line_len,
          posts,POSTS_LEN,
          &ix)) {
          work = get_work_amount(line,line_len);
          spent_this_street += work;

          if (bDebug) {
            printf("line %d street %d POSTS work = %d, spent_this_street = %d\n",
              line_no,street,work,spent_this_street);
          }

          continue;
        }
        else if (!strncmp(line,dealt_to,DEALT_TO_LEN)) {
          for (n = 0; n < line_len; n++) {
            if (line[n] == '[')
              break;
          }

          if (n < line_len) {
            n++;

            for (m = n; m < line_len; m++) {
              if (line[m] == ']')
                break;
            }

            if (m < line_len) {
              if (!bAbbrev) {
                for (p = 0; p < 5; p++)
                  hole_cards[p] = line[n+p];

                for (q = 0; q < NUM_HOLE_CARDS_IN_HOLDEM_HAND; q++) {
                  card_string[0] = hole_cards[q*3 + 0];
                  card_string[1] = hole_cards[q*3 + 1];

                  retval = card_value_from_card_string(
                    card_string,&holdem_cards[q]);

                  if (retval) {
                    printf("invalid card string %s on line %d\n",
                      card_string,line_no);
                    return 13;
                  }

                  cards[q] = holdem_cards[q];
                }
              }
              else {
                for (rank_ix1 = 0; rank_ix1 < NUM_CARDS_IN_SUIT; rank_ix1++) {
                  if (line[n] == rank_chars[rank_ix1])
                    break;
                }

                if (rank_ix1 == NUM_CARDS_IN_SUIT)
                  rank_ix1 = 0;

                for (rank_ix2 = 0; rank_ix2 < NUM_CARDS_IN_SUIT; rank_ix2++) {
                  if (line[n+3] == rank_chars[rank_ix2])
                    break;
                }

                if (rank_ix2 == NUM_CARDS_IN_SUIT)
                  rank_ix2 = 0;

                if (rank_ix1 >= rank_ix2) {
                  hole_cards[0] = line[n];
                  hole_cards[1] = line[n+3];
                }
                else {
                  hole_cards[0] = line[n+3];
                  hole_cards[1] = line[n];
                }

                if (hole_cards[0] == hole_cards[1])
                  hole_cards[2] = ' ';
                else {
                  if (line[n+1] == line[n+4])
                    hole_cards[2] = 's';
                  else
                    hole_cards[2] = 'o';
                }
              }
            }

            if (bHandSpecified) {
              if (bSuited) {
                if (hole_cards[1] != hole_cards[4])
                  bSkipping = true;
              }
              else {
                if (hole_cards[1] == hole_cards[4])
                  bSkipping = true;
              }

              if (((hole_cards[0] != hand[0]) || (hole_cards[3] != hand[1])) &&
                  ((hole_cards[0] != hand[1]) || (hole_cards[3] != hand[0])))
                bSkipping = true;
            }
          }
        }
        else if (Contains(true,
          line,line_len,
          collected,COLLECTED_LEN,
          &ix)) {

          for (end_ix = ix + COLLECTED_LEN; end_ix < line_len; end_ix++) {
            if (line[end_ix] == ' ')
              break;
          }

          line[end_ix] = 0;
          sscanf(&line[ix + COLLECTED_LEN],"%d",&work);

          if (!collected_from_pot_count) {
            spent_this_hand += spent_this_street;
            street++;
            spent_this_street = 0;
          }

          collected_from_pot += work;
          collected_from_pot_count++;

          if (bDebug) {
            printf("line %d street %d COLLECTED work = %d, collected_from_pot = %d\n",
              line_no,street,work,collected_from_pot);
          }

          continue;
        }
        else if (!strncmp(line,uncalled_bet,UNCALLED_BET_LEN)) {
          sscanf(&line[UNCALLED_BET_LEN],"%d",&uncalled_bet_amount);
          spent_this_street -= uncalled_bet_amount;

          if (bDebug) {
            printf("line %d street %d UNCALLED uncalled_bet_amount = %d, spent_this_street = %d\n",
              line_no,street,uncalled_bet_amount,spent_this_street);
          }

          continue;
        }
        else if (Contains(true,
          line,line_len,
          folds,FOLDS_LEN,
          &ix)) {
          spent_this_hand += spent_this_street;

          if (bDebug) {
            printf("line %d street %d FOLDS spent_this_street = %d, spent_this_hand = %d\n",
              line_no,street,spent_this_street,spent_this_hand);
          }

          bSkipping = true;

          ending_balance = starting_balance - spent_this_hand + collected_from_pot;
          delta = ending_balance - starting_balance;

          if ((!bSkipZero || (delta != 0)) && (!bSkipFolded || bOnlyFolded)) {
            if (!bSawFlop || bHaveFlop) {
              if (!bStealthTwoPair || bHaveStealthTwoPair) {
                if (!bSawRiver || bHaveRiver) {
                  if (!bSpentMoneyOnTheRiver || bSpentRiverMoney) {
                    if (bTerse)
                      printf("%d\n",delta);
                    else {
                      printf("%s %10d",hole_cards,delta);

                      if (bShowBoard && bHaveRiver)
                        printf(" %s",board_cards);

                      if (bShowHand && bHaveFlop)
                        printf(" %s",poker_hand.GetHand());

                      if (bShowHandType && bHaveFlop)
                        printf(" %s",plain_hand_types[poker_hand.GetHandType()]);

                      if (bVerbose)
                        printf(" %s %3d\n",filename,num_hands);
                      else
                        putchar(0x0a);
                    }
                  }
                }
              }
            }
          }

          continue;
        }
        else if (Contains(true,
          line,line_len,
          bets,BETS_LEN,
          &ix)) {
          work = get_work_amount(line,line_len);
          spent_this_street += work;

          if (street == 3)
            bSpentRiverMoney = true;

          if (bDebug) {
            printf("line %d street %d BETS work = %d, spent_this_street = %d\n",
              line_no,street,work,spent_this_street);
          }
        }
        else if (Contains(true,
          line,line_len,
          calls,CALLS_LEN,
          &ix)) {
          work = get_work_amount(line,line_len);
          spent_this_street += work;

          if (street == 3)
            bSpentRiverMoney = true;

          if (bDebug) {
            printf("line %d street %d CALLS work = %d, spent_this_street = %d\n",
              line_no,street,work,spent_this_street);
          }
        }
        else if (Contains(true,
          line,line_len,
          raises,RAISES_LEN,
          &ix)) {
          work = get_work_amount(line,line_len);
          spent_this_street = work;

          if (street == 3)
            bSpentRiverMoney = true;

          if (bDebug) {
            printf("line %d street %d RAISES work = %d, spent_this_street = %d\n",
              line_no,street,work,spent_this_street);
          }
        }
      }
      else if (bSkipping)
        continue;
      else {
        if (!strncmp(line,flop,FLOP_LEN)) {
          if (bStealthTwoPair) {
            if (hole_cards[0] != hole_cards[3]) {
              if (((hole_cards[0] == line[FLOP_LEN]) ||
                   (hole_cards[0] == line[FLOP_LEN+3]) ||
                   (hole_cards[0] == line[FLOP_LEN+6])) &&
                  ((hole_cards[3] == line[FLOP_LEN]) ||
                   (hole_cards[3] == line[FLOP_LEN+3]) ||
                   (hole_cards[3] == line[FLOP_LEN+6]))) {
                bHaveStealthTwoPair = true;
              }
            }
          }

          for (n = 0; n < 8; n++)
            board_cards[n] = line[FLOP_LEN+n];

          board_cards[n] = ' ';

          for (q = 0; q < NUM_CARDS_IN_FLOP; q++) {
            card_string[0] = board_cards[q*3 + 0];
            card_string[1] = board_cards[q*3 + 1];

            retval = card_value_from_card_string(
              card_string,&cards[NUM_HOLE_CARDS_IN_HOLDEM_HAND+q]);

            if (retval) {
              printf("invalid card string %s on line %d\n",
                card_string,line_no);
              return 14;
            }
          }

          poker_hand.NewCards(cards[0],cards[1],cards[2],cards[3],cards[4]);
          poker_hand.Evaluate();

          bHaveFlop = true;
        }
        else if (!strncmp(line,river,RIVER_LEN)) {
          for (n = 9; n < 11; n++)
            board_cards[n] = line[RIVER_LEN+n];

          board_cards[n++] = ' ';

          for (m = 0; m < 2; m++,n++)
            board_cards[n] = line[RIVER_LEN+14+m];

          board_cards[n] = 0;

          for (q = 0; q < NUM_CARDS_IN_FLOP + NUM_CARDS_AFTER_FLOP; q++) {
            card_string[0] = board_cards[q*3 + 0];
            card_string[1] = board_cards[q*3 + 1];

            retval = card_value_from_card_string(
              card_string,&holdem_cards[NUM_HOLE_CARDS_IN_HOLDEM_HAND+q]);

            if (retval) {
              printf("invalid card string %s on line %d\n",
                card_string,line_no);
              return 15;
            }
          }

          holdem_hand.NewCards(holdem_cards[0],holdem_cards[1],holdem_cards[2],
            holdem_cards[3],holdem_cards[4],holdem_cards[5],holdem_cards[6]);

          poker_hand = holdem_hand.BestPokerHand();

          bHaveRiver = true;
        }
        else if (!strncmp(line,summary,SUMMARY_LEN)) {
          if (bDebug)
            printf("line %d SUMMARY line detected; skipping\n",line_no);

          bSkipping = true;

          ending_balance = starting_balance - spent_this_hand + collected_from_pot;
          delta = ending_balance - starting_balance;

          if ((!bSkipZero || (delta != 0)) && (!bOnlyFolded)) {
            if (!bSawFlop || bHaveFlop) {
              if (!bStealthTwoPair || bHaveStealthTwoPair) {
                if (!bSawRiver || bHaveRiver) {
                  if (!bSpentMoneyOnTheRiver || bSpentRiverMoney) {
                    if (bTerse)
                      printf("%d\n",delta);
                    else {
                      printf("%s %10d",hole_cards,delta);

                      if (bShowBoard && bHaveRiver)
                        printf(" %s",board_cards);

                      if (bShowHand && bHaveFlop)
                        printf(" %s",poker_hand.GetHand());

                      if (bShowHandType && bHaveFlop)
                        printf(" %s",plain_hand_types[poker_hand.GetHandType()]);

                      if (bVerbose)
                        printf(" %s %3d\n",filename,num_hands);
                      else
                        putchar(0x0a);
                    }
                  }
                }
              }
            }
          }

          continue;
        }

        if (!strncmp(line,street_marker,STREET_MARKER_LEN)) {
          num_street_markers++;

          if (num_street_markers > 1) {
            if (street <= 3)
              spent_this_hand += spent_this_street;

            street++;
            spent_this_street = 0;
          }
        }
      }
    }

    fclose(fptr);
  }

  fclose(fptr0);

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

static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index)
{
  int m;
  int n;
  int tries;
  char chara;

  tries = line_len - string_len + 1;

  if (tries <= 0)
    return false;

  for (m = 0; m < tries; m++) {
    for (n = 0; n < string_len; n++) {
      chara = line[m + n];

      if (!bCaseSens) {
        if ((chara >= 'A') && (chara <= 'Z'))
          chara += 'a' - 'A';
      }

      if (chara != string[n])
        break;
    }

    if (n == string_len) {
      *index = m;
      return true;
    }
  }

  return false;
}

int get_work_amount(char *line,int line_len)
{
  int ix;
  int chara;
  int work_amount;

  work_amount = 0;

  for (ix = line_len - 1; (ix >= 0); ix--) {
    chara = line[ix];

    if ((chara >= '0') && (chara <= '9'))
      break;
  }

  if (ix + 1 != line_len);
    line[ix + 1] = 0;

  for ( ; (ix >= 0); ix--) {
    chara = line[ix];

    if ((chara < '0') || (chara > '9'))
      break;
  }

  sscanf(&line[ix+1],"%d",&work_amount);

  return work_amount;
}
