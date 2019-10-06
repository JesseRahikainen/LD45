#include "characterScreen.h"

#include "../Graphics/graphics.h"
#include "../Graphics/images.h"
#include "../Graphics/spineGfx.h"
#include "../Graphics/camera.h"
#include "../Graphics/debugRendering.h"
#include "../Graphics/imageSheets.h"
#include "../UI/text.h"
#include "../UI/button.h"
#include "resources.h"
#include "hubScreen.h"
#include "../sound.h"

static bool firstTimeEntered = true;

static bool showingHelp;
static const char* helpText;
static int helpBtn;

static int statAddButtons[7];
static int collectionAddButtons[3];

static void setupDeckButtons( void );

static void hideHelp( int id )
{
	playClick( );
	btn_Destroy( helpBtn );
	helpBtn = -1;
	showingHelp = false;
}

static void showHelp( const char* text )
{
	helpText = text;
	showingHelp = true;
	helpBtn = btn_Create( vec2( 400, 300 ), vec2( 800, 600 ), vec2( 800, 600 ), NULL, font14, CLR_WHITE, VEC2_ZERO, NULL, whiteImg, clr( 0, 0, 0, 0.95f ), 1, 100, NULL, hideHelp );
}

static void showDeckHelp( int id )
{
	playClick( );
	showHelp( "Deck creation is done by adding and removing card types.\n"
		"The maximum number of points in a deck is equal to your Deck Building stat.\n"
		"This Deck Building stat restriction can be circumvented by using a net deck.\n\n"
		"The maximum number of points of a card type allowed in your deck is restricted\n"
		"by the amount of that type in your Card Collection.\n\n"
		"Click anywhere to continue" );
}

static void showMetaHelp( int id )
{
	playClick( );
	showHelp( "The meta can effect how your opponents will play\n"
		"and what decks they will build. Keep this in mind when in a\n"
		"tournament.\n\n"
		"The meta will change occasionally. When it changes so will\n"
		"the net decks. So keep an eye out for it.\n\n"
		"Net decks will let you get around not being able to make decks\n"
		"with more points in them than your Deck Building stat.\n"
		"To use a Net Deck you will have to have enough points for it in\n"
		"your Card Collection.\n"
		"Using a Net Deck will make you predictable and give you a\n"
		"penalty when playing against an opponent.\n\n"
		"Click anywhere to continue" );
}

static int useNetDeckOneBtn;
static int useNetDeckTwoBtn;
static void useNetDeck( int id )
{
	if( showingHelp ) return;

	playCard( );

	if( id == useNetDeckOneBtn ) {
		player.deck = netDeckOne;
	} else {
		player.deck = netDeckTwo;
	}
	setupDeckButtons( );
}

static void destroyAllStatAddButtons( void )
{
	for( int i = 0; i < 7; ++i ) {
		btn_Destroy( statAddButtons[i] );
		statAddButtons[i] = -1;
	}
}

static void destroyCollectionAddButtons( void )
{
	for( int i = 0; i < 3; ++i ) {
		btn_Destroy( collectionAddButtons[i] );
		collectionAddButtons[i] = -1;
	}
}

static void addDeckBuilding( int id )
{
	if( showingHelp ) return;

	playClick( );

	player.deckBuilding += 1;
	player.statPointsAvailable -= 1;

	setupDeckButtons( );

	if( player.statPointsAvailable <= 0 ) {
		destroyAllStatAddButtons( );
	}
}

static void addCheating( int id )
{
	if( showingHelp ) return;

	playClick( );

	player.cheating += 1;
	player.statPointsAvailable -= 1;

	if( player.statPointsAvailable <= 0 ) {
		destroyAllStatAddButtons( );
	}
}

static void addTrading( int id )
{
	if( showingHelp ) return;

	playClick( );

	player.trading += 1;
	player.statPointsAvailable -= 1;

	if( player.statPointsAvailable <= 0 ) {
		destroyAllStatAddButtons( );
	}
}

static void addObservation( int id )
{
	if( showingHelp ) return;

	playClick( );

	player.observation += 1;
	player.statPointsAvailable -= 1;

	if( player.statPointsAvailable <= 0 ) {
		destroyAllStatAddButtons( );
	}
}

static void addStrategy( int id )
{
	if( showingHelp ) return;

	playClick( );

	player.strategy += 1;
	player.statPointsAvailable -= 1;

	if( player.statPointsAvailable <= 0 ) {
		destroyAllStatAddButtons( );
	}
}

static void addBluffing( int id )
{
	if( showingHelp ) return;

	playClick( );

	player.bluffing += 1;
	player.statPointsAvailable -= 1;

	if( player.statPointsAvailable <= 0 ) {
		destroyAllStatAddButtons( );
	}
}

static void addBold( int id )
{
	if( showingHelp ) return;

	playClick( );

	player.bold += 1;
	player.statPointsAvailable -= 1;

	if( player.statPointsAvailable <= 0 ) {
		destroyAllStatAddButtons( );
	}
}

static void addCollectionCombo( int id )
{
	if( showingHelp ) return;

	playClick( );

	player.collection.combo += 1;
	player.cardPointsAvailable -= 1;

	setupDeckButtons( );

	if( player.cardPointsAvailable <= 0 ) {
		destroyCollectionAddButtons( );
	}
}

static void addCollectionControl( int id )
{
	if( showingHelp ) return;

	playClick( );

	player.collection.control += 1;
	player.cardPointsAvailable -= 1;

	setupDeckButtons( );

	if( player.cardPointsAvailable <= 0 ) {
		destroyCollectionAddButtons( );
	}
}

static void addCollectionAggro( int id )
{
	if( showingHelp ) return;

	playClick( );

	player.collection.aggro += 1;
	player.cardPointsAvailable -= 1;

	setupDeckButtons( );

	if( player.cardPointsAvailable <= 0 ) {
		destroyCollectionAddButtons( );
	}
}

static int addComboBtn;
static int subComboBtn;
static int addControlBtn;
static int subControlBtn;
static int addAggroBtn;
static int subAggroBtn;

static void addComboDeck( int id )
{
	if( showingHelp ) return;

	playCard( );

	++player.deck.combo;
	setupDeckButtons( );
}

static void subComboDeck( int id )
{
	if( showingHelp ) return;

	playCard( );

	--player.deck.combo;
	setupDeckButtons( );
}

static void addControlDeck( int id )
{
	if( showingHelp ) return;

	playCard( );
	++player.deck.control;
	setupDeckButtons( );
}

static void subControlDeck( int id )
{
	if( showingHelp ) return;

	playCard( );
	--player.deck.control;
	setupDeckButtons( );
}

static void addAggroDeck( int id )
{
	if( showingHelp ) return;

	playCard( );
	++player.deck.aggro;
	setupDeckButtons( );
}

static void subAggroDeck( int id )
{
	if( showingHelp ) return;

	playCard( );
	--player.deck.aggro;
	setupDeckButtons( );
}

static void setupDeckButtons( void )
{
	bool needAddCombo;
	bool needSubCombo;
	bool needAddControl;
	bool needSubControl;
	bool needAddAggro;
	bool needSubAggro;

	// see what buttons are needed
	if( player.deck.isNetDeck ) {
		// net decks can't be edited
		needAddCombo = false;
		needSubCombo = false;
		needAddControl = false;
		needSubControl = false;
		needAddAggro = false;
		needSubAggro = false;
	} else {
		bool deckFull = ( ( player.deck.control + player.deck.combo + player.deck.aggro ) >= player.deckBuilding );

#define TEST_TYPE( sb, ab, dv, cv ) ( sb ) = ( dv ) != 0; ab = !deckFull && ( dv < cv )
		TEST_TYPE( needSubAggro, needAddAggro, player.deck.aggro, player.collection.aggro );
		TEST_TYPE( needSubControl, needAddControl, player.deck.control, player.collection.control );
		TEST_TYPE( needSubCombo, needAddCombo, player.deck.combo, player.collection.combo );
#undef TEST_TYPE
	}

#define TEST_ADD_BUTTON( b, id, pos, c, r, to ) \
	if( b ) { if( ( id ) == -1 ) ( id ) = btn_Create( ( pos ), vec2( 20, 20 ), vec2( 19, 19 ), ( c ), font20, darkGreen, ( to ), buttonImgs, -1, CLR_WHITE, 1, 10, NULL, ( r ) ); \
	} else { btn_Destroy( id ); ( id ) = -1; }

	TEST_ADD_BUTTON( needAddCombo, addComboBtn, vec2( 355, 375 ), "+", addComboDeck, VEC2_ZERO );
	TEST_ADD_BUTTON( needSubCombo, subComboBtn, vec2( 385, 375 ), "-", subComboDeck, vec2( 0, -2 ) );

	TEST_ADD_BUTTON( needAddControl, addControlBtn, vec2( 355, 405 ), "+", addControlDeck, VEC2_ZERO );
	TEST_ADD_BUTTON( needSubControl, subControlBtn, vec2( 385, 405 ), "-", subControlDeck, vec2( 0, -2 ) );

	TEST_ADD_BUTTON( needAddAggro, addAggroBtn, vec2( 355, 435 ), "+", addAggroDeck, VEC2_ZERO );
	TEST_ADD_BUTTON( needSubAggro, subAggroBtn, vec2( 385, 435 ), "-", subAggroDeck, vec2( 0, -2 ) );
#undef TEST_ADD_BUTTON
}

static void resetDeck( int id )
{
	if( showingHelp ) return;

	playClick( );
	player.deck.aggro = 0;
	player.deck.combo = 0;
	player.deck.control = 0;
	player.deck.isNetDeck = false;

	setupDeckButtons( );
}

static void exitScreen( int id )
{
	if( showingHelp ) return;

	playClick( );
	gsmEnterState( &globalFSM, &hubScreenState );
}

static int characterScreen_Enter( void )
{
	snd_PlayStreaming( menuMusic, 0.5f, 0.0f );

	cam_TurnOnFlags( 0, 1 );

	addComboBtn = -1;
	subComboBtn = -1;
	addControlBtn = -1;
	subControlBtn = -1;
	addAggroBtn = -1;
	subAggroBtn = -1;
	useNetDeckOneBtn = -1;
	useNetDeckTwoBtn = -1;
	helpBtn = -1;

	gfx_SetClearColor( CLR_BLACK );

	btn_Init( );
	btn_RegisterSystem( );

	if( player.statPointsAvailable > 0 ) {
		// create add stats buttons
		int i = 0;
		statAddButtons[i++] = btn_Create( vec2( 60 + 12, 56 + 12 ),  vec2( 24, 24 ), vec2( 23, 23 ), "+", font24, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 10, NULL, addDeckBuilding );
		statAddButtons[i++] = btn_Create( vec2( 60 + 12, 118 + 12 ), vec2( 24, 24 ), vec2( 23, 23 ), "+", font24, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 10, NULL, addCheating );
		statAddButtons[i++] = btn_Create( vec2( 60 + 12, 178 + 12 ), vec2( 24, 24 ), vec2( 23, 23 ), "+", font24, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 10, NULL, addTrading );
		statAddButtons[i++] = btn_Create( vec2( 60 + 12, 236 + 12 ), vec2( 24, 24 ), vec2( 23, 23 ), "+", font24, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 10, NULL, addObservation );

		statAddButtons[i++] = btn_Create( vec2( 410 + 12, 56 + 12 ),  vec2( 24, 24 ), vec2( 23, 23 ), "+", font24, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 10, NULL, addStrategy );
		statAddButtons[i++] = btn_Create( vec2( 410 + 12, 136 + 12 ), vec2( 24, 24 ), vec2( 23, 23 ), "+", font24, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 10, NULL, addBluffing );
		statAddButtons[i++] = btn_Create( vec2( 410 + 12, 221 + 12 ), vec2( 24, 24 ), vec2( 23, 23 ), "+", font24, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 10, NULL, addBold );
	}

	if( player.cardPointsAvailable > 0 ) {
		// create add collection buttons
		int i = 0;
		collectionAddButtons[i++] = btn_Create( vec2( 20 + 12, 377 + 12 ), vec2( 24, 24 ), vec2( 23, 23 ), "+", font24, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 10, NULL, addCollectionCombo );
		collectionAddButtons[i++] = btn_Create( vec2( 20 + 12, 459 + 12 ), vec2( 24, 24 ), vec2( 23, 23 ), "+", font24, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 10, NULL, addCollectionControl );
		collectionAddButtons[i++] = btn_Create( vec2( 20 + 12, 539 + 12 ), vec2( 24, 24 ), vec2( 23, 23 ), "+", font24, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 10, NULL, addCollectionAggro );
	}

	setupDeckButtons( );

	btn_Create( vec2( 400, 470 ), vec2( 100, 28 ), vec2( 99, 27 ), "Reset Deck", font20, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 10, NULL, resetDeck );

	btn_Create( vec2( 457, 349 ), vec2( 18, 18 ), vec2( 17, 17 ), "?", font12, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 10, NULL, showDeckHelp );

	btn_Create( vec2( 575, 395 ), vec2( 18, 18 ), vec2( 17, 17 ), "?", font12, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 10, NULL, showMetaHelp );

	btn_Create( vec2( 400, 570 ), vec2( 150, 50 ), vec2( 149, 49 ), "Exit", font20, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 10, NULL, exitScreen );

	if( firstTimeEntered ) {
		showHelp( "Welcome to the game!\n\n"
			"You start out as a new player in the Conjury: The Convocation tournaments.\n"
			"By playing in tournaments you will be able to trade and increase your skills\n"
			"with the final goal being to a win a Masters Tournament.\n\n"
			"Start off by assigning points to your skills and your card collection.\n"
			"Then build a deck by filling it with card types from your collection.\n"
			"Once all that is done hit 'Exit' and continue to the hub menu. From there\n"
			"select a tournament you want to compete in. Opponents will become more powerful\n"
			"in higher ranked tournaments.\n\n"
			"Each tournament is single elimination in which you will face off against three\n"
			"opponents. Defeat each to win.\n\n"
			"You gain more skill points by winning tournaments. You gain more card type points\n"
			"by trading, which can be done on the hub menu.\n\n"
			"Click anywhere to continue."
		);
		firstTimeEntered = false;
	}

	return 1;
}

static int characterScreen_Exit( void )
{
	btn_DestroyAll( );
	return 1;
}

static void characterScreen_ProcessEvents( SDL_Event* e )
{}

static void characterScreen_Process( void )
{}

static void drawDeck( Deck d, Vector2 topLeft )
{
	char text[64];

	// combo
	SDL_snprintf( text, 63, "Combo: %i", d.combo );
	txt_DisplayString( text, topLeft, CLR_WHITE, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font14, 1, 0 );

	// control
	topLeft.y += 30.0f;
	SDL_snprintf( text, 63, "Control: %i", d.control );
	txt_DisplayString( text, topLeft, CLR_LIGHT_GREY, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font14, 1, 0 );

	// aggro
	topLeft.y += 30.0f;
	SDL_snprintf( text, 63, "Aggro: %i", d.aggro );
	txt_DisplayString( text, topLeft, CLR_WHITE, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font14, 1, 0 );
}

static bool canUseDeck( Deck deck, Deck collection )
{
	return ( ( deck.aggro <= collection.aggro ) && ( deck.combo <= collection.combo ) && ( deck.control <= collection.control ) );;;
}

static void characterScreen_Draw( void )
{
	char text[256];

	// character stat area ***********************
	// character points left
	SDL_snprintf( text, 255, "Character stats points available: %i", player.statPointsAvailable );
	txt_DisplayString( text, vec2( 400, 15 ), ( player.statPointsAvailable > 0 ) ? CLR_YELLOW : CLR_WHITE, HORIZ_ALIGN_CENTER, VERT_ALIGN_TOP, font14, 1, 0 );

	// deckBuilding
	SDL_snprintf( text, 255, "Deck building: %i\nSets the total number of points\nyou can put into your deck.", player.deckBuilding );
	txt_DisplayString( text, vec2( 100, 40 ), CLR_WHITE, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font14, 1, 0 );

	// cheating
	SDL_snprintf( text, 255, "Cheating: %i\nHow good you are at not getting\ncaught.", player.cheating );
	txt_DisplayString( text, vec2( 100, 100 ), CLR_LIGHT_GREY, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font14, 1, 0 );

	// trading
	SDL_snprintf( text, 255, "Trading: %i\nGood trading means you can build\nyour collection faster.", player.trading );
	txt_DisplayString( text, vec2( 100, 160 ), CLR_WHITE, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font14, 1, 0 );

	// observation
	SDL_snprintf( text, 255, "Observation: %i\nGives you a chance to adjust to\nan opponent's plays.", player.observation );
	txt_DisplayString( text, vec2( 100, 220 ), CLR_LIGHT_GREY, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font14, 1, 0 );

	// strategy
	SDL_snprintf( text, 255, "Strategy: %i\nPlay style charactized by tactical thinking.\nGood against the Bold style and works\nwell with Combo cards.", player.strategy );
	txt_DisplayString( text, vec2( 450, 40 ), CLR_WHITE, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font14, 1, 0 );

	// bluffing
	SDL_snprintf( text, 255, "Bluffing: %i\nPlay style centered around misdirection.\nGood against the Strategy style and works\nwell with Control cards.", player.bluffing );
	txt_DisplayString( text, vec2( 450, 120 ), CLR_LIGHT_GREY, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font14, 1, 0 );

	// bold
	SDL_snprintf( text, 255, "Bold: %i\nAggresive style that tries to hit hard and fast.\nGood against the Bluffing style and works\nwell with Aggro cards.", player.bold );
	txt_DisplayString( text, vec2( 450, 210 ), CLR_WHITE, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font14, 1, 0 );

	// character collection area **********************************************
	SDL_snprintf( text, 255, "Character card points available: %i", player.cardPointsAvailable );
	txt_DisplayString( text, vec2( 15, 315 ), ( player.cardPointsAvailable > 0 ) ? CLR_YELLOW : CLR_WHITE, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font14, 1, 0 );

	txt_DisplayString( "Card Collection", vec2( 30, 340 ), CLR_WHITE, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font14, 1, 0 );

	// combo
	SDL_snprintf( text, 255, "Combo: %i\nCards meant to work together.\nGood against Aggro cards and\nworks well with the Strategy style.", player.collection.combo );
	txt_DisplayString( text, vec2( 60, 360 ), CLR_WHITE, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font14, 1, 0 );

	// control
	SDL_snprintf( text, 255, "Control: %i\nCards meant to restrict actions.\nGood against Combo cards and\nworks well with the Bluffing style.", player.collection.control );
	txt_DisplayString( text, vec2( 60, 440 ), CLR_LIGHT_GREY, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font14, 1, 0 );

	// aggro
	SDL_snprintf( text, 255, "Aggro: %i\nCards meant to deal damage.\nGood against Control cards and\nworks well with Bold style.", player.collection.aggro );
	txt_DisplayString( text, vec2( 60, 520 ), CLR_WHITE, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font14, 1, 0 );

	// character deck area ****************************************************

	txt_DisplayString( "Current Deck", vec2( 360, 340 ), CLR_WHITE, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font14, 1, 0 );

	drawDeck( player.deck, vec2( 400, 365 ) );

	// deck warning message
	if( !player.deck.isNetDeck && ( player.deck.control + player.deck.combo + player.deck.aggro ) < player.deckBuilding ) {
		txt_DisplayString( "Deck can hold\nmore cards.", vec2( 350, 490 ), CLR_RED, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font14, 1, 0 );
	} else if( player.deck.isNetDeck ) {
		txt_DisplayString( "Cannot edit a\nnet deck.", vec2( 350, 490 ), CLR_RED, HORIZ_ALIGN_LEFT, VERT_ALIGN_TOP, font14, 1, 0 );
	}


	// meta area **************************************************************
	SDL_snprintf( text, 255,
		"Play Style Meta: %s\n"
		"Card Type Meta: %s",
		getStyleName( metaStyle ), getTypeName( metaType ) );
	txt_DisplayString( text, vec2( 750, 400 ), CLR_YELLOW, HORIZ_ALIGN_RIGHT, VERT_ALIGN_BOTTOM, font14, 1, 0 );

	txt_DisplayString( "Net Deck One", vec2( 575, 430 ), CLR_WHITE, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font14, 1, 0 );
	drawDeck( netDeckOne, vec2( 545, 445 ) );

	if( canUseDeck( netDeckOne, player.collection ) ) {
		if( useNetDeckOneBtn == -1 ) {
			useNetDeckOneBtn = btn_Create( vec2( 578, 553 ), vec2( 100, 45 ), vec2( 99, 44 ), "Use\nNet Deck One", font14, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 10, NULL, useNetDeck );
		}
	} else {
		btn_Destroy( useNetDeckOneBtn );
		txt_DisplayString( "Not enough points\nin collection to use.", vec2( 578, 553 ), CLR_RED, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font12, 1, 0 );
	}

	txt_DisplayString( "Net Deck Two", vec2( 700, 430 ), CLR_WHITE, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font14, 1, 0 );
	drawDeck( netDeckTwo, vec2( 670, 445 ) );

	if( canUseDeck( netDeckTwo, player.collection ) ) {
		if( useNetDeckTwoBtn == -1 ) {
			useNetDeckTwoBtn = btn_Create( vec2( 703, 553 ), vec2( 100, 45 ), vec2( 99, 44 ), "Use\nNet Deck Two", font14, darkGreen, VEC2_ZERO, buttonImgs, -1, CLR_WHITE, 1, 10, NULL, useNetDeck );
		}
	} else {
		btn_Destroy( useNetDeckTwoBtn );
		txt_DisplayString( "Not enough points\nin collection to use.", vec2( 703, 553 ), CLR_RED, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font12, 1, 0 );
	}

	// help
	if( showingHelp ) {
		txt_DisplayString( helpText, vec2( 400, 300 ), CLR_WHITE, HORIZ_ALIGN_CENTER, VERT_ALIGN_CENTER, font14, 1, 100 );
	}
}

static void characterScreen_PhysicsTick( float dt )
{}

GameState characterScreenState = { characterScreen_Enter, characterScreen_Exit, characterScreen_ProcessEvents,
	characterScreen_Process, characterScreen_Draw, characterScreen_PhysicsTick };