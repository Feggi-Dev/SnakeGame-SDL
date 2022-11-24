#include <vector>
#include <queue>
#include <fstream>
#include <sstream>
#include <math.h>
#include <ctime>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

/*
		 The apple is red, the snake is green, the board limit is blue.
							 Commands:
		Use the arrow keys or "WASD" keys to move in the respective way.
							  Rules:
				  - The snake can't go backwards.
			    - If the snake eats itself, you lose.
			- If the snake goes on the blue border, you lose.
		
		  If you lose, press <<enter>> to start a new game.
				Press a command key to start moving!	 
*/

using namespace std;

SDL_Window *window = NULL;
SDL_Surface *surface = NULL;
TTF_Font *ScoreFont = NULL;
TTF_Font *GameOverFont = NULL;
SDL_Surface *TextSnakeGame = NULL;
SDL_Surface *TextScore = NULL;
SDL_Surface *TextHighScore = NULL;
SDL_Surface *TextEnd = NULL;
Mix_Chunk *AppleEatenSound = NULL;
Mix_Chunk *GameOverSound = NULL;

const int BoardWidth = 15;
const int BoardHeight = 15;
const int BlockSize = 40;
const int WindowWidth = BoardWidth * BlockSize + 2 * BlockSize + BlockSize / 5;
const int WindowHeight = BoardHeight * BlockSize + 4 * BlockSize + BlockSize / 5;

const SDL_Color White = {0xff, 0xff, 0xff};
const SDL_Color Black = {0x00, 0x00, 0x00};
const SDL_Color AppleColor = {0xff, 0x00, 0x00};
const SDL_Color SnakeColor = {0x00, 0xff, 0x00};
const SDL_Color BoardLimitColor = {0x00, 0x00, 0xff};

struct Sound{
	Sound(){
	Mix_Init(0);
	Mix_OpenAudio(10000, MIX_DEFAULT_FORMAT, 2, 1024);
	AppleEatenSound = Mix_LoadWAV("AppleEatenSound.wav");
	GameOverSound = Mix_LoadWAV("GameOverSound.wav");
	}
	void Apple()	{Mix_PlayChannel(-1, AppleEatenSound, 0);};
	void GameOver()	{Mix_PlayChannel(-1, GameOverSound, 0);};
};

void DrawSquare(unsigned int x, unsigned int y, const SDL_Color col){
	Uint8* pixel_ptr = (Uint8*)surface->pixels + (y * BlockSize * WindowWidth + x * BlockSize) * 4;
	for (unsigned int i = 0; i < BlockSize; i++){
		for (unsigned int j = 0; j < BlockSize; j++){
			if(i < BlockSize / 5 || j < BlockSize / 5){
				*(pixel_ptr + j * 4) = 0x00;
				*(pixel_ptr + j * 4 + 1) = 0x00;
				*(pixel_ptr + j * 4 + 2) = 0x00;
			}
			else{
				*(pixel_ptr + j * 4) = col.b;
				*(pixel_ptr + j * 4 + 1) = col.g;
				*(pixel_ptr + j * 4 + 2) = col.r;
			}
		}
		pixel_ptr += WindowWidth * 4;
	}
}

class SnakeGame{
	public:
		SnakeGame();
		void MoveUp()		{Sy--;};
   	 	void MoveDown()		{Sy++;};
    	void MoveLeft()		{Sx--;};
    	void MoveRight()	{Sx++;};
    	void Movement(char);
    	void DrawSnake(){DrawSquare(Sx, Sy, SnakeColor);};
		bool MakeTail();
		bool CheckSelfCollision();
		bool CheckLose();
		bool AppleIsEaten();
		void NewApplePosition();
		void PrintScore(SDL_Color Col);
		void PrintHighScore();
		void GameOver();
		void Restart();
	private:
		int Sx, Sy;	//Snake coords
		int Ax, Ay;	//Apple coords
		vector <pair <int, int> > List;
		vector <pair <int, int> > Tail;
};

SnakeGame::SnakeGame(){
	//Snake initial coords
	Sx = BoardWidth / 2;
	Sy = BoardHeight / 2 + 2;
	//List initialization for apple random spawning
	for(int i = 1; i < BoardWidth + 1; i++){
		for(int j = 3; j < BoardHeight + 3; j++){
			List.push_back(make_pair(i, j));
		}
	}
	//Drawing board limits
	for(int i = 0; i < BoardWidth + 2; i++){
		for(int j = 2; j < BoardHeight + 4; j++){
			if(i == 0 || j == 2 || i == BoardWidth + 1 || j == BoardHeight + 3)
				DrawSquare(i, j, BoardLimitColor);
		}
	}
	//Printing title
	TextSnakeGame = TTF_RenderText_Blended(ScoreFont, "SnakeGame!", SnakeColor);
	SDL_Rect dst = {(surface -> w - TextSnakeGame -> w) / 2, 0, 0, 0};
	SDL_BlitSurface(TextSnakeGame, NULL, surface, &dst);
}

enum Directions{
	Up, Down, Left, Right
};

void SnakeGame::Movement(char Movement){
	switch(Movement){
		case Up:
			SnakeGame::MoveUp();
			break;
		case Down:
			SnakeGame::MoveDown();
			break;
		case Left:
			SnakeGame::MoveLeft();
			break;
		case Right:
			SnakeGame::MoveRight();
			break;
	}
}

bool SnakeGame::MakeTail(){
	Tail.push_back(make_pair(Sx, Sy));
	if(!SnakeGame::AppleIsEaten()){
	DrawSquare(Tail[0].first, Tail[0].second, Black);
		Tail.erase(Tail.begin());
		return false;
	}
	return true;
}

bool SnakeGame::CheckSelfCollision(){
	for(int i = 0; i < Tail.size(); i++){
		if(Tail[i].first == Sx && Tail[i].second == Sy)
			return true;
	}
	return false;
}

bool SnakeGame::CheckLose(){
  	if(Sx == 0 || Sy == 2 || Sx == BoardWidth + 1 || Sy == BoardHeight + 3)
    	return true;
	if(SnakeGame::CheckSelfCollision())
		return true;
    return false;
}

bool SnakeGame::AppleIsEaten(){
	if(Sx == Ax && Sy == Ay)
		return true;
	return false;
}

void SnakeGame::NewApplePosition(){
	for(int i = 0; i < List.size(); i++){
		for(int j = 0; j < Tail.size(); j++){
			if((Tail[j].first == List[i].first && Tail[j].second == List[i].second) || (List[i].first == Sx && List[i].second == Sy)){
				List.erase(List.begin() + i);
				i--;
			}
		}
	}
	int r = rand() % List.size();
	Ax = List[r].first;
	Ay = List[r].second;
	DrawSquare(Ax, Ay, AppleColor);
	for(int i = 0; i < Tail.size(); i++){
		List.push_back(make_pair(Tail[i].first, Tail[i].second));
	}
}

void SnakeGame::PrintScore(SDL_Color Col){
	stringstream ss;
	ss << Tail.size();
	string StrDef = "Score:" + ss.str();
	TextScore = TTF_RenderText_Shaded(ScoreFont, StrDef.c_str(), Col, Black);
	SDL_Rect dst = {BlockSize, BlockSize, 0, 0};
	SDL_BlitSurface(TextScore, NULL, surface, &dst);
	SDL_UpdateWindowSurface(window);
	SDL_FreeSurface(TextScore);
}

void SnakeGame::PrintHighScore(){
	int High;
	ifstream HighScores("HighScore.txt");
	HighScores >> High;
	stringstream ss;
	if(Tail.size() > High){
		ss << Tail.size();
		ofstream HighScores ("HighScore.txt");
 		HighScores << ss.str(); 
	}
	else
		ss << High;
	string StrDef = "HighScore:" + ss.str() + " ";
	TextHighScore = TTF_RenderText_Shaded(ScoreFont, StrDef.c_str(), White, Black);
	int measure = ss.str().size() == 1 ? 1 : 2;
	SDL_Rect dst = {(11 - measure) * BlockSize, BlockSize, 0, 0};
	SDL_BlitSurface(TextHighScore, NULL, surface, &dst);
	SDL_UpdateWindowSurface(window);
	SDL_FreeSurface(TextHighScore);
	HighScores.close();
}

void SnakeGame::GameOver(){
	TextEnd = TTF_RenderText_Blended(GameOverFont, "Game Over!", White);
	SDL_Rect dst = {(surface -> w - TextEnd -> w) / 2, (surface -> h - TextEnd -> h) / 2, 0, 0};
	SDL_BlitSurface(TextEnd, NULL, surface, &dst);
	SDL_UpdateWindowSurface(window);
}

void SnakeGame::Restart(){
	//Erase score
	SnakeGame::PrintScore(Black);
	//Draw starting board
	for(int i = 0; i < BoardWidth + 2; i++){
		for(int j = 2; j < BoardHeight + 4; j++){
			if(i == 0 || j == 2 || i == BoardWidth + 1 || j == BoardHeight + 3)
				DrawSquare(i, j, BoardLimitColor);
			else
				DrawSquare(i, j, Black);
		}
	}
	//There is no tail now
	Tail.clear();
	//Print Score:0
	SnakeGame::PrintScore(White);
	//Snake initial coords
	Sx = BoardWidth / 2;
	Sy = BoardHeight / 2 + 2;
	//First apple spawn
	SnakeGame::NewApplePosition();
}

int main(int argc, char* argv[]){

	srand(time(NULL));

	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow("SnakeGame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WindowWidth, WindowHeight, SDL_WINDOW_SHOWN);
	surface = SDL_GetWindowSurface(window);
	TTF_Init();
	ScoreFont = TTF_OpenFont("VcrOsdMono.ttf", BlockSize);
	GameOverFont = TTF_OpenFont("VcrOsdMono.ttf", 2 * BlockSize);
	
	Sound Sound;
	SnakeGame SnakeGame;
	SnakeGame.PrintScore(White);
	SnakeGame.PrintHighScore();	
	SnakeGame.NewApplePosition();
	
	queue <char> Move;
	
	SDL_Event event;
	
	bool quit = false;
	bool IsOver = false;
	bool Restart = false;
	
	while (!quit){
		
		while (SDL_PollEvent(&event) != 0){
			if(event.type == SDL_QUIT)	quit = true;
			if(event.key.state == SDL_PRESSED && Move.size() < 2){
				if((event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) && Move.front() != Down)			Move.push(Up);
				else if((event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) && Move.front() != Up)	Move.push(Down);
				else if((event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a) && Move.front() != Right)	Move.push(Left);
				else if((event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d) && Move.front() != Left)	Move.push(Right);
			}
			if(IsOver && event.key.state == SDL_PRESSED && event.key.keysym.sym == SDLK_RETURN){
				SnakeGame.Restart(); 
				Move = queue<char>();
				IsOver = false;
			}
		}

		if(!IsOver){
			if(SnakeGame.CheckLose()){
				Sound.GameOver();
				SnakeGame.GameOver();
				IsOver = true;
			}
		}
		
		if(!IsOver){
			
			if(SnakeGame.MakeTail()){
				Sound.Apple();
				SnakeGame.NewApplePosition();
				SnakeGame.PrintScore(White);
				SnakeGame.PrintHighScore();	
			}
			
			if(Move.size() == 2)
				Move.pop();
			if(!Move.empty())
				SnakeGame.Movement(Move.front());
			
			SnakeGame.DrawSnake();
			
			SDL_UpdateWindowSurface(window);
	
			SDL_Delay(100);	
		}
		
	}
	
	SDL_FreeSurface(TextEnd);
	SDL_FreeSurface(surface);	
	TTF_CloseFont(ScoreFont);
	TTF_CloseFont(GameOverFont);
	TTF_Quit();
	SDL_Quit();   
	SDL_DestroyWindow(window); 
	
	return 0;
	
}
