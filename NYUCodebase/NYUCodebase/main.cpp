
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdlib.h> 
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <math.h>

using namespace std;

#define TEX_ADJUST 3.0f/692.0f

#define MAX_SPEED = 5.0;
// 60 FPS (1.0f/60.0f)
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
#define TILE_SIZE 0.133f //0.0625
#define SPRITE_COUNT_X 30
#define SPRITE_COUNT_Y 30
#define LEVEL_HEIGHT 64
#define LEVEL_WIDTH 64
#define WORLD_OFFSET_X 0
#define WORLD_OFFSET_Y 0
float timeLeftOver = 0.0f;

enum direction
{
	LEFT, RIGHT, TOP, BOTTOM, NONE
};

GLuint LoadTexture(const char *image_path)
{
	SDL_Surface *surface = IMG_Load(image_path);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, surface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SDL_FreeSurface(surface);
	return textureID;
}

float lerp(float v0, float v1, float t)
{
	return (1.0 - t)*v0 + t*v1;
}

class Entity
{
public:
	Entity(float x, float y,float vx =0.0, float vy = 0.0) :x(x), y(y), x_velocity(vx), y_velocity(vy),top(false), bottom(false), right(false), left(false){}
	Entity() :x(0.0), y(0.0), y_velocity(0.0),x_velocity(0.0),top(false), bottom(false), right(false), left(false){}
	float x;
	float y;
	float width;
	float height;
	float y_velocity;
	float x_velocity;
	bool top;
	bool bottom;
	bool right;
	bool left;
};

class Player : public Entity
{
public:
	Player() :Entity(){}
	Player(float x,float y) :Entity(x,y,0.0,0.0),x_accelartion(0.0f), y_accelartion(0.0f){}
	float x_accelartion;
	float y_accelartion;
};

class Block : public Entity
{
public:
	Block() :Entity(){}
	Block(float x, float y) :Entity(x, y){}

};

class Enemy : public Entity
{
public:
	Enemy() :Entity(){}
	Enemy(float x, float y) :Entity(x, y){}
};

class Game
{
private:
	bool done;
	SDL_Window* displayWindow;
	vector<unsigned int> Sprite;
	//vector <GLfloat[]> vertex;
	//vector <GLfloat[]> uvcoord;
	int vertexnumber;
	vector<Block> blocks;
	vector<Enemy> enemies;
	vector<unsigned int> font;
	Player player;
	unsigned char** levelData;
	vector<vector<double>> vertexData;
	vector<vector<float>> texCoordData;
	float elapsed;
	int mapHeight;
	int mapWidth;
	int run_animation_player[8];
	int numFrames;
	int score;
	float earthquake;
	float direction;
	float animationElapsed;
	float framesPerSecond;
	float lastFrameTicks;
	float gravity;
	int current_index_player_animation;
	float x_friction;
	float y_friction;
public:
	vector<Mix_Chunk*> sounds;
	vector<Mix_Music*> music;
	void clearup();
	void initialize();
	void render();
	void update();
	void read_file();
	bool readHeader(std::ifstream &stream);
	bool readLayerData(std::ifstream &stream);
	bool readEntityData(std::ifstream &stream);
	void placeEntity(string& type, float placeX, float placeY);
	void Draw();
	void drawLevel(int spriteTexture);
	void DrawSpriteSheetSpriteTiled(int spriteTexture, int x, int y, float rotation, float scale);
	void DrawSpriteSheetSpriteTiledEntity(int spriteTexture,  const string& type,float x, float y, float rotation, float scale);
	void camera();
	std::pair<int,int> worldToTileCoor(float worldX, float worldY);
	bool is_solid(int tile);
	void animation_update();
	void player_update();
	void player_block_collide();
	void player_enemy_collide();
	void collision();
	bool entity_collide_tile(const std::pair<float, float>& entity1, const std::pair<float, float>&entity2);
	void entity_block_collide(Entity* entity);
	bool is_done();
	bool entity_collide(const Entity &entity1, const Entity &entity2);
	float y_penetration(const std::pair<float, float>& entity_1, const std::pair<float, float>& entity_2);
	float x_penetration(const std::pair<float, float>& entity_1, const std::pair<float, float>& entity_2);
};

std::pair<int, int> Game::worldToTileCoor(float worldX, float worldY)
{
	int gridX = int((worldX + (WORLD_OFFSET_X)) / TILE_SIZE);
	int gridY = int((worldY + (WORLD_OFFSET_Y)) / TILE_SIZE);
	return std::pair<int, int>(gridX, gridY);
}

bool Game::is_solid(int tile)
{
	if (tile != 0)
		return true;
	else
		return false;
}

/*
void Game::Draw()
{
	float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
	float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
	for (int y = 0; y < LEVEL_HEIGHT; y++)
	{
		for (int x = 0; x < LEVEL_WIDTH; x++)
		{
			if (levelData[y][x] != 0)
			{
				float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
				float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
				vertexnumber += 4;
				vertex.insert(vertex.end(), {
					TILE_SIZE * x, -TILE_SIZE * y,
					TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
				});
				uvcoord.insert(uvcoord.end(), { u + TEX_ADJUST, v + TEX_ADJUST,
					u + TEX_ADJUST, v + (spriteHeight)-TEX_ADJUST,
					u + spriteWidth - TEX_ADJUST, v + (spriteHeight)-TEX_ADJUST,
					u + spriteWidth - TEX_ADJUST, v + TEX_ADJUST
				});
			}
		}
	}
}*/

void Game::clearup()
{
	Mix_FreeChunk(sounds[0]);
	Mix_FreeChunk(sounds[1]);
	Mix_FreeMusic(music[0]);
	SDL_Quit();
}

void Game::drawLevel(int spriteTexture)
{
	vector<GLfloat> vertex;
	vector<GLfloat> uvcoord;
	vertexnumber = 0;
	float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
	float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
	for (int y = 0; y < LEVEL_HEIGHT; y++)
	{
		for (int x = 0; x < LEVEL_WIDTH; x++)
		{
			if (levelData[y][x] != 0)
			{
				float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
				float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
				vertexnumber += 4;
				vertex.insert(vertex.end(), {
					TILE_SIZE * x, -TILE_SIZE * y,
					TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
				});

				uvcoord.insert(uvcoord.end(), { u + TEX_ADJUST, v + TEX_ADJUST,
					u + TEX_ADJUST, v + (spriteHeight)-TEX_ADJUST,
					u + spriteWidth - TEX_ADJUST, v + (spriteHeight)-TEX_ADJUST,
					u + spriteWidth - TEX_ADJUST, v + TEX_ADJUST
				});
			}
		}
	}
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, spriteTexture);
	glMatrixMode(GL_MODELVIEW);
	glVertexPointer(2, GL_FLOAT, 0, vertex.data());
	glEnableClientState(GL_VERTEX_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, uvcoord.data());
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_QUADS, 0, vertexnumber);
	//glMatrixMode(GL_PROJECTION);
	glDisable(GL_TEXTURE_2D);
}

void Game::DrawSpriteSheetSpriteTiled(int spriteTexture, int x, int y, float rotation = 0.0, float scale = 0.5)
{
	if (levelData[y][x] == 0)
		return;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, spriteTexture);

	glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	//glPushMatrix();
	//glTranslatef(player.x, player.y, 0.0f);
	//glTranslatef(-TILE_SIZE * 46.2574, TILE_SIZE * 26.09/2.0, 0.0);
	//glRotatef(rotation, 0.0, 0.0, 1.0);
	float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
	float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;

	float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
	float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;

	GLfloat quad[] = {
		TILE_SIZE * x, -TILE_SIZE * y,
		TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
		(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
		(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
	};
	glVertexPointer(2, GL_FLOAT, 0, quad);
	glEnableClientState(GL_VERTEX_ARRAY);
	GLfloat quadUVs[] =
	{ u + TEX_ADJUST, v + TEX_ADJUST,
	u + TEX_ADJUST, v + (spriteHeight)-TEX_ADJUST,
	u + spriteWidth - TEX_ADJUST, v + (spriteHeight)-TEX_ADJUST,
	u + spriteWidth - TEX_ADJUST, v + TEX_ADJUST
	};
	glTexCoordPointer(2, GL_FLOAT, 0, quadUVs);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_QUADS, 0, 4);
	//glMatrixMode(GL_PROJECTION);
	glDisable(GL_TEXTURE_2D);
	//glPopMatrix();
	//glLoadIdentity();
}

void Game::DrawSpriteSheetSpriteTiledEntity(int spriteTexture, const string& type,float x, float y,  float rotation = 0.0, float scale = 0.5)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, spriteTexture);

	glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	//glTranslatef(x, y, 0.0);

	int index = 0;
	if (type == "Player")
		index = 20;
	if (type == "Enemey")
		index = 170;

	float u = (float)(((int)index) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
	float v = (float)(((int)index) / SPRITE_COUNT_Y) / (float)SPRITE_COUNT_Y;
	float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
	float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;

	GLfloat quad[] = {
		x, y,
		x, y - TILE_SIZE,
		x + TILE_SIZE, y - TILE_SIZE,
		x + TILE_SIZE, y
	};
	glVertexPointer(2, GL_FLOAT, 0, quad);
	glEnableClientState(GL_VERTEX_ARRAY);
	GLfloat quadUVs[] =
	{ u, v + 1.0 / 692.0,
	u, v + (spriteHeight),
	u + spriteWidth - 1.0 / 692.0, v + (spriteHeight),
	u + spriteWidth - 1.0 / 692.0, v + 1.0 / 692.0
	};
	glTexCoordPointer(2, GL_FLOAT, 0, quadUVs);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_QUADS, 0, 4);
	//glMatrixMode(GL_PROJECTION);
	glDisable(GL_TEXTURE_2D);
	//glPopMatrix();
	//glLoadIdentity();
}

float Game::y_penetration(const std::pair<float, float>& entity_1, const std::pair<float, float>& entity_2)
{
	float y_distance = fabs(entity_1.second - entity_2.second);
	float y_penetration = fabs(y_distance - TILE_SIZE*0.5 - TILE_SIZE*0.5);
	return y_penetration;
}

float Game::x_penetration(const std::pair<float, float>& entity_1, const std::pair<float, float>& entity_2)
{
	float x_distance = fabs(entity_1.first - entity_2.first);
	float x_penetration = fabs(x_distance - TILE_SIZE*0.5 -TILE_SIZE*0.5);
	return x_penetration;
}

/*
void Game::camera()
{
	offsetMaxX = WORLD_SIZE_X - VIEWPORT_SIZE_X
		offsetMaxY = WORLD_SIZE_Y - VIEWPORT_SIZE_Y
		offsetMinX = 0
		offsetMinY = 0
		camX = playerX - VIEWPORT_SIZE_X / 2
		camY = playerY - VIEWPORT_SIZE_Y / 2
		if camX > offsetMaxX:
	camX = offsetMaxX
		else if camX < offsetMinX:
	camX = offsetMinX
}*/

bool Game::is_done()
{
	return done;
}

void Game::collision()
{
	player_block_collide(); 
	player_enemy_collide();
}

bool Game::entity_collide_tile(const std::pair<float, float>& entity1, const std::pair<float, float>&entity2)
{
	float left1 = entity1.first - TILE_SIZE/2;
	float right1 = entity1.first + TILE_SIZE/2;
	float top1 = entity1.second + TILE_SIZE/2;
	float bottom1 = entity1.second - TILE_SIZE/2;
	float left2 = entity2.first - TILE_SIZE/2;
	float right2 = entity2.first + TILE_SIZE/2;
	float top2 = entity2.second + TILE_SIZE/2;
	float bottom2 = entity2.second - TILE_SIZE/2;
	if (bottom1 >= top2)
	{
		return false;
	}
	if (top1 <= bottom2)
	{
		return false;
	}
	if (right1 <= left2)
	{
		return false;
	}
	if (left1 >= right2)
	{
		return false;
	}
	return true;
}

void Game::player_enemy_collide()
{
	std::pair<float,float> playercoord(player.x, player.y);
	for (int i = 0; i < enemies.size(); i++)
	{
		std::pair<float, float> enemycoord(enemies[i].x, enemies[i].y);
		if (entity_collide_tile(playercoord, enemycoord))
		{
			enemies.erase(enemies.begin() + i);
			Mix_PlayChannel(-1, sounds[1], 0);
		}
	}
}

void Game::player_block_collide()
{
	player.bottom = false;
	player.top = false;
	player.left = false;
	player.right = false;
	player.y += player.y_velocity * FIXED_TIMESTEP;
	std::pair<float, float> entityGrid(player.x, player.y);
	std::pair<float, float> leveldataworldcoor;
	for (int y = 0; y < LEVEL_HEIGHT; y++)
	{
		for (int x = 0; x < LEVEL_WIDTH; x++)
		{
			if (is_solid(levelData[y][x]))
			{
				std::pair<float, float>leveldataworldcoor(float(x)*TILE_SIZE, float(-y)*TILE_SIZE);
				if (entity_collide_tile(entityGrid, leveldataworldcoor))
				{
					if (player.y > leveldataworldcoor.second)
					{
						player.bottom = true;
						player.y += y_penetration(entityGrid, leveldataworldcoor)+0.00001;
						player.y_velocity = 0;
					}
					if (entityGrid.second < leveldataworldcoor.second)
					{
						player.top = true;
						player.y -= y_penetration(entityGrid, leveldataworldcoor)+0.00001;
						player.y_velocity = 0;
					}
				}
			}
		}
	}

	player.x += player.x_velocity * FIXED_TIMESTEP;
	entityGrid.second = player.y;
	entityGrid.first = player.x;
	for (int y = 0; y < LEVEL_HEIGHT; y++)
	{
		for (int x = 0; x < LEVEL_WIDTH; x++)
		{
			if (is_solid(levelData[y][x]))
			{
				std::pair<float, float>leveldataworldcoor(float(x)*TILE_SIZE, float(-y)*TILE_SIZE);
				if (entity_collide_tile(entityGrid, leveldataworldcoor))
				{
					if (entityGrid.first < leveldataworldcoor.first)
					{
						player.right = true;
						player.x -= (x_penetration(entityGrid, leveldataworldcoor) + 0.00001);
						player.x_velocity = 0;
					}
					if (entityGrid.first > leveldataworldcoor.first)
					{
						player.left = true;
						player.x += (x_penetration(entityGrid, leveldataworldcoor) + 0.00001);
						player.x_velocity = 0;
					}
				}
			}
		}
	}
}

void Game::initialize()
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	glMatrixMode(GL_PROJECTION);
	glOrtho(-1.33, 1.33, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, 800, 600);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	Mix_Chunk *jump = Mix_LoadWAV("jump.wav");
	Mix_Chunk *eat = Mix_LoadWAV("eat.wav");
	Mix_Music *bgm = Mix_LoadMUS("bgm.mp3");
	music.push_back(bgm);
	sounds.push_back(jump);
	sounds.push_back(eat);
	bool done = false; 
	GLuint sheet = LoadTexture("spritesheet_rgba.png");
	Sprite.push_back(sheet);
	read_file();
	//Draw();
	framesPerSecond = 30.0f;
	elapsed = 0.0f;
	lastFrameTicks = 0.0f;
	current_index_player_animation = 0;
	x_friction = 3.0f;
	y_friction = 3.0f;
	gravity = 3.0f;
	earthquake = 0.0f;
	Game::direction = 2.0;
	Mix_PlayMusic(music[0], -1);
}

void Game::player_update()
{
	player.x_velocity = lerp(player.x_velocity, 0.0f, FIXED_TIMESTEP*x_friction);
	//player.y_velocity = lerp(player.y_velocity, 0.0f, FIXED_TIMESTEP*y_friction);
	player.x_velocity += player.x_accelartion * FIXED_TIMESTEP;
	player.y_velocity += -gravity * elapsed;
}

void Game::update()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
		{
			done = true;
			Mix_FreeChunk(sounds[0]);
			Mix_FreeChunk(sounds[1]);
		}
		else if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.scancode == SDL_SCANCODE_UP)
			{
				if (player.bottom)
				{
					player.y_velocity = 2.0;
					Mix_PlayChannel(-1, sounds[0], 0);
				}
			}
		}
	}
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_LEFT])
	{
		player.x_accelartion = -2.0f;
	}
	else if (keys[SDL_SCANCODE_RIGHT])
	{
		player.x_accelartion = 2.0f;
	}
	else
	{
		player.x_accelartion = 0.0f;
	}
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;
	float fixedElapsed = elapsed + timeLeftOver;
	if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS)
	{
		fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
	}
	while (fixedElapsed >= FIXED_TIMESTEP)
	{
		fixedElapsed -= FIXED_TIMESTEP;
		collision();
	}
	timeLeftOver = fixedElapsed;
	player_update();
	/*if (earthquake >= 0.2)
		direction = -2.0;
	else if (earthquake <=-0.2)
		direction = 2.0;
	earthquake += elapsed*direction*2.0;*/
}

void Game::render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	//glLoadIdentity();
	float camera;
	if (player.x < 2.66 / 2.0)
		camera = -2.66 / 2.0;
	else if (player.x >128.0*TILE_SIZE - 1.33)
		camera = -128.0*TILE_SIZE + 1.33;
	else
		camera = -player.x;
	glTranslatef(camera, -player.y /*+ earthquake*/, 0.0);
	/*for (int y = 0; y < LEVEL_HEIGHT; y++)
	{
		for (int x = 0; x < LEVEL_WIDTH; x++) 
		{
			DrawSpriteSheetSpriteTiled(Sprite[0], x,y);
		}
	}*/
	drawLevel(Sprite[0]);
	DrawSpriteSheetSpriteTiledEntity(Sprite[0], "Player",player.x,player.y);
	for (int i = 0; i < enemies.size(); i++)
	{
		DrawSpriteSheetSpriteTiledEntity(Sprite[0], "Enemey", enemies[i].x, enemies[i].y);
	}
	glPopMatrix();
	SDL_GL_SwapWindow(displayWindow);
}


void Game::read_file()
{
	ifstream infile;
	infile.open("tiled.txt");
	string line;
	int counter = 0;
	while (getline(infile, line)) 
	{
		if (counter == 0)
		{
			counter++;
			bool res = readHeader(infile);
			if (!res)
			{
				return;
			}
		}
		else if (line == "[layer]") 
		{
			readLayerData(infile);
		}
		else if (line == "[Entity]") 
		{
			readEntityData(infile);
		}
	}
	infile.close();
}

bool Game::readHeader(std::ifstream &stream)
{
	string line;
	mapWidth = -1;
	mapHeight = -1;
	while (getline(stream, line)) 
	{
		if (line == "")
		{ 
			break;
		}
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "width") 
		{
			mapWidth = atoi(value.c_str());
		}
		else if (key == "height")
		{
			mapHeight = atoi(value.c_str());
		}
	}
	if (mapWidth == -1 || mapHeight == -1) 
	{
		return false;
	}
	else 
	{ // allocate our map data
		levelData = new unsigned char*[mapHeight];
		for (int i = 0; i < mapHeight; i++)
		{
			levelData[i] = new unsigned char[mapWidth];
		}
		return true;
	}
}

bool Game::readLayerData(std::ifstream &stream) 
{
	string line;
	while (getline(stream, line)) 
	{
		if (line == "")
		{ 
			break; 
		}
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "data") 
		{
			for (int y = 0; y < mapHeight; y++)
			{
				getline(stream, line);
				istringstream lineStream(line);
				string tile;
				for (int x = 0; x < mapWidth; x++) 
				{
					getline(lineStream, tile, ',');
					unsigned char val = (unsigned char)atoi(tile.c_str());
					if (val > 0) 
					{
						// be careful, the tiles in this format are indexed from 1 not 0
						levelData[y][x] = (val - 1);
					}
					else 
					{
						levelData[y][x] = 0;
					}
				}
			}
		}
	}
	return true;
}

bool Game::readEntityData(std::ifstream &stream) {
	string line;
	string type;
	while (getline(stream, line))
	{
		if (line == "") 
		{
			break; 
		}
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "type") 
		{
			type = value;
		}
		else if (key == "location") 
		{
			istringstream lineStream(value);
			string xPosition, yPosition;
			getline(lineStream, xPosition, ',');
			getline(lineStream, yPosition, ',');
			float placeX = (float)atoi(xPosition.c_str());
			float placeY = (float)atoi(yPosition.c_str());
			placeEntity(type, placeX, placeY);
		}
	}
	return true;
}

void Game::placeEntity(string& type, float placeX, float placeY)
{
	float worldX = placeX * TILE_SIZE;
	float worldY = -placeY * TILE_SIZE;
	if (type == "Enemy")
	{
		enemies.push_back(Enemy(worldX, worldY));
	}
	else if (type == "Player")
	{
		player = Player(worldX, worldY);
	}
}


int main(int argc, char *argv[])
{
	Game Assignment_5;
	Assignment_5.initialize();
	while (!Assignment_5.is_done())
	{
		Assignment_5.render();
		Assignment_5.update();
	}
	Assignment_5.clearup();
	return 0;
}