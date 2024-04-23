#ifndef __ASTEROIDS_H__
#define __ASTEROIDS_H__

#include "GameUtil.h"
#include "GameSession.h"
#include "IKeyboardListener.h"
#include "IGameWorldListener.h"
#include "IScoreListener.h" 
#include "ScoreKeeper.h"
#include "Player.h"
#include "IPlayerListener.h"
#include "GUIComponent.h"
#include "GUIContainer.h"
#include "Sprite.h"
#include "GameObject.h"
#include "fstream"
#include "vector"


class GameObject;
class Spaceship;
class GUILabel;

enum State{demoMode,noName, startMode, gameMode, highScoreMode};

struct leaderboardEntry {
	string pName;
	int pScore;
};

class Asteroids : public GameSession, public IKeyboardListener, public IGameWorldListener, public IScoreListener, public IPlayerListener
{
public:
	Asteroids(int argc, char *argv[]);
	virtual ~Asteroids(void);

	virtual void Start(void);
	virtual void Stop(void);

	State state;

	// Declaration of IKeyboardListener interface ////////////////////////////////

	void OnKeyPressed(uchar key, int x, int y);
	void OnKeyReleased(uchar key, int x, int y);
	void OnSpecialKeyPressed(int key, int x, int y);
	void OnSpecialKeyReleased(int key, int x, int y);

	// Declaration of IScoreListener interface //////////////////////////////////

	void OnScoreChanged(int score);

	// Declaration of the IPlayerLister interface //////////////////////////////

	void OnPlayerKilled(int lives_left);

	// Declaration of IGameWorldListener interface //////////////////////////////

	void OnWorldUpdated(GameWorld* world);
	void OnObjectAdded(GameWorld* world, shared_ptr<GameObject> object) {}
	void OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object);

	// Override the default implementation of ITimerListener ////////////////////
	void OnTimer(int value);

	//removes all objects from the game world when necessary
	void cleanObjects();

	//methods relating to the high score
	vector<leaderboardEntry> readLeaderboard(string leaderboardFile);
	void writeLeaderboard(string leaderboardFile);
	void updateLeaderboard(const string& pName, int pScore);

	State getState() { return state; }

	string getName() { return name; }
	void setName(string n) { name = n; }

	Player getPlayer() { return mPlayer; }

	int demoCount = 0;

private:
	shared_ptr<Spaceship> mSpaceship;
	shared_ptr<GUILabel> mScoreLabel;
	shared_ptr<GUILabel> mLivesLabel;
	shared_ptr<GUILabel> mGameOverLabel;
	shared_ptr<GUILabel> mStartLabel1;
	shared_ptr<GUILabel> mStartLabel2;
	shared_ptr<GUILabel> mStartLabel3;
	shared_ptr<GUILabel> mHighScoreLabel;
	shared_ptr<GUILabel> mNewPlayerLabel;
	shared_ptr<GUILabel> mHighScoreContentLabel1;
	shared_ptr<GUILabel> mHighScoreContentLabel2;
	shared_ptr<GUILabel> mHighScoreContentLabel3;
	shared_ptr<GUILabel> mHighScoreContentLabel4;
	shared_ptr<GUILabel> mHighScoreContentLabel5;
	shared_ptr<GUILabel> mHighScoreBackLabel;
	shared_ptr<GUILabel> mPlayerNameLabel;

	shared_ptr<Asteroids> thisPtr;

	uint mLevel;
	uint mAsteroidCount;
	
	string name = "";
	int finalScore = 0;
	const string leaderboardFileName = "leaderboard.txt";
	vector<leaderboardEntry> leaderboard;

	void ResetSpaceship();
	shared_ptr<GameObject> CreateSpaceship();
	void CreateGUI();
	void CreateAsteroids(const uint num_asteroids);
	shared_ptr<GameObject> CreateExplosion();
	void  moveDemoShip();
	
	// States that  the application can be in
	const static uint SHOW_DEMO_MODE = 0;
	const static uint CREATE_NEW_PLAYER = 1;
	const static uint SHOW_START_SCREEN = 2;
	const static uint START_NEW_GAME = 3;
	const static uint USE_LIFE = 4;
	const static uint START_NEXT_LEVEL = 5;
	const static uint SHOW_GAME_OVER = 6;
	const static uint SHOW_HIGH_SCORE = 7;
	

	ScoreKeeper mScoreKeeper;
	Player mPlayer = Player();
	
};

#endif