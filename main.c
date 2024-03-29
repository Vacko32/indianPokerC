#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum Color {
  HEARTS,
  DIAMONDS,
  CLUBS,
  SPADES
};

struct Card {
  int rank;
  enum Color suit;
};

struct Deck {
  struct Card cards[52];
  int size;
};

struct Deck deck_create() {
  struct Deck deck;
  deck.size = 52;
  for (int i = 0; i < 52; i++) {
    deck.cards[i].rank = i % 13 + 2;
    if (i % 4 == 0) {
      deck.cards[i].suit = HEARTS;
    } else if (i % 4 == 1) {
      deck.cards[i].suit = DIAMONDS;
    } else if (i % 4 == 2) {
      deck.cards[i].suit = CLUBS;
    } else if (i % 4 == 3) {
      deck.cards[i].suit = SPADES;
    }
  }
  return deck;
};

struct Deck deck_shuffle(struct Deck deck) {
  srand(time(NULL));
  for (int k = 0; k < 100; k++) {
    int j = rand();

    int i = rand();
    j = j % 52;
    i = i % 52;

    struct Card temp = deck.cards[i];
    deck.cards[i] = deck.cards[j];
    deck.cards[j] = temp;
  }
  return deck;
}

struct Player {
  struct Card hand[2];
  char name[20];
  int handvalue;
  int stack;
  int status;
  int betroundsize;
};

struct Player player_create(char* name) {
  struct Player player;
  player.stack = 0;
  player.status = 1;
  strncpy(player.name, name, 20);
  player.name[20] = '\0';
  player.handvalue = 0;
  return player;
};

int evaluate_hand(struct Player player) {
  if (player.hand[0].rank == player.hand[1].rank) {
    return 1000000 + player.hand[0].rank;
  } else if (player.hand[0].suit == player.hand[1].suit) {
    if (player.hand[0].rank > player.hand[1].rank) {
      return 1000 + player.hand[0].rank * 20 + player.hand[1].rank;
    }

    return 1000 + player.hand[1].rank * 20 + player.hand[0].rank;
  } else {
    if (player.hand[0].rank > player.hand[1].rank) {
      return player.hand[0].rank * 20 + player.hand[1].rank;
    }

    return player.hand[1].rank * 20 + player.hand[0].rank;
  }
};

struct Game {
  struct Deck deck;
  struct Player players[10];
  struct Player winners[10];
  int blind_index;
  int round_number;
  int player_count;
  int pot;
  int player_to_move_index;
  int winner_count;
  int to_call;
};

struct Game game_create() {
  struct Game game;
  game.deck = deck_create();
  game.round_number = 0;
  game.blind_index = 0;
  game.player_count = 0;
  game.pot = 0;
  game.player_to_move_index = 0;
  game.winner_count = 0;
  game.to_call = 0;
  return game;
};

void evaluate_winner(struct Game* game) {
  int highestHandValue = 0;

  for (int i = 0; i < game->player_count; i++) {
    if (game->players[i].status == 1) {
      game->players[i].handvalue = evaluate_hand(game->players[i]);
      printf("Player %s has handvalue %d\n", game->players[i].name, game->players[i].handvalue);
      if (game->players[i].handvalue > highestHandValue) {
        highestHandValue = game->players[i].handvalue;
      }
    }
  }

  int winners = 0;
  for (int i = 0; i < game->player_count; i++) {
    if (game->players[i].status == 1) {
      if (game->players[i].handvalue == highestHandValue) {
        game->winners[winners] = game->players[i];
        printf("%d\n", i);
        printf("Player %s won\n", game->winners[winners].name);
        game->winner_count++;
        winners++;
      }
    }
  }
};

void add_player(struct Game* game, struct Player player) {
  if (game->player_count == 10) {
    return;
  }
  game->players[game->player_count] = player;
  game->players[game->player_count].stack = 10000;
  game->player_count++;
};
void deal(struct Game* game) {
  for (int i = 0; i < game->player_count; i++) {
    game->players[i].hand[0] = game->deck.cards[game->deck.size - 1];
    game->deck.size--;

    game->players[i].hand[1] = game->deck.cards[game->deck.size - 1];
    game->deck.size--;
    printf("Player %s has %d %d and %d %d\n", game->players[i].name, game->players[i].hand[0].rank,
           game->players[i].hand[0].suit, game->players[i].hand[1].rank,
           game->players[i].hand[1].suit);
  }
};

void place_bet(struct Game* game, int player_index, int bet) {
  if (game->players[player_index].stack >= bet) {
    game->players[player_index].stack -= bet;
    game->players[player_index].betroundsize += bet;
    game->pot += bet;
    printf("Player %s placed bet %d\n", game->players[player_index].name, bet);
  } else {
    printf("Player %s invalid betsize", game->players[player_index].name);
  }
  if (bet > game->to_call) {
    game->to_call = bet;
  }
};

void next_round(struct Game* game) {
  for (int i = 0; i < game->player_count; i++) {
    game->players[i].status = 1;
    game->players[i].betroundsize = 0;
  }
  game->round_number++;
  game->blind_index = (game->blind_index + 1) % game->player_count;
  if (game->deck.size < 2 * game->player_count) {
    game->deck = deck_create();
  }

  game->deck = deck_shuffle(game->deck);
  game->pot = 0;

  deal(game);
  place_bet(game, game->blind_index, game->players[game->blind_index].stack / 20);
};

void end_round(struct Game* game) {
  printf("There are %d winners\n", game->winner_count);
  evaluate_winner(game);
  int not_folded = 0;
  for (int i = 0; i < 10; i++) {
    if (game->winners[i].status == 1) {
      not_folded++;
    }
  }
  printf("There are %d winners\n", game->winner_count);

  printf("Pot is %d\n", game->pot);
  for (int i = 0; i < game->winner_count; i++) {
    printf("not folded is %d\n", not_folded);
    game->winners[i].stack += game->pot / not_folded;
    printf("Player %s won %d\n", game->winners[i].name, game->pot / game->winner_count);
    printf("Player %s has %d\n", game->winners[i].name, game->winners[i].stack);
  }
};

void start_game_add_players(struct Game* game) {
  int pickedsolution = 0;
  while (pickedsolution != 2) {
    printf("Enter 1 to add player, 2 to do no action\n");

    pickedsolution = 0;
    scanf("%d", &pickedsolution);
    if (pickedsolution == 1) {
      char name[20];
      printf("Enter player name\n");
      scanf("%19s", name);
      struct Player player = player_create(name);
      add_player(game, player);
    } else {
      pickedsolution = 2;
      break;
    }
  }
};

/*
struct Game make_move_bot1(struct Game game, int player_index) {

};
struct Game make_move_bot2(struct Game game, int player_index) {

};
struct Game make_move_bot3(struct Game game, int player_index) {

};
*/

bool check_bets_same(struct Game* game) {
  int buffer = 0;
  for (int i = 0; i < game->player_count; i++) {
    if (game->players[i].status == 1) {
      if (buffer == 0) {
        buffer = game->players[i].betroundsize;

      } else {
        int betcomparation = game->players[i].betroundsize;
        if (betcomparation != buffer) {
          printf("False\n");
          printf("Buffer: %d\n", buffer);
          printf("Betcomparation: %d\n", betcomparation);
          return false;
        }
      }
    }
  }
  printf("True\n");
  return true;
};

void player_fold(struct Game* game, int player_index) { game->players[player_index].status = 0; };

void make_move_bot1(struct Game* game, int player_index) {
  int bet = 0;
  int handstrenght = evaluate_hand(game->players[player_index]);
  printf("Player %s has handvalue %d\n", game->players[player_index].name, handstrenght);
  printf("Player %s calls\n", game->players[player_index].name);
  printf("game to call is and player betroundsize is %d %d\n", game->to_call,
         game->players[player_index].betroundsize);
  bet = game->to_call - game->players[player_index].betroundsize;
  place_bet(game, player_index, bet);
};

void round_process(struct Game* game) {
  game->player_to_move_index = (game->blind_index + 1) % game->player_count;

  while (check_bets_same(game) == false) {
    if (game->players[game->player_to_move_index].status == 1) {
      make_move_bot1(game, game->player_to_move_index);
    }
    game->player_to_move_index = (game->player_to_move_index + 1) % game->player_count;
  }
};

int main() {
  struct Game game = game_create();
  start_game_add_players(&game);
  next_round(&game);
  round_process(&game);
  end_round(&game);

  return 0;
}