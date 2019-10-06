#include "resources.h"

#include "../Graphics/images.h"
#include "../Graphics/imageSheets.h"
#include "../UI/text.h"
#include "../Utils/stretchyBuffer.h"
#include "../System/random.h"
#include "../sound.h"

int font12;
int font14;
int font20;
int font22;
int font24;
int font60;

int whiteImg;
Vector2 whiteImgSize;
int aggroCardImg;
int comboCardImg;
int controlCardImg;
int strategyCardImg;
int bluffingCardImg;
int boldCardImg;
int unknownCardImg;

int diceBaseImg;

int* dicePipImgs;
int* buttonImgs;
int* fieldImgs;

int menuMusic;
int matchMusic;

int cheatingSnd;

Duelist player;
Duelist currOpponent;

Tournament currTournament;

bool playerWonMatch;
bool wasTournamentMasters;
bool tradedRecently;
bool hasBeatGame;

PlayStyle metaStyle;
CardType metaType;
Deck netDeckOne;
Deck netDeckTwo;
int metaCountDown;

Color darkGreen;

static int cardSnds[3];
static int clickSnds[3];
static int rollSnds[5];

#define LOAD_AND_TEST( file, func, id ) \
	id = func( file ); if( id < 0 ) { \
		SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Error loading resource file %s.", file ); }

#define LOAD_AND_TEST_IMG( file, shaderType, id ) \
	id = img_Load( file, shaderType ); if( id < 0 ) { \
		SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Error loading resource file %s.", file ); }

#define LOAD_AND_TEST_FNT( file, size, id ) \
	id = txt_LoadFont( file, size ); if( id < 0 ) { \
		SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Error loading resource file %s.", file ); }

#define LOAD_AND_TEST_SS( file, ids, cnt ) \
	{ ids = NULL; cnt = img_LoadSpriteSheet( file, ST_DEFAULT, &ids ); if( cnt < 0 ) { \
		SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Error loading resource file %s.", file ); } }

#define LOAD_AND_TEST_MUSIC( file, id ) \
	id = snd_LoadStreaming( file, true, 0 ); if( id < 0 ) { \
		SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Error loading resource file %s.", file ); }

#define LOAD_AND_TEST_SOUND( file, id ) \
	id = snd_LoadSample( file, 1, false ); if( id < 0 ) { \
		SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Error loading resource file %s.", file ); }

bool loadResources( void )
{
	//const char* fontFile = "Fonts/KenneyMini.ttf";
	//const char* fontFile = "Fonts/kenpixel_mini_square.ttf";
	const char* fontFile = "Fonts/OpenSans-Regular.ttf";
	LOAD_AND_TEST_FNT( fontFile, 18, font14 );
	LOAD_AND_TEST_FNT( fontFile, 16, font12 );
	LOAD_AND_TEST_FNT( fontFile, 20, font20 );
	LOAD_AND_TEST_FNT( fontFile, 22, font22 );
	LOAD_AND_TEST_FNT( fontFile, 24, font24 );
	LOAD_AND_TEST_FNT( fontFile, 58, font60 );

	LOAD_AND_TEST_IMG( "Images/white.png", ST_DEFAULT, whiteImg );
	img_GetSize( whiteImg, &whiteImgSize );
	LOAD_AND_TEST_IMG( "Images/aggro_type_card.png", ST_DEFAULT, aggroCardImg );
	LOAD_AND_TEST_IMG( "Images/combo_type_card.png", ST_DEFAULT, comboCardImg );
	LOAD_AND_TEST_IMG( "Images/control_type_card.png", ST_DEFAULT, controlCardImg );
	LOAD_AND_TEST_IMG( "Images/strategy_style_card.png", ST_DEFAULT, strategyCardImg );
	LOAD_AND_TEST_IMG( "Images/bluffing_style_card.png", ST_DEFAULT, bluffingCardImg );
	LOAD_AND_TEST_IMG( "Images/unknown_card.png", ST_DEFAULT, unknownCardImg );
	LOAD_AND_TEST_IMG( "Images/bold_style_card.png", ST_DEFAULT, boldCardImg );
	LOAD_AND_TEST_IMG( "Images/die_base.png", ST_DEFAULT, diceBaseImg );

	int unusedCnt;
	LOAD_AND_TEST_SS( "Images/die_pips.ss", dicePipImgs, unusedCnt );
	LOAD_AND_TEST_SS( "Images/button_border.ss", buttonImgs, unusedCnt );
	LOAD_AND_TEST_SS( "Images/playarea_border.ss", fieldImgs, unusedCnt );

	LOAD_AND_TEST_MUSIC( "Sounds/bu-a-magic-ship.ogg", matchMusic );
	LOAD_AND_TEST_MUSIC( "Sounds/bu-a-woeful-brother.ogg", menuMusic );

	LOAD_AND_TEST_SOUND( "Sounds/cheating.ogg", cheatingSnd );

	LOAD_AND_TEST_SOUND( "Sounds/card1.ogg", cardSnds[0] );
	LOAD_AND_TEST_SOUND( "Sounds/card2.ogg", cardSnds[1] );
	LOAD_AND_TEST_SOUND( "Sounds/card3.ogg", cardSnds[2] );

	LOAD_AND_TEST_SOUND( "Sounds/click1.ogg", clickSnds[0] );
	LOAD_AND_TEST_SOUND( "Sounds/click2.ogg", clickSnds[1] );
	LOAD_AND_TEST_SOUND( "Sounds/click3.ogg", clickSnds[2] );

	LOAD_AND_TEST_SOUND( "Sounds/roll1.ogg", rollSnds[0] );
	LOAD_AND_TEST_SOUND( "Sounds/roll2.ogg", rollSnds[1] );
	LOAD_AND_TEST_SOUND( "Sounds/roll3.ogg", rollSnds[2] );
	LOAD_AND_TEST_SOUND( "Sounds/roll4.ogg", rollSnds[3] );
	LOAD_AND_TEST_SOUND( "Sounds/roll5.ogg", rollSnds[4] );

	metaType = CT_AGGRO;
	metaStyle = PS_BOLD;

	darkGreen = clr_hex( 0x4cff00ff );

	tradedRecently = false;
	hasBeatGame = false;

	memset( &player, 0, sizeof( player ) );
	player.cardPointsAvailable = 3;
	player.statPointsAvailable = 16;

	metaCountDown = 0;
	setupMeta( );

	return true;
}

void getScale( const Vector2* baseSize, const Vector2* desiredSize, Vector2* outSize )
{
	outSize->x = desiredSize->x / baseSize->x;
	outSize->y = desiredSize->y / baseSize->y;
}

// for when we don't need to store the results
int stealthRollDice( int numDice, int target )
{
	int successes = 0;

	for( int i = 0; i < numDice; ++i ) {
		int roll = rand_GetRangeS32( NULL, 1, 6 );
		if( roll >= target ) {
			++successes;
		}
	}

	return successes;
}

// pass in an array, will fill it with dice and return the number of successes
int rollDice( int* sbDice, int target )
{
	int successes = 0;

	for( size_t i = 0; i < sb_Count( sbDice ); ++i ) {
		sbDice[i] = rand_GetRangeS32( NULL, 1, 6 );
		if( sbDice[i] >= target ) {
			++successes;
		}
	}

	return successes;
}

bool caughtCheating( int cheatAmt, int cheatStat )
{
	if( cheatAmt == 0 ) return false;

	int cheatAmtRoll = stealthRollDice( cheatAmt, 4 );
	int cheatStatRoll = stealthRollDice( cheatStat, 4 );

	return cheatAmtRoll >= cheatStatRoll;
}

/*
Control beats Combo
Combo beats Aggro
Aggro beats Control

Strat beats Bold
Bold beats Bluff
Bluff beats Strat

Bold pairs with Aggro
Strategy pairs with Combo
Bluff pairs with Control

CT_NONE, CT_AGGRO, CT_COMBO, CT_CONTROL

PS_NONE, PS_BOLD, PS_STRATEGY, PS_BLUFFING,
*/

// give the style or type, what does it win against
static PlayStyle styleWinsAgainst[] = { PS_NONE, PS_BLUFFING, PS_BOLD, PS_STRATEGY };
static CardType typeWinsAgainst[] = { CT_NONE, CT_CONTROL, CT_AGGRO, CT_COMBO };

// given the style or type, what does it lose against
static PlayStyle styleLosesAgainst[] = { PS_NONE, PS_STRATEGY, PS_BLUFFING, PS_BOLD };
static CardType typeLosesAgainst[] = { CT_NONE, CT_COMBO, CT_CONTROL, CT_AGGRO };

// bonus pairings
static PlayStyle typeMatchesWith[] = { PS_NONE, PS_BOLD, PS_STRATEGY, PS_BLUFFING };
static CardType styleMatchesWith[] = { CT_NONE, CT_AGGRO, CT_COMBO, CT_CONTROL };

static int chooseHighest( int one, int two, int three )
{
	if( one > two ) {
		if( one > three ) {
			return 0;
		}
	} else { // two > one
		if( two > three ) {
			return 1;
		}
	}

	return 2;
}

static PlayStyle getBestStyle( Duelist* me )
{
	int choice = chooseHighest( me->bluffing, me->bold, me->strategy );

	switch( choice ) {
	case 0:
		return PS_BLUFFING;
		break;
	case 1:
		return PS_BOLD;
		break;
	default:
		return PS_STRATEGY;
		break;
	}
}

static CardType getBestType( Duelist* me )
{
	int choice = chooseHighest( me->deck.aggro, me->deck.combo, me->deck.control );

	switch( choice ) {
	case 0:
		return CT_AGGRO;
		break;
	case 1:
		return CT_COMBO;
		break;
	default:
		return CT_CONTROL;
		break;
	}
}

static void stupidAI_Choose( Duelist* me, DuelistState* myState )
{
	// always play the meta
	myState->chosenStyle = metaStyle;
	myState->chosenType = metaType;

	// never cheats
	me->cheating = 0;
}

static void stupidAI_Observe( Duelist* me, DuelistState* myState, Duelist* opponent, DuelistState* opponentState )
{
	// stupid AI will never adjust
}

static void averageAI_Choose( Duelist* me, DuelistState* myState )
{
	// will play against the meta
	myState->chosenStyle = styleLosesAgainst[metaStyle];
	myState->chosenType = typeLosesAgainst[metaType];

	// will cheat 50% of the time
	if( rand_Choice( NULL ) ) {
		myState->cheating = rand_GetRangeS32( NULL, 1, me->cheating / 4 );
		if( myState->cheating < 0 ) {
			myState->cheating = 0;
		}
	} else {
		myState->cheating = 0;
	}
}

static void averageAI_Observe( Duelist* me, DuelistState* myState, Duelist* opponent, DuelistState* opponentState )
{
	// look at either the style or type, then choose the opposite of that
	//  then choose randomly between our highest and what beats them
	if( rand_Choice( NULL ) ) {
		// adjust style to beat opponent
		if( rand_Choice( NULL ) ) {
			myState->chosenStyle = getBestStyle( me );
		} else {
			myState->chosenStyle = styleLosesAgainst[opponentState->chosenStyle];
		}
	} else {
		// adjust type to beat opponent
		if( rand_Choice( NULL ) ) {
			myState->chosenType = getBestType( me );
		} else {
			myState->chosenType = typeLosesAgainst[opponentState->chosenType];
		}
	}
}


static void geniusAI_Choose( Duelist* me, DuelistState* myState )
{
	// find our highest stat and match with the appropriate card/type
	//  play our deck
	int stats[6];
	stats[0] = me->bluffing;
	stats[1] = me->bold;
	stats[2] = me->strategy;
	stats[3] = me->deck.aggro;
	stats[4] = me->deck.combo;
	stats[5] = me->deck.control;

	int best = -1;
	int bestIdx = -1;
	for( int i = 0; i < 6; ++i ) {
		if( stats[i] == best ) {
			if( rand_Choice( NULL ) ) {
				best = stats[i];
				bestIdx = i;
			}
		} else if( stats[i] > best ) {
			best = stats[i];
			bestIdx = i;
		}
	}

	switch( bestIdx ) {
	case 0:
		myState->chosenStyle = PS_BLUFFING;
		myState->chosenType = CT_CONTROL;
		break;
	case 1:
		myState->chosenStyle = PS_BOLD;
		myState->chosenType = CT_AGGRO;
		break;
	case 2:
		myState->chosenStyle = PS_STRATEGY;
		myState->chosenType = CT_COMBO;
		break;
	case 3:
		myState->chosenStyle = PS_BOLD;
		myState->chosenType = CT_AGGRO;
		break;
	case 4:
		myState->chosenStyle = PS_STRATEGY;
		myState->chosenType = CT_COMBO;
		break;
	default:
		myState->chosenStyle = PS_BLUFFING;
		myState->chosenType = CT_CONTROL;
		break;
	}

	// random chance of switching up on the non-chosen one to our best
	if( rand_GetRangeS32( NULL, 1, 4 ) == 1 ) {
		if( bestIdx <= 2 ) {
			// style controlled, switch type
			myState->chosenType = getBestType( me );
		} else {
			// type controlled, switch style
			myState->chosenStyle = getBestStyle( me );
		}
	}

	myState->cheating = rand_GetToleranceS32( NULL, rand_GetToleranceS32( NULL, me->cheating / 2, 2 ), 1 );
	if( myState->cheating < 0 ) {
		myState->cheating = 0;
	}
}

int getTypeScoreFromDuelist( Duelist* d, CardType type )
{
	switch( type ) {
	case CT_AGGRO:
		return d->deck.aggro;
	case CT_COMBO:
		return d->deck.combo;
	default:
		return d->deck.control;
	}
}

int getStyleScoreFromDuelist( Duelist* d, PlayStyle style )
{
	switch( style ) {
	case PS_BLUFFING:
		return d->bluffing;
	case PS_BOLD:
		return d->bold;
	default:
		return d->strategy;
	}
}

static void geniusAI_Observe( Duelist* me, DuelistState* myState, Duelist* opponent, DuelistState* opponentState )
{
	// it's almost always better to lower your target than to add a die, so we'll try to do that, but we don't want
	//  to reduce the dice pool too much
	int observeChoice = rand_GetRangeS32( NULL, 0, 2 );

	if( observeChoice == 0 ) {
		// look at cheating, if they're cheating more than us than see if we can cheat a little bit more
		if( opponentState->cheating > myState->cheating ) {
			myState->cheating += rand_GetRangeS32( NULL, 1, 3 );
		}
	} else {
		int assumedScore;
		CardType oppType;
		PlayStyle oppStyle;

		if( observeChoice == 1 ) {
			// look at type, assume they're playing to match
			assumedScore = getTypeScoreFromDuelist( opponent, opponentState->chosenType );
			assumedScore += rand_GetToleranceS32( NULL, 5, 1 ); // assume they have stats between 4 and 6

			oppType = typeLosesAgainst[opponentState->chosenType];
			oppStyle = typeMatchesWith[oppType];
		} else {
			// look at style, assume they're playing to match
			assumedScore = getStyleScoreFromDuelist( opponent, opponentState->chosenStyle );
			assumedScore += rand_GetToleranceS32( NULL, 5, 1 ); // assume their deck has a score between 4 and 6

			oppStyle = styleLosesAgainst[opponentState->chosenStyle];
			oppType = styleMatchesWith[oppStyle];
		}

		// see if playing against their assumed choices would give use an advantage
		int speculativeScore = getTypeScoreFromDuelist( me, oppType ) + getStyleScoreFromDuelist( me, oppStyle );
		int currScore = getTypeScoreFromDuelist( me, myState->chosenType ) + getStyleScoreFromDuelist( me, myState->chosenStyle );

		// assume using advantage will give us a bonus of about 2 dice, if it's not enough then don't switch
		if( speculativeScore < ( currScore - 2 ) ) {
			return;
		}

		if( speculativeScore < ( assumedScore - 2 ) ) {
			return;
		}

		// looks like it gives us an advantage, switch over
		myState->chosenStyle = oppStyle;
		myState->chosenType = oppType;
	}
}

void aiChoose( Duelist* me, DuelistState* myState )
{
	switch( me->smarts ) {
	case S_STUPID:
		stupidAI_Choose( me, myState );
		break;
	case S_AVERAGE:
		averageAI_Choose( me, myState );
		break;
	case S_GENIUS:
		geniusAI_Choose( me, myState );
		break;
	}
}

void aiObserve( Duelist* me, DuelistState* myState, Duelist* opponent, DuelistState* opponentState )
{
	switch( me->smarts ) {
	case S_STUPID:
		stupidAI_Observe( me, myState, opponent, opponentState );
		break;
	case S_AVERAGE:
		averageAI_Observe( me, myState, opponent, opponentState );
		break;
	case S_GENIUS:
		geniusAI_Observe( me, myState, opponent, opponentState );
		break;
	}
}

// 100 names
const char* names[] = {
	"Maragaret", "Ethel", "Cecile", "Melissia", "Deidra", "Sheba", "Elissa", "Louann", "Tu", "Shenna", "Vergie", "Iris", "Shantay", "Yi", "Janella", "Merna", "Delila",
	"Jettie", "Wendolyn", "Kacy", "Jina", "Migdalia", "Riva", "Marietta", "Judy", "Trisha", "Anabel", "Kathryn", "Sau", "Izetta", "Stacee", "Julieta", "Bertie", "Bonnie",
	"Rosy", "Rachelle", "Windy", "Tara", "Sharlene", "Giovanna", "Ilene", "Margarette", "Hope", "Janeth", "Mika", "Georgie", "Melinda", "Tressa", "Natasha", "Cori", "Yong",
	"Lucas", "Pasquale", "Tobias", "Dalton", "Nicky", "Thanh", "Coleman", "Hilton", "Nicholas", "Porfirio", "Ellsworth", "Hayden", "Winford", "Hipolito", "Homer", "Pete",
	"Merlin", "Cyrus", "Mel", "Moises", "Jules", "Brain", "Milan", "Sang", "Aron", "Laurence", "Cameron", "Vincent", "Leslie", "Marcos", "Giuseppe", "Bernard", "Isaiah",
	"Enrique", "Buster", "Gerald", "Daren", "Major", "Fred", "Jerry", "Santiago", "Osvaldo", "Hubert", "Nolan", "Darin", "Rafael", "Lindsay", "Christoper", "Emil",
};

static const char* randomName( )
{
	int idx = rand_GetU32( NULL ) % 100;
	const char* n = names[idx];

	return n;
}

static void setRandomNonStyle( Duelist* d, int value )
{
	int choice = rand_GetU32( NULL ) % 4;

	switch( choice ) {
	case 0:
		d->deckBuilding = value;
		break;
	case 1:
		d->cheating = value;
		break;
	case 2:
		d->trading = value;
		break;
	default:
		d->observation = value;
		break;
	}
}

static void setRandomStyle( Duelist* d, int value )
{
	int choice = rand_GetU32( NULL ) % 3;

	switch( choice ) {
	case 0:
		d->bluffing = value;
		break;
	case 1:
		d->bold = value;
		break;
	default:
		d->strategy = value;
		break;
	}
}

static int rateDeckOnStyle( PlayStyle style, Deck deck )
{
	switch( style ) {
	case PS_BLUFFING:
		return deck.control;
	case PS_BOLD:
		return deck.aggro;
	default:
		return deck.combo;
	}
}

static Deck generateDeckFromStyle( PlayStyle style, int deckBuilding )
{
	int types[3] = { 0, 0, 0 };

	CardType baseType = styleMatchesWith[style];
	int baseIdx = baseType - 1;
	types[baseIdx] = deckBuilding;

	for( int i = 0; ( i < ( deckBuilding / 2 ) ) && ( types[baseIdx] > 0 ); ++i ) {
		int other = ( baseIdx + rand_GetRangeS32( NULL, 1, 2 ) ) % 3;
		--types[baseIdx];
		++types[other];
	}

	Deck d;
	d.aggro = types[0];
	d.combo = types[1];
	d.control = types[2];

	return d;
}

static Deck generateDeckFromBoth( PlayStyle style, CardType type, int deckBuilding )
{
	int types[3] = { 0, 0, 0 };

	// divide evenly among the type and the type that matches the style 
	CardType styleType = styleMatchesWith[style];

	types[styleType - 1] = deckBuilding / 2;
	types[type - 1] = deckBuilding - types[styleType - 1];



	for( int i = 0; i < ( deckBuilding / 4 ); ++i ) {
		int addIdx = rand_GetRangeS32( NULL, 0, 2 );
		int subIdx = rand_GetRangeS32( NULL, 0, 2 );

		if( types[subIdx] > 0 ) {
			--types[subIdx];
			++types[addIdx];
		}
	}

	Deck d;
	d.aggro = types[0];
	d.combo = types[1];
	d.control = types[2];

	return d;
}

Duelist generateNoobPlayer( )
{
	Duelist d;

	d.name = randomName( );
	d.deckBuilding = 1;
	d.cheating = 1;
	d.trading = 1;
	d.observation = 1;
	d.strategy = 1;
	d.bluffing = 1;
	d.bold = 1;

	if( rand_Choice( NULL ) ) {
		d.deck = netDeckOne;
	} else {
		d.deck = netDeckTwo;
	}

	d.smarts = S_STUPID;

	return d;
}

Duelist generateAmateurPlayer( )
{
	Duelist d;

	d.name = randomName( );
	d.deckBuilding = 2;
	d.cheating = 2;
	d.trading = 2;
	d.observation = 2;
	d.strategy = 2;
	d.bluffing = 2;
	d.bold = 2;

	setRandomNonStyle( &d, 3 );
	setRandomStyle( &d, 1 );
	setRandomStyle( &d, 4 );

	PlayStyle bestStyle = getBestStyle( &d );

	if( rateDeckOnStyle( bestStyle, netDeckOne ) > rateDeckOnStyle( bestStyle, netDeckTwo ) ) {
		d.deck = netDeckOne;
	} else {
		d.deck = netDeckTwo;
	}

	if( rand_Choice( NULL ) ) {
		d.smarts = S_STUPID;
	} else {
		d.smarts = S_AVERAGE;
	}

	return d;
}

Duelist generateJourneymanPlayer( )
{
	Duelist d;

	d.name = randomName( );

	if( rand_Choice( NULL ) ) {
		// balanced
		d.deckBuilding = 3;
		d.cheating = 3;
		d.trading = 3;
		d.observation = 3;
		d.strategy = 3;
		d.bluffing = 3;
		d.bold = 3;

		setRandomNonStyle( &d, 5 );
		setRandomStyle( &d, 5 );
	} else {
		// focused builder
		d.deckBuilding = 6;
		d.cheating = 3;
		d.trading = 3;
		d.observation = 4;
		d.strategy = 2;
		d.bluffing = 2;
		d.bold = 2;

		setRandomStyle( &d, 1 );
		setRandomStyle( &d, 5 );
	}
	

	if( rand_Choice( NULL ) ) {
		// use the meta
		d.deck = generateDeckFromBoth( metaStyle, metaType, d.deckBuilding );
	} else {
		// use our best style
		d.deck = generateDeckFromStyle( getBestStyle( &d ), d.deckBuilding );
	}
	d.deck.isNetDeck = false;

	d.smarts = S_AVERAGE;

	return d;
}

Duelist generateAdeptPlayer( )
{
	Duelist d;

	int type = rand_GetRangeS32( NULL, 0, 2 );
	switch( type ) {
	case 0: // focused builder
		d.cheating = 2;
		d.trading = 4;
		d.observation = 4;
		d.strategy = 5;
		d.bluffing = 5;
		d.bold = 5;

		setRandomNonStyle( &d, 5 );
		setRandomStyle( &d, 2 );
		setRandomStyle( &d, 8 );

		d.deckBuilding = 8;

		break;
	case 1: // balanced
		d.deckBuilding = 6;
		d.cheating = 4;
		d.trading = 4;
		d.observation = 4;
		d.strategy = 5;
		d.bluffing = 5;
		d.bold = 5;

		setRandomNonStyle( &d, 5 );
		setRandomStyle( &d, 4 );
		setRandomStyle( &d, 6 );
		break;
	default: // cheater
		d.deckBuilding = 6;
		d.trading = 2;
		d.observation = 1;
		d.strategy = 5;
		d.bluffing = 5;
		d.bold = 5;

		setRandomStyle( &d, 4 );
		setRandomStyle( &d, 6 );

		d.cheating = 9;

		break;
	}

	d.name = randomName( );
	

	d.deck = generateDeckFromStyle( getBestStyle( &d ), d.deckBuilding );

	if( rand_Choice( NULL ) ) {
		d.smarts = S_AVERAGE;
	} else {
		d.smarts = S_GENIUS;
	}

	return d;
}

Duelist generateMasterPlayer( )
{
	Duelist d;

	// chooses a single focus

	d.name = randomName( );

	int type = rand_GetRangeS32( NULL, 0, 2 );
	switch( type ) {
	case 0: // focused builder
		d.deckBuilding = 10;
		d.cheating = 4;
		d.trading = 6;
		d.observation = 10;
		d.strategy = 3;
		d.bluffing = 3;
		d.bold = 3;

		setRandomStyle( &d, 9 );

		break;
	case 1: // balanced
		d.deckBuilding = 6;
		d.cheating = 5;
		d.trading = 5;
		d.observation = 5;
		d.strategy = 7;
		d.bluffing = 7;
		d.bold = 7;

		setRandomNonStyle( &d, 9 );
		setRandomStyle( &d, 6 );
		break;
	default: // cheater
		d.deckBuilding = 8;
		d.cheating = 11;
		d.trading = 1;
		d.observation = 2;
		d.strategy = 8;
		d.bluffing = 8;
		d.bold = 8;

		setRandomStyle( &d, 7 );

		break;
	}
	
	d.deck = generateDeckFromStyle( getBestStyle( &d ), d.deckBuilding );

	d.smarts = S_GENIUS;

	return d;
}

Tournament generateLocalTournament( )
{
	Tournament t;

	t.playerPos = 0;

	// 50% chance noob, 50% chance amateur
	for( int i = 0; i < 3; ++i ) {
		if( rand_Choice( NULL ) ) {
			t.members[i] = generateNoobPlayer( );
		} else {
			t.members[i] = generateAmateurPlayer( );
		}
	}

	return t;
}

Tournament generateNationalTournament( )
{
	Tournament t;

	t.playerPos = 0;

	// 25% chance amateur, 50% chance journeyman, 25% adept
	for( int i = 0; i < 3; ++i ) {
		int c = rand_GetRangeS32( NULL, 0, 3 );

		if( c == 0 ) {
			t.members[i] = generateAmateurPlayer( );
		} else if( c == 1 ) {
			t.members[i] = generateAdeptPlayer( );
		} else {
			t.members[i] = generateJourneymanPlayer( );
		}
	}

	return t;
}

Tournament generateInternationalTournament( )
{
	Tournament t;

	t.playerPos = 0;

	// 25% chance journeyman, 50% chance adept, 25% master
	for( int i = 0; i < 3; ++i ) {
		int c = rand_GetRangeS32( NULL, 0, 3 );

		if( c == 0 ) {
			t.members[i] = generateJourneymanPlayer( );
		} else if( c == 1 ) {
			t.members[i] = generateMasterPlayer( );
		} else {
			t.members[i] = generateAdeptPlayer( );
		}
	}

	return t;
}

Tournament generateMastersTournament( )
{
	Tournament t;

	t.playerPos = 0;

	// everyone is a master
	for( int i = 0; i < 3; ++i ) {
		t.members[i] = generateMasterPlayer( );
	}

	return t;
}

int imageForStyle( PlayStyle style )
{
	switch( style ) {
	case PS_BLUFFING:
		return bluffingCardImg;
	case PS_BOLD:
		return boldCardImg;
	case PS_STRATEGY:
		return strategyCardImg;
	default:
		return unknownCardImg;
	}
}

int imageForType( CardType type )
{
	switch( type ) {
	case CT_AGGRO:
		return aggroCardImg;
	case CT_COMBO:
		return comboCardImg;
	case CT_CONTROL:
		return controlCardImg;;
	default:
		return unknownCardImg;
	}
}

void calculatePool( Duelist* us, DuelistState* ourState, DuelistState* theirState )
{
	int target = 4;
	int dicePool = getStyleScoreFromDuelist( us, ourState->chosenStyle ) + getTypeScoreFromDuelist( us, ourState->chosenType ) + ourState->cheating;

	if( typeWinsAgainst[ourState->chosenType] == theirState->chosenType ) {
		ourState->hasTypeAdvantage = true;
		target -= 1;
	} else {
		ourState->hasTypeAdvantage = false;
	}

	if( styleWinsAgainst[ourState->chosenStyle] == theirState->chosenStyle ) {
		ourState->hasStyleAdvantage = true;
		target -= 1;
	} else {
		ourState->hasStyleAdvantage = false;
	}

	if( styleMatchesWith[ourState->chosenStyle] == ourState->chosenType ) {
		ourState->hasMatch = true;
		dicePool += 1;
	} else {
		ourState->hasMatch = false;
	}

	if( us->deck.isNetDeck ) {
		target += 1;
	}

	ourState->rolledDice = dicePool;
	ourState->target = target;
}

void setupMeta( void )
{
	if( metaCountDown > 0 ) {
		--metaCountDown;
		return;
	}

	metaStyle = rand_GetRangeS32( NULL, 1, 3 );
	metaType = rand_GetRangeS32( NULL, 1, 3 );

	netDeckOne = generateDeckFromBoth( metaStyle, metaType, rand_GetRangeS32( NULL, 3, 6 ) );
	netDeckTwo = generateDeckFromBoth( metaStyle, metaType, rand_GetRangeS32( NULL, 3, 6 ) );

	metaCountDown = rand_GetToleranceS32( NULL, 3, 1 );
}

const char* getTypeName( CardType type )
{
	switch( type ) {
	case CT_AGGRO:
		return "Aggro";
	case CT_COMBO:
		return "Combo";
	case CT_CONTROL:
		return "Control";
	default:
		return "ERROR";
	}
}

const char* getStyleName( PlayStyle style )
{
	switch( style ) {
	case PS_BLUFFING:
		return "Bluffing";
	case PS_BOLD:
		return "Bold";
	case PS_STRATEGY:
		return "Strategy";
	default:
		return "ERROR";
	}
}

void playRoll( void )
{
	snd_Play( rollSnds[rand_GetU32( NULL ) % 5], 1.0f, rand_GetToleranceFloat( NULL, 1.0f, 0.1f ), 0.0f, 0 );
}

void playClick( void )
{
	snd_Play( clickSnds[rand_GetU32( NULL ) % 3], 1.0f, rand_GetToleranceFloat( NULL, 1.0f, 0.1f ), 0.0f, 0 );
}

void playCard( void )
{
	snd_Play( cardSnds[rand_GetU32( NULL ) % 3], 1.0f, rand_GetToleranceFloat( NULL, 1.0f, 0.1f ), 0.0f, 0 );
}