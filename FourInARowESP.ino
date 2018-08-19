#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// TODO: Set your ssid, password and (fixed) ip address
const char* ssid = "***";
const char* password = "***";
ESP8266WebServer server(80);
IPAddress ip(999, 999, 999, 999);
IPAddress gateway_ip(999, 999, 999, 999);
IPAddress subnet_mask(999, 999, 999, 999);

volatile int player1 = 0;
volatile int player2 = 0;
volatile int currentPlayer = 0;
volatile int columnCount[6];
volatile int rowCount = 5;

String msg;

void setup() {

	WiFi.config(ip, gateway_ip, subnet_mask);
	WiFi.begin(ssid, password);

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
	}

	server.on("/", handleRootPath);
	server.begin();

	// Start serial swap UART pins to prevent serial garbage, check https://github.com/esp8266/Arduino/issues/3047
	Serial.begin(9600);
	Serial.swap();
}

void loop() {
	server.handleClient();
}

void handleRootPath() {
	
	if (server.args() > 0) {
		String argname = server.argName(0);
		String message = "";

		// Game start request
		if (argname == "start") {
			if (player1 == 0 && player2 == 0) {
				// Player 1 is online, waiting for other player 
				player1 = 1;
				message = "Player 1 online, waiting for player 2.";
			}
			else if (player1 == 1 && player2 == 0)
			{
				// Player 2 is also online
				currentPlayer = 1;
				player2 = 1;

				// Reset game and start
				Serial.print("r");
				message = "Player 2 is online, game started. Waiting for player 1 to enter a column value.";
			}
			else {
				// Error
				message = "Error: Game already started.";
			}
		}
		else if (argname == "restart") {
			for (int i = 0; i < 6; i++)
			{
				columnCount[i] = 0;
			}

			currentPlayer = 0;
			player1 = 0;
			player2 = 0;
			Serial.print("r");
			message = "Game is restarted.";
		}
		else if (argname == "column" && server.argName(1) == "playerId") {
			// Also get the playerId
			String argPlayer = server.arg(1);
			if (argPlayer != String(currentPlayer)) {
				message = "Wrong player! (" + argPlayer + ")";
			}
			else {
				String column = server.arg(0);

				if (columnCount[column.toInt() - 1] < rowCount) {
					columnCount[column.toInt() - 1]++;
					Serial.print(column);
					currentPlayer = (currentPlayer == 1) ? 2 : 1;
					message = "Column " + column + " was selected. Waiting for player " + currentPlayer + " to enter a column value.";
				}
				else {
					message = "Column " + column + " is full!";
				}
			}
		}
		else if (argname == "color") {
			Serial.print("c");
			message = "Colors are changed.";
		}

		server.send(200, "text/plain", message);
	}
}