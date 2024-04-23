#include "Asteroid.h"
#include "Asteroids.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"
#include "fstream"
#include "vector"
#include "algorithm"
#include "stdlib.h"


// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

/** Constructor. Takes arguments from command line, just in case. */
Asteroids::Asteroids(int argc, char *argv[])
	: GameSession(argc, argv)
{
	// reset the level  and asteroid count at the start of a new game
	mLevel = 0;
	mAsteroidCount = 0;
	state = noName;
}

/** Destructor. */
Asteroids::~Asteroids(void)
{
}



// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

/** Start an asteroids game. */
void Asteroids::Start()
{
	//load the leaderboard into the applications memory
	leaderboard = readLeaderboard(leaderboardFileName);

	// Create a shared pointer for the Asteroids game object - DO NOT REMOVE
	thisPtr = shared_ptr<Asteroids>(this);

	// Add this class as a listener of the game world
	mGameWorld->AddListener(thisPtr.get());

	// Add this as a listener to the world and the keyboard
	mGameWindow->AddKeyboardListener(thisPtr);

	// Add a score keeper to the game world
	mGameWorld->AddListener(&mScoreKeeper);

	// Add this class as a listener of the score keeper
	mScoreKeeper.AddListener(thisPtr);

	// Create an ambient light to show sprite textures
	GLfloat ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glEnable(GL_LIGHT0);

	Animation *explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile("explosion", 64, 1024, 64, 64, "explosion_fs.png");
	Animation *asteroid1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
	Animation *spaceship_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship", 128, 128, 128, 128, "spaceship_fs.png");


	//Create the GUI
	CreateGUI();

	
	
	// Start the game
	GameSession::Start();
	
}

/** Stop the current game. */
void Asteroids::Stop()
{
	// Stop the game
	GameSession::Stop();
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IKeyboardListener /////////////////////

void Asteroids::OnKeyPressed(uchar key, int x, int y)
{
	string letter;
	//switch statement to control what buttons do what in each state
	switch (state)
	{
		//gameMode at the top as it requires the least latency
	case gameMode:
		switch (key)
		{
		case ' ':
			//Shoots a bullet when space bar is pressed
			mSpaceship->Shoot();
			break;
		}
		break;
	case noName:
		letter = (1, static_cast<char>(key));
		setName(getName().append(letter));

		SetTimer(10, CREATE_NEW_PLAYER);

		break;
	case demoMode:
		SetTimer(100, SHOW_START_SCREEN);
		break;
	case startMode:
		switch (key)
		{
			//If s is clicked then the game should start
		case 's':
			SetTimer(500, START_NEW_GAME);
			break;
		case 'd':
			//Starts a timer that enters the demo mode
			SetTimer(500, SHOW_DEMO_MODE);
			break;
		case 'h':
			//Starts a timer that brings up the high scores
			SetTimer(500, SHOW_HIGH_SCORE);
			break;
		}
		break;
	case highScoreMode:
		//todo: add method to make start screen return
		switch (key)
		{
		case 'b':
			SetTimer(10, SHOW_START_SCREEN);
			break;
		}
		break;
	default:
		break;
	}
		
}
	
	
void Asteroids::OnKeyReleased(uchar key, int x, int y) {}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y)
{
	switch (state)
	{
	case noName:
		switch (key) {
		case GLUT_KEY_F2:
			if (name != "") {
				SetTimer(100, SHOW_START_SCREEN);
			}
			break;
		}
		break;
	case gameMode:
		switch (key)
			{
			// If up arrow key is pressed start applying forward thrust
			case GLUT_KEY_UP: mSpaceship->Thrust(10); break;
			// If left arrow key is pressed start rotating anti-clockwise
			case GLUT_KEY_LEFT: mSpaceship->Rotate(90); break;
			// If right arrow key is pressed start rotating clockwise
			case GLUT_KEY_RIGHT: mSpaceship->Rotate(-90); break;
			}
		break;
	// Default case - do nothing
	default: break;
	}
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y)
{
	switch (key)
	{
	// If up arrow key is released stop applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(0); break;
	// If left arrow key is released stop rotating
	case GLUT_KEY_LEFT: mSpaceship->Rotate(0); break;
	// If right arrow key is released stop rotating
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(0); break;
	// Default case - do nothing
	default: break;
	} 
}


// PUBLIC INSTANCE METHODS IMPLEMENTING IGameWorldListener ////////////////////

void Asteroids::OnWorldUpdated(GameWorld* world)
{
	moveDemoShip();
}

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
	if (object->GetType() == GameObjectType("Asteroid"))
	{
		shared_ptr<GameObject> explosion = CreateExplosion();
		explosion->SetPosition(object->GetPosition());
		explosion->SetRotation(object->GetRotation());
		mGameWorld->AddObject(explosion);
		mAsteroidCount--;
		if (mAsteroidCount <= 0 && mPlayer.getLives() > 0 && state == gameMode)
		{ 
			SetTimer(500, START_NEXT_LEVEL); 
		}
	}
}

// PUBLIC INSTANCE METHODS IMPLEMENTING ITimerListener ////////////////////////

void Asteroids::OnTimer(int value)
{
	if (value == SHOW_DEMO_MODE)
	{
		
		if (mStartLabel1->GetVisible())mStartLabel1->SetVisible(false);
		if (mStartLabel2->GetVisible())mStartLabel2->SetVisible(false);
		if (mStartLabel3->GetVisible())mStartLabel3->SetVisible(false);
		mScoreLabel->SetText("Score: 0");
		mScoreLabel->SetVisible(true);

		// Create a spaceship and add it to the world
		mGameWorld->AddObject(CreateSpaceship());

		// Create some asteroids and add them to the world
		CreateAsteroids(10);

		// Add a player (watcher) to the game world
		mGameWorld->AddListener(&mPlayer);

		// Add this class as a listener of the player
		mPlayer.AddListener(thisPtr);

		mPlayer.setLives(3);

		mScoreKeeper.setScore(0);

		mLevel = 0;
		demoCount = 0;
		state = demoMode;
	}
	if (value == CREATE_NEW_PLAYER)
	{
		mPlayerNameLabel->SetText(name);
	}
	if (value == SHOW_START_SCREEN)
	{
		state = startMode;
		
		
		if (mNewPlayerLabel->GetVisible())mNewPlayerLabel->SetVisible(false);
		if (mPlayerNameLabel->GetVisible())mPlayerNameLabel->SetVisible(false);
		if(mGameOverLabel->GetVisible())mGameOverLabel->SetVisible(false);
		if (mLivesLabel->GetVisible())mLivesLabel->SetVisible(false);
		if (mHighScoreLabel->GetVisible())mHighScoreLabel->SetVisible(false);
		if (mScoreLabel->GetVisible())mScoreLabel->SetVisible(false);
		if (mHighScoreContentLabel1->GetVisible())mHighScoreContentLabel1->SetVisible(false);
		if (mHighScoreContentLabel2->GetVisible())mHighScoreContentLabel2->SetVisible(false);
		if (mHighScoreContentLabel3->GetVisible())mHighScoreContentLabel3->SetVisible(false);
		if (mHighScoreContentLabel4->GetVisible())mHighScoreContentLabel4->SetVisible(false);
		if (mHighScoreContentLabel5->GetVisible())mHighScoreContentLabel5->SetVisible(false);
		if (mHighScoreBackLabel->GetVisible())mHighScoreBackLabel->SetVisible(false);
		mStartLabel1->SetVisible(true);
		mStartLabel2->SetVisible(true);
		mStartLabel3->SetVisible(true);
		
		cleanObjects();
	}
	if (value == START_NEW_GAME)
	{
		state = gameMode;

		cleanObjects();

		if (mStartLabel1->GetVisible())mStartLabel1->SetVisible(false);
		if (mStartLabel2->GetVisible())mStartLabel2->SetVisible(false);
		if (mStartLabel3->GetVisible())mStartLabel3->SetVisible(false);
		mScoreLabel->SetText("Score: 0");
		mLivesLabel->SetText("Lives: 3");
		mScoreLabel->SetVisible(true);
		mLivesLabel->SetVisible(true);

		// Create a spaceship and add it to the world
		mGameWorld->AddObject(CreateSpaceship());

		// Create some asteroids and add them to the world
		CreateAsteroids(10);

		// Add a player (watcher) to the game world
		mGameWorld->AddListener(&mPlayer);

		// Add this class as a listener of the player
		mPlayer.AddListener(thisPtr);

		mPlayer.setLives(3);

		mScoreKeeper.setScore(0);

		mLevel = 0;
	}
	if (value == USE_LIFE)
	{
		if (state == gameMode)
		{
			mSpaceship->Reset();
			mGameWorld->AddObject(mSpaceship);
		}
		
	}

	if (value == START_NEXT_LEVEL)
	{
		mLevel++;
		int num_asteroids = 10 + 2 * mLevel;
		CreateAsteroids(num_asteroids);
	}

	if (value == SHOW_GAME_OVER)
	{
		mGameOverLabel->SetVisible(true);

		mScoreLabel->SetVisible(false);
		
	}
	if (value == SHOW_HIGH_SCORE)
	{
		state = highScoreMode;

		// Format the lives left message using an string-based stream
		std::ostringstream score1_stream;
		score1_stream << "1. " << leaderboard[0].pName << ": " << leaderboard[0].pScore;
		// Get the lives left message as a string
		std::string score1_msg = score1_stream.str();
		mHighScoreContentLabel1->SetText(score1_msg);
		// Format the lives left message using an string-based stream
		std::ostringstream score2_stream;
		score2_stream << "2. " << leaderboard[1].pName << ": " << leaderboard[1].pScore;
		// Get the lives left message as a string
		std::string score2_msg = score2_stream.str();
		mHighScoreContentLabel2->SetText(score2_msg);
		// Format the lives left message using an string-based stream
		std::ostringstream score3_stream;
		score3_stream << "3. " << leaderboard[2].pName << ": " << leaderboard[2].pScore;		
		std::string score3_msg = score3_stream.str();
		mHighScoreContentLabel3->SetText(score3_msg);
		// Format the lives left message using an string-based stream
		std::ostringstream score4_stream;
		score4_stream << "4. " << leaderboard[3].pName << ": " << leaderboard[3].pScore;
		// Get the lives left message as a string
		std::string score4_msg = score4_stream.str();
		mHighScoreContentLabel3->SetText(score4_msg);
		// Format the lives left message using an string-based stream
		std::ostringstream score5_stream;
		score5_stream << "5. " << leaderboard[4].pName << ": " << leaderboard[4].pScore;
		// Get the lives left message as a string
		std::string score5_msg = score5_stream.str();
		mHighScoreContentLabel5->SetText(score5_msg);

		if (mNewPlayerLabel->GetVisible())mNewPlayerLabel->SetVisible(false);
		if (mPlayerNameLabel->GetVisible())mPlayerNameLabel->SetVisible(false);
		if (mGameOverLabel->GetVisible())mGameOverLabel->SetVisible(false);
		if (mScoreLabel->GetVisible())mScoreLabel->SetVisible(false);
		if (mLivesLabel->GetVisible())mLivesLabel->SetVisible(false);
		if (mStartLabel1->GetVisible())mStartLabel1->SetVisible(false);
		if (mStartLabel2->GetVisible())mStartLabel2->SetVisible(false);
		if (mStartLabel3->GetVisible())mStartLabel3->SetVisible(false);
		mHighScoreLabel->SetVisible(true);
		mHighScoreContentLabel1->SetVisible(true);
		mHighScoreContentLabel2->SetVisible(true);
		mHighScoreContentLabel3->SetVisible(true);
		mHighScoreContentLabel4->SetVisible(true);
		mHighScoreContentLabel5->SetVisible(true);
		mHighScoreBackLabel->SetVisible(true);
	}

}

// PROTECTED INSTANCE METHODS /////////////////////////////////////////////////
shared_ptr<GameObject> Asteroids::CreateSpaceship()
{
	// Create a raw pointer to a spaceship that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mSpaceship = make_shared<Spaceship>();
	mSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
	mSpaceship->SetBulletShape(bullet_shape);
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mSpaceship->SetSprite(spaceship_sprite);
	mSpaceship->SetScale(0.1f);
	// Reset spaceship back to centre of the world
	mSpaceship->Reset();
	// Return the spaceship so it can be added to the world
	return mSpaceship;

}

void Asteroids::CreateAsteroids(const uint num_asteroids)
{
	mAsteroidCount = num_asteroids;
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
	}
}


void Asteroids::CreateGUI()
{
	// Add a (transparent) border around the edge of the game display for gui
	mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));
	// Create a new GUILabel and wrap it up in a shared_ptr
	mScoreLabel = make_shared<GUILabel>("Score: 0");
	// Set the vertical alignment of the label to GUI_VALIGN_TOP
	mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> score_component = static_pointer_cast<GUIComponent>(mScoreLabel);
	mGameDisplay->GetContainer()->AddComponent(score_component, GLVector2f(0.0f, 1.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mLivesLabel = make_shared<GUILabel>("Lives: 3");
	// Set the vertical alignment of the label to GUI_VALIGN_BOTTOM
	mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> lives_component = static_pointer_cast<GUIComponent>(mLivesLabel);
	mGameDisplay->GetContainer()->AddComponent(lives_component, GLVector2f(0.0f, 0.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mGameOverLabel = shared_ptr<GUILabel>(new GUILabel("GAME OVER"));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> game_over_component = static_pointer_cast<GUIComponent>(mGameOverLabel);
	mGameDisplay->GetContainer()->AddComponent(game_over_component, GLVector2f(0.5f, 0.5f));
	

	// Create a new GUILabel and wrap it up in a shared_ptr
	mNewPlayerLabel = make_shared<GUILabel>("Enter name:    Press F2 when your done!");
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mNewPlayerLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	//Set the horizontal alignment of the lable to GUI_HALIGN_CENTER
	mNewPlayerLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> player_name_component = static_pointer_cast<GUIComponent>(mNewPlayerLabel);
	mGameDisplay->GetContainer()->AddComponent(player_name_component, GLVector2f(0.5f, 0.7f));

	// Create a new GUILabel to show the players entered name and wrap it up in a shared_ptr
	mPlayerNameLabel = make_shared<GUILabel>("|");
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mPlayerNameLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	//Set the horizontal alignment of the lable to GUI_HALIGN_CENTER
	mPlayerNameLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> new_player_component = static_pointer_cast<GUIComponent>(mPlayerNameLabel);
	mGameDisplay->GetContainer()->AddComponent(new_player_component, GLVector2f(0.5f, 0.4f));
	
	// Create a new GUILabel for the get name message and wrap it up in a shared_ptr
	mStartLabel1 = make_shared<GUILabel>("Click s to Start!");
	mStartLabel2 = make_shared<GUILabel>(" Click d to see Demo!");
	mStartLabel3 = make_shared<GUILabel>(" Click h to see High-Scores!");
	mStartLabel1->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mStartLabel1->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mStartLabel2->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mStartLabel2->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mStartLabel3->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mStartLabel3->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> start_component_1 = static_pointer_cast<GUIComponent>(mStartLabel1);
	shared_ptr<GUIComponent> start_component_2 = static_pointer_cast<GUIComponent>(mStartLabel2);
	shared_ptr<GUIComponent> start_component_3 = static_pointer_cast<GUIComponent>(mStartLabel3);
	mGameDisplay->GetContainer()->AddComponent(start_component_1, GLVector2f(0.5f, 0.6f));
	mGameDisplay->GetContainer()->AddComponent(start_component_2, GLVector2f(0.5f, 0.5f));
	mGameDisplay->GetContainer()->AddComponent(start_component_3, GLVector2f(0.5f, 0.4f));

	//making label components for the high-score page and adding them to the container
	mHighScoreLabel = make_shared<GUILabel>("High-Scores: ");
	mHighScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	mHighScoreLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	shared_ptr<GUIComponent> highScoreComponent = static_pointer_cast<GUIComponent>(mHighScoreLabel);
	mHighScoreContentLabel1 = make_shared<GUILabel>("1.  ");
	mHighScoreContentLabel1->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mHighScoreContentLabel1->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	shared_ptr<GUIComponent> highScoreContentComponent1 = static_pointer_cast<GUIComponent>(mHighScoreContentLabel1);
	mHighScoreContentLabel2 = make_shared<GUILabel>("2.  ");
	mHighScoreContentLabel2->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mHighScoreContentLabel2->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	shared_ptr<GUIComponent> highScoreContentComponent2 = static_pointer_cast<GUIComponent>(mHighScoreContentLabel2);
	mHighScoreContentLabel3 = make_shared<GUILabel>("3.  ");
	mHighScoreContentLabel3->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mHighScoreContentLabel3->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	shared_ptr<GUIComponent> highScoreContentComponent3 = static_pointer_cast<GUIComponent>(mHighScoreContentLabel3);
	mHighScoreContentLabel4 = make_shared<GUILabel>("4.  ");
	mHighScoreContentLabel4->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mHighScoreContentLabel4->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	shared_ptr<GUIComponent> highScoreContentComponent4 = static_pointer_cast<GUIComponent>(mHighScoreContentLabel4);
	mHighScoreContentLabel5 = make_shared<GUILabel>("5.  ");
	mHighScoreContentLabel5->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mHighScoreContentLabel5->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	shared_ptr<GUIComponent> highScoreContentComponent5 = static_pointer_cast<GUIComponent>(mHighScoreContentLabel5);
	mHighScoreBackLabel = make_shared<GUILabel>("Press b to go back!");
	mHighScoreBackLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mHighScoreBackLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	shared_ptr<GUIComponent> highScoreBackComponent = static_pointer_cast<GUIComponent>(mHighScoreBackLabel);
	mGameDisplay->GetContainer()->AddComponent(highScoreComponent, GLVector2f(0.5f, 1.0f));
	mGameDisplay->GetContainer()->AddComponent(highScoreContentComponent1, GLVector2f(0.5f, 0.8f));
	mGameDisplay->GetContainer()->AddComponent(highScoreContentComponent2, GLVector2f(0.5f, 0.7f));
	mGameDisplay->GetContainer()->AddComponent(highScoreContentComponent3, GLVector2f(0.5f, 0.6f));
	mGameDisplay->GetContainer()->AddComponent(highScoreContentComponent4, GLVector2f(0.5f, 0.5f));
	mGameDisplay->GetContainer()->AddComponent(highScoreContentComponent5, GLVector2f(0.5f, 0.4f));
	mGameDisplay->GetContainer()->AddComponent(highScoreBackComponent, GLVector2f(0.5f, 0.1f));

	
	//set visibility of all components to false to begin with except the new player labels
	mGameOverLabel->SetVisible(false);
	mScoreLabel->SetVisible(false);
	mLivesLabel->SetVisible(false);
	mStartLabel1->SetVisible(false);
	mStartLabel2->SetVisible(false);
	mStartLabel3->SetVisible(false);
	mHighScoreLabel->SetVisible(false);
	mHighScoreContentLabel1->SetVisible(false);
	mHighScoreContentLabel2->SetVisible(false);
	mHighScoreContentLabel3->SetVisible(false);
	mHighScoreContentLabel4->SetVisible(false);
	mHighScoreContentLabel5->SetVisible(false);
	mHighScoreBackLabel->SetVisible(false);

	
	
	mNewPlayerLabel->SetVisible(true);
	mPlayerNameLabel->SetVisible(true);

}

void Asteroids::OnScoreChanged(int score)
{
	// Format the score message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Score: " << score;
	// Get the score message as a string
	std::string score_msg = msg_stream.str();
	mScoreLabel->SetText(score_msg);
}

void Asteroids::OnPlayerKilled(int lives_left)
{
	shared_ptr<GameObject> explosion = CreateExplosion();
	explosion->SetPosition(mSpaceship->GetPosition());
	explosion->SetRotation(mSpaceship->GetRotation());
	mGameWorld->AddObject(explosion);

	// Format the lives left message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Lives: " << lives_left;
	// Get the lives left message as a string
	std::string lives_msg = msg_stream.str();
	mLivesLabel->SetText(lives_msg);

	if (state == gameMode) {
		if (lives_left > 0) 
		{
			SetTimer(1000, USE_LIFE);
		}
		else
		{
			finalScore = mScoreKeeper.getScore();

			updateLeaderboard(name, finalScore);
			writeLeaderboard(leaderboardFileName);

			SetTimer(500, SHOW_GAME_OVER);
			SetTimer(3000, SHOW_START_SCREEN);
		
		}
	}
	else
	{

	}
	
}

shared_ptr<GameObject> Asteroids::CreateExplosion()
{
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
	shared_ptr<Sprite> explosion_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	explosion_sprite->SetLoopAnimation(false);
	shared_ptr<GameObject> explosion = make_shared<Explosion>();
	explosion->SetSprite(explosion_sprite);
	explosion->Reset();
	return explosion;
}

void Asteroids::cleanObjects() {
	GameObjectList objects = mGameWorld->getGameObjects();
	for (uint i = 0; i < objects.size(); i++) {
		objects = mGameWorld->getGameObjects();
		mGameWorld->RemoveObject(objects.front());
	}
}

vector<leaderboardEntry> Asteroids::readLeaderboard(const string& leaderboardFile)
{
	
	ifstream file(leaderboardFile);
	if (file.is_open()) {
		leaderboardEntry entry;
		while (file >> entry.pName >> entry.pScore) {
			leaderboard.push_back(entry);
		}
		file.close();
	}
	return leaderboard;
}

void Asteroids::writeLeaderboard(const string& leaderboardFile)
{
	ofstream file(leaderboardFile);
	if (file.is_open()) {
		for (const leaderboardEntry& entry : leaderboard) {
			file << entry.pName << " " << entry.pScore << endl;
		}
		file.close();
	}
}

void Asteroids::updateLeaderboard(const string& pName, int pScore)
{
	leaderboard.push_back({ pName, pScore });

	sort(leaderboard.begin(), leaderboard.end(), [](const leaderboardEntry& a, const leaderboardEntry& b) {
		return a.pScore > b.pScore;
	});
}

void Asteroids::moveDemoShip() {
	if (state == demoMode) {
		if (mSpaceship) {
			demoCount = demoCount + 1;

			if (demoCount % 50 == 0) {

				int randint = rand() % 2;
				if (randint == 0) {
					mSpaceship->AddAngle(rand() % 20);
				}
				else if (randint == 1) {
					mSpaceship->AddAngle(-1 * (rand() % 20));
				}
				mSpaceship->Thrust(rand() % 10);
			}
			if (demoCount % 200 == 0) {
				mSpaceship->Shoot();
			}
		}
	}
}
