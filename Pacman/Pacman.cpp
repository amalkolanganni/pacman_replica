#include "Pacman.h"
#include <time.h>
#include <sstream>
#include <iostream>

//Constructor Method
Pacman::Pacman(int argc, char* argv[]) : Game(argc, argv), _cPacmanSpeed(0.1f), _cPacmanFrameTime(250)
{
	//Local variables
	int i;
	
	srand(time(NULL));

	_paused = false;
	_pKeyDown = false;
	_rKeyDown = false;

	//Initialise munchies
	for (i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchies[i] = new Enemy();
		_munchies[i]->currentFrameTime = 0;
		_munchies[i]->frameCount = rand() % 1;
		_munchies[i]->frameTime = rand() % 500 + 50;
	}

	//Initialise ghosts
	_ghost[0] = new MovingEnemy();
	_ghost[0]->direction = 0;
	_ghost[0]->speed = 0.2f;

	//Initialise member variables
	_pacman = new Player();
	_pacman->dead = false;
	_pacman->mCollision = false;

	//Initialise cherry
	_cherry = new Enemy();

	//Initialise sounds
	_pop = new SoundEffect();

	//Initialise important Game aspects
	Audio::Initialise();
	Graphics::Initialise(argc, argv, this, 800, 700, false, 25, 25, "Pacman", 60);
	Input::Initialise();

	// Start the Game Loop - This calls Update and Draw in game loop
	Graphics::StartGameLoop();
}

//Destructor Method
Pacman::~Pacman()
{
	delete _pacman->texture;
	delete _pacman->sourceRect;
	delete _pacman->pPosition;
	delete _pacman;

	delete _cherry->mTexture;
	delete _cherry->mSourceRect;
	delete _cherry;

	delete _ghost[0]->texture;
	delete _ghost[0]->position;
	delete _ghost[0]->sourceRect;
	delete _ghost;

	delete _pop;

	//Clean up the texture
	delete _munchies[0]->mTexture;	

	for (int i = 0; i < MUNCHIECOUNT; i++)
	{
		delete _munchies[i]->mPosition;
		delete _munchies[i]->rect;
		delete _munchies[i];
	}

	delete[] _munchies;
}

void Pacman::LoadContent()
{
	//Local variables
	int i;

	// Load Pacman
	_pacman->texture = new Texture2D();
	_pacman->texture->Load("Textures/Pacman.tga", false);
	_pacman->pPosition = new Vector2(300.0f, 400.0f);
	_pacman->sourceRect = new Rect(0.0f, 0.0f, 32, 32);
	_pacman->direction = 0;
	_pacman->currentFrameTime = 0;
	_pacman->frame = 0;
	_pacman->speedMultiplier = 1.0f;
	_pacman->score = 0;

	// Load Ghosts
	_ghost[0]->texture = new Texture2D();
	_ghost[0]->texture->Load("Textures/GhostBlue.png", false);
	_ghost[0]->position = new Vector2(rand() % Graphics::GetViewportWidth(), rand() % Graphics::GetViewportHeight());
	_ghost[0]->sourceRect = new Rect(0.0f, 0.0f, 20, 20);

	// Load Sounds
	_pop->Load("Sounds/pop.wav");

	// Load Munchie
	Texture2D* munchieTex = new Texture2D();
	munchieTex->Load("Textures/Munchie.tga", true);

	for (i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchies[i]->mTexture = munchieTex;
		_munchies[i]->blueTexture = new Texture2D();
		_munchies[i]->blueTexture->Load("Textures/Munchie.tga", true);
		_munchies[i]->invertedTexture = new Texture2D();
		_munchies[i]->invertedTexture->Load("Textures/MunchieInverted.tga", true);
		_munchies[i]->rect = new Rect(100.0f, 450.0f, 12, 12);
		_munchies[i]->currentFrameTime = 0;
		_munchies[i]->frameTime = 500;
		_munchies[i]->frameCount = 0;
		_munchies[i]->mPosition = new Vector2(rand() % Graphics::GetViewportWidth(), rand() % Graphics::GetViewportHeight());
	}

	// Load Cherry
	_cherry->mTexture = new Texture2D();
	_cherry->mTexture->Load("Textures/Cherry.png", true);
	_cherry->mPosition = new Vector2(rand() % Graphics::GetViewportWidth(), rand() % Graphics::GetViewportHeight());
	_cherry->mSourceRect = new Rect(100.0f, 450.0f, 31, 31);
	_cherry->currentFrameTime = 0;
	_cherry->frameTime = 500;
	_cherry->frameCount = 0;

	// Set string position
	_stringPosition = new Vector2(10.0f, 25.0f);

	// Set Menu Paramters

	_menuBackground = new Texture2D();
	_menuBackground->Load("Textures/Transparency.png", false);
	_menuRectangle = new Rect(0.0f, 0.0f, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
	_menuStringPosition = new Vector2(Graphics::GetViewportWidth() / 2.0f, Graphics::GetViewportHeight() / 2.0f);

}


//Call functions in the update function
void Pacman::Update(int elapsedTime)
{
	// Gets the current state of the keyboard
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();

	// Gets the current state of the mouse
	Input::MouseState* mousestate = Input::Mouse::GetState();

	if (!_started)
	{
		//Check for start
		if (keyboardState->IsKeyDown(Input::Keys::RETURN))
		{
			_started = true;
		}
	}

	else
	{
		CheckPaused(keyboardState, Input::Keys::P);

		if (!_paused)
		{
			Input(elapsedTime, keyboardState, mousestate);
			UpdatePacman(elapsedTime);
			UpdateGhost(_ghost[0], elapsedTime);
			CheckGhostCollisions();
			CheckMunchieCollisions();
			CheckViewportCollision();

			for (int i = 0; i < MUNCHIECOUNT; i++) 
			{ 
				UpdateMunchie(_munchies[i], elapsedTime);
			}
		}	
	}
}

void Pacman::Input(int elapsedTime, Input::KeyboardState* state, Input::MouseState* mouseState)
{
	// Gets the current state of the keyboard
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();

	float pacmanSpeed = _cPacmanSpeed * elapsedTime * _pacman->speedMultiplier;

	// Mouse Interaction
	if (mouseState->LeftButton == Input::ButtonState::PRESSED)
	{
		_cherry->mPosition->X = mouseState->X;
		_cherry->mPosition->Y = mouseState->Y;
	}

	// Speed Multiplier
	if (keyboardState->IsKeyDown(Input::Keys::LEFTSHIFT))
	{
		// Apply Multiplier
		_pacman->speedMultiplier = 2.0f;
	}
	else
	{
		// Reset Multiplier
		_pacman->speedMultiplier = 1.0f;
	}

	// Checks if D key is pressed
	if (keyboardState->IsKeyDown(Input::Keys::D))
	{
		_pacman->pPosition->X += pacmanSpeed; //Moves Pacman across X axis
		_pacmanDirection = 0;
	}

	// Checks if A key is pressed
	else if (keyboardState->IsKeyDown(Input::Keys::A))
	{
		_pacman->pPosition->X += -pacmanSpeed; //Moves Pacman across X axis
		_pacmanDirection = 2;
	}

	// Checks if W key is pressed
	else if (keyboardState->IsKeyDown(Input::Keys::W))
	{
		_pacman->pPosition->Y += -pacmanSpeed; //Moves Pacman across Y axis
		_pacmanDirection = 3;
	}

	// Checks if S key is pressed
	else if (keyboardState->IsKeyDown(Input::Keys::S))
	{
		_pacman->pPosition->Y += pacmanSpeed; //Moves Pacman across Y axis
		_pacmanDirection = 1;
	}

	// Checks if R key is pressed
	else if (keyboardState->IsKeyDown(Input::Keys::R) && !_rKeyDown)
	{
		_cherry->mPosition = new Vector2(rand() % Graphics::GetViewportWidth(), rand() % Graphics::GetViewportHeight());
		_pKeyDown = true;
	}
}
	
void Pacman::CheckPaused(Input::KeyboardState* state, Input::Keys pauseKey)
{
	// Gets the current state of the keyboard
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();

	// Pause control
	if (keyboardState->IsKeyDown(Input::Keys::P) && !_pKeyDown)
	{
		_pKeyDown = true;
		_paused = !_paused;
	}

	if (keyboardState->IsKeyUp(Input::Keys::P))
		_pKeyDown = false;
}

void Pacman::UpdatePacman(int elapsedTime)
{

	_pacman->currentFrameTime += elapsedTime;

	if (_pacman->currentFrameTime > _cPacmanFrameTime)
	{
		_pacman->frame++;

		if (_pacman->frame >= 2)
			_pacman->frame = 0;

		_pacman->currentFrameTime = 0;
	}

	_pacman->sourceRect->Y = _pacman->sourceRect->Height * _pacmanDirection; //Changes pacman sprite direction
	_pacman->sourceRect->X = _pacman->sourceRect->Width * _pacman->frame;
}

void Pacman::UpdateGhost(MovingEnemy* ghost, int elapsedtime)
{
	if (ghost->direction == 0) //Moves Right
	{
		ghost->position->X += ghost->speed * elapsedtime;
	}
	else if (ghost->direction == 1) //Moves Left
	{
		ghost->position->X -= ghost->speed * elapsedtime;
	}

	if (ghost->position->X + ghost->sourceRect->Width >= Graphics::GetViewportHeight()) //Hits Right Edge
	{
		ghost->direction = 1; //Change direction
	}
	else if (ghost->position->X <= 0) //Hits Left Edge
	{
		ghost->direction = 0; //Change direction
	}
}

void Pacman::CheckGhostCollisions()
{
	// Local Variables
	int i = 0;
	int bottom1 = _pacman->pPosition->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _pacman->pPosition->X;
	int left2 = 0;
	int right1 = _pacman->pPosition->X + _pacman->sourceRect->Width;
	int right2 = 0;
	int top1 = _pacman->pPosition->Y;
	int top2 = 0;

	for (i = 0; i < GHOSTCOUNT; i++)
	{
		// Populate variables with Ghost data
		bottom2 = _ghost[i]->position->Y + _ghost[i]->sourceRect->Height;
		left2 = _ghost[i]->position->X;
		right2 = _ghost[i]->position->X + _ghost[i]->sourceRect->Width;
		top2 = _ghost[i]->position->Y;

		if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2)
			&& (left1 < right2))
		{
			_pacman->dead = true;
			i = GHOSTCOUNT;
		}
	}
}

void Pacman::CheckMunchieCollisions()
{
	// Local Variables
	int i = 0;
	int bottom1 = _pacman->pPosition->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _pacman->pPosition->X;
	int left2 = 0;
	int right1 = _pacman->pPosition->X + _pacman->sourceRect->Width;
	int right2 = 0;
	int top1 = _pacman->pPosition->Y;
	int top2 = 0;

	for (i = 0; i < MUNCHIECOUNT; i++)
	{
		// Populate variables with Munchie data
		bottom2 = _munchies[i]->mPosition->Y + _munchies[i]->rect->Height;
		left2 = _munchies[i]->mPosition->X;
		right2 = _munchies[i]->mPosition->X + _munchies[i]->rect->Width;
		top2 = _munchies[i]->mPosition->Y;

		if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2)
			&& (left1 < right2))
		{
			Audio::Play(_pop);
			_pacman->score = _pacman->score + 100;
			_pacman->mCollision = true;
			i = MUNCHIECOUNT;
		}
	}
}

void Pacman::CheckViewportCollision()
{
	// Checks if Pacman is trying to disappear
	if (_pacman->pPosition->X > Graphics::GetViewportWidth()) //Gets current width
	{
		// Pacman hits right wall - reset his position
		_pacman->pPosition->X = -_pacman->sourceRect->Width;
	}

	// Checks if Pacman is trying to disappear
	if (_pacman->pPosition->X > Graphics::GetViewportWidth()) //Gets current width
	{
		// Pacman hits left wall - reset his position
		_pacman->pPosition->X = _pacman->sourceRect->Width;
	}

	// Checks if Pacman is trying to disappear
	if (_pacman->pPosition->Y > Graphics::GetViewportHeight()) //Gets current height
	{
		// Pacman hits top wall - reset his position
		_pacman->pPosition->Y = -_pacman->sourceRect->Height;
	}

	// Checks if Pacman is trying to disappear
	if (_pacman->pPosition->Y > Graphics::GetViewportHeight()) //Gets current height
	{
		// Pacman hits bottom wall - reset his position
		_pacman->pPosition->Y = _pacman->sourceRect->Height;
	}

}


void Pacman::UpdateMunchie(Enemy* munchie, int elapsedTime)
{

	for (int i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchies[i]->currentFrameTime += elapsedTime;

		if (_munchies[i]->currentFrameTime > _munchies[i]->frameTime)
		{
			_munchies[i]->frameCount++;

			if (_munchies[i]->frameCount >= 2)
				_munchies[i]->frameCount = 0;

			_munchies[i]->currentFrameTime = 0;
		}
	}
}

void Pacman::UpdateCherry(Enemy* cherry, int elapsedTime)
{
	_cherry->currentFrameTime += elapsedTime;

	if (_cherry->currentFrameTime > _cherry->frameTime)
	{
		_cherry->frameCount++;

		if (_cherry->frameCount >= 2)
			_cherry->frameCount = 0;

		_cherry->currentFrameTime = 0;
	}
}

void Pacman::Draw(int elapsedTime)
{
	
	// Allows us to easily create a string
	std::stringstream stream;
	stream << "Pacman X: " << _pacman->pPosition->X << " Y: " << _pacman->pPosition->Y << " Score: " << _pacman->score;

	SpriteBatch::BeginDraw(); // Starts Drawing

	// Draws Pacman
	if (!_pacman->dead)
	{
		SpriteBatch::Draw(_pacman->texture, _pacman->pPosition, _pacman->sourceRect);
	}

	// Draws Ghosts
	SpriteBatch::Draw(_ghost[0]->texture, _ghost[0]->position, _ghost[0]->sourceRect);

	SpriteBatch::Draw(_cherry->mTexture, _cherry->mPosition, _cherry->mSourceRect); // Draws Cherry


	// Draws Munchies
	if (!_pacman->mCollision)
	{
		for (int i = 0; i < MUNCHIECOUNT; i++)
		{
			if (_munchies[i]->frameCount == 0)
			{
				// Draws Red Munchie
				SpriteBatch::Draw(_munchies[i]->invertedTexture, _munchies[i]->rect, nullptr, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE);

				_munchies[i]->frameCount++;
			}
			else
			{
				// Draw Blue Munchie
				SpriteBatch::Draw(_munchies[i]->blueTexture, _munchies[i]->rect, nullptr, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE);

				_munchies[i]->frameCount++;

				if (_munchies[i]->frameCount >= 60)
					_munchies[i]->frameCount = 0;
			}
		}
	}

	// Audio Check
	if (!_pop->IsLoaded())
	{
		std::cout << "_pop member sound effect has failed to load" << std::endl;
	}

	// Draws String
	SpriteBatch::DrawString(stream.str().c_str(), _stringPosition, Color::Green);

	if (_paused)
	{
		std::stringstream menuStream; 
		menuStream << "PAUSED!";
		SpriteBatch::Draw(_menuBackground, _menuRectangle, nullptr); 
		SpriteBatch::DrawString(menuStream.str().c_str(), _menuStringPosition, Color::Green);
	}

	if (!_started)
	{
		std::stringstream menuStream;
		menuStream << "PRESS 'ENTER' TO START!";
		SpriteBatch::Draw(_menuBackground, _menuRectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menuStringPosition, Color::Blue);
	}

	if (_pacman->dead)
	{
		std::stringstream menuStream;
		menuStream << "GAME OVER!";
		SpriteBatch::Draw(_menuBackground, _menuRectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menuStringPosition, Color::Red);
	}

	SpriteBatch::EndDraw(); // Ends Drawing
}