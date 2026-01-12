#include "snake.h"
#include "../../game_config.h"
#include "engine/core/config.h"
#include "engine/core/input.h"
#include "raylib.h"

typedef struct Snake {
    Vector2 body[MAX_SNAKE_LENGTH];
    int length;
    Vector2 direction;
    Vector2 lastMoveDirection;
    float moveTimer;
    float interval;
}Snake;

static Snake snake = {0};

void InitSnake(int startX, int startY) {
    snake.length = SNAKE_INITIAL_LENGTH;
    snake.direction = (Vector2){1, 0};
    snake.moveTimer = 0.0f;
    snake.lastMoveDirection = (Vector2){1, 0};
    snake.interval = SNAKE_SPEED_START;

    // Initialize body segments
    for (int i = 0; i < snake.length; i++) {
        snake.body[i] = (Vector2){ (float)(startX - i), (float)startY };
    }
}

void UpdateSnake(void) {
    // Handling Input
    if (Input_IsPressed(ACTION_UP) && snake.lastMoveDirection.y == 0)
    snake.direction = (Vector2) {0, -1};
    if (Input_IsPressed(ACTION_DOWN) && snake.lastMoveDirection.y == 0)
    snake.direction = (Vector2) {0, 1};
    if (Input_IsPressed(ACTION_LEFT) && snake.lastMoveDirection.x == 0)
    snake.direction = (Vector2) {-1, 0};
    if (Input_IsPressed(ACTION_RIGHT) && snake.lastMoveDirection.x == 0)
    snake.direction = (Vector2) {1, 0};

    // Move Timer
    snake.moveTimer += GetFrameTime();
    if (snake.moveTimer >= snake.interval) {
        snake.moveTimer = 0.0f;

        // Moving the body
        for (int i = snake.length -1 ; i > 0 ;i--) {
            snake.body[i] = snake.body[i-1];
        }
        // Move head
        snake.body[0].x += snake.direction.x;
        snake.body[0].y += snake.direction.y;

        snake.lastMoveDirection = snake.direction;
    }
}

void DrawSnake(void) {
    for ( int i = 0;i < snake.length; i++) {
        Color color = (i == 0) ? DARKGREEN : GREEN;
        DrawRectangle(
            (int)snake.body[i].x * CELL_SIZE,
            (int)snake.body[i].y * CELL_SIZE,
            CELL_SIZE,
            CELL_SIZE,
            color
        );
    }
}

Vector2 GetSnakeHeadPosition(void) {
    return snake.body[0];
}

void GrowSnake(void) {
    if (snake.length < MAX_SNAKE_LENGTH) {
        snake.body[snake.length] = snake.body[snake.length-1];
        snake.length++;
        if (snake.interval > SNAKE_SPEED_MIN)
        snake.interval -= SNAKE_SPEED_BUFF;
    }
}

bool CheckSnakeCollision(int gridWidth, int gridHeight) {
    Vector2 head = snake.body[0];

    // Check Wall Collision
    if (head.x < 0 || head.x >= gridWidth || head.y < 0 || head.y >= gridHeight)
        return true;

    // Check Self Collision
    for (int i = 1; i < snake.length;i++) {
        if (head.x == snake.body[i].x && head.y == snake.body[i].y)
        return true;
    }
    return false;
}

