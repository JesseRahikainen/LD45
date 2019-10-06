#include "hubScreen.h"

#include "../Graphics/graphics.h"
#include "../Graphics/images.h"
#include "../Graphics/spineGfx.h"
#include "../Graphics/camera.h"
#include "../Graphics/debugRendering.h"
#include "../Graphics/imageSheets.h"
#include "../UI/button.h"
#include "../UI/text.h"
#include "resources.h"
#include "characterScreen.h"
#include "matchScreen.h"
#include "../sound.h"

static const char* tradeResult;
static bool showingTradeResult;
static int hideTradeBtn;
static bool wasInMatch;
static int pointGainDifficulty;
static bool firstMatch = true;

static bool testStatPointGain( void )
{
	playRoll( );

	if( firstMatch ) {
		firstMatch = false;
		return true;
	}

	int playerTotal = player.deckBuilding + player.cheating + player.trading + player.observation + player.strategy + player.bluffing + player.bold + player.statPointsAvailable;

	if( playerTotal < pointGainDifficulty ) {
		return true;
	}

	int totalRoll = stealthRollDice( playerTotal, 4 );
	int matchRoll = stealthRollDice( pointGainDifficulty, 4 );

	return ( totalRoll >= matchRoll );
}

static void hideTradeResult( int id )
{
	playClick( );
	showingTradeResult = false;
	btn_Destroy( hideTradeBtn );
	hideTradeBtn = -1;
}

static void startTournament( void )
{
	playClick( );
	wasInMatch = true;
	currTournament.playerPos = 0;
	snd_StopStreaming( menuMusic );
	gsmEnterState( &globalFSM, &matchScreenState );
}

static void playLocal( int id )
{
	if( showingTradeResult ) return;
	currTournament = generateLocalTournament( );
	wasTournamentMasters = false;
	pointGainDifficulty = 24;
	startTournament( );
}

static void playNational( int id )
{
	if( showingTradeResult ) return;
	currTournament = generateNationalTournament( );
	wasTournamentMasters = false;
	pointGainDifficulty = 51;
	startTournament( );
}

static void playInterational( int id )
{
	if( showingTradeResult ) return;
	currTournament = generateInternationalTournament( );
	wasTournamentMasters = false;
	pointGainDifficulty = 67;
	startTournament( );
}

static void playMasters( int id )
{
	if( showingTradeResult ) return;
	currTournament = generateMastersTournament( );
	wasTournamentMasters = true;
	pointGainDifficulty = 1000;
	startTournament( );
}

static void editCharacter( int id )
{
	if( showingTradeResult ) return;

	playClick( );

	gsmEnterState( &globalFSM, &characterScreenState );
}

static void showTradingResult( const char* text )
{
	showingTradeResult = true;
	tradeResult = text;
	hideTradeBtn = btn_Create( vec2( 400, 300 ), vec2( 800, 600 ), vec2( 800, 600 ), NULL, -1, CLR_WHITE, VEC2_ZERO, NULL, whiteImg, clr( 0.0f, 0.0f, 0.0f, 0.9f ), 1, 25, NULL, hideTradeResult );
}

static void trade( int id )
{
	const char* text;
	if( tradedRecently ) {
		playClick( );
		text = "You've traded too recently.\nYou need to play a tournament\nbefore trading again.\n\nClick anywhere to continue";
	} else {

		playRoll( );
		// for right now trading will just be an opposed trade roll vs a collection roll, if the trade roll wins you get a card point
		int collectionSize = player.collection.aggro + player.collection.combo + player.collection.control;

		int collectionRoll = stealthRollDice( collectionSize, 4 );
		int tradeRoll = stealthRollDice( player.trading, 4 );

		if( tradeRoll > ( collectionRoll * 2 ) ) {
			// crit success
			text = "You've got an exceptionally good trade.\nYou gain 2 card points.\n\nClick anywhere to continue";
			player.cardPointsAvailable += 2;
		} else if( tradeRoll >= collectionRoll ) {
			// success
			text = "You've successfully traded.\nYou gain 1 card point.\n\nClick anywhere to continue";
			player.cardPointsAvailable += 1;
		} else {
			// failure
			text = "You were unable to find any good trades.\n\nClick anywhere to continue";
		}

		tradedRecently = true;
	}

	showTradingResult( text );
}

static int hubScreen_Enter( void )
{
	snd_PlayStreaming( menuMusic, 0.5f, 0.0f );

	cam_TurnOnFlags( 0, 1 );

	gfx_SetClearColor( CLR_BLACK );

	btn_RegisterSystem( );
	btn_Init( );

	// start tournament buttons
	btn_Create( vec2( 600, 325 ), vec2( 275, 65 ), vec2( 276, 66 ), "Play Local Tournament", font20, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 0, NULL, playLocal );
	btn_Create( vec2( 600, 400 ), vec2( 275, 65 ), vec2( 276, 66 ), "Play National Tournament", font20, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 0, NULL, playNational );
	btn_Create( vec2( 600, 475 ), vec2( 275, 65 ), vec2( 276, 66 ), "Play International Tournament", font20, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 0, NULL, playInterational );
	btn_Create( vec2( 600, 550 ), vec2( 275, 65 ), vec2( 276, 66 ), "Play Masters Tournament", font20, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 0, NULL, playMasters );

	btn_Create( vec2( 200, 475 ), vec2( 275, 65 ), vec2( 276, 66 ), "Edit Character & Deck", font20, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 0, NULL, editCharacter );

	btn_Create( vec2( 200, 550 ), vec2( 275, 65 ), vec2( 276, 66 ), "Trade", font20, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 0, NULL, trade );

	// if we entered from a match do some upkeep
	if( wasInMatch ) {

		setupMeta( );
		tradedRecently = false;

		if( playerWonMatch ) {
			if( wasTournamentMasters && !hasBeatGame ) {
				player.statPointsAvailable += 2;
				player.cardPointsAvailable += 2;
				showTradingResult( "You won a masters tournament and beat the game!\nGood job!\n\nClick anywhere to continue" );

				hasBeatGame = true;
			} else {
				if( testStatPointGain( ) ) {
					player.statPointsAvailable += 1;
					showTradingResult( "You won a tournament and gained a stat point!\n\nClick anywhere to continue" );
				} else {
					showTradingResult( "You won a tournament!\nBut it wasn't challenging enough to teach you anything.\nTry a more difficult tournament.\n\nClick anywhere to continue" );
				}
			}
		}

		wasInMatch = false;
	}

	return 1;
}

static int hubScreen_Exit( void )
{
	btn_DestroyAll( );

	return 1;
}

static void hubScreen_ProcessEvents( SDL_Event* e )
{}

static void hubScreen_Process( void )
{}

static void hubScreen_Draw( void )
{
	char charText[512];

	SDL_snprintf( charText, 511,
		"Player Stats\n"
	"  Deck Building: %i\n"
	"  Cheating: %i\n"
	"  Trading: %i\n"
	"  Observation: %i\n"
	"  Strategy: %i\n"
	"  Bluffing: %i\n"
	"  Bold: %i",
		player.deckBuilding, player.cheating, player.trading, player.observation, player.strategy, player.bluffing, player.bold );
	txt_DisplayString( charText, vec2( 20, 20 ), CLR_WHITE, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font20, 1, 0 );

	SDL_snprintf( charText, 511,
		"Collection\n"
		"  Aggro: %i\n"
		"  Combo: %i\n"
		"  Control: %i\n\n"
		"Current Deck\n"
		"  Aggro: %i\n"
		"  Combo: %i\n"
		"  Control: %i",
		player.collection.aggro, player.collection.combo, player.collection.control, player.deck.aggro, player.deck.combo, player.deck.control );
	txt_DisplayString( charText, vec2( 200, 20 ), CLR_WHITE, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font20, 1, 0 );

	const char* goalText = "The final goal of the game is to win a\n"
		"masters tournament.\n"
		"Play through minor tournaments to get\n"
		"your skills up and trade to build your\n"
		"collection.\n"
		"Good luck!";
	Color goalClr = CLR_WHITE;
	if( hasBeatGame ) {
		goalText = "You've won a masters tournament and\n"
			"beaten the game.\nCongratulations!";
		goalClr = CLR_YELLOW;
	}
	txt_DisplayString( goalText, vec2( 450, 20 ), CLR_WHITE, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font20, 1, 0 );

	SDL_snprintf( charText, 511,
		"Play Style Meta: %s\n"
		"Card Type Meta: %s",
		getStyleName( metaStyle ), getTypeName( metaType ) );
	txt_DisplayString( charText, vec2( 750, 275 ), CLR_YELLOW, HORIZ_ALIGN_RIGHT, VERT_ALIGN_BOTTOM, font20, 1, 0 );

	int pos = 0;
	if( player.statPointsAvailable != 0 ) {
		pos += SDL_snprintf( charText + pos, 511 - pos, "You have stat points available to spend!\n\n" );
	}

	if( player.cardPointsAvailable != 0 ) {
		pos += SDL_snprintf( charText + pos, 511 - pos, "You have card points available to spend!\n" );
	}

	if( pos >> 0 ) {
		txt_DisplayString( charText, vec2( 20, 350 ), CLR_YELLOW, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font20, 1, 0 );
	}

	if( showingTradeResult ) {
		txt_DisplayString( tradeResult, vec2( 400, 300 ), CLR_WHITE, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font20, 1, 25 );
	}
}

static void hubScreen_PhysicsTick( float dt )
{}

GameState hubScreenState = { hubScreen_Enter, hubScreen_Exit, hubScreen_ProcessEvents,
	hubScreen_Process, hubScreen_Draw, hubScreen_PhysicsTick };
