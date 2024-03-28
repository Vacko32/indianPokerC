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
};

struct Player player_create(char *name) {
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
};

struct Game game_create() {
  struct Game game;
  game.deck = deck_create();
  game.round_number = 0;
  game.blind_index = 0;
  game.player_count = 0;
  game.pot = 0;
  return game;
};

struct Game evaluate_winner(struct Game game) {
  int highestHandValue = 0;

  for (int i = 0; i < game.player_count; i++) {
    if (game.players[i].status == 1) {
      game.players[i].handvalue = evaluate_hand(game.players[i]);
      printf("Player %s has %d\n", game.players[i].name, game.players[i].handvalue);
      if (game.players[i].handvalue > highestHandValue) {
        highestHandValue = game.players[i].handvalue;
      }
    }
  }

  int winners = 0;
  for (int i = 0; i < game.player_count; i++) {
    if (game.players[i].status == 1) {
      if (game.players[i].handvalue == highestHandValue) {
        game.winners[winners] = game.players[i];
        printf("%d\n", i);
        printf("Player %s won\n", game.winners[winners].name);
        winners++;
      }
    }
  }
  return game;
};

struct Game add_player(struct Game game, struct Player player) {
  if (game.player_count == 10) {
    return game;
  }
  game.players[game.player_count] = player;
  game.players[game.player_count].stack = 10000;
  game.player_count++;
  return game;
};
struct Game deal(struct Game game) {
  for (int i = 0; i < game.player_count; i++) {
    game.players[i].hand[0] = game.deck.cards[game.deck.size - 1];
    game.deck.size--;

    game.players[i].hand[1] = game.deck.cards[game.deck.size - 1];
    game.deck.size--;
    printf("Player %s has %d %d and %d %d\n", game.players[i].name, game.players[i].hand[0].rank,
           game.players[i].hand[0].suit, game.players[i].hand[1].rank,
           game.players[i].hand[1].suit);
  }

  return game;
};

struct Game place_bet(struct Game game, int player_index, int bet) {
  game.players[player_index].stack -= bet;
  game.pot += bet;
  return game;
};

struct Game next_round(struct Game game) {
  for (int i = 0; i < game.player_count; i++) {
    game.players[i].status = 1;
  }
  game.round_number++;
  game.blind_index = (game.blind_index + 1) % game.player_count;
  if (game.deck.size < 2 * game.player_count) {
    game.deck = deck_create();
  }

  game.deck = deck_shuffle(game.deck);
  game.pot = 0;

  game = deal(game);
  game = place_bet(game, game.blind_index, game.players[game.blind_index].stack / 20);
  return game;
};

struct Game end_round(struct Game game) {
  game = evaluate_winner(game);
  int winnercount = 0;
  for (int i = 0; i < 10; i++) {
    if (game.winners[i].status == 1) {
      winnercount++;
    }
  }
  printf("There are %d winners\n", winnercount);

  printf("Pot is %d\n", game.pot);
  for (int i = 0; i < winnercount; i++) {
    game.winners[i].stack += game.pot / winnercount;
    printf("Player %s won %d\n", game.winners[i].name, game.pot / winnercount);
    printf("Player %s has %d\n", game.winners[i].name, game.winners[i].stack);
  }
  return game;
};

struct Game start_game_add_players(struct Game game) {
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
      game = add_player(game, player);
    } else {
      pickedsolution = 2;
      return game;
      break;
    }
  }
  return game;
};

struct Game round_process(game){

};

int main() {
  struct Game game = game_create();
  game = start_game_add_players(game);
  game = next_round(game);
  game = end_round(game);

  return 0;
}