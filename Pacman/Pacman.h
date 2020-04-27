#pragma once

// If Windows and not in Debug, this will run without a console window
// You can use this to output information when debugging using cout or cerr
#ifdef WIN32 
	#ifndef _DEBUG
		#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
	#endif
#endif

#define MUNCHIECOUNT 50
#define GHOSTCOUNT 1

// Just need to include main header file
#include "S2D/S2D.h"

// Reduces the amount of typing by including all classes in S2D namespace
using namespace S2D;

//Structure Definitions
struct Player
{
	float speedMultiplier;
	int currentFrameTime;
	int direction;
	int frame;
	int score;
	Rect* sourceRect;
	Texture2D* texture;
	Vector2* pPosition;
	bool dead;
	bool mCollision;
};

struct Enemy
{
	int frameCount;
	Rect* rect;
	Rect* mSourceRect;
	Texture2D* blueTexture;
	Texture2D* invertedTexture;
	Texture2D* mTexture;
	int currentFrameTime;
	int frameTime;
	Vector2* mPosition;
};

struct MovingEnemy
{
	Vector2* position;
	Texture2D* texture;
	Rect* sourceRect;
	int direction;
	float speed;
};

// Declares the Pacman class which inherits from the Game class.
// This allows us to overload the Game class methods to help us
// load content, draw and update our game.
class Pacman : public Game
{
private:

	// Data to represent Pacman
	Player* _pacman;

	// Data to represent Munchie
	Enemy* _munchies[MUNCHIECOUNT];

	// Data to represent Cherry
	Enemy* _cherry;

	// Data to represent Ghost
	MovingEnemy* _ghost[GHOSTCOUNT];

	// Data to represent Sounds
	SoundEffect* _pop;

	// Position for String
	Vector2* _stringPosition;

	// Constant data for Game Variables 
	const float _cPacmanSpeed;
	const int _cPacmanFrameTime;

	// Pause key
	bool _pKeyDown;

	// Reset key
	bool _rKeyDown;

	// Data for Menu
	Texture2D* _menuBackground; 
	Rect* _menuRectangle; 
	Vector2* _menuStringPosition; 
	bool _paused;
	bool _started;
	int _pacmanDirection;

	//Input methods
	void Input(int elapsedTime, Input::KeyboardState* state, Input::MouseState* mouseState);

	//Check methods
	void CheckPaused(Input::KeyboardState* state, Input::Keys pauseKey);
	void CheckViewportCollision();
	void CheckGhostCollisions();
	void CheckMunchieCollisions();

	//Update methods
	void UpdatePacman(int elapsedTime);
	void UpdateMunchie(Enemy* munchie, int elapsedTime);
	void UpdateCherry(Enemy* cherry, int elapsedTime);
	void UpdateGhost(MovingEnemy*, int elapsedTime);


public:
	/// <summary> Constructs the Pacman class. </summary>
	Pacman(int argc, char* argv[]);

	/// <summary> Destroys any data associated with Pacman class. </summary>
	virtual ~Pacman();

	/// <summary> All content should be loaded in this method. </summary>
	void virtual LoadContent();

	/// <summary> Called every frame - update game logic here. </summary>
	void virtual Update(int elapsedTime);

	/// <summary> Called every frame - draw game here. </summary>
	void virtual Draw(int elapsedTime);
};