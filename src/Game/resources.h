#ifndef RESOURCES
#define RESOURCES

#include <stdbool.h>
#include "../Math/vector2.h"
#include "../Graphics/color.h"

extern int font12;
extern int font14;
extern int font20;
extern int font22;
extern int font24;
extern int font60;

extern int whiteImg;
extern Vector2 whiteImgSize;
extern int aggroCardImg;
extern int comboCardImg;
extern int controlCardImg;
extern int strategyCardImg;
extern int bluffingCardImg;
extern int boldCardImg;
extern int unknownCardImg;

extern int diceBaseImg;

extern int* dicePipImgs;
extern int* buttonImgs;
extern int* fieldImgs;

extern int menuMusic;
extern int matchMusic;

extern int cheatingSnd;

bool loadResources( void );
void getScale( const Vector2* baseSize, const Vector2* desiredSize, Vector2* outSize );
int stealthRollDice( int numDice, int target );
int rollDice( int* sbDice, int target ); // pass in an array, will fill it with dice and return the number of successes
bool caughtCheating( int cheatAmt, int cheatStat );

typedef struct {
	int combo;
	int aggro;
	int control;
	bool isNetDeck;
} Deck;

typedef enum {
	S_STUPID,
	S_AVERAGE,
	S_GENIUS
} Smarts;

typedef struct {
	const char* name;
	int deckBuilding;
	int cheating;
	int trading;
	int observation;
	int strategy;
	int bluffing;
	int bold;

	int statPointsAvailable;
	int cardPointsAvailable;

	int tournamentsWon;

	Deck deck;
	Deck collection;

	Smarts smarts;
} Duelist;

typedef enum {
	CT_NONE,
	CT_AGGRO,
	CT_COMBO,
	CT_CONTROL,
} CardType;

typedef enum {
	PS_NONE,
	PS_BOLD,
	PS_STRATEGY,
	PS_BLUFFING,
} PlayStyle;

typedef struct {
	int cheating;
	CardType chosenType;
	PlayStyle chosenStyle;

	bool hasTypeAdvantage;
	bool hasStyleAdvantage;
	bool hasMatch;

	int finalRoll;
	int rolledDice;
	int target;
} DuelistState;

typedef struct {
	int playerPos;
	Duelist members[3];
} Tournament;

extern PlayStyle metaStyle;
extern CardType metaType;
extern Deck netDeckOne;
extern Deck netDeckTwo;
extern int metaCountDown;

extern Duelist player;
extern Duelist currOpponent;

extern Tournament currTournament;
extern bool wasTournamentMasters;
extern bool playerWonMatch;
extern bool tradedRecently;
extern bool hasBeatGame;

extern Color darkGreen;

int imageForStyle( PlayStyle style );
int imageForType( CardType type );

int getTypeScoreFromDuelist( Duelist* d, CardType type );
int getStyleScoreFromDuelist( Duelist* d, PlayStyle style );

void calculatePool( Duelist* us, DuelistState* ourState, DuelistState* theirState );

Duelist generateNoobPlayer( );
Duelist generateAmateurPlayer( );
Duelist generateJourneymanPlayer( );
Duelist generateAdeptPlayer( );
Duelist generateMasterPlayer( );

Tournament generateLocalTournament( );
Tournament generateNationalTournament( );
Tournament generateInternationalTournament( );
Tournament generateMastersTournament( );

void aiChoose( Duelist* me, DuelistState* myState );
void aiObserve( Duelist* me, DuelistState* myState, Duelist* opponent, DuelistState* opponentState );

void setupMeta( void );

const char* getTypeName( CardType type );
const char* getStyleName( PlayStyle style );

void playRoll( void );
void playClick( void );
void playCard( void );

#endif
