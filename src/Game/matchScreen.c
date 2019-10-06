#include "matchScreen.h"

#include "../Graphics/graphics.h"
#include "../Graphics/images.h"
#include "../Graphics/spineGfx.h"
#include "../Graphics/camera.h"
#include "../Graphics/debugRendering.h"
#include "../Graphics/imageSheets.h"
#include "resources.h"
#include "../System/random.h"
#include "../UI/button.h"
#include "../UI/text.h"
#include "../Utils/stretchyBuffer.h"
#include "../System/platformLog.h"
#include "../Utils/helpers.h"
#include "hubScreen.h"
#include "../sound.h"

static const Vector2 OPPONENT_FIELD_POS = { 400.0f, 177.0f };
static const Vector2 PLAYER_FIELD_POS = { 400.0f, 423.0f };
static const Vector2 PLAY_FIELD_SIZE = { 240.0f, 170.0f };

typedef struct {
	Vector2 position;
	int value;
	bool success;
} DisplayDie;

static DisplayDie* sbDice = NULL;

static void addDisplayDie( Vector2 pos, int value, bool success )
{
	DisplayDie newDie;
	newDie.position = pos;
	newDie.value = value;
	newDie.success = success;

	sb_Push( sbDice, newDie );
}

static void clearDisplayDice( void )
{
	sb_Clear( sbDice );
}

static void clearState( DuelistState* state )
{
	state->cheating = 0;
	state->chosenStyle = PS_NONE;
	state->chosenType = CT_NONE;
}

static Color dieSuccessPipColor = { 0.2f, 0.8f, 0.0f, 1.0f };
static Color dieFailPipColor = { 0.8f, 0.2f, 0.0f, 1.0f };
static void drawDisplayDice( void )
{
	for( size_t i = 0; i < sb_Count( sbDice ); ++i ) {
		Color pipClr = sbDice[i].success ? dieSuccessPipColor : dieFailPipColor;
		img_Draw_s( diceBaseImg, 1, sbDice[i].position, sbDice[i].position, 0.5f, 0.5f, 75 );
		img_Draw_s_c( dicePipImgs[sbDice[i].value - 1], 1, sbDice[i].position, sbDice[i].position, 0.5f, 0.5f, pipClr, pipClr, 75 );
	}
}

static DuelistState playerState;
static DuelistState opponentState;

static bool opponentCheatingRevealed;
static bool opponentTypeRevealed;
static bool opponentStyleRevealed;

static bool lockValues;

static int continueButton;
static void createContinueButton( const char* title, ButtonResponse response )
{
	continueButton = btn_Create( vec2( 400.0f, 300.0f ), vec2( 188.0f, 46.0f ), vec2( 189.0f, 47.0f ), title, font20, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, -50, NULL, response );
}

static void destroyContinueButton( void )
{
	btn_Destroy( continueButton );
	continueButton = -1;
}

static int addCheatBtn;
static int subCheatBtn;

static void addCheat( int btnID )
{
	if( lockValues ) return;

	playClick( );
	if( playerState.cheating < 100 ) {
		++playerState.cheating;
	}
}

static void reduceCheat( int btnID )
{
	if( lockValues ) return;

	playClick( );
	if( playerState.cheating > 0 ) {
		--playerState.cheating;
	}
}

static void createCheatButtons( void )
{
	addCheatBtn = btn_Create( vec2( 427.0f, 569.0f ), vec2( 25.0f, 25.0f ), vec2( 25.0f, 25.0f ), "+", font20, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, -50, NULL, addCheat );
	subCheatBtn = btn_Create( vec2( 373.0f, 569.0f ), vec2( 25.0f, 25.0f ), vec2( 25.0f, 25.0f ), "-", font20, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, -50, NULL, reduceCheat );
}

static void destroyCheatButtons( void )
{
	btn_Destroy( addCheatBtn );
	btn_Destroy( subCheatBtn );
	addCheatBtn = -1;
	subCheatBtn = -1;
}


static int aggroCardBtn = -1;
static int comboCardBtn = -1;
static int controlCardBtn = -1;
static int strategyCardBtn = -1;
static int bluffingCardBtn = -1;
static int aggressiveCardBtn = -1;
static int chosenTypeBtn = -1;
static int chosenStyleBtn = -1;
static bool refreshCardButtons = false;

static void chooseType( int id )
{
	if( lockValues ) return;

	playCard( );
	if( id == aggroCardBtn ) {
		playerState.chosenType = CT_AGGRO;
	} else if( id == comboCardBtn ) {
		playerState.chosenType = CT_COMBO;
	} else if( id == controlCardBtn ) {
		playerState.chosenType = CT_CONTROL;
	} else if( id == chosenTypeBtn ) {
		playerState.chosenType = CT_NONE;
	}
	refreshCardButtons = true;
}

static void chooseStyle( int id )
{
	if( lockValues ) return;

	playCard( );
	if( id == strategyCardBtn ) {
		playerState.chosenStyle = PS_STRATEGY;
	} else if( id == bluffingCardBtn ) {
		playerState.chosenStyle = PS_BLUFFING;
	} else if( id == aggressiveCardBtn ) {
		playerState.chosenStyle = PS_BOLD;
	} else if( id == chosenStyleBtn ) {
		playerState.chosenStyle = PS_NONE;
	}
	refreshCardButtons = true;
}

static void destroyPlayerCardButtons( void )
{
	btn_Destroy( aggroCardBtn );
	btn_Destroy( comboCardBtn );
	btn_Destroy( controlCardBtn );
	btn_Destroy( strategyCardBtn );
	btn_Destroy( bluffingCardBtn );
	btn_Destroy( aggressiveCardBtn );
	btn_Destroy( chosenTypeBtn );
	btn_Destroy( chosenStyleBtn );

	aggroCardBtn = -1;
	comboCardBtn = -1;
	controlCardBtn = -1;
	strategyCardBtn = -1;
	bluffingCardBtn = -1;
	aggressiveCardBtn = -1;
	chosenTypeBtn = -1;
	chosenStyleBtn = -1;
}

static void setPlayerCardButtons( void )
{
	Vector2 cardSize = vec2( 73.0f, 119.0f );
	Vector2 textOffset = VEC2_ZERO;

	destroyPlayerCardButtons( );

	Vector2 chosenTypePos = vec2( 340.0f, 423.0f );
	Vector2 chosenStylePos = vec2( 460.0f, 423.0f );

	char amtText[8];
	SDL_snprintf( amtText, 7, "%i", player.deck.aggro );
	if( playerState.chosenType == CT_AGGRO ) {
		chosenTypeBtn = btn_Create( chosenTypePos, cardSize, cardSize, amtText, font60, CLR_DARK_GREY, textOffset, NULL, aggroCardImg, CLR_WHITE, 1, 0, NULL, chooseType );
	} else {
		Vector2 pos = vec2( 20.0f, 360.0f );
		vec2_AddScaled( &pos, &cardSize, 0.5f, &pos );
		aggroCardBtn = btn_Create( pos, cardSize, cardSize, amtText, font60, CLR_DARK_GREY, textOffset, NULL, aggroCardImg, CLR_WHITE, 1, 0, NULL, chooseType );
	}

	SDL_snprintf( amtText, 7, "%i", player.deck.combo );
	if( playerState.chosenType == CT_COMBO ) {
		chosenTypeBtn = btn_Create( chosenTypePos, cardSize, cardSize, amtText, font60, CLR_DARK_GREY, textOffset, NULL, comboCardImg, CLR_WHITE, 1, 0, NULL, chooseType );
	} else {
		Vector2 pos = vec2( 108.0f, 360.0f );
		vec2_AddScaled( &pos, &cardSize, 0.5f, &pos );
		comboCardBtn = btn_Create( pos, cardSize, cardSize, amtText, font60, CLR_DARK_GREY, textOffset, NULL, comboCardImg, CLR_WHITE, 1, 0, NULL, chooseType );
	}

	SDL_snprintf( amtText, 7, "%i", player.deck.control );
	if( playerState.chosenType == CT_CONTROL ) {
		chosenTypeBtn = btn_Create( chosenTypePos, cardSize, cardSize, amtText, font60, CLR_DARK_GREY, textOffset, NULL, controlCardImg, CLR_WHITE, 1, 0, NULL, chooseType );
	} else {
		Vector2 pos = vec2( 196.0f, 360.0f );
		vec2_AddScaled( &pos, &cardSize, 0.5f, &pos );
		controlCardBtn = btn_Create( pos, cardSize, cardSize, amtText, font60, CLR_DARK_GREY, textOffset, NULL, controlCardImg, CLR_WHITE, 1, 0, NULL, chooseType );
	}


	SDL_snprintf( amtText, 7, "%i", player.bold );
	if( playerState.chosenStyle == PS_BOLD ) {
		chosenStyleBtn = btn_Create( chosenStylePos, cardSize, cardSize, amtText, font60, CLR_DARK_GREY, textOffset, NULL, boldCardImg, CLR_WHITE, 1, 0, NULL, chooseStyle );
	} else {
		Vector2 pos = vec2( 531.0f, 360.0f );
		vec2_AddScaled( &pos, &cardSize, 0.5f, &pos );
		aggressiveCardBtn = btn_Create( pos, cardSize, cardSize, amtText, font60, CLR_DARK_GREY, textOffset, NULL, boldCardImg, CLR_WHITE, 1, 0, NULL, chooseStyle );
	}

	SDL_snprintf( amtText, 7, "%i", player.strategy );
	if( playerState.chosenStyle == PS_STRATEGY ) {
		chosenStyleBtn = btn_Create( chosenStylePos, cardSize, cardSize, amtText, font60, CLR_DARK_GREY, textOffset, NULL, strategyCardImg, CLR_WHITE, 1, 0, NULL, chooseStyle );
	} else {
		Vector2 pos = vec2( 619.0f, 360.0f );
		vec2_AddScaled( &pos, &cardSize, 0.5f, &pos );
		strategyCardBtn = btn_Create( pos, cardSize, cardSize, amtText, font60, CLR_DARK_GREY, textOffset, NULL, strategyCardImg, CLR_WHITE, 1, 0, NULL, chooseStyle );
	}

	SDL_snprintf( amtText, 7, "%i", player.bluffing );
	if( playerState.chosenStyle == PS_BLUFFING ) {
		chosenStyleBtn = btn_Create( chosenStylePos, cardSize, cardSize, amtText, font60, CLR_DARK_GREY, textOffset, NULL, bluffingCardImg, CLR_WHITE, 1, 0, NULL, chooseStyle );
	} else {
		Vector2 pos = vec2( 707.0f, 360.0f );
		vec2_AddScaled( &pos, &cardSize, 0.5f, &pos );
		bluffingCardBtn = btn_Create( pos, cardSize, cardSize, amtText, font60, CLR_DARK_GREY, textOffset, NULL, bluffingCardImg, CLR_WHITE, 1, 0, NULL, chooseStyle );
	}
}

static void createDiceDisplay( Vector2 upLeft, Vector2 bottomRight, int* sbRolls, int target )
{
	size_t num = sb_Count( sbRolls );

	if( num == 0 ) {
		return;
	}

	int numRows = (int)sqrtf( (float)num );
	int numCols = (int)ceilf( (float)num / (float)numRows );

	Vector2 size;
	vec2_Subtract( &bottomRight, &upLeft, &size );

	float xStep = size.x / ( numCols + 1 );
	float yStep = size.y / ( numRows + 1 );

	for( size_t i = 0; i < num; ++i ) {
		Vector2 offset = vec2( ( ( i / numRows ) + 1 ) * xStep, ( ( i % numRows ) + 1 ) * yStep );
		Vector2 pos;
		vec2_Add( &upLeft, &offset, &pos );
		addDisplayDie( pos, sbRolls[i], ( sbRolls[i] >= target ) );
	}
}

static void createPlayerDice( int* sbRolledDice, int target )
{
	playRoll( );
	Vector2 upLeft = PLAYER_FIELD_POS;
	Vector2 bottomRight;
	vec2_AddScaled( &upLeft, &PLAY_FIELD_SIZE, 0.5f, &bottomRight );
	createDiceDisplay( upLeft, bottomRight, sbRolledDice, target );
}

static void createOpponentDice( int* sbRolledDice, int target )
{
	Vector2 upLeft = OPPONENT_FIELD_POS;
	Vector2 bottomRight;
	vec2_AddScaled( &upLeft, &PLAY_FIELD_SIZE, 0.5f, &bottomRight );
	createDiceDisplay( upLeft, bottomRight, sbRolledDice, target );
}

static void drawCenterText( const char* text )
{
	txt_DisplayTextArea( ( const uint8_t* )text, vec2( 250.0f, 280.0f ), vec2( 300.0f, 40.0f ), CLR_DARK_GREY, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER,
		font20, 0, NULL, 1, 0 );
}

static void drawAreaFade( void )
{
	Color c = clr( 0.0f, 0.0f, 0.0f, 0.75f );
	Vector2 fieldScale;
	getScale( &whiteImgSize, &PLAY_FIELD_SIZE, &fieldScale );

	img_Draw_sv_c( whiteImg, 1, PLAYER_FIELD_POS, PLAYER_FIELD_POS, fieldScale, fieldScale, c, c, 10 );
	img_Draw_sv_c( whiteImg, 1, OPPONENT_FIELD_POS, OPPONENT_FIELD_POS, fieldScale, fieldScale, c, c, 10 );
}

static int winnerImg = -1;
static int loserImg = -1;
static int cheatingImg = -1;

static void loadLocalImages( void )
{
	winnerImg = img_Load( "Images/winner_result.png", ST_DEFAULT );
	loserImg = img_Load( "Images/lost_result.png", ST_DEFAULT );
	cheatingImg = img_Load( "Images/caught_result.png", ST_DEFAULT );
}

static void unloadLocalImages( void )
{
	img_Clean( winnerImg );
	img_Clean( loserImg );
	img_Clean( cheatingImg );

	winnerImg = -1;
	loserImg = -1;
	cheatingImg = -1;
}

static void resultsDrawText( Vector2 center, Vector2 size, Duelist* duelist, DuelistState* state )
{
	char text[256];
	int pos = 0;

	pos = SDL_snprintf( text, ARRAY_SIZE( text ) - pos - 1, "Base %id at Target 4",
		getTypeScoreFromDuelist( duelist, state->chosenType ) + getStyleScoreFromDuelist( duelist, state->chosenStyle ) + state->cheating );

	// if they have a type advantage;
	if( state->hasTypeAdvantage ) {
		pos += SDL_snprintf( text + pos, ARRAY_SIZE( text ) - pos - 1, "\nType Advantage\n  -1 Target" );
	}

	// if they have a style advtange
	if( state->hasStyleAdvantage ) {
		pos += SDL_snprintf( text + pos, ARRAY_SIZE( text ) - pos - 1, "\nStyle Advantage\n  -1 Target" );
	}

	// if their style and type are harmonious
	if( state->hasMatch ) {
		pos += SDL_snprintf( text + pos, ARRAY_SIZE( text ) - pos - 1, "\nType-Style Match\n  +1 Dice" );
	}

	// they they have a netdeck
	if( duelist->deck.isNetDeck ) {
		pos += SDL_snprintf( text + pos, ARRAY_SIZE( text ) - pos - 1, "\nNetdeck Penalty\n  +1 Target" );
	}

	SDL_snprintf( text + pos, ( 255 - pos ), "\n %id at Target %i", state->rolledDice, state->target );

	Vector2 topLeft;
	vec2_AddScaled( &center, &size, -0.5f, &topLeft );
	txt_DisplayTextArea( (const uint8_t*)text, topLeft, size, CLR_WHITE, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font12, 0, NULL, 1, 12 );
}

// this screen will be made of a number of states
//  1. Initial Selection
//  2. Observation Roll
//     a. Roll
//     b. Show results
//  3. Observation Adjustment
//  4. Match Roll
//     a. Show Modifiers
//     b. Roll
//     c. Show results
//  5. Match Results
//     a. Cheater caught
static GameStateMachine screenFSM;

static int initSelect_Enter( void );
static int initSelect_Exit( void );
static void initSelect_Draw( void );
GameState initSelectState = { initSelect_Enter, initSelect_Exit, NULL,
	NULL, initSelect_Draw, NULL };

// Show winner *******************************************************************
#define WINNER_TOTAL_TIME 1.5f
static float winnerDelay;

static int playerResultImg;
static int opponentResultImg;

static int nextPlayerResultImg;
static int nextOpponentResultImg;

static void goHome( int id )
{
	gsmEnterState( &globalFSM, &hubScreenState );
}

static void startNextMatch( int id )
{
	clearDisplayDice( );
	gsmEnterState( &screenFSM, &initSelectState );
}

static int showWinner_Enter( void )
{
	destroyContinueButton( );

	winnerDelay = WINNER_TOTAL_TIME;

	playRoll( );

	if( playerState.finalRoll > opponentState.finalRoll ) {
		nextPlayerResultImg = playerResultImg = winnerImg;
		nextOpponentResultImg = opponentResultImg = loserImg;
		playerWonMatch = true;

		if( caughtCheating( playerState.cheating, player.cheating ) ) {
			nextPlayerResultImg = cheatingImg;
			nextOpponentResultImg = winnerImg;
			playerWonMatch = false;
		}
	} else {
		nextPlayerResultImg = playerResultImg = loserImg;
		nextOpponentResultImg = opponentResultImg = winnerImg;
		playerWonMatch = false;

		if( caughtCheating( opponentState.cheating, currOpponent.cheating ) ) {
			nextPlayerResultImg = winnerImg;
			nextOpponentResultImg = cheatingImg;
			playerWonMatch = true;
		}
	}

	return 1;
}

static int showWinner_Exit( void )
{
	destroyContinueButton( );

	return 1;
}

static void showWinner_Draw( void )
{
	drawAreaFade( );

	// draw the summary
	resultsDrawText( PLAYER_FIELD_POS, PLAY_FIELD_SIZE, &player, &playerState );
	resultsDrawText( OPPONENT_FIELD_POS, PLAY_FIELD_SIZE, &currOpponent, &opponentState );

	Vector2 pos;
	char text[8];

	SDL_snprintf( text, 7, "%i", playerState.finalRoll );
	pos.x = PLAYER_FIELD_POS.x + ( PLAY_FIELD_SIZE.x / 4.0f );
	pos.y = PLAYER_FIELD_POS.y - ( PLAY_FIELD_SIZE.y / 4.0f );
	txt_DisplayString( text, pos, CLR_WHITE, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font60, 1, 100 );

	SDL_snprintf( text, 7, "%i", opponentState.finalRoll );
	pos.x = OPPONENT_FIELD_POS.x + ( PLAY_FIELD_SIZE.x / 4.0f );
	pos.y = OPPONENT_FIELD_POS.y - ( PLAY_FIELD_SIZE.y / 4.0f );
	txt_DisplayString( text, pos, CLR_WHITE, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font60, 1, 100 );

	Vector2 playerResultPos = vec2( 400, 454 );
	Vector2 opponentResultPos = vec2( 400, 146 );
	img_Draw( playerResultImg, 1, playerResultPos, playerResultPos, 120 );
	img_Draw( opponentResultImg, 1, opponentResultPos, opponentResultPos, 120 );

	if( winnerDelay >= ( WINNER_TOTAL_TIME / 2.0f ) ) {
		drawCenterText( "Trying to find cheaters." );
	}
}

static void showWinner_PhysicsTick( float dt )
{
	winnerDelay -= dt;

	if( ( winnerDelay < ( WINNER_TOTAL_TIME / 2.0f ) ) && ( nextPlayerResultImg != playerResultImg ) ) {
		snd_Play( cheatingSnd, 1.0f, 1.0f, 0.0f, 0 );
		playerResultImg = nextPlayerResultImg;
		opponentResultImg = nextOpponentResultImg;
	}

	if( ( winnerDelay <= 0.0f ) && ( continueButton == -1 ) ) {
		if( !playerWonMatch ) {
			createContinueButton( "Go Home", goHome );
		} else {
			++currTournament.playerPos;
			if( currTournament.playerPos < 3 ) {
				createContinueButton( "Next Match", startNextMatch );
			} else {
				createContinueButton( "Victory!", goHome );
			}
		}
	}
}

GameState showWinnerState = { showWinner_Enter, showWinner_Exit, NULL,
	NULL, showWinner_Draw, showWinner_PhysicsTick };

// Show results ******************************************************************
static void showResultsDone( int id )
{
	gsmEnterState( &screenFSM, &showWinnerState );
}

#define RESULTS_TOTAL_TIME 1.5f
static float showResultsDelay;
static int showResults_Enter( void )
{
	opponentStyleRevealed = true;
	opponentTypeRevealed = true;
	opponentCheatingRevealed = true;

	// do the resolution
	calculatePool( &player, &playerState, &opponentState );
	calculatePool( &currOpponent, &opponentState, &playerState );

	int* sbPlayerDice = NULL;
	int* sbOpponentDice = NULL;

	sb_Add( sbPlayerDice, playerState.rolledDice );
	sb_Add( sbOpponentDice, opponentState.rolledDice );

	// roll off until we have a winner
	do {
		playerState.finalRoll = rollDice( sbPlayerDice, playerState.target );
		opponentState.finalRoll = rollDice( sbOpponentDice, opponentState.target );
	} while( playerState.finalRoll == opponentState.finalRoll );

	createPlayerDice( sbPlayerDice, playerState.target );
	createOpponentDice( sbOpponentDice, opponentState.target );

	sb_Release( sbPlayerDice );
	sb_Release( sbOpponentDice );

	showResultsDelay = RESULTS_TOTAL_TIME;
	return 1;
}

static int showResults_Exit( void )
{
	return 1;
}

static void showResults_Draw( void )
{
	drawAreaFade( );

	// draw the summary
	resultsDrawText( PLAYER_FIELD_POS, PLAY_FIELD_SIZE, &player, &playerState );
	resultsDrawText( OPPONENT_FIELD_POS, PLAY_FIELD_SIZE, &currOpponent, &opponentState );

	if( showResultsDelay < ( RESULTS_TOTAL_TIME * 0.5f ) ) {
		Vector2 pos;
		char text[8];

		SDL_snprintf( text, 7, "%i", playerState.finalRoll );
		pos.x = PLAYER_FIELD_POS.x + ( PLAY_FIELD_SIZE.x / 4.0f );
		pos.y = PLAYER_FIELD_POS.y - ( PLAY_FIELD_SIZE.y / 4.0f );
		txt_DisplayString( text, pos, CLR_WHITE, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font60, 1, 100 );

		SDL_snprintf( text, 7, "%i", opponentState.finalRoll );
		pos.x = OPPONENT_FIELD_POS.x + ( PLAY_FIELD_SIZE.x / 4.0f );
		pos.y = OPPONENT_FIELD_POS.y - ( PLAY_FIELD_SIZE.y / 4.0f );
		txt_DisplayString( text, pos, CLR_WHITE, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font60, 1, 100 );
	}
}

static void showResults_PhysicsTick( float dt )
{
	showResultsDelay -= dt;
	if( ( showResultsDelay <= 0.0f ) && continueButton == -1 ) {
		createContinueButton( "GO TO RESULTS", showResultsDone );
	}
}

GameState showResultsState = { showResults_Enter, showResults_Exit, NULL,
	NULL, showResults_Draw, showResults_PhysicsTick };

// Player or Both Observation adjustment *****************************************
static int revealStyleBtn = -1;
static int revealTypeBtn = -1;
static int revealCheatingBtn = -1;

static void observeAdjustDone( int id )
{
	lockValues = true;
	gsmEnterState( &screenFSM, &showResultsState );
}

static void reveal( int id )
{
	if( id == revealStyleBtn ) {
		playCard( );
		opponentStyleRevealed = true;
	} else if( id == revealTypeBtn ) {
		playCard( );
		opponentTypeRevealed = true;
	} else {
		playClick( );
		opponentCheatingRevealed = true;
	}

	btn_Destroy( revealStyleBtn );
	btn_Destroy( revealTypeBtn );
	btn_Destroy( revealCheatingBtn );

	lockValues = false;

	createContinueButton( "CONTINUE", observeAdjustDone );
}

static DuelistState newOpponentState;
static int observeAdjust_Enter( void )
{
	// calculate the what they should do based on their ai, should all be done before the player makes any changes
	newOpponentState = opponentState;
	if( playerState.finalRoll == opponentState.finalRoll ) {
		playCard( );
		aiObserve( &currOpponent, &newOpponentState, &player, &playerState );
	}

	// create buttons for revealing
	Vector2 pos;
	Vector2 size;
	Vector2 finalSize;

	size = PLAY_FIELD_SIZE;
	size.x /= 2.0f;

	//  reveal style
	pos = OPPONENT_FIELD_POS;
	pos.x += size.x * 0.5f;
	finalSize = size;
	finalSize.x -= 4.0f;
	finalSize.y -= 4.0f;
	revealStyleBtn = btn_Create( pos, finalSize, finalSize, "Reveal style", font20, CLR_DARK_GREY, vec2( 0, 73 ), NULL, whiteImg, CLR_WHITE, 1, -55, NULL, reveal );

	//  reveal type
	pos = OPPONENT_FIELD_POS;
	pos.x -= size.x * 0.5f;
	finalSize = size;
	finalSize.x -= 4.0f;
	finalSize.y -= 4.0f;
	revealTypeBtn = btn_Create( pos, finalSize, finalSize, "Reveal type", font20, CLR_DARK_GREY, vec2( 0, 73 ), NULL, whiteImg, CLR_WHITE, 1, -55, NULL, reveal );

	//  reveal cheating
	pos = OPPONENT_FIELD_POS;
	size = PLAY_FIELD_SIZE;
	pos.y = ( pos.y - ( size.y * 0.5f ) ) / 2.0f;
	size.y = pos.y * 2.0f;
	finalSize = size;
	finalSize.x -= 4.0f;
	finalSize.y -= 4.0f;
	revealCheatingBtn = btn_Create( pos, finalSize, finalSize, "Reveal cheating", font20, CLR_DARK_GREY, vec2( 0, 35 ), NULL, whiteImg, CLR_WHITE, 1, -55, NULL, reveal );

	destroyContinueButton( );
	return 1;
}

static int observeAdjust_Exit( void )
{
	opponentStyleRevealed = false;
	opponentTypeRevealed = false;
	opponentCheatingRevealed = false;

	// finalize the opponent state
	opponentState = newOpponentState;

	destroyContinueButton( );

	return 1;
}

static void observeAdjust_Draw( void )
{
	//btn_DebugDraw( CLR_WHITE, CLR_RED, CLR_GREEN );
	if( !opponentCheatingRevealed && !opponentTypeRevealed && !opponentStyleRevealed ) {
		char* text = "Choose what to reveal.";
		if( playerState.finalRoll == opponentState.finalRoll ) {
			text = "Choose what to reveal. Your opponent is doing the same.";
		}
		drawCenterText( text );
	}

	if( refreshCardButtons ) {
		setPlayerCardButtons( );
		refreshCardButtons = false;
	}
}

GameState observeAdjustState = { observeAdjust_Enter, observeAdjust_Exit, NULL,
	NULL, observeAdjust_Draw, NULL };

// Opponent Observation adjustment ***********************************************
static float opponentAdjustDelay;
static int opponentAdjust_Enter( void )
{
	opponentAdjustDelay = 0.5f;

	// calculate the what they should do based on their ai
	playCard( );
	aiObserve( &currOpponent, &opponentState, &player, &playerState );
	//llog( LOG_VERBOSE, "ai choices: %i  %i  %i", opponentState.chosenStyle, opponentState.chosenType, opponentState.cheating );

	destroyContinueButton( );
	return 1;
}

static int opponentAdjust_Exit( void )
{
	return 1;
}

static void opponentAdjust_Draw( void )
{
	drawCenterText( "Opponent is adjusting their choices." );
}

static void opponentAdjust_PhysicsTick( float dt )
{
	opponentAdjustDelay -= dt;
	if( opponentAdjustDelay <= 0.0f ) {
		gsmEnterState( &screenFSM, &showResultsState );
	}
}

GameState opponentAdjustState = { opponentAdjust_Enter, opponentAdjust_Exit, NULL,
	NULL, opponentAdjust_Draw, opponentAdjust_PhysicsTick };

// Observation roll **************************************************************
#define OBSERVE_TOTAL_TIME 1.0f
static float observationRollDelay;

static void finishObserveRoll( int id )
{
	if( playerState.finalRoll >= opponentState.finalRoll ) {
		gsmEnterState( &screenFSM, &observeAdjustState );
	} else {
		gsmEnterState( &screenFSM, &opponentAdjustState );
	}
}

static int observationRoll_Enter( void )
{
	observationRollDelay = OBSERVE_TOTAL_TIME;

	// roll both duelists observations
	int* sbRolledDice = NULL;
	sb_Add( sbRolledDice, player.observation );
	playerState.target = 4 + ( player.deck.isNetDeck ? 1 : 0 );
	playerState.rolledDice = player.observation;
	playerState.finalRoll = rollDice( sbRolledDice, playerState.target );
	createPlayerDice( sbRolledDice, playerState.target );


	sb_Clear( sbRolledDice );
	sb_Add( sbRolledDice, currOpponent.observation );
	opponentState.target = 4 + ( currOpponent.deck.isNetDeck ? 1 : 0 );
	opponentState.rolledDice = player.observation;
	opponentState.finalRoll = rollDice( sbRolledDice, opponentState.target );
	createOpponentDice( sbRolledDice, opponentState.target );

	sb_Release( sbRolledDice );

	destroyContinueButton( );
	return 1;
}

static int observationRoll_Exit( void )
{
	clearDisplayDice( );
	return 1;
}

static void observationDrawText( Vector2 center, Vector2 size, Duelist* duelist, DuelistState* state )
{
	char text[256];
	int pos;
	pos = SDL_snprintf( text, 255, "Observation Stat: %i", duelist->observation );
	if( duelist->deck.isNetDeck ) {
		pos = pos + SDL_snprintf( text + pos, ( 255 - pos ), "\nNet deck penalty:\n  +1 Target" );
	}
	SDL_snprintf( text + pos, ( 255 - pos ), "\n %i Dice at Target %i", state->rolledDice, state->target );

	Vector2 topLeft;
	vec2_AddScaled( &center, &size, -0.5f, &topLeft );
	txt_DisplayTextArea( (const uint8_t*)text, topLeft, size, CLR_WHITE, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font12, 0, NULL, 1, 12 );
}

static void observationRoll_Draw( void )
{
	drawAreaFade( );

	observationDrawText( PLAYER_FIELD_POS, PLAY_FIELD_SIZE, &player, &playerState );
	observationDrawText( OPPONENT_FIELD_POS, PLAY_FIELD_SIZE, &currOpponent, &opponentState );

	if( continueButton == -1 ) {
		drawCenterText( "Rolling observation." );
	}

	if( observationRollDelay < ( OBSERVE_TOTAL_TIME * 0.5f ) ) {
		Vector2 pos;
		char text[8];

		SDL_snprintf( text, 7, "%i", playerState.finalRoll );
		pos.x = PLAYER_FIELD_POS.x + ( PLAY_FIELD_SIZE.x / 4.0f );
		pos.y = PLAYER_FIELD_POS.y - ( PLAY_FIELD_SIZE.y / 4.0f );
		txt_DisplayString( text, pos, CLR_WHITE, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font60, 1, 100 );

		SDL_snprintf( text, 7, "%i", opponentState.finalRoll );
		pos.x = OPPONENT_FIELD_POS.x + ( PLAY_FIELD_SIZE.x / 4.0f );
		pos.y = OPPONENT_FIELD_POS.y - ( PLAY_FIELD_SIZE.y / 4.0f );
		txt_DisplayString( text, pos, CLR_WHITE, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font60, 1, 100 );
	}
}

static void observationRoll_PhysicsTick( float dt )
{
	observationRollDelay -= dt;
	if( ( observationRollDelay <= 0.0f ) && ( continueButton == -1 ) ) {
		char* label;
		if( playerState.finalRoll >= opponentState.finalRoll ) {
			label = "OBSERVE OPPONENT";
		} else {
			label = "CONTINUE";
		}
		createContinueButton( label, finishObserveRoll );
	}
}

GameState observationRollState = { observationRoll_Enter, observationRoll_Exit, NULL,
	NULL, observationRoll_Draw, observationRoll_PhysicsTick };

// Initial selection state *******************************************************
static void finishInitState( int id )
{
	gsmEnterState( &screenFSM, &observationRollState );
}

static int initSelect_Enter( void )
{
	clearState( &playerState );
	clearState( &opponentState );

	opponentCheatingRevealed = false;
	opponentStyleRevealed = false;
	opponentTypeRevealed = false;
	currOpponent = currTournament.members[currTournament.playerPos];

	destroyContinueButton( );
	setPlayerCardButtons( );
	refreshCardButtons = false;
	lockValues = false;

	aiChoose( &currOpponent, &opponentState );

	return 1;
}

static int initSelect_Exit( void )
{
	//btn_DestroyAll( );
	return 1;
}

static void initSelect_Draw( void )
{
	if( refreshCardButtons ) {
		setPlayerCardButtons( );
		refreshCardButtons = false;
		if( ( playerState.chosenStyle != PS_NONE ) && ( playerState.chosenType != CT_NONE ) ) {
			createContinueButton( "CONTINUE", finishInitState );
		} else {
			destroyContinueButton( );
		}
	}

	if( ( playerState.chosenStyle == PS_NONE ) || ( playerState.chosenType == CT_NONE ) ) {
		drawCenterText( "Set how much you want to cheat and choose a style and type." );
	}

	//btn_DebugDraw( CLR_YELLOW, CLR_GREEN, CLR_RED );
}

// ******************** General stuff
static bool showHelp;

static void toggleHelp( int id )
{
	showHelp = !showHelp;
}

static void createTestDuelist( Duelist* d )
{
	assert( true || "Test only, don't do this." );
	d->deckBuilding = rand_GetToleranceS32( NULL, 3, 2 );
	d->cheating = rand_GetToleranceS32( NULL, 3, 2 );
	d->observation = rand_GetToleranceS32( NULL, 3, 2 );
	d->trading = rand_GetToleranceS32( NULL, 3, 2 );
	d->strategy = rand_GetToleranceS32( NULL, 3, 2 );
	d->bluffing = rand_GetToleranceS32( NULL, 3, 2 );
	d->bold = rand_GetToleranceS32( NULL, 3, 2 );
	d->deck.aggro = rand_GetToleranceS32( NULL, 3, 2 );
	d->deck.combo = rand_GetToleranceS32( NULL, 3, 2 );
	d->deck.control = rand_GetToleranceS32( NULL, 3, 2 );
	d->deck.isNetDeck = rand_Choice( NULL );
	d->smarts = S_AVERAGE;
}

static int matchScreen_Enter( void )
{
	snd_PlayStreaming( matchMusic, 0.5f, 0.0f );
	// init values
	showHelp = false;

	playerState.cheating = 0;
	playerState.chosenStyle = PS_NONE;
	playerState.chosenType = CT_NONE;

	opponentState.cheating = 0;
	opponentState.chosenStyle = PS_NONE;
	opponentState.chosenType = CT_NONE;

	opponentCheatingRevealed = false;
	opponentTypeRevealed = false;
	opponentStyleRevealed = false;
	clearDisplayDice( );

	aggroCardBtn = -1;
	comboCardBtn = -1;
	controlCardBtn = -1;
	strategyCardBtn = -1;
	bluffingCardBtn = -1;
	aggressiveCardBtn = -1;
	chosenTypeBtn = -1;
	chosenStyleBtn = -1;

	// create test player and test opponent
	//createTestDuelist( &player );
	//createTestDuelist( &currOpponent );

	cam_TurnOnFlags( 0, 1 );

	gfx_SetClearColor( CLR_BLACK );

	btn_Init( );
	btn_RegisterSystem( );

	screenFSM.currentState = NULL;
	gsmEnterState( &screenFSM, &initSelectState );

	createCheatButtons( );

	loadLocalImages( );

	btn_Create( vec2( 15, 15 ), vec2( 24, 24 ), vec2( 24, 24 ), "?", font12, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 100, NULL, toggleHelp );

	return 1;
}

static int matchScreen_Exit( void )
{
	snd_StopStreaming( matchMusic );
	unloadLocalImages( );

	btn_DestroyAll( );
	return 1;
}

static void matchScreen_ProcessEvents( SDL_Event* e )
{
	gsmProcessEvents( &screenFSM, e );
}

static void matchScreen_Process( void )
{
	gsmProcess( &screenFSM );
}

static void matchScreen_Draw( void )
{
	Vector2 centerLineScale;
	Vector2 centerLineSize = vec2( 800.0f, 56.0f );
	getScale( &whiteImgSize, &centerLineSize, &centerLineScale );

	// base draw
	img_Draw_sv( whiteImg, 1, vec2( 400.0f, 300.0f ), vec2( 400.0f, 300.0f ), centerLineScale, centerLineScale, -100 );

	// play field areas
	img_Draw3x3v( fieldImgs, 1, PLAYER_FIELD_POS, PLAYER_FIELD_POS, PLAY_FIELD_SIZE, PLAY_FIELD_SIZE, -100 );
	img_Draw3x3v( fieldImgs, 1, OPPONENT_FIELD_POS, OPPONENT_FIELD_POS, PLAY_FIELD_SIZE, PLAY_FIELD_SIZE, -100 );

	//void txt_DisplayString( const char* utf8Str, Vector2 pos, Color clr, HorizTextAlignment hAlign, VertTextAlignment vAlign,
//		int fontID, int camFlags, int8_t depth )

	txt_DisplayString( "CARD TYPES", vec2( 100.0f, 300.0f ), CLR_DARK_GREY, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font22, 1, -50 );
	txt_DisplayString( "PLAY STYLES", vec2( 700.0f, 300.0f ), CLR_DARK_GREY, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font22, 1, -50 );

	// player cheating display
	char cheatTxt[32];
	SDL_snprintf( cheatTxt, 31, "%i", playerState.cheating );
	txt_DisplayString( cheatTxt, vec2( 400.0f, 569.0f ), CLR_GREY, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font22, 1, -50 );

	SDL_snprintf( cheatTxt, 31, "CHEATING STAT %i", player.cheating );
	txt_DisplayString( cheatTxt, vec2( 400.0f, 538.0f ), CLR_GREY, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font24, 1, -50 );

	// opponent cheating
	if( opponentCheatingRevealed ) {
		SDL_snprintf( cheatTxt, 31, "%i", opponentState.cheating );
		txt_DisplayString( cheatTxt, vec2( 400.0f, 27.0f ), CLR_GREY, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font22, 1, -50 );

		txt_DisplayString( "CHEATING", vec2( 400.0f, 50.0f ), CLR_GREY, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font24, 1, -50 );
	} else {
		txt_DisplayString( "CHEATING?", vec2( 400.0f, 50.0f ), CLR_GREY, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font24, 1, -50 );
	}

	txt_DisplayString( "YOU", vec2( 20.0f, 580.0f ), CLR_GREEN, HORIZ_ALIGN_LEFT, VERT_ALIGN_BASE_LINE, font60, 1, -50 );
	txt_DisplayString( currOpponent.name, vec2( 755.0f, 0.0f ), CLR_RED, HORIZ_ALIGN_RIGHT, VERT_ALIGN_TOP, font60, 1, -50 );

	// draw opponents decks
	img_Draw( unknownCardImg, 1, vec2( 145, 180 ), vec2( 145, 180 ), -25 );
	img_Draw( unknownCardImg, 1, vec2( 655, 180 ), vec2( 655, 180 ), -25 );

	// draw opponent hidden choices
	if( opponentTypeRevealed ) {
		Vector2 pos = vec2( 340, 177 );
		img_Draw( imageForType( opponentState.chosenType ), 1, pos, pos, -25 );
		char text[8];
		SDL_snprintf( text, 7, "%i", getTypeScoreFromDuelist( &currOpponent, opponentState.chosenType ) );
		txt_DisplayString( text, pos, CLR_DARK_GREY, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font60, 1, -24 );
	} else {
		img_Draw( unknownCardImg, 1, vec2( 340, 177 ), vec2( 340, 177 ), -25 );
	}

	if( opponentStyleRevealed ) {
		Vector2 pos = vec2( 460, 177 );
		img_Draw( imageForStyle( opponentState.chosenStyle ), 1, pos, pos, -25 );
		char text[8];
		SDL_snprintf( text, 7, "%i", getStyleScoreFromDuelist( &currOpponent, opponentState.chosenStyle ) );
		txt_DisplayString( text, pos, CLR_DARK_GREY, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font60, 1, -24 );
	} else {
		img_Draw( unknownCardImg, 1, vec2( 460, 177 ), vec2( 460, 177 ), -25 );
	}

	drawDisplayDice( );

	gsmDraw( &screenFSM );

	if( showHelp ) {
		Vector2 desiredSize = vec2( 200, 180 );
		Vector2 scale;
		Vector2 pos = vec2( 40, 44 );
		Vector2 textPos = vec2( 40, 44 );
		Color c = clr( 0, 0, 0, 0.9f );

		vec2_AddScaled( &pos, &desiredSize, 0.5f, &pos );
		
		getScale( &whiteImgSize, &desiredSize, &scale );
		img_Draw_sv_c( whiteImg, 1, pos, pos, scale, scale, c, c, 115 );
		txt_DisplayString( "Aggro beats Control\nControl beats Combo\nCombo beats Aggro\n\nBold beats Bluffing\nBluffing beats Strategy\nStrategy beats Bold\n\nAggro works with Bold\nCombo works with Strategy\nControl works with Bluffing",
			textPos, CLR_LIGHT_GREY, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font12, 1, 116 );
	}
}

static void matchScreen_PhysicsTick( float dt )
{
	gsmPhysicsTick( &screenFSM, dt );
}

GameState matchScreenState = { matchScreen_Enter, matchScreen_Exit, matchScreen_ProcessEvents,
	matchScreen_Process, matchScreen_Draw, matchScreen_PhysicsTick };