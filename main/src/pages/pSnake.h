#ifndef __pSNAKE_h
#define __pSNAKE_h

#include "../../include/Page.h"
#include "../../include/UserInterface.h"
#include "../../../config.h"

#define SNAKE_CUBE_SIZE 3
#define SNAKE_PADDING_SIZE 1
#define SNAKE_GRID_WIDTH (SCREEN_WIDTH / (SNAKE_CUBE_SIZE + SNAKE_PADDING_SIZE))
#define SNAKE_GRID_HEIGHT (SCREEN_HEIGHT / (SNAKE_CUBE_SIZE + SNAKE_PADDING_SIZE))

enum MoveDirection {
  Up,
  Right,
  Down,
  Left
};

typedef struct Point {
  unsigned int x = 0;
  unsigned int y = 0;
  bool operator==(Point const & b) {
    return this->x == b.x && this->y == b.y;
  }
} Point;

typedef struct SnakeNode {
  Point p;
  SnakeNode *next = nullptr;
};

class pSnake : public Page {
  MoveDirection direction = Right;
  MoveDirection turn = Up;
  int length = 3;
  int speed = 2; // moves per second
  SnakeNode *first_node = nullptr;
  SnakeNode *last_node = nullptr;
  Point food_point;
  long last_move_ms = 0;
  bool game_over = false;

  void add_node(MoveDirection dir, bool grow = false) {
    Point p;

    if (last_node != nullptr) {
      p = last_node->p;

      switch(dir) {
        case Up:
          p.y = (p.y + 1) % SNAKE_GRID_HEIGHT;
          break;
        case Down:
          p.y = (p.y - 1) % SNAKE_GRID_HEIGHT;
          break;
        case Left:
          p.x = (p.x - 1) % SNAKE_GRID_WIDTH;
          break;
        case Right:
          p.x = (p.x + 1) % SNAKE_GRID_WIDTH;
          break;
      }
    } else {
      p.x = SNAKE_GRID_WIDTH / 2;
      p.y = SNAKE_GRID_HEIGHT / 2;
    }

    SnakeNode *node = new SnakeNode();
    node->p = p;
    node->next = nullptr;

    // Append Last Node
    if (last_node != nullptr) {
      last_node->next = node;
    }

    last_node = node;

    // Remove First Node
    if (first_node != nullptr) {
      if (grow) {
        return;
      }

      SnakeNode *first = first_node;
      first_node = first_node->next;
      free(first);
    } else {
      first_node = node;
    }
  }

  int get_snake_length() {
    SnakeNode *n = first_node;
    int i = 0;
    while(n != nullptr) {
      i++;
      n = n->next;
    }
    return i;
  }

  bool snake_intersects() {
    SnakeNode *n = first_node;
    while (n != nullptr) {
      if (n != last_node && n->p == last_node->p) {
        return true;
      }
      n = n->next;
    }

    return false;
  }

  void Enter(bool) override {
    // Add 3 starting nodes:
    for (int i = 0; i < 3; i++)
      add_node(Right, true);
    food_point.x = last_node->p.x + 5;
    food_point.y = last_node->p.y;
    last_move_ms = millis();
    srand(micros());
  }

  void Render() override {
    SnakeNode *n = first_node;

    // Draw Score
    UI.display->setCursor(0, 0);
    UI.display->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    UI.display->print("Score: " + String(length - 3));

    // Draw Snake
    while (n != nullptr) {
      UI.display->fillRect(
          n->p.x * (SNAKE_CUBE_SIZE + SNAKE_PADDING_SIZE),
          n->p.y * (SNAKE_CUBE_SIZE + SNAKE_PADDING_SIZE),
          SNAKE_CUBE_SIZE, SNAKE_CUBE_SIZE, SSD1306_WHITE
      );

      n = n->next;
    }

    // Draw Food Chunk
    UI.display->drawRect(
        food_point.x * (SNAKE_CUBE_SIZE + SNAKE_PADDING_SIZE),
        food_point.y * (SNAKE_CUBE_SIZE + SNAKE_PADDING_SIZE),
        SNAKE_CUBE_SIZE, SNAKE_CUBE_SIZE, SSD1306_WHITE
    );
  }

  void Loop() override {
    if (game_over) {
      if (millis() - last_move_ms > 3000) {
        Hardware::setMotorSpeed(0);
      }
      return;
    }

    if (millis() - last_move_ms > 1000 / speed) {
      Hardware::setMotorSpeed(0);
      last_move_ms = millis();

      // Adjust Direction
      if (turn != Up) {
        switch(direction) {
          case Up:
            direction = turn == Right ? Right : Left;
            break;
          case Down:
            direction = turn == Right ? Left : Right;
            break;
          case Right:
            direction = turn == Right ? Down : Up;
            break;
          case Left:
            direction = turn == Right ? Up : Down;
            break;
        }
        Serial.println("Direction: " + String(direction));
        turn = Up;
      }

      // Do Snek Things
      add_node(direction, get_snake_length() < length);

      if (snake_intersects()) {
        UI.toast("You have died.", 3000);
        Serial.println("DED DANGER NOODUL! D:");
        Hardware::setMotorSpeed(255);
        game_over = true;
      }

      if (last_node->p == food_point) {
        length++;
        speed++;
        food_point.x = rand() % SNAKE_GRID_WIDTH;
        food_point.y = rand() % SNAKE_GRID_HEIGHT;

        Serial.println("Snek go chomp.");
        Hardware::setMotorSpeed(128);
      }

      Rerender();
    }
  }

  void onEncoderChange(int diff) {
    if (diff < 0) {
      turn = Right;
    } else {
      turn = Left;
    }
  }
};

#endif