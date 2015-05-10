
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
#include <random>
#include <math.h>

using namespace std;

#define TEX_ADJUST 3.0f/692.0f

#define MAX_SPEED = 5.0;
// 60 FPS (1.0f/60.0f)
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
#define TILE_SIZE 0.099f //0.0625
#define SPRITE_COUNT_X 30
#define SPRITE_COUNT_Y 30
#define LEVEL_HEIGHT 64
#define LEVEL_WIDTH 64
#define LEVEL_HEIGHT_PVP 26
#define LEVEL_WIDTH_PVP 26
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

class Particle
{
public:
	float x;
	float y;
	float lifetime;
	float x_velocity;
	float y_velocity;
	Particle() :x(0), y(0), lifetime(0), x_velocity(0), y_velocity(0){}
	Particle(float x, float y, float x_velocity, float y_velocity, float life) :x(x), y(y), x_velocity(x_velocity), y_velocity(y_velocity), lifetime(life){}
};

class ParticleEmitter
{
public:
	vector<Particle> parts;
	float x;
	float y;
	float vxrange;
	float vyrange;
	ParticleEmitter(unsigned int particle_number,float px, float py, float vx = 10.0f, float vy = 5.0f)
	{
		x = px;
		y = py;
		vxrange = vx;
		vyrange = vy;
		for (int i = 0; i < particle_number; i++)
		{
			float yv = static_cast <float> (rand()) / static_cast <float> (RAND_MAX/vy);
			float xv = static_cast <float> (rand()) / static_cast <float> (RAND_MAX/vx) - vx/2.0f;
			parts.push_back(Particle(x, y, xv, yv, 3.0));
		}
	}
	ParticleEmitter() :x(0.0),y(0.0){}
	~ParticleEmitter()
	{
		parts.clear();
	}
};

class Entity
{
public:
	Entity(float x, float y, float vx = 0.0, float vy = 0.0, const string& type = "Player") :x(x), y(y), x_accelartion(vx), y_accelartion(vy), x_velocity(0.0),y_velocity(0.0),top(false), bottom(false), right(false), left(false), type(type),dead(0),time(3.0f),health(1){}
	Entity() :x(0.0), y(0.0), y_accelartion(0.0), x_accelartion(0.0), top(false), bottom(false), right(false), left(false){}
	float x;
	float y;	
	float x_accelartion;
	float y_accelartion;
	float width;
	float height;
	float y_velocity;
	float x_velocity;
	bool top;
	bool bottom;
	bool right;
	bool left;
	string type;
	int pickable;
	int hitable;
	int dead;
	int health;
	float time;
};

class Player : public Entity
{
public:
	Player() :Entity(){}
	Player(float x,float y) :Entity(x,y),health(3),wudi(0.0f){}
	int health;
	int direction;
	float wudi;
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
	int level;
	int player_alive;
	int menu_option;
	Player player;
	Player player_2;
	vector<Entity> entities;
	vector<Entity> enemies;
	vector<Entity> items;
	vector<Entity> items_2;
	vector<ParticleEmitter> particles;
	//vector<Entity> drop;
	vector<Entity> icons;
	unsigned int** levelData;
	vector<vector<double>> vertexData;
	vector<vector<float>> texCoordData;
	float elapsed; 
	int current_level;
	int mapHeight;
	int mapWidth;
	float direction;
	float animationElapsed;
	float animationElapsed_2;
	float framesPerSecond;
	float lastFrameTicks;
	float gravity;
	int current_index_player_animation;
	int current_index_player_2_animation;
	float x_friction;
	float y_friction;
	vector<int> level1_switch;
public:
	vector<Mix_Chunk*> sounds;
	vector<Mix_Music*> music;
	void clearup();
	void initialize();
	void render();
	void render_menu();
	void render_level();
	void render_pvp();
	void update_menu();
	void update_level(); 
	void update_pvp();
	void update();
	void action();
	void level1_update();
	void level2_update();
	void level3_update();
	void switchlevel(int level);
	void use_item(int num);
	void use_item_2(int num);
	void use_item_3(int num);
	void drop_item(int num);
	void read_file(const string& file_name);
	bool readHeader(std::ifstream &stream);
	bool readLayerData(std::ifstream &stream);
	bool readEntityData(std::ifstream &stream);
	bool readEnemyData(std::ifstream &stream);
	void placeEntity(string& type, float placeX, float placeY);
	void placeEnemy(string& type, float placeX, float placeY);
	void Draw();
	void DrawText(GLuint fontTexture, string text, float size, float spacing, float r, float g, float b, float a, float x, float y);
	void drawLevel(int spriteTexture);
	void drawLevelpvp(int spriteTexture);
	void drawParticles(int spritetexture, const string& type, int index);
	void DrawSpriteSheetSpriteTiled(int spriteTexture, int x, int y, float rotation, float scale);
	void DrawSpriteSheetSpriteTiledEntity(int spriteTexture,  const string& type,float x, float y, float rotation, float scale);
	void camera();
	bool onetimeitem(const string& type);
	std::pair<int,int> worldToTileCoor(float worldX, float worldY);
	bool is_solid(int tile);
	void player_animation_update();
	void player_2_animation_update();
	void player_update();
	void player_2_update();
	void entity_update();
	void enemy_update();
	void particle_update();
	void icons_update();
	void throwbugs();
	void player_block_collide(int lock);
	void player_2_block_collide();
	void player_enemy_collide();
	void player_entity_collide();
	void player_2_entity_collide();
	void entity_block_collide();
	void entity_entity_collide();
	void enemy_block_collide();
	void collision(int lock);
	void player_dead();
	bool entity_collide_tile(const std::pair<float, float>& entity1, const std::pair<float, float>&entity2);
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


void Game::clearup()
{
	Mix_FreeChunk(sounds[0]);
	Mix_FreeChunk(sounds[1]);
	Mix_FreeChunk(sounds[2]);
	Mix_FreeChunk(sounds[3]);
	Mix_FreeMusic(music[0]);
	SDL_Quit();
}


void Game::DrawText(GLuint fontTexture, string text, float size, float spacing, float r, float g, float b, float a, float x, float y)
{
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float texture_size = 1.0 / 16.0f;
	vector<float> vertexData;
	vector<float> texCoordData;
	vector<float> colorData;
	for (int i = 0; i < text.size(); i++)
	{
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
		colorData.insert(colorData.end(), { r, g, b, a, r, g, b, a, r, g, b, a, r, g, b, a });
		vertexData.insert(vertexData.end(), { ((size + spacing) * i) + (-0.5f * size), 0.5f * size, ((size + spacing) * i) +
			(-0.5f * size), -0.5f * size, ((size + spacing) * i) + (0.5f * size), -0.5f * size, ((size + spacing) * i) + (0.5f * size), 0.5f
			* size });
		texCoordData.insert(texCoordData.end(), { texture_x, texture_y, texture_x, texture_y + texture_size, texture_x +
			texture_size, texture_y + texture_size, texture_x + texture_size, texture_y });
	}
	glLoadIdentity();
	glTranslatef(x, y, 0.0);
	glColorPointer(4, GL_FLOAT, 0, colorData.data());
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, vertexData.data());
	glEnableClientState(GL_VERTEX_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoordData.data());
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_QUADS, 0, text.size() * 4.0);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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
				float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_Y) / (float)SPRITE_COUNT_Y;
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

void Game::drawLevelpvp(int spriteTexture)
{
	vector<GLfloat> vertex;
	vector<GLfloat> uvcoord;
	vertexnumber = 0;
	float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
	float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
	for (int y = 0; y < LEVEL_HEIGHT_PVP; y++)
	{
		for (int x = 0; x < LEVEL_WIDTH_PVP; x++)
		{
			if (levelData[y][x] != 0)
			{
				float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
				float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_Y) / (float)SPRITE_COUNT_Y;
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


void Game::drawParticles(int spriteTexture, const string& type, int index)
{
	float u;
	float v;
	if (type == "Fish")
	{
		u = (float)((410) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
		v = (float)((410) / SPRITE_COUNT_Y) / (float)SPRITE_COUNT_Y;
	}
	vector<GLfloat> vertex;
	vector<GLfloat> uvcoord;
	int vertexnumber_p = 0;
	float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
	float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
	for (int i = 0; i < particles[index].parts.size(); i++)
	{
		vertexnumber_p += 4;
		vertex.insert(vertex.end(), {
			particles[index].parts[i].x, particles[index].parts[i].y,
			particles[index].parts[i].x, (particles[index].parts[i].y) - TILE_SIZE,
			particles[index].parts[i].x + TILE_SIZE, (particles[index].parts[i].y) - TILE_SIZE,
			particles[index].parts[i].x + TILE_SIZE, particles[index].parts[i].y
		});
		uvcoord.insert(uvcoord.end(), { u + TEX_ADJUST, v + TEX_ADJUST,
			u + TEX_ADJUST, v + (spriteHeight)-TEX_ADJUST,
			u + spriteWidth - TEX_ADJUST, v + (spriteHeight)-TEX_ADJUST,
			u + spriteWidth - TEX_ADJUST, v + TEX_ADJUST
		});
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
	glDrawArrays(GL_QUADS, 0, vertexnumber_p);
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
		index = current_index_player_animation;
	else if (type == "Player_2")
		index = current_index_player_2_animation;
	else if (type == "Enemey")
		index = 170;
	else if (type == "NPC")
		index = 110;
	else if (type == "Fish")
		index = 380;
	else if (type == "Wood")
		index = 631;
	else if (type == "Switch_0")
		index = 252;
	else if (type == "Switch_1")
		index = 250;
	else if (type == "Bomb")
		index = 257;
	else if (type == "Block_s")
		index = 158;
	else if (type == "Mushroom")
		index = 458;
	else if (type == "Bee")
		index = 354;
	else if (type == "Bug")
		index = 294;
	else if (type == "Heart")
		index = 373;
	else if (type == "Worm")
		index = 718;
	else if (type == "Lolipop")
		index = 627;
	else if (type == "Loliabr")
		index = 657;
	else if (type == "Spring")
		index = 795;
	else if (type == "Box")
		index = 191;
	else if (type == "Fish_up")
		index = 410;
	else if (type == "Fish_down")
		index = 411;
	else if (type == "Water")
		index = 40;
	else if (type == "Boss")
		index = 49;
	else if (type == "Explode")
		index = 896;

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

bool Game::is_solid(int tile)
{
	if (tile == 122 || tile == 153 || tile == 128 || tile == 151 || tile == 152 ||
		tile == 498 || tile == 499 || tile == 529 || tile == 526 || tile == 191 || tile == 496 ||
		tile == 184 || tile == 182 || tile == 214 ||
		tile == 556 || tile == 586 ||
		tile == 92 || tile == 62 ||
		tile == 481 || tile == 512)
		return true;
	else
		return false;
}

bool Game::onetimeitem(const string& type)
{
	if (type == "Fish"||type == "Lolipop")
		return true;
	else
		return false;
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


bool Game::is_done()
{
	return done;
}

void Game::collision(int lock)
{
	player_block_collide(lock); 
	//player_entity_collide();
	//entity_block_collide();
	//entity_entity_collide();
}

void Game::player_dead()
{
	player_alive = 0;
	player.y_velocity = 4.0;
	player.x_velocity = 0.0;
	player.x_accelartion = 0.0;
}

bool is_enemy(const Entity& tmp)
{
	if (tmp.type == "Bee" || tmp.type == "Worm" || tmp.type == "Bug" || tmp.type == "Fish" ||
		tmp.type == "Fish_up" || tmp.type == "Fish_down" || tmp.type == "Boss")
		return true;
	else
		return false;
}

void Game::entity_entity_collide()
{
	for (int i = 0; i < entities.size(); i++)
	{
		for (int p = 0; p < entities.size(); p++)
		{
			if (p != i && entities[p].hitable == 1 && is_enemy(entities[i]))
			{			
				if (entity_collide_tile(pair<float, float>(entities[p].x, entities[p].y), pair<float, float>(entities[i].x, entities[i].y)))
				{
					if (entities[p].type == "Bomb")
					{
						if (entities[i].type == "Fish")
							entities[i].y_velocity = 2.0f;
						else
							entities[i].y_velocity = 0.5;
						entities[i].health--;
						if (entities[i].health <= 0)
						{
							entities[i].dead = 1;
							if (entities[i].type == "Boss")
							{
								entities.push_back(Entity(entities[i].x, entities[i].y, 0.0, 0.0, "Lolipop"));
							}
							icons.push_back(Entity(entities[i].x, entities[i].y, 0.0, 0.0, "Explode"));
							entities.erase(entities.begin() + p);
						}
						else
						{
							icons.push_back(Entity(entities[i].x, entities[i].y, 0.0, 0.0, "Explode"));
							entities.erase(entities.begin() + p);
						}
						Mix_PlayChannel(-1, sounds[1], 0);
					}
				}
			}
		}
	}
}

void Game::player_entity_collide()
{
	if (player.wudi <= 0.0f)
	{
		for (int i = 0; i < entities.size(); i++)
		{
			if (entities[i].dead == 0)
			{
				if (entity_collide_tile(pair<float, float>(player.x, player.y), pair<float, float>(entities[i].x, entities[i].y)))
				{
					if (entities[i].type == "Bomb" && entities[i].hitable == 1 && entities[i].time <= 2.5)
					{
						player.y_velocity = 2.5;
						entities.erase(entities.begin() + i);
						icons.push_back(Entity(player.x, player.y, 0.0, 0.0, "Explode"));
						Mix_PlayChannel(-1, sounds[3], 0);
						break;
					}
					if (entities[i].type == "Bee" || entities[i].type == "Bug" || entities[i].type == "Worm" ||
						entities[i].type == "Fish" || entities[i].type == "Fish_up" || entities[i].type == "Fish_down")
					{
						player.health--;
						player.x_velocity = float(player.direction) * -1.0f;
						player.y_velocity = 1.0;
						player.wudi = 0.5f;
						Mix_PlayChannel(-1, sounds[2], 0);
					}
				}
			}
		}
	}
}

void Game::player_2_entity_collide()
{
	for (int i = 0; i < entities.size(); i++)
	{
		if (player.wudi <= 0)
		{
			if (entity_collide_tile(pair<float, float>(player.x, player.y), pair<float, float>(entities[i].x, entities[i].y)))
			{
				if (entities[i].type == "Bomb" && entities[i].hitable == 1 && entities[i].time <= 8.0)
				{
					player.health--;
					player.x_velocity = float(player.direction) * -1.0f;
					player.y_velocity = 1.0;
					player.wudi = 0.5f;
					entities.erase(entities.begin() + i);
					icons.push_back(Entity(player.x, player.y, 0.0, 0.0, "Explode"));
					Mix_PlayChannel(-1, sounds[2], 0);
					break;
				}
			}
		}
		if (player_2.wudi <= 0)
		{
			if (entity_collide_tile(pair<float, float>(player_2.x, player_2.y), pair<float, float>(entities[i].x, entities[i].y)))
			{
				if (entities[i].type == "Bomb" && entities[i].hitable == 1 && entities[i].time <= 8.0)
				{
					player_2.health--;
					player_2.x_velocity = float(player_2.direction) * -1.0f;
					player_2.y_velocity = 1.0;
					player_2.wudi = 0.5f;
					entities.erase(entities.begin() + i);
					icons.push_back(Entity(player_2.x, player_2.y, 0.0, 0.0, "Explode"));
					Mix_PlayChannel(-1, sounds[2], 0);
					break;
				}
			}
		}
	}
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



void Game::player_block_collide(int lock)
{
	if (lock != 1)
	{
		player.bottom = false;
	}
	player.top = false;
	player.left = false;
	player.right = false;
	player.y += player.y_velocity * FIXED_TIMESTEP;
	std::pair<float, float> entityGrid(player.x, player.y);
	std::pair<float, float> leveldataworldcoor;
	if (player.dead == 0)
	{
		for (int y = 0; y < mapHeight; y++)
		{
			for (int x = 0; x < mapWidth; x++)
			{
				if (is_solid(levelData[y][x]))
				{
					std::pair<float, float>leveldataworldcoor(float(x)*TILE_SIZE, float(-y)*TILE_SIZE);
					if (entity_collide_tile(entityGrid, leveldataworldcoor))
					{
						if (player.y > leveldataworldcoor.second)
						{
							player.bottom = true;
							if (player.dead == 0)
							{
								player.y += y_penetration(entityGrid, leveldataworldcoor);
								player.y_velocity = 0;
							}
						}
						if (entityGrid.second < leveldataworldcoor.second)
						{
							player.top = true;
							if (player.dead == 0)
							{
								player.y -= y_penetration(entityGrid, leveldataworldcoor);
								player.y_velocity = 0;
							}
						}
					}
				}
			}
		}
	}
	player.x += player.x_velocity * FIXED_TIMESTEP;
	entityGrid.second = player.y;
	entityGrid.first = player.x;
	if (player.dead == 0)
	{
		for (int y = 0; y < mapHeight; y++)
		{
			for (int x = 0; x < mapWidth; x++)
			{
				if (is_solid(levelData[y][x]))
				{
					std::pair<float, float>leveldataworldcoor(float(x)*TILE_SIZE, float(-y)*TILE_SIZE);
					if (entity_collide_tile(entityGrid, leveldataworldcoor))
					{
						if (entityGrid.first < leveldataworldcoor.first)
						{
							player.right = true;
							if (player.dead == 0)
							{
								player.x -= (x_penetration(entityGrid, leveldataworldcoor));
								player.x_velocity = 0;
							}
						}
						if (entityGrid.first > leveldataworldcoor.first)
						{
							player.left = true;
							if (player.dead == 0)
							{
								player.x += (x_penetration(entityGrid, leveldataworldcoor));
								player.x_velocity = 0;
							}
						}
					}
				}
			}
		}
	}
}

void Game::player_2_block_collide()
{
	player_2.bottom = false;
	player_2.top = false;
	player_2.left = false;
	player_2.right = false;
	player_2.y += player_2.y_velocity * FIXED_TIMESTEP;
	std::pair<float, float> entityGrid(player_2.x, player_2.y);
	std::pair<float, float> leveldataworldcoor;
	if (player_2.dead == 0)
	{
		for (int y = 0; y < mapHeight; y++)
		{
			for (int x = 0; x < mapWidth; x++)
			{
				if (is_solid(levelData[y][x]))
				{
					std::pair<float, float>leveldataworldcoor(float(x)*TILE_SIZE, float(-y)*TILE_SIZE);
					if (entity_collide_tile(entityGrid, leveldataworldcoor))
					{
						if (player_2.y > leveldataworldcoor.second)
						{
							player_2.bottom = true;
							if (player_2.dead == 0)
							{
								player_2.y += y_penetration(entityGrid, leveldataworldcoor);
								player_2.y_velocity = 0;
							}
						}
						if (entityGrid.second < leveldataworldcoor.second)
						{
							player_2.top = true;
							if (player_2.dead == 0)
							{
								player_2.y -= y_penetration(entityGrid, leveldataworldcoor);
								player_2.y_velocity = 0;
							}
						}
					}
				}
			}
		}
	}
	player_2.x += player_2.x_velocity * FIXED_TIMESTEP;
	entityGrid.second = player_2.y;
	entityGrid.first = player_2.x;
	if (player_2.dead == 0)
	{
		for (int y = 0; y < mapHeight; y++)
		{
			for (int x = 0; x < mapWidth; x++)
			{
				if (is_solid(levelData[y][x]))
				{
					std::pair<float, float>leveldataworldcoor(float(x)*TILE_SIZE, float(-y)*TILE_SIZE);
					if (entity_collide_tile(entityGrid, leveldataworldcoor))
					{
						if (entityGrid.first < leveldataworldcoor.first)
						{
							player_2.right = true;
							if (player_2.dead == 0)
							{
								player_2.x -= (x_penetration(entityGrid, leveldataworldcoor));
								player_2.x_velocity = 0;
							}
						}
						if (entityGrid.first > leveldataworldcoor.first)
						{
							player_2.left = true;
							if (player_2.dead == 0)
							{
								player_2.x += (x_penetration(entityGrid, leveldataworldcoor));
								player_2.x_velocity = 0;
							}
						}
					}
				}
			}
		}
	}
}

void Game::entity_block_collide()
{
	for (int i = 0; i < entities.size(); i++)
	{				
		entities[i].bottom = false;
		entities[i].top = false;
		entities[i].right = false;
		entities[i].left = false;
		entities[i].y += entities[i].y_velocity * FIXED_TIMESTEP;
		std::pair<float, float> entityGrid(entities[i].x, entities[i].y);
		std::pair<float, float> leveldataworldcoor;
		for (int y = 0; y < mapHeight; y++)
		{
			for (int x = 0; x < mapWidth; x++)
			{
				std::pair<float, float>leveldataworldcoor(float(x)*TILE_SIZE, float(-y)*TILE_SIZE);
				if (is_solid(levelData[y][x])||(entities[i].type == "Box" && levelData[y][x] == 10))
				{
					if (entity_collide_tile(entityGrid, leveldataworldcoor))
					{
						if (entities[i].type == "Box" && levelData[y][x] == 10 && entities[i].hitable == 1)
						{
							levelData[y+1][x] = 191;
							entities.erase(entities.begin() + i);
							return;
						}
						if (entities[i].y > leveldataworldcoor.second)
						{
							if (entities[i].dead == 0)
							{
								entities[i].bottom = true;
								entities[i].y += y_penetration(entityGrid, leveldataworldcoor);
								entities[i].y_velocity = 0;
							}
						}
						if (entityGrid.second < leveldataworldcoor.second)
						{
							if (entities[i].dead == 0)
							{
								entities[i].top = true;
								entities[i].y -= y_penetration(entityGrid, leveldataworldcoor);
								entities[i].y_velocity = 0;
							}
						}
					}
				}
			}
		}

		entities[i].x += entities[i].x_velocity * FIXED_TIMESTEP;
		entityGrid.second = entities[i].y;
		entityGrid.first = entities[i].x;
		for (int y = 0; y < mapHeight; y++)
		{
			for (int x = 0; x < mapWidth; x++)
			{
				if (is_solid(levelData[y][x]))
				{
					std::pair<float, float>leveldataworldcoor(float(x)*TILE_SIZE, float(-y)*TILE_SIZE);
					if (entity_collide_tile(entityGrid, leveldataworldcoor))
					{
						if (entityGrid.first < leveldataworldcoor.first)
						{
							if (entities[i].dead == 0)
							{
								entities[i].right = true;
								entities[i].x -= (x_penetration(entityGrid, leveldataworldcoor));
								entities[i].x_velocity = 0;
								if (entities[i].type == "Bug")
								{
									entities[i].x_accelartion = -entities[i].x_accelartion;
								}
							}
						}
						if (entityGrid.first > leveldataworldcoor.first)
						{
							if (entities[i].dead == 0)
							{
								entities[i].left = true;
								entities[i].x += (x_penetration(entityGrid, leveldataworldcoor));
								entities[i].x_velocity = 0;
								if (entities[i].type == "Bug")
								{
									entities[i].x_accelartion = -entities[i].x_accelartion;
								}
							}
						}
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
	Mix_Chunk *hurt = Mix_LoadWAV("hurt.wav");
	Mix_Chunk *mega = Mix_LoadWAV("mega.wav");
	Mix_Music *bgm = Mix_LoadMUS("bgm.mp3");
	music.push_back(bgm);
	sounds.push_back(jump);
	sounds.push_back(eat);
	sounds.push_back(hurt);
	sounds.push_back(mega);
	bool done = false; 
	GLuint sheet = LoadTexture("spritesheet_rgba.png");
	GLuint font = LoadTexture("font1.png");
	Sprite.push_back(sheet);
	Sprite.push_back(font);
	current_level = 0;
	switchlevel(0);
	menu_option = 1;
	framesPerSecond = 30.0f;
	elapsed = 0.0f;
	lastFrameTicks = 0.0f;
	x_friction = 3.0f;
	y_friction = 3.0f;
	gravity = 2.0f;
	Game::direction = 2.0;
	current_index_player_animation = 28;
	current_index_player_2_animation = 58;
	Mix_PlayMusic(music[0], -1);
}

void Game::particle_update()
{
	for (int p = 0; p < particles.size(); p++)
	{
		for (int i = 0; i < particles[p].parts.size(); i++)
		{
			float vx = particles[p].vxrange;
			float vy = particles[p].vyrange;
			particles[p].parts[i].x += particles[p].parts[i].x_velocity*FIXED_TIMESTEP;
			particles[p].parts[i].y_velocity -= gravity * elapsed;
			particles[p].parts[i].y += particles[p].parts[i].y_velocity*FIXED_TIMESTEP;
			particles[p].parts[i].lifetime -= elapsed;
			if (particles[p].parts[i].lifetime <= 0)
			{
				particles[p].parts.erase(particles[p].parts.begin() + i);
				particles[p].parts.push_back(Particle(particles[p].x, particles[p].y, static_cast <float> (rand()) / static_cast <float> (RAND_MAX / vx) -vx/2.0f, static_cast <float> (rand()) / static_cast <float> (RAND_MAX / vy), static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 5.0f)));
			}
		}
	}
}

void Game::icons_update()
{
	for (int i = 0; i < icons.size(); i++)
	{
		icons[i].time -= elapsed*2.0f;
		if (icons[i].time <= 0.0f)
			icons.erase(icons.begin() + i);
	}
}

void Game::player_update()
{
	player.x_velocity = lerp(player.x_velocity, 0.0f, FIXED_TIMESTEP*x_friction);
	player.x_velocity += player.x_accelartion * FIXED_TIMESTEP;
	player.y_velocity += -gravity * elapsed;
	player_animation_update();
	if (player.wudi >= 0.0f)
		player.wudi -= FIXED_TIMESTEP;
	if (player.health <= 0)
	{
		player.dead = 1;
	}
}

void Game::player_2_update()
{
	player_2.x_velocity = lerp(player_2.x_velocity, 0.0f, FIXED_TIMESTEP*x_friction);
	player_2.x_velocity += player_2.x_accelartion * FIXED_TIMESTEP;
	player_2.y_velocity += -gravity * elapsed;
	player_2_animation_update();
	if (player_2.wudi >= 0.0f)
		player_2.wudi -= FIXED_TIMESTEP;
	if (player_2.health <= 0)
		player_2.dead = 1;
}

void Game::entity_update()
{
	for (int i = 0; i < entities.size(); i++)
	{
		if (entities[i].y<-TILE_SIZE*mapHeight || entities[i].x<-0.05 || entities[i].x>mapWidth*TILE_SIZE)
		{
			entities.erase(entities.begin() + i);
		}
		else if (entities[i].type == "Bug")
		{
			if (entities[i].x < player.x) 
				entities[i].x_accelartion = 1.0;
			else
				entities[i].x_accelartion = -1.0;
			if (entities[i].right || entities[i].left)
				entities[i].y_velocity = 1.0;
			if (levelData[int(-entities[i].y / TILE_SIZE)][int(entities[i].x / TILE_SIZE)] == 40 || levelData[int(-entities[i].y / TILE_SIZE)][int(entities[i].x / TILE_SIZE)] == 578)
				entities[i].y_accelartion = 0.0f;
			else
				entities[i].y_accelartion = -gravity;
			entities[i].y_velocity = lerp(entities[i].y_velocity, 0.0f, 0.0f);
			entities[i].y_velocity += entities[i].y_accelartion*FIXED_TIMESTEP;
			entities[i].x_velocity = lerp(entities[i].x_velocity, 0.0f, FIXED_TIMESTEP*x_friction);
			entities[i].x_velocity += entities[i].x_accelartion * FIXED_TIMESTEP;
		}
		else if (entities[i].type == "Bee" && entities[i].dead ==0)
		{
			if (entities[i].x < player.x)
				entities[i].x_velocity = 0.3;
			else
				entities[i].x_velocity = -0.3;
			if (entities[i].y < player.y)
				entities[i].y_velocity = 0.3;
			else
				entities[i].y_velocity = -0.3;
		}
		else if (entities[i].type == "Fish_up")
		{
			entities[i].y_velocity += -gravity*elapsed;	
			if (entities[i].y_velocity < 0)
				entities[i].type = "Fish_down";
		}
		else if (entities[i].type == "Fish_down")
		{
			entities[i].y_velocity += -gravity*elapsed;
			if (entities[i].y < -6.0)
				entities.erase(entities.begin() + i);

		}
		else if (entities[i].type == "Bomb" && entities[i].hitable == 1)
		{
			if (levelData[int(-entities[i].y / TILE_SIZE)][int(entities[i].x / TILE_SIZE)] == 40 || levelData[int(-entities[i].y / TILE_SIZE)][int(entities[i].x / TILE_SIZE)] == 578)
				entities[i].y_accelartion = 0.0f;
			else
				entities[i].y_accelartion = -gravity;
			entities[i].y_velocity = lerp(entities[i].y_velocity, 0.0f, 0.0f);
			entities[i].y_velocity += entities[i].y_accelartion*FIXED_TIMESTEP;
			entities[i].x_velocity = lerp(entities[i].x_velocity, 0.0f, FIXED_TIMESTEP*x_friction);
			entities[i].x_velocity += entities[i].x_accelartion * FIXED_TIMESTEP;
			entities[i].time -= FIXED_TIMESTEP;
			if (entities[i].time <= 0)
				entities.erase(entities.begin() + i);
		}
		else if (entities[i].type == "Boss" && entities[i].pickable == 1 && entities[i].dead == 0)
		{
			entities[i].time -= elapsed;
			if (fabs(player.x - entities[i].x) <= 1.0f && entities[i].bottom == true)
			{
				entities[i].y_velocity = 1.5f;
				entities[i].x_accelartion = 1.0f*player.direction;
			}
			if (entities[i].x < player.x)
				entities[i].x_accelartion = 1.0f;
			else
				entities[i].x_accelartion = -1.0f;
			if (entities[i].time <= 0.0f)
			{
				Entity tmp(entities[i].x, entities[i].y, 0.0, 0.0, "Bug");
				Entity tmp_1(entities[i].x, entities[i].y, 0.0, 0.0, "Bee");
				tmp.x_velocity = -3.0f * (entities[i].x - player.x) / fabs(entities[i].x - player.x);
				tmp_1.x_velocity = -3.0f * (entities[i].x - player.x) / fabs(entities[i].x - player.x);
				tmp_1.y_velocity = 1.0f;
				entities.push_back(tmp);
				entities.push_back(tmp_1);
				entities[i].time = 10.0f;
			}
			entities[i].y_accelartion = -gravity;
			entities[i].y_velocity += entities[i].y_accelartion*FIXED_TIMESTEP;
			entities[i].x_velocity = lerp(entities[i].x_velocity, 0.0f, FIXED_TIMESTEP*x_friction);
			entities[i].x_velocity += entities[i].x_accelartion * FIXED_TIMESTEP;
		}
		else
		{
			if (levelData[int(-entities[i].y / TILE_SIZE)][int(entities[i].x / TILE_SIZE)] == 40 || levelData[int(-entities[i].y / TILE_SIZE)][int(entities[i].x / TILE_SIZE)] == 578)
				entities[i].y_accelartion = 0.0f;
			else
				entities[i].y_accelartion = -gravity;
			entities[i].y_velocity = lerp(entities[i].y_velocity, 0.0f, 0.0f);
			entities[i].y_velocity += entities[i].y_accelartion*FIXED_TIMESTEP;
			entities[i].x_velocity = lerp(entities[i].x_velocity, 0.0f, FIXED_TIMESTEP*x_friction);
			entities[i].x_velocity += entities[i].x_accelartion * FIXED_TIMESTEP;
		}
	}
}

void Game::player_animation_update()
{
	animationElapsed += player.x_accelartion;
	if (fabs(animationElapsed) > 1.0 / framesPerSecond)
	{
		if (current_index_player_animation == 28)
			current_index_player_animation = 29;
		else
			current_index_player_animation = 28;
		animationElapsed = 0.0;
		if (fabs(player.x_velocity) < 0.1)
			current_index_player_animation = 28;
	}
}

void Game::player_2_animation_update()
{
	animationElapsed_2 += player_2.x_accelartion;
	if (fabs(animationElapsed_2) > 1.0 / framesPerSecond)
	{
		if (current_index_player_2_animation == 58)
			current_index_player_2_animation = 59;
		else
			current_index_player_2_animation = 58;
		animationElapsed_2 = 0.0;
		if (fabs(player_2.x_velocity) < 0.1)
			current_index_player_2_animation = 58;
	}
}

void Game::update()
{
	if (current_level == 0)
		update_menu();
	else if (current_level >= 4)
		update_pvp();
	else
		update_level();
}

void Game::update_menu()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
		{
			done = true;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.scancode == SDL_SCANCODE_UP)
			{
				if (menu_option == 1)
					menu_option = -1;
				else if (menu_option == -1)
					menu_option = 6;
				else
					menu_option = 1;
					
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN)
			{
				if (menu_option == 1)
					menu_option = 6;
				else if (menu_option == -1)
					menu_option = 1;
				else
					menu_option = -1;
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_Z)
			{
				switchlevel(menu_option);
			}
		}
	}
}

void Game::update_pvp()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
		{
			done = true;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.scancode == SDL_SCANCODE_UP)
			{
				if (player.bottom)
				{
					player.y_velocity = 1.0;
					Mix_PlayChannel(-1, sounds[0], 0);
				}
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_W)
			{
				if (player_2.bottom)
				{
					player_2.y_velocity = 1.0;
					Mix_PlayChannel(-1, sounds[0], 0);
				}
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
			{
				current_level = 0;
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_1)
			{
				use_item_3(1);
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_Q)
			{
				use_item_2(1);
			}
		}
	}
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_LEFT])
	{
		player.x_accelartion = -2.0f;
		player.direction = -1;
	}
	else if (keys[SDL_SCANCODE_RIGHT])
	{
		player.x_accelartion = 2.0f;
		player.direction = 1;
	}
	else
	{
		player.x_accelartion = 0.0f;
	}
	const Uint8 *keys_2 = SDL_GetKeyboardState(NULL);
	if (keys_2[SDL_SCANCODE_A])
	{
		player_2.x_accelartion = -2.0f;
		player_2.direction = -1;
	}
	else if (keys_2[SDL_SCANCODE_D])
	{
		player_2.x_accelartion = 2.0f;
		player_2.direction = 1;
	}
	else
	{
		player_2.x_accelartion = 0.0f;
	}
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;
	float fixedElapsed = elapsed + timeLeftOver;
	if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS)
	{
		fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
	}
	int inloopjumplock = 0;
	while (fixedElapsed >= FIXED_TIMESTEP)
	{
		fixedElapsed -= FIXED_TIMESTEP;
		player_block_collide(inloopjumplock);
		player_2_block_collide();
		if (player.bottom == 1)
			inloopjumplock = 1;
	}
	player_update();
	entity_block_collide();
	entity_update();
	icons_update();
	particle_update();
	player_2_update();
	player_2_entity_collide();
	timeLeftOver = fixedElapsed;
	if (player.y >= 0.0 || player.y <= -2.6)
	{
		if (current_level>4)
			switchlevel(--current_level);
		else if (particles.size() == 0)
		{
			particles.push_back(ParticleEmitter(100, 13 * TILE_SIZE, -13 * TILE_SIZE, 2.0f, 1.0f));
		}
	}
	if (player_2.y >= 0.0 || player_2.y <= -2.6)
	{
		if (current_level < 8)
			switchlevel(++current_level);
		else if (particles.size() == 0)
		{
			particles.push_back(ParticleEmitter(100, 13 * TILE_SIZE, -13 * TILE_SIZE, 2.0f, 1.0f));
		}
	}
}

void Game::update_level()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
		{
			done = true;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.scancode == SDL_SCANCODE_UP)
			{
				if (player.bottom)
				{
					player.y_velocity = 1.0;
					Mix_PlayChannel(-1, sounds[0], 0);
				}
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
			{
				current_level = 0;
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_Z)
			{
				action();
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_1)
			{
				use_item(1);
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_2)
			{
				use_item(2);
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_3)
			{
				use_item(3);
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_Q)
			{
				drop_item(0);
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_W)
			{
				drop_item(1);
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_E)
			{
				drop_item(2);
			}
		}
	}
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_LEFT])
	{
		player.x_accelartion = -2.0f;
		player.direction = -1;
	}
	else if (keys[SDL_SCANCODE_RIGHT])
	{
		player.x_accelartion = 2.0f;
		player.direction = 1;
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
	int inloopjumplock = 0;
	while (fixedElapsed >= FIXED_TIMESTEP)
	{
		fixedElapsed -= FIXED_TIMESTEP;
		collision(inloopjumplock);
		if (player.bottom == 1)
			inloopjumplock = 1;
	}
	entity_block_collide();
	entity_entity_collide();
	player_entity_collide();
	inloopjumplock = 0;
	timeLeftOver = fixedElapsed;
	player_update();
	entity_update();
	particle_update();
	icons_update();
	if (current_level == 1)
		level1_update();
	else if (current_level == 2)
		level2_update();
	else if (current_level == 3)
		level3_update();
	if (player.y >= 0.0 || player.y <=-6.4)
		switchlevel(current_level);
}

void Game::level1_update()
{
	for (int i = 0; i < entities.size(); i++)
	{
		if (entities[i].type == "Switch_0")
		{
			levelData[59][22] = 0;
			levelData[59][23] = 0;
		}
		else if (entities[i].type == "Switch_1")
		{
			levelData[59][22] = 128;
			levelData[59][23] = 128;
		}
	}
}

void Game::level2_update()
{
	int fish = 0;
	for (int i = 0; i < entities.size(); i++)
	{
		if (entities[i].type == "Switch_1")
		{
			for (int i = 50; i < 54; i++)
			{
				levelData[i][28] = 0;
				levelData[i][27] = 0;
			}
		}
		else if (entities[i].type == "Switch_0")
		{
			for (int i = 50; i < 54; i++)
			{
				levelData[i][28] = 526;
				levelData[i][27] = 526;
			}
		}
		if (entities[i].type == "Fish_up" || entities[i].type == "Fish_down")
			fish = 1;
	}
	if (fish == 0)
	{
		if (player.x > 1.782 && player.x < 2.3)
		{
			Entity tmp(2.05, -6.0f, 0.0f, 3.0f, "Fish_up");
			tmp.y_velocity = 3.0;
			entities.push_back(tmp);
		}
	}
}

void Game::level3_update()
{
	for (int i = 0; i < entities.size(); i++)
	{
		if (entities[i].type == "Switch_1" && particles.empty())
		{
			particles.push_back(ParticleEmitter(100, 29 * TILE_SIZE, -13 * TILE_SIZE));
		}
		else if (entities[i].type == "Switch_0")
		{
			particles.clear();
		}
	}
}

void Game::render()
{
	if (current_level == 0)
		render_menu();
	else if (current_level >= 4)
		render_pvp();
	else
		render_level();
}

void Game::render_menu()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	DrawText(Sprite[1], "FISH BOMBER", 0.2, 0.003, 1.0, 1.0, 0.0, 1.0, -1.0, 0.5);
	DrawText(Sprite[1], "START", 0.15, 0.003, 0.0, 1.0, 1.0, 1.0, -0.3, 0.0);
	DrawText(Sprite[1], "FIGHT", 0.15, 0.003, 0.0, 1.0, 1.0, 1.0, -0.3, -0.3);
	DrawText(Sprite[1], "EXIT", 0.15, 0.003, 0.0, 1.0, 1.0, 1.0, -0.3, -0.6);
	glLoadIdentity();
	float menu_position;
	if (menu_option == 1)
		menu_position = -0.1 + 0.15;
	else if (menu_option == 6)
		menu_position = -0.1 - 0.15;
	else
		menu_position = -0.1 - 0.45;
	DrawSpriteSheetSpriteTiledEntity(Sprite[0], "Bomb", -0.5, menu_position);
	SDL_GL_SwapWindow(displayWindow);
}

void Game::render_pvp()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(-TILE_SIZE*LEVEL_WIDTH_PVP*0.5f,TILE_SIZE*LEVEL_HEIGHT_PVP*(0.5f), 0.0);
	drawLevelpvp(Sprite[0]);
	DrawSpriteSheetSpriteTiledEntity(Sprite[0], "Player", player.x, player.y);
	DrawSpriteSheetSpriteTiledEntity(Sprite[0], "Player_2", player_2.x, player_2.y);
	for (int i = 0; i < entities.size(); i++)
	{
		DrawSpriteSheetSpriteTiledEntity(Sprite[0], entities[i].type, entities[i].x, entities[i].y);
	}
	for (int i = 0; i < icons.size(); i++)
	{
		DrawSpriteSheetSpriteTiledEntity(Sprite[0], icons[i].type, icons[i].x, icons[i].y);
	}
	for (int i = 0; i < particles.size(); i++)
	{
		drawParticles(Sprite[0], "Fish", 0);
	}
	glPopMatrix();
	for (int i = 0; i < player.health; i++)
	{
		DrawSpriteSheetSpriteTiledEntity(Sprite[0], "Heart", -1.2 + i*0.1, 1.0);
	}	for (int i = 0; i < player_2.health; i++)
	{
		DrawSpriteSheetSpriteTiledEntity(Sprite[0], "Heart", 1.2 - i*0.1, 1.0);
	}
	SDL_GL_SwapWindow(displayWindow);
}

void Game::render_level()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	float camera_x;
	if (player.x < 2.66 / 2.0)
		camera_x = -2.66 / 2.0;
	else if (player.x >64.0*TILE_SIZE - 1.33)
		camera_x = -64.0*TILE_SIZE + 1.33;
	else
		camera_x = -player.x;
	float camera_y;
	if (player.y > -2.0 / 2.0)
		camera_y = 2.0 / 2.0;
	else if (player.y < -64.0*TILE_SIZE + 1.0)
		camera_y = 64.0*TILE_SIZE - 1.0;
	else
		camera_y = -player.y;
	glTranslatef(camera_x, camera_y, 0.0);
	drawLevel(Sprite[0]);
	for (int i = 0; i < particles.size(); i++)
	{
		drawParticles(Sprite[0], "Fish", 0);
	}
	DrawSpriteSheetSpriteTiledEntity(Sprite[0], "Player",player.x,player.y);
	for (int i = 0; i < entities.size(); i++)
	{
		DrawSpriteSheetSpriteTiledEntity(Sprite[0], entities[i].type, entities[i].x, entities[i].y);
	}
	for (int i = 0; i < icons.size(); i++)
	{
		DrawSpriteSheetSpriteTiledEntity(Sprite[0], icons[i].type, icons[i].x,icons[i].y);
	}
	glPopMatrix();
	for (int i = 0; i < items.size(); i++)
	{
		DrawSpriteSheetSpriteTiledEntity(Sprite[0], items[i].type, -1.33 + i*0.2, 1.0);
	}
	for (int i = 0; i < player.health; i++)
	{
		DrawSpriteSheetSpriteTiledEntity(Sprite[0], "Heart", 1.2 - i*0.1, 1.0);
	}
	SDL_GL_SwapWindow(displayWindow);
}

void Game::action()
{
	for (int i = 0; i < entities.size(); i++)
	{
		if (entity_collide_tile(pair<float, float>(player.x, player.y), pair<float, float>(entities[i].x, entities[i].y)))
		{
			if (entities[i].type == "NPC")
			{
				if (current_level == 1)
				{
					int has = 0;
					for (int p = 0; p < items.size(); p++)
					{
						if (items[p].type == "Fish")
						{
							has = 1;
							break;
						}
					}
					if (has == 0)
					{
						Entity tmp(entities[i].x, entities[i].y + 0.1, 0, 0, "Fish");
						icons.push_back(tmp);
					}
					else
					{
						switchlevel(2);
					}
				}
				else if (current_level == 2)
				{
					int has = 0;
					for (int p = 0; p < items.size(); p++)
					{
						if (items[p].type == "Lolipop")
						{
							has = 1;
							break;
						}
					}
					if (has == 0)
					{
						Entity tmp(entities[i].x, entities[i].y + 0.1, 0, 0, "Lolipop");
						icons.push_back(tmp);
					}
					else
					{
						switchlevel(3);
					}
				}
				else if (current_level == 3)
				{
					int has = 0;
					for (int p = 0; p < items.size(); p++)
					{
						if (items[p].type == "Lolipop")
						{
							has = 1;
							break;
						}
					}
					if (has == 0)
					{
						Entity tmp(entities[i].x, entities[i].y + 0.1, 0, 0, "Lolipop");
						icons.push_back(tmp);
						for (int i = 0; i < entities.size(); i++)
						{
							if (entities[i].type == "Boss")
							{
								entities[i].pickable = 1;
							}
						}
					}
					else
					{
						levelData[29][53] = 184;
						levelData[29][52] = 184;
						levelData[30][50] = 184;
						levelData[20][51] = 183;
					}
				}
			}
			else if (entities[i].type == "Switch_0")
			{
				entities[i].type = "Switch_1";
			}
			else if (entities[i].type == "Switch_1")
			{
				entities[i].type = "Switch_0";
			}
			else
			{
				int has = 0;
				for (int p = 0; p < items.size(); p++)
				{
					if (items[p].type == entities[i].type)
					{
						has = 1;
						break;
					}
				}
				if (has == 0)
				{
					items.push_back(Entity(-1.33 + items.size()*0.2, -1.0, 0.0, 0.0, entities[i].type));
					if (onetimeitem(entities[i].type))
						entities.erase(entities.begin() + i);
				}
			}
		}
	}
}

void Game::use_item_2(int num)
{
	if (num <= items_2.size())
	{
		Entity tmp(player_2.x, player_2.y, 0.0, 0.0, items[num - 1].type);
		tmp.hitable = 1;
		tmp.x_velocity = 1.0 * player_2.direction;
		tmp.time = 10.0f;
		entities.push_back(tmp);
	}
}

void Game::use_item_3(int num)
{
	if (num <= items.size())
	{
		Entity tmp(player.x, player.y, 0.0, 0.0, items[num - 1].type);
		tmp.hitable = 1;
		tmp.x_velocity = 1.0 * player.direction;
		tmp.time = 10.0f;
		entities.push_back(tmp);
	}
}

void Game::use_item(int num)
{
	if (num <= items.size())
	{
		Entity tmp(player.x, player.y, 0.0, 0.0, items[num - 1].type);
		tmp.hitable = 1;
		tmp.x_velocity = 2.0 * player.direction;
		entities.push_back(tmp);
	}
}

void Game::drop_item(int num)
{
	if (num < items.size())
	{
		items.erase(items.begin() + num);
	}
}

void Game::switchlevel(int level)
{
	if (level == -1)
	{
		done = true;
		return;
	}
	else
		current_level = level;
	for (int y = 0; y < mapHeight; y++)
	{
		delete levelData[y];
		levelData[y] = NULL;
	}
	player_alive = 1;
	entities.clear();
	particles.clear();
	icons.clear();
	items.clear();
	//enemies.clear();
	if (level == 1)
	{
		read_file("level1.txt");
	}
	else if (level == 2)
	{
		read_file("level2.txt");
	}
	else if (level == 3)
	{
		read_file("level3.txt");
	}
	else if (level == 4)
	{
		read_file("level_pvp-2.txt");
		items.push_back(Entity(-1.33 + items.size()*0.2, -1.0, 0.0, 0.0, "Bomb"));
		items_2.push_back(Entity(-1.33 + items.size()*0.2, -1.0, 0.0, 0.0, "Bomb"));
	}
	else if (level == 5)
	{
		read_file("level_pvp-1.txt");
		items.push_back(Entity(-1.33 + items.size()*0.2, -1.0, 0.0, 0.0, "Bomb"));
		items_2.push_back(Entity(-1.33 + items.size()*0.2, -1.0, 0.0, 0.0, "Bomb"));
	}
	else if (level == 6)
	{
		read_file("level_pvp.txt");
		items.push_back(Entity(-1.33 + items.size()*0.2, -1.0, 0.0, 0.0, "Bomb"));
		items_2.push_back(Entity(-1.33 + items.size()*0.2, -1.0, 0.0, 0.0, "Bomb"));
	}
	else if (level == 7)
	{
		read_file("level_pvp+1.txt");
		items.push_back(Entity(-1.33 + items.size()*0.2, -1.0, 0.0, 0.0, "Bomb"));
		items_2.push_back(Entity(-1.33 + items.size()*0.2, -1.0, 0.0, 0.0, "Bomb"));
	}
	else if (level == 8)
	{
		read_file("level_pvp+2.txt");
		items.push_back(Entity(-1.33 + items.size()*0.2, -1.0, 0.0, 0.0, "Bomb"));
		items_2.push_back(Entity(-1.33 + items.size()*0.2, -1.0, 0.0, 0.0, "Bomb"));
	}
}


void Game::read_file(const string& file_name)
{
	ifstream infile;
	infile.open(file_name);
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
		levelData = new unsigned int*[mapHeight];
		for (int i = 0; i < mapHeight; i++)
		{
			levelData[i] = new unsigned int[mapWidth];
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
					int val = (int)atoi(tile.c_str());
					if (val > 0) 
					{
						// be careful, the tiles in this format are indexed from 1 not 0
						levelData[y][x] = val- 1;
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

bool Game::readEnemyData(std::ifstream &stream) {
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
			placeEnemy(type, placeX, placeY);
		}
	}
	return true;
}


void Game::placeEntity(string& type, float placeX, float placeY)
{
	float worldX = placeX * TILE_SIZE;
	float worldY = -placeY * TILE_SIZE;
	if (type == "Player")
	{
		player = Player(worldX, worldY);
	}
	else if (type == "Player_2")
	{
		player_2 = Player(worldX, worldY);
	}
	else
	{
		Entity tmp(worldX, worldY, 0.0, 0.0, type);
		if (type == "Bug")
			tmp.x_accelartion = 1.0;
		if (type == "Bee")
		{
			tmp.x_accelartion = 1.0;
			tmp.y_velocity = 1.0;
		}
		if (type == "Boss")
		{
			tmp.health = 10;
		}
		entities.push_back(tmp);
	}
}

void Game::placeEnemy(string& type, float placeX, float placeY)
{
	float worldX = placeX * TILE_SIZE;
	float worldY = -placeY * TILE_SIZE;
	Entity tmp(worldX, worldY, 0.0, 0.0, type);
	if (type == "Bug")
	{
		tmp.x_accelartion = 1.0;
	}
	enemies.push_back(tmp);
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