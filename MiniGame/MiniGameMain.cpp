#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH{ 360 };
int DISPLAY_HEIGHT{ 360 };
int DISPLAY_SCALE{ 1 };


enum GameObjectType
{
	TYPE_NULL = -1,
	TYPE_SNAKE,
	TYPE_TRAIL,
	TYPE_FRUIT,
};

enum SnakeState
{
	STATE_ALIVE = 0,
	STATE_DEAD,
};

struct GameState
{
	int score = 0;
	SnakeState snakeState = STATE_ALIVE;
	float snake_speed = 2.;
	int frame = 0;
	int x_pos_history[100] = { 0 };
	int y_pos_history[100] = { 0 };
};
GameState gameState;

void HandleSnakeControls();
void CreateFruit();
void UpdateFruit();
void CreateTrail();
void UpdateTrail();
void SnakeControls();


// MAIN
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::CentreAllSpriteOrigins();
	Play::CreateGameObject(TYPE_SNAKE, { 30,30 }, 10, "snake");
	Play::CreateGameObject(TYPE_TRAIL, { 30,30 }, 10, "snake");
	CreateFruit();
	CreateTrail();
}

bool MainGameUpdate(float elapsedTime)
{
	Play::ClearDrawingBuffer(Play::cGrey);

	SnakeControls();
	UpdateFruit();
	UpdateTrail();
	Play::PresentDrawingBuffer();
	return Play::KeyDown(VK_ESCAPE);
}

int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
}



void HandleSnakeControls()
{
	GameObject& obj_snake = Play::GetGameObjectByType(TYPE_SNAKE);
	bool KeyHasBeenPressed = false;

	if (Play::KeyDown(VK_LEFT) && obj_snake.velocity.x == 0)
	{
		obj_snake.velocity = { -gameState.snake_speed, 0 };
		obj_snake.pos.y = gameState.y_pos_history[0];
	}
	if (Play::KeyDown(VK_RIGHT) && obj_snake.velocity.x == 0)
	{
		obj_snake.velocity = { gameState.snake_speed, 0 };
		obj_snake.pos.y = gameState.y_pos_history[0];
	}
	if (Play::KeyDown(VK_UP) && obj_snake.velocity.y == 0)
	{
		obj_snake.velocity = { 0, -gameState.snake_speed };
		obj_snake.pos.x = gameState.x_pos_history[0];
	}
	if (Play::KeyDown(VK_DOWN) && obj_snake.velocity.y == 0)
	{
		obj_snake.velocity = { 0, gameState.snake_speed };
		obj_snake.pos.x = gameState.x_pos_history[0];
	}

	if (Play::IsLeavingDisplayArea(obj_snake))
	{
		obj_snake.velocity = { 0,0 };
		gameState.snakeState = STATE_DEAD;
	}
	
	
	
	gameState.frame++;
	if ((gameState.frame % 10) == 0 && gameState.snakeState == STATE_ALIVE)
	{
		for (int i = 99; i > 0; i--)
		{
			// need to change this method to vectors
			gameState.x_pos_history[i] = gameState.x_pos_history[i - 1]; //move elements along 1
			gameState.y_pos_history[i] = gameState.y_pos_history[i - 1];
		}

		gameState.x_pos_history[0] = obj_snake.pos.x;
		gameState.y_pos_history[0] = obj_snake.pos.y;
	}
	if (gameState.frame == 1000)
	{
		gameState.frame = 0; //so son't pass the maximum int limit
	}

	Play::UpdateGameObject(obj_snake);
	Play::DrawObject(obj_snake);
}



void CreateFruit()
{
	int x_pos = Play::RandomRollRange(30, DISPLAY_WIDTH - 30);
	int y_pos = Play::RandomRollRange(30, DISPLAY_HEIGHT - 30);
	Play::CreateGameObject(TYPE_FRUIT, { x_pos, y_pos }, 10, "fruit");
}

void UpdateFruit()
{
	GameObject& obj_fruit = Play::GetGameObjectByType(TYPE_FRUIT);
	GameObject& obj_snake = Play::GetGameObjectByType(TYPE_SNAKE);
	std::vector<int> vTrails = Play::CollectGameObjectIDsByType(TYPE_TRAIL);

	if (Play::IsColliding(obj_fruit, obj_snake))
	{
		gameState.score += 1;
		obj_fruit.pos = { Play::RandomRollRange(30, DISPLAY_WIDTH - 30),
			Play::RandomRollRange(30, DISPLAY_HEIGHT - 30) };
		
		for (int id : vTrails)
		{
			GameObject& obj_trail = Play::GetGameObject(id);
			while (Play::IsColliding(obj_fruit, obj_trail) || Play::IsColliding(obj_fruit, obj_snake))
			{
				obj_fruit.pos = { Play::RandomRollRange(30, DISPLAY_WIDTH - 30),
					Play::RandomRollRange(30, DISPLAY_HEIGHT - 30) };
			}
		}

		Play::CreateGameObject(TYPE_TRAIL, { 0,0 }, 10, "snake");
	}
	Play::DrawObject(obj_fruit);
}

void CreateTrail()
{
	std::vector<int> vTrail(5);
	for (int id : vTrail)
	{
		int id = Play::CreateGameObject(TYPE_TRAIL, { 0, 0 }, 10, "snake");
	}
}

void UpdateTrail()
{
	GameObject& obj_snake = Play::GetGameObjectByType(TYPE_SNAKE);
	std::vector<int> vTrails = Play::CollectGameObjectIDsByType(TYPE_TRAIL);

	int n{ 0 };
	for (int j = 0; j < (5 + gameState.score); j ++)
	{
		int id = vTrails.at(n);
		GameObject& obj_trail = Play::GetGameObject(id);
		obj_trail.pos = { gameState.x_pos_history[j], gameState.y_pos_history[j] };
		Play::DrawObject(obj_trail);
		n++;
	}

}

int stopCount = 0;
void DeadAnimation()
{
	bool IsTransparent = false;
	GameObject& obj_snake = Play::GetGameObjectByType(TYPE_SNAKE);
	std::vector <int> vTrail = Play::CollectGameObjectIDsByType(TYPE_TRAIL);

	if (gameState.frame % 30)
	{
		IsTransparent = !IsTransparent;
		stopCount++;
	}

	if (IsTransparent == true)
	{
		Play::DrawObjectTransparent(obj_snake, 0.0f);

		for (int id : vTrail)
		{
			GameObject& obj_trail = Play::GetGameObject(id);
			Play::DrawObjectTransparent(obj_trail, 0.0f);
		}
	}

	if (stopCount == 12)
	{
		gameState.score = 0;
		gameState.snakeState = STATE_ALIVE;
		obj_snake.pos = { 30, 30 };
		stopCount = 0;
	}
}

void SnakeControls()
{
	GameObject& obj_snake = Play::GetGameObjectByType(TYPE_SNAKE);
	std::vector<int> vTrail = Play::CollectGameObjectIDsByType(TYPE_TRAIL);

	switch (gameState.snakeState)
	{
	case(STATE_ALIVE):
		HandleSnakeControls();
		break;
	case(STATE_DEAD):
		DeadAnimation();
		break;
	}

}