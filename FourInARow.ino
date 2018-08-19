/*
 Name:		FourInARow.ino
 Created:	1/11/2018
 Author:	Leon van den Beukel
*/

#include <SoftwareSerial.h>
#include <FastLED.h>

#define NUM_LEDS 30
#define DATA_PIN 6

CRGB LEDs[NUM_LEDS];
CRGB colorPlayer1 = CRGB::Blue;
CRGB colorPlayer2 = CRGB::Red;

SoftwareSerial SSerial(8, 9);

volatile int rowCount = 5;
volatile int colCount = 6;
volatile int currentPlayer = 1;
volatile int grid[6][5];
volatile int winningPositions[6][5];
volatile int dropCount;
volatile int messageDisplayed = 0;
volatile int gamewon = 0;
volatile int colorPattern = 1; 

int drop(int column, int player);
int fourConnected(int player);
void reset();
void showValues();
void showLeds();
void changeColors();

void setup() {

	randomSeed(analogRead(0));

	Serial.begin(9600);
	while (!Serial) {}

	SSerial.begin(9600);
	while (!SSerial) {}
	Serial.println("SSerial started at 9600");
	Serial.println(" ");

	FastLED.delay(3000);
	FastLED.addLeds<WS2812B, DATA_PIN, GRB>(LEDs, NUM_LEDS);
}

void loop() {

	reset();

	while (gamewon == 0) {

		if (SSerial.available() > 0) {
			int selectedColumn = (SSerial.read() - 48) - 1;
			play(selectedColumn);
		}
		else if (Serial.available() > 0) {
			int selectedColumn = (Serial.read() - 48) - 1;
			play(selectedColumn);
		}
		else {
			if (messageDisplayed == 0) {
				Serial.print("Player ");
				Serial.print(currentPlayer);
				Serial.print(", please select a column between 1 and ");
				Serial.println(colCount);
				messageDisplayed = 1;
			}
		}
	}
}

void play(int selectedColumn) {
	if (selectedColumn == 33 || selectedColumn == 65) {	// r or R (restart)				
		Serial.println("Game will restart");
		reset();
	}
	else if (selectedColumn == 99-49 || selectedColumn == 67-49) { // c or C to change colors
		Serial.println("Colors changed.");
		changeColors();
		showLeds();
	}
	else if (selectedColumn < 0 || selectedColumn > colCount - 1) {
		Serial.print("Invalid column, select between 1 and ");
		Serial.println(colCount);
	}
	else if (selectedColumn >= 0 && selectedColumn <= colCount) {

		int droppedOnRow = drop(selectedColumn, currentPlayer);

		if (droppedOnRow >= 0) {
			showValues();
			showLeds();
			dropCount++;
			gamewon = fourConnected(currentPlayer);

			if (gamewon == 0) {
				currentPlayer = (currentPlayer == 1) ? 2 : 1;

				Serial.print("Player ");
				Serial.print(currentPlayer, DEC);
				Serial.print(" selected column ");
				Serial.print(selectedColumn + 1, DEC);
				Serial.print(" and is dropped on row ");
				Serial.print(droppedOnRow + 1, DEC);
				Serial.print(". Total dropcount: ");
				Serial.println(dropCount);

				if (dropCount == (rowCount * colCount)) {
					Serial.println("Game ended, total reached, no winner.");
					reset();
				}
			}
			else if (gamewon > 0) {
				Serial.print("Game is won by player ");
				Serial.println(currentPlayer);
				showWinningPattern(currentPlayer);
				reset();
			}

			messageDisplayed = 0;
		}
		else if (droppedOnRow == -1) {
			Serial.println("Column is full, retry!");
		}
	}
}

int drop(int column, int player) {

	for (int row = 0; row < rowCount; row++)
	{
		if (grid[column][row] == 0) {
			int offset = colCount * row;
			LEDs[column + offset] = player == 1 ? colorPlayer1 : colorPlayer2;
			FastLED.show();
			delay(200);
			LEDs[column + offset] = CRGB::Black;
			FastLED.show();
			delay(200);
		}

		if (grid[column][row] == 0) {
			grid[column][row] = player;
			return row;
		}
		else if (row == (rowCount - 1)) {
			return -1;
		}
	}
}

int fourConnected(int player) {

	// Horizontal Check
	for (int j = 0; j < rowCount - 3; j++) {
		for (int i = 0; i < colCount; i++) {
			if (grid[i][j] == player && grid[i][j + 1] == player && grid[i][j + 2] == player && grid[i][j + 3] == player) {
				winningPositions[i][j] = player;
				winningPositions[i][j + 1] = player;
				winningPositions[i][j + 2] = player;
				winningPositions[i][j + 3] = player;
				return 1;
			}
		}
	}

	// Vertical Check
	for (int i = 0; i < colCount - 3; i++) {
		for (int j = 0; j < rowCount; j++) {
			if (grid[i][j] == player && grid[i + 1][j] == player && grid[i + 2][j] == player && grid[i + 3][j] == player) {
				winningPositions[i][j] = player;
				winningPositions[i + 1][j] = player;
				winningPositions[i + 2][j] = player;
				winningPositions[i + 3][j] = player;
				return 2;
			}
		}
	}

	// Ascending DiagonalCheck 
	for (int i = 3; i < colCount; i++) {
		for (int j = 0; j < rowCount - 3; j++) {
			if (grid[i][j] == player && grid[i - 1][j + 1] == player && grid[i - 2][j + 2] == player && grid[i - 3][j + 3] == player) {
				winningPositions[i][j] = player;
				winningPositions[i - 1][j + 1] = player;
				winningPositions[i - 2][j + 2] = player;
				winningPositions[i - 3][j + 3] = player;
				return 3;
			}
		}
	}
	// Descendin gDiagonalCheck
	for (int i = 3; i < colCount; i++) {
		for (int j = 3; j < rowCount; j++) {
			if (grid[i][j] == player && grid[i - 1][j - 1] == player && grid[i - 2][j - 2] == player && grid[i - 3][j - 3] == player) {
				winningPositions[i][j] = player;
				winningPositions[i - 1][j - 1] = player;
				winningPositions[i - 2][j - 2] = player;
				winningPositions[i - 3][j - 3] = player;
				return 4;
			}
		}
	}

	return 0;
}

void reset() {
	for (int row = 0; row < rowCount; row++)
	{
		for (int column = 0; column < colCount; column++)
		{
			grid[column][row] = 0;
			winningPositions[column][row] = 0;
		}
	}

	for (int led = 0; led < NUM_LEDS; led++) {
		LEDs[led] = CRGB::Black;
	}
	FastLED.show();

	dropCount = 0;
	currentPlayer = 1; // (int)random(1, 3);
	gamewon = 0;
	messageDisplayed = 0;
}

void showValues() {
	for (int row = 0; row < rowCount; row++)
	{
		Serial.print(grid[0][row]);
		Serial.print(",");
		Serial.print(grid[1][row]);
		Serial.print(",");
		Serial.print(grid[2][row]);
		Serial.print(",");
		Serial.print(grid[3][row]);
		Serial.print(",");
		Serial.print(grid[4][row]);
		Serial.print(",");
		Serial.print(grid[5][row]);
		Serial.println();
	}
}

void showLeds() {

	for (int row = 0; row < rowCount; row++)
	{
		int offset = colCount * row;

		for (int col = 0; col < colCount; col++) {
			if (grid[col][row] == 1 && grid[col][row] != 2) {
				LEDs[col + offset] = colorPlayer1;
			}
			else if (grid[col][row] == 2 && grid[col][row] != 1) {
				LEDs[col + offset] = colorPlayer2;
			}
			else {
				LEDs[col + offset] = CRGB::Black;
			}
		}
	}

	FastLED.show();
}

void showWinningPattern(int player) {

	CRGB color = (player == 1) ? colorPlayer1 : colorPlayer2;
	int black = 0;

	// Flash the winning pattern
	for (int i = 0; i < 15; i++) {
		for (int row = 0; row < rowCount; row++)
		{
			int offset = colCount * row;

			for (int col = 0; col < colCount; col++) {
				if (winningPositions[col][row] == player) {
					LEDs[col + offset] = (black == 0) ? CRGB::Black : color;
				}
			}
		}
		FastLED.show();
		delay(200);
		black = (black == 0) ? 1 : 0;
	}

	// Flash all
	for (int i = 0; i < 4; i++) {
		allToColor(color);
		delay(200);
		allToColor(CRGB::Black);
		delay(200);
	}
}

void allToColor(CRGB color) {

	for (int i = 0; i < NUM_LEDS; i++)
	{
		LEDs[i] = color;
		FastLED.show();
		delay(10);
	}

	FastLED.show();
}

void changeColors() {

	colorPattern++;

	if (colorPattern == 1)
	{
		colorPlayer1 = CRGB::Blue;
		colorPlayer2 = CRGB::Red;
	}
	else if (colorPattern == 2) {
		colorPlayer1 = CRGB::Yellow;
		colorPlayer2 = CRGB::Purple;
	}
	else if (colorPattern == 3) {
		colorPlayer1 = CRGB::Green;
		colorPlayer2 = CRGB::Orange;
	}
	else if (colorPattern == 4) {
		colorPlayer1 = CRGB::Violet;
		colorPlayer2 = CRGB::LightGreen;
	}
	else if (colorPattern == 5) {
		colorPlayer1 = CRGB::Wheat;
		colorPlayer2 = CRGB::DarkBlue;
	}
	else {
		colorPattern = 1;
		colorPlayer1 = CRGB::Blue;
		colorPlayer2 = CRGB::Red;
	}
}