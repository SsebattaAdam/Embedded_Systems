/*
-------------------------------------------------------
Name 	          Registration Number 	       Student Number
-------------------------------------------------------
SSEBATTA ADAM	     21/U/0534	               2100700534
-------------------------------------------------------
BUTERABA LYNATTE JOANITTA 21/U/09727/PS    	2100709727
-------------------------------------------------------
NALUKWAGO HADIJAH	  22/U/6566	             2200706566
-------------------------------------------------------
NABUKENYA MARIAM	21/U/17130/EVE	         2100717130
-------------------------------------------------------
GAFABUSA WILLY	    21/U/13385/EVE	           2100713385
-------------------------------------------------------


*/



#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

// Variables for counting tourists
uint8_t childrenCount = 0;
uint8_t adultsCount = 0;
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
void rotate(int numrotate);

int registeredVehicles = 0;
const int MAX_CARS = 5;
int shouldExecuteAfterOption6 = 0; // Initialize the flag as 0

unsigned char car[] = "incoming tourist";
unsigned char reg[] = "take registration";
unsigned char conc[] = "Cancelled";
unsigned char succ[] = "Success";
unsigned char drink[] = "Put no of bottles";
unsigned char frig[] = "Fridge";
unsigned char close[] = "GATE CLOSING";
unsigned char open[] = "GATE OPENING";
unsigned char regInProg[] = "Registration in progress";


unsigned int bottleNumber = 0;
unsigned int money = 1500;
int keypadvalue =0;
int userInputBottleNumber =0;


#define EEPROM_ADDRESS 0x00
#define EEPROM_ADDRESS_Adults 0x40
#define EEPROM_ADDRESS_children 0x48
#define EEPROM_ADDRESS_total 0x70
#define EEPROM_ADDRESS_numberPlate 0x50
#define FRIDGE_BOTTLE_COUNT_ADDR 0x84

// Function for the UART
#define USART_BAUDRATE 9600
#define F_CPU 16000000UL
#include <util/delay.h>
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

unsigned char welcome[] = "Welcome to Queen Elizabeth";
#define PASSWORD_EEPROM_ADDRESS 0x10

// Initialize UART for ATmega2560
void initUART() {
	// Set baud rate to 9600 (for 16 MHz frequency)
	UBRR0H = (BAUD_PRESCALE >> 8);
	UBRR0L = BAUD_PRESCALE;

	// Enable transmitter and receiver
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);

	// Set frame format: 8 data bits, 1 stop bit, no parity
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

	// Debug message
	sendString("Hello, welcome to Queen Elizabeth National Park\r\n");
}

// Transmit a character via UART
void sendChar(char data) {
	// Wait until the buffer is empty
	while (!(UCSR0A & (1 << UDRE0)));
	// Put data into the buffer, send the data
	UDR0 = data;
}

// Transmit a string via UART
void sendString(const char *str) {
	// Loop through each character in the string and send it
	while (*str != '\0') {
		sendChar(*str);
		str++;
	}
}
// UART character receive function
char UART_Receive() {
	while (!(UCSR0A & (1 << RXC0))); // Wait for data to be received
	return UDR0; // Return the received data
}
void rotateMotor(int rotations){
	int steps = 20;
	int revolutions = rotations *steps;
	DDRK |=(1<<PK0);
	for (int x =0; x < revolutions; x++)
	{
		PORTK ^=(1<<PK0);
		_delay_ms(850);
	}
	
}
void regInprog(){
	PORTB &= ~(1 << PB5);
	PORTA = 0X01;
	latch2();
	PORTA = 0XC0;
	latch2();
	// Data mode
	PORTB |= (1 << PB5);
	PORTB &= ~(1 << PB6);
	for (int x =0; x<12; x++)
	{
		PORTA = regInProg[x];
		latch2();
	}

}
void gateOpen(){
	PORTB &= ~(1 << PB5);
	PORTA = 0X01;
	latch2();
	PORTA = 0XC0;
	latch2();
	// Data mode
	PORTB |= (1 << PB5);
	PORTB &= ~(1 << PB6);
	for (int x =0; x<12; x++)
	{
		PORTA = open[x];
		latch2();
	}

}
void gateclose(){
	PORTB &= ~(1 << PB5);
	PORTA = 0X01;
	latch2();
	PORTA = 0XC0;
	latch2();
	// Data mode
	PORTB |= (1 << PB5);
	PORTB &= ~(1 << PB6);
	for (int x =0; x<12; x++)
	{
		PORTA = close[x];
		latch2();
	}
	
}
void executeAfterOption6() {
	
	// Call the gateOpenCloses() function after the 3-second delay
	
	gateclose();
	
}

void virtualTerminal() {
	// Send the sentence over UART
	sendString("Select action:\r\n");
	sendString("1) Login\r\n");
	sendString("2) Take registration\r\n");
	sendString("3) How many tourists, categorized by age group, are in the park\r\n");
	sendString("4) Which vehicles are still in the park\r\n");
	sendString("5) input the number of bottles in the fridge\r\n");
	sendString("6) check the number of bottles in the fridge\r\n");
	sendString("7)  SignOut of the vistors /To remove a car from the park\r\n");
	sendString("8) Is the park full or not\r\n");
	_delay_ms(1000); // Delay for 1 second
}



// Function to clear EEPROM
void clearEEPROM() {
	for (uint16_t addr = 0; addr < E2END; addr++) {
		eeprom_update_byte((uint8_t *)addr, 0xFF);
	}
}

// Set login password in EEPROM
void setLoginPassword(const char* password) {
	eeprom_update_block(password, (void*)PASSWORD_EEPROM_ADDRESS, strlen(password) + 1);
}

// Login function
int login() {
	sendString("Enter the password: ");
	char enteredPassword[20];
	int i = 0;

	// Receive and process the entered password
	while (1) {
		char c = UART_Receive();
		if (c == '\r' || c == '\n') {
			enteredPassword[i] = '\0';
			break;
		}
		enteredPassword[i] = c;
		i++;
	}

	// Retrieve the password from EEPROM
	char storedPassword[20];
	eeprom_read_block(storedPassword, (void*)PASSWORD_EEPROM_ADDRESS, sizeof(storedPassword));

	// Compare entered password to the stored password
	if (strcmp(enteredPassword, storedPassword) == 0) {
		sendString("Login successful. Access granted.\r\n");
		return 1;
		} else {
		sendString("Login failed. Access denied.\r\n");
		return 0;
	}
}
void latch() {
	PORTH |= (1 << PH7);
	_delay_ms(10);
	PORTH &= ~(1 << PH7);
	_delay_ms(10);
	
}
void latch2() {
		PORTB |= (1 << PB7);
		_delay_ms(10);
	PORTB &= ~(1 << PB7);
	_delay_ms(10);

}
void displayParkFullStatus() {
	// Check if the park is full based on the number of registered vehicles
	if (registeredVehicles >= MAX_CARS) {
		sendString("The park is full.\r\n");
		} else {
		sendString("The park is not full. There is space for more vehicles.\r\n");
	}
}
void stopContin(){
	 if (shouldExecuteAfterOption6) {
		 gateOpen();
		 _delay_ms(50000);
		 executeAfterOption6(); // Call the function to execute the code
		 shouldExecuteAfterOption6 = 0; // Reset the flag
	 }
}

void displayMenu() {
	int loggedIn = 0; // Initialize as not logged in
	int totalVisitors = 0;
	int carCounter = 0;
	int MAX_CARS =5;
	int cumulativeGrandTotal = 0;
	char numberPlates[MAX_CARS][3]; // Assuming a maximum of MAX_CARS cars

	while (1) {
		
		//lcd 2
		PORTB &= ~(1 << PB5);
		PORTB &= ~(1 << PB6);
		PORTA = 0B00001110;
		latch2();
		
		// Data mode
		PORTH |= (1 << PH5);
		PORTH &= ~(1 << PH6);

		// Print "car" on the LCD
		printLcdFridge("FRIDGE, ENTER");
		PORTB &= ~(1 << PB5);
		PORTJ = 0XC0;
		latch();
		printLcdFridge("THE NUMBER OF ");
		PORTB &= ~(1 << PB5);
		PORTJ = 0X90;
		latch();
		printLcdFridge("BOTTLES");


		 if (carCounter >= MAX_CARS) {
			 sendString("Maximum number of cars reached. Registration is closed.\r\n");
			 break; // Skip registration and continue to the next iteration
		 }
		// Display the menu
		if (!loggedIn) {
			sendString("Please login to continue.\r\n");
			int loginSuccess = login(); // Perform login
			if (loginSuccess) {
				loggedIn = 1;
				sendString("Login successful. You can now access the functionalities.\r\n");
				} else {
				sendString("Login failed. Please try again.\r\n");
			}
			} else {
				
			sendString("Enter your choice: ");
			char choice = UART_Receive();

			switch (choice) {
				case '1':
				loggedIn = login(); // Allow re-login
				break;

				case '2':
				regInprog();
				// Registration section
				sendString("Enter the number plate: ");
				char numberPlate[10];
				int plateLength = 8;

				// Receive and process the number plate
				int i = 0;
				while (1) {
					char c = UART_Receive();
					if (c == '\r' || c == '\n') {
						break;
					}
					numberPlate[i] = c;
					i++;
				}
				numberPlate[i] = '\0';
				plateLength = i;

				// Check if the number plate is already registered
				int plateExists = 0;
				for (int j = 0; j < carCounter; j++) {
					if (strcmp(numberPlate, numberPlates[j]) == 0) {
						plateExists = 1;
						break;
					}
				}

				if (plateExists) {
					sendString("This number plate is already registered.\r\n");
					continue; // Skip registration and continue to the next iteration
				}

				// Store the number plate
				strncpy(numberPlates[carCounter], numberPlate, 10);
				// Store the number plate in EEPROM
				eeprom_update_block(numberPlate, (void *)(EEPROM_ADDRESS_numberPlate + carCounter * 11), 11);

				sendString("Enter the number of adults: ");
				char adultsInput[4]; // Maximum 3 digits for adults
				int adults = 0;

				// Receive and process the number of adults
				i = 0;
				while (1) {
					char c = UART_Receive();
					if (c == '\r' || c == '\n') {
						break;
					}
					adultsInput[i] = c;
					i++;
				}
				adultsInput[i] = '\0';
				adults = atoi(adultsInput);

				sendString("Enter the number of children: ");
				char childrenInput[4]; // Maximum 3 digits for children
				int children = 0;

				// Receive and process the number of children
				i = 0;
				while (1) {
					char c = UART_Receive();
					if (c == '\r' || c == '\n') {
						break;
					}
					childrenInput[i] = c;
					i++;
				}
				childrenInput[i] = '\0';
				children = atoi(childrenInput);

				// Calculate the total cost for adults and children
				int childCost = 100; // Cost per child
				int adultCost = 200; // Cost per adult
				int totalChildCost = children * childCost;
				int totalAdultCost = adults * adultCost;
				
				 // Convert totalAdultCost to a string
				 char costString[12]; // Assuming a maximum of 11 characters for the cost
				 sprintf(costString, "%d", totalAdultCost);
				 // EEPROM write: Assuming EEPROM is large enough and you've included the necessary libraries
				 uint16_t eepromAddress = 0x110 + carCounter * 11;
				 eeprom_update_block(costString, (void *)eepromAddress, 11);
				 
				 char chldtcostEE[12];
				 sprintf(chldtcostEE, "%d", totalChildCost);
				 uint16_t eepromAddress1 = 0x130 + carCounter * 11;
				 eeprom_update_block(chldtcostEE, (void *)eepromAddress1, 11);

				// Calculate the grand total cost
				int grandTotal = totalChildCost + totalAdultCost;
				
				 // Calculate the grand total cost
				 /*int grandTotal = totalChildCost + totalAdultCost;*/

			

				 // Update the cumulative grand total
				 cumulativeGrandTotal += grandTotal;

				 // Display the cumulative grand total
				 sendString("Cumulative Grand Total Cost: $");
				 char cumulativeGrandTotalStr[10];
				 itoa(cumulativeGrandTotal, cumulativeGrandTotalStr, 10);
				 sendString(cumulativeGrandTotalStr);
				 sendString("\r\n");
				
				

				// Sum of adults and children
				totalVisitors += adults + children;
				carCounter++; // Increment the car counter
				

				// Display the registration details on the LCD and console
				sendString("Number Plate: ");
				sendString(numberPlate);
				sendString("\r\n");

				sendString("Adults: ");
				sendString(adultsInput);
				sendString("\r\n");

				sendString("Children: ");
				sendString(childrenInput);
				sendString("\r\n");

				sendString("Total Visitors: ");
				char totalVisitorsStr[10];
				itoa(totalVisitors, totalVisitorsStr, 10);
				sendString(totalVisitorsStr);
				sendString("\r\n");

				// Display the total cost for adults and children
				sendString("Total Child Cost: $");
				char totalChildCostStr[10];
				itoa(totalChildCost, totalChildCostStr, 10);
				sendString(totalChildCostStr);
				sendString("\r\n");

				sendString("Total Adult Cost: $");
				char totalAdultCostStr[10];
				itoa(totalAdultCost, totalAdultCostStr, 10);
				sendString(totalAdultCostStr);
				sendString("\r\n");

				// Display the grand total cost
				sendString("Grand Total Cost: $");
				char grandTotalStr[10];
				itoa(grandTotal, grandTotalStr, 10);
				sendString(grandTotalStr);
				sendString("\r\n");
				// After calculating the number of adults and children, and the total visitors
				int adultsData = adults;
				int childrenData = children;
				int totalVisitorsData = totalVisitors;

			 // Store total visitors in EEPROM
			 // Convert inputs to strings
			 char adultsInputStr[4];
			 itoa(adults, adultsInputStr, 10);
			 char childrenInputStr[4];
			 itoa(children, childrenInputStr, 10);
			 
			 itoa(totalVisitors, totalVisitorsStr, 10);

			 // Store data in EEPROM as strings
			 eeprom_update_block(adultsInputStr, (void *)EEPROM_ADDRESS_Adults , 4);
			 eeprom_update_block(childrenInputStr, (void *)(EEPROM_ADDRESS_children + carCounter * 4), 4);
			 eeprom_update_block(totalVisitorsStr, (void *)(EEPROM_ADDRESS_total + carCounter * 10), 10);
			 carCounter++;
				break;


				case '3':
				// Display the total number of tourists categorized by age group
				sendString("Total number of tourists, categorized by age group:\r\n");

				// Read the total number of adults from EEPROM
				char adultsFromEEPROMStr[4];
				eeprom_read_block(adultsFromEEPROMStr, (void *)EEPROM_ADDRESS_Adults, 4);
				int adultsFromEEPROM = atoi(adultsFromEEPROMStr);

				// Read the total number of children from EEPROM
				char childrenFromEEPROMStr[4];
				eeprom_read_block(childrenFromEEPROMStr, (void *)(EEPROM_ADDRESS_Adults), 4);
				int childrenFromEEPROM = atoi(childrenFromEEPROMStr);

				// Calculate the combined total
				int totalTourists = adultsFromEEPROM + childrenFromEEPROM;

				sendString("Total Adults: ");
				char adultsStr[10];
				itoa(adultsFromEEPROM, adultsStr, 10);
				sendString(adultsStr);
				sendString("\r\n");

				sendString("Total Children: ");
				char childrenStr[10];
				itoa(childrenFromEEPROM, childrenStr, 10);
				sendString(childrenStr);
				sendString("\r\n");

				sendString("Combined Total: ");
				char totalStr[10];
				itoa(totalTourists, totalStr, 10);
				sendString(totalStr);
				sendString("\r\n");

				break;



				case '4':
				// Display the vehicles still in the park
				sendString("Vehicles still in the park:\r\n");

				for (int i = 0; i < carCounter; i++) {
					char vehicleDetails[100];
					snprintf(vehicleDetails, sizeof(vehicleDetails), "Vehicle %d: %s\r\n", i + 1, numberPlates[i]);
					sendString(vehicleDetails);
				}
				break;
				
				case '5':
				if (loggedIn) {
					// Attendant wants to input the number of bottles in the fridge
					sendString("Enter the number of bottles in the fridge: ");
					char fridgeInput[4];
					int fridgeBottleCount = 0;

					// Receive and process the number of bottles
					int i = 0;
					while (1) {
						char c = UART_Receive();
						if (c == '\r' || c == '\n') {
							break;
						}
						fridgeInput[i] = c;
						i++;
					}
					fridgeInput[i] = '\0';
					fridgeBottleCount = atoi(fridgeInput);

					// Store the number of bottles in EEPROM
					eeprom_update_word((uint16_t *)FRIDGE_BOTTLE_COUNT_ADDR, fridgeBottleCount);


					sendString("Fridge bottle count stored in EEPROM.\r\n");
					} else {
					sendString("Please log in to access this option.\r\n");
				}
				break;
				case '6':
				
				if (loggedIn) {
					
					// Attendant wants to check the number of bottles in the fridge
					uint16_t currentBottleCount = eeprom_read_word((uint16_t *)FRIDGE_BOTTLE_COUNT_ADDR);
					char response[50]; // Define a buffer for the response
					sprintf(response, "Current Bottle Count:  %u\n", currentBottleCount);
					sendString(response); // Send the response string
					} else {
					sendString("Please log in to access this option.\r\n");
				}
				  shouldExecuteAfterOption6 = 1; // Set the flag to execute code after option 6
				  stopContin();
				  return 0;
					  
				
				case '7':
				if (loggedIn) {
					// Attendant wants to remove a car from the park
					sendString("Enter the number plate to remove: ");
					char plateToRemove[10];
					int plateLengthToRemove = 8;

					// Receive and process the number plate to remove
					int i = 0;
					while (1) {
						char c = UART_Receive();
						if (c == '\r' || c == '\n') {
							break;
						}
						plateToRemove[i] = c;
						i++;
					}
					plateToRemove[i] = '\0';
					plateLengthToRemove = i;

					int plateIndexToRemove = -1;
					// Find the index of the car with the specified number plate
					for (int j = 0; j < carCounter; j++) {
						if (strcmp(plateToRemove, numberPlates[j]) == 0) {
							plateIndexToRemove = j;
							

							gateOpen();
							_delay_ms(100000);
							gateclose();
							break;
						}
					}

					if (plateIndexToRemove == -1) {
						sendString("Car with this number plate is not in the park.\r\n");
						} else {
						// Unregister visitors associated with this car
						int removedAdults = eeprom_read_word((uint16_t *)(EEPROM_ADDRESS_Adults + plateIndexToRemove * 2 * sizeof(int)));
						int removedChildren = eeprom_read_word((uint16_t *)(EEPROM_ADDRESS_children + (plateIndexToRemove * 2 + 1) * sizeof(int)));

						// Deduct the removed visitors from the cumulative count
						totalVisitors -= (removedAdults + removedChildren);

						// Remove the number plate from the list
						for (int k = plateIndexToRemove; k < carCounter - 1; k++) {
							strcpy(numberPlates[k], numberPlates[k + 1]);
							// Copy visitors data from the next car to the current car
							eeprom_update_word((uint16_t *)(EEPROM_ADDRESS_Adults + k * 2 * sizeof(int)),
							eeprom_read_word((uint16_t *)(EEPROM_ADDRESS_children + (k + 1) * 2 * sizeof(int))));
							eeprom_update_word((uint16_t *)(EEPROM_ADDRESS_Adults + k * 2 * sizeof(int)), 0);
							eeprom_update_word((uint16_t *)(EEPROM_ADDRESS_children + (k * 2 + 1) * sizeof(int)), 0);

							eeprom_read_word((uint16_t *)(EEPROM_ADDRESS_total + ((k + 1) * 2 + 1) * sizeof(int)));
						}

						// Clear the data of the last car
						eeprom_update_word((uint16_t *)(EEPROM_ADDRESS_Adults + (carCounter - 1) * 2 * sizeof(int)), 0);
						eeprom_update_word((uint16_t *)(EEPROM_ADDRESS_children + ((carCounter - 1) * 2 + 1) * sizeof(int)), 0);
						strcpy(numberPlates[carCounter - 1], "");

						carCounter--;

						// Display confirmation
						sendString("Car removed from the park. Visitors unregistered.\r\n");
					}
					} else {
					sendString("Please log in to access this option.\r\n");
				}
				
				break;

				 case '8':
				 // Check if the park is full
				 displayParkFullStatus();
				 break;
				


				
				default:
				sendString("Invalid choice. Please select a valid option.\r\n");
			}
			
			
		}
	}
}

void printLcdFridge(char* word){
	PORTH |=(1<<PH5);
	int x=0;
	while(word[x] != 0){
		PORTJ = word[x];
		latch();
		x++;
	}
	
}

void ComandsLCDFRIDGE(){
		// Switch to command mode
		PORTH &= ~(1 << PH5);
		latch();

		// Clear the LCD display
		PORTJ = 0x01;
		latch();
		PORTJ = 0XC0;
		latch();
		// Set the LCD back to data mode
		PORTH |= (1 << PH5);
		PORTH &= ~(1 << PH6);
}
void totalamount(int bottleNumber){
	// Read the current bottle count from EEPROM
	uint16_t currentBottleCount = eeprom_read_word((uint16_t*)FRIDGE_BOTTLE_COUNT_ADDR);
	// Capture the number of bottles from your input source
	int userInputBottleNumber = bottleNumber; // Example number

	// Calculate the total amount
	money = 1500;
	unsigned int totalAmount = userInputBottleNumber * money;

	// Display the number of bottles and the total amount
	char displayStr[20]; // Adjust the size as needed
	sprintf(displayStr, "Bottles: %d FEE: %u", userInputBottleNumber, totalAmount);
	
	// Print the combined information on the LCD
	for (int i = 0; displayStr[i] != '\0'; i++) {
		PORTJ = displayStr[i];
		latch();
	}
	 // Check if there are enough bottles in the fridge
	 if (userInputBottleNumber <= currentBottleCount) {
		 // Subtract the user-input number of bottles from the current count
		 currentBottleCount -= userInputBottleNumber;

		 // Write the updated bottle count back to EEPROM
		 eeprom_update_word((uint16_t*)FRIDGE_BOTTLE_COUNT_ADDR, currentBottleCount);

		 
	 }
	 
}

void selectOption(){
	printLcdFridge("TO CONTINUE, PRESS # or * TO STOP");
}
void printwelc(){
	if ((PIND & 0b00000001) ==0) { // Check if the switch is closed (LOW)

		// Data mode
		PORTB |= (1 << PB5);
		PORTB &= ~(1 << PB6);
		
		// Print "car" on the LCD
		for (int w = 0; w < 17; w++) {
			PORTA = car[w];
			latch2();
		}

		PORTA = 0X80;
		latch2();
		for (int x = 0; x < 20; x++) {
			PORTA = reg[x];
			latch2();
		}
		
	}

}
int main(void) {
	DDRL = 0b00001111;
	DDRH = 0XFF;
	DDRJ = 0XFF;
	DDRD = 0X00;
	DDRC = 0Xff;
	DDRF = 0XFF;
	DDRA =0XFF;
	DDRB =0XFF;
	
	// LCD initialization
	PORTH &= ~(1 << PH5);
	PORTH &= ~(1 << PH6);
	PORTJ = 0B00001110;
	latch();
	
	//lcd 2
	PORTB &= ~(1 << PB5);
	PORTB &= ~(1 << PB6);
	PORTA = 0B00001110;
	latch2();

	
	// Interrupt enabling
	sei();
	EIMSK |= (1 << INT0);
	

	
	
	// Initialize the UART module
	initUART();
	// CLEAR
	
	clearEEPROM();
	// Set the login password ()
	setLoginPassword("12345");

	// setLoginPassword("12345");

	virtualTerminal();
	
	
	int loggedIn = 0; // Variable to track login state
	
	// Initialize the total visitors from EEPROM
	int totalVisitors = eeprom_read_word((uint16_t *)EEPROM_ADDRESS);

	// Ensure that totalVisitors is at least 0
	if (totalVisitors < 0) {
		totalVisitors = 0;
	}

	int carCounter = 0; // Counter for the number of registered cars
	char numberPlates[3][10]; // Store number plates of the registered cars
		// Call the menu function to start the menu system
		displayMenu();
		
		//clear
		 eeprom_update_word((uint16_t *)FRIDGE_BOTTLE_COUNT_ADDR, 0);
		
		// Read the current bottle count from EEPROM
		uint16_t currentBottleCount = eeprom_read_word((uint16_t*)FRIDGE_BOTTLE_COUNT_ADDR);

		

	/* Replace with your application code */
	while (1) {
		
	//KEYPAD CODE  FRIDGE
	
			PORTL = 0B11111110;
			if ((PINL & 0B00010000) == 0) {
	
				userInputBottleNumber = 1;
				
				ComandsLCDFRIDGE();
				
				totalamount(1);
				_delay_ms(50000);
				ComandsLCDFRIDGE();
				selectOption();
				
				
			}
			else if((PINL & 0B00100000) == 0){
			userInputBottleNumber =4;
				
				ComandsLCDFRIDGE();
				totalamount(4);
				_delay_ms(50000);
				ComandsLCDFRIDGE();
				selectOption();
			}
			else if((PINL & 0B01000000) == 0){
				userInputBottleNumber = 7;
				
				ComandsLCDFRIDGE();
				
				totalamount(7);
				_delay_ms(50000);
				ComandsLCDFRIDGE();
				selectOption();
				
				
			}
			else if((PINL & 0B10000000) == 0) {
				// User canceled the transaction by pressing '*'
				bottleNumber = 0; // Set the bottle number to 0 to represent cancellation

				// Switch to command mode
				PORTH &= ~(1 << PH5);
				latch();

				// Clear the LCD display
				PORTJ = 0x01;
				latch();
				PORTJ = 0XC0;
				latch();
				// Set the LCD back to data mode
				PORTH |= (1 << PH5);
				PORTH &= ~(1 << PH6);

				// Print the "Cancilled" message
				for (int x = 0; x < 10; x++) {
					PORTJ = conc[x];
					latch();
				}
				
			}
			
			
			PORTL = 0B11111101;
			
			if ((PINL & 0B00010000) == 0) {
				userInputBottleNumber = 2;
				
				ComandsLCDFRIDGE();
				
				totalamount(2);
				_delay_ms(50000);
				ComandsLCDFRIDGE();
				selectOption();
				
			}
			else if((PINL & 0B00100000) == 0){
				userInputBottleNumber = 5;
			
				ComandsLCDFRIDGE();
				totalamount(5);
				_delay_ms(50000);
				ComandsLCDFRIDGE();
				selectOption();
				
			}
			else if((PINL & 0B01000000) == 0){
				userInputBottleNumber = 8;
				ComandsLCDFRIDGE();
				totalamount(8);
				_delay_ms(50000);
				ComandsLCDFRIDGE();
				selectOption();
		
			}
			else if((PINL & 0B10000000) == 0) {
				// User canceled the transaction by pressing '*'
				bottleNumber = 0; // Set the bottle number to 0 to represent cancellation

				// Switch to command mode
				PORTH &= ~(1 << PH5);
				latch();

				// Clear the LCD display
				PORTJ = 0x01;
				latch();
				PORTJ = 0XC0;
				latch();
				// Set the LCD back to data mode
				PORTH |= (1 << PH5);
				PORTH &= ~(1 << PH6);

				// Print the "Cancilled" message
				for (int x = 0; x < 10; x++) {
					PORTJ = succ[x];
					latch();
				}
				
			}
			
			PORTL = 0B11111011;
			
			if ((PINL & 0B00010000) == 0) {
				userInputBottleNumber = 3;
				ComandsLCDFRIDGE();
				totalamount(3);
				_delay_ms(50000);
				ComandsLCDFRIDGE();
				selectOption();
				
			}
			else if((PINL & 0B00100000) == 0){
				userInputBottleNumber = 6;
				ComandsLCDFRIDGE();
				totalamount(6);
				_delay_ms(50000);
				ComandsLCDFRIDGE();
				selectOption();
				
			}
			else if((PINL & 0B01000000) == 0){
				userInputBottleNumber = 9;
				ComandsLCDFRIDGE();
				totalamount(9);
				_delay_ms(50000);
				ComandsLCDFRIDGE();
				selectOption();
				
			}
			else if((PINL & 0B10000000) == 0) {
				
				rotateMotor(3);
				ComandsLCDFRIDGE();
				printLcdFridge("INSERT COINS ");
				
				_delay_ms(500000);
				
				PORTH &=~(1<<PH5);
				PORTJ = 0X01;
				latch();
				PORTJ = 0X80;
				latch();
				printLcdFridge("DISPENSING ");
				for (int x=0; x < userInputBottleNumber; x++)
				{
					rotateMotor(2);
					
				}
				
				
				
			}
			
		
			

			

		}	
		
}




ISR(INT0_vect) {
		PORTC ^=(1<<0);

		printwelc();
}


