// Code by https://github.com/isometimes/rpi4-osdev
// Comments by Adrian, Natasha
// Game logic by Adrian
// Visual Game Design by Natasha

#include "fb.h"
#include "io.h"

// The screen
#define WIDTH           1920
#define HEIGHT          1080
#define MARGIN          30

/*  
    Screens work like this:
      (0,0)  -----> x 
    |           (1,0)
    |
    |
    y (0,1)
*/

#define FIELD_SIZE      80
#define WAVE_RADIUS     10
#define WAVE_LENGTH     2 * WAVE_RADIUS
#define MARGIN_FIELD    10

//========GAME=GLOBALS=============
struct GameField {
    int hasBoat;
    int wasFound; //1 for yes, 0 for no
    unsigned int x; // x value on the screen
    unsigned int y; // y value on the screen
};

struct GameField oldPoint;
struct GameField* fieldd = &oldPoint;
int xPos;
int yPos;
int xEne = 0;
int yEne = 0;
int boats = 9;
int eBoats = 9;
int random[9] = {0,0,0,0,0,0,0,0,0};
//=================================

void initializeGameField(struct GameField field[10][10]) {
    //initializes the game field
    for (int i = 0; i < 10; i++) {
        for (int j= 0; j < 10; j++) {
            field[i][j].hasBoat = 0;
            field[i][j].wasFound = 0;
            field[i][j].x = 0; // has to be changed to the x value of the field
            field[i][j].y = 0; // has to be changed to the y value of the field
        }
    }
}

//COLORS
enum {
    BLUE   = 0x11,
    GREEN = 0x22,
    LIGHT_BLUE_STRONG  = 0x33,
    LIGHT_GREEN = 0xAA,
    LIGHT_BLUE = 0x99,
    LIGHT_BLUE_COLD = 0xBB,
    ORANGE = 0x66,
    PINK = 0x55,
    RED = 0x44,
    BLACK = 0x00,
    LIGHT_GREY = 0x77,
    GREY = 0x88,
    WHITE   = 0xFF
};

// KEY HANDLER
unsigned char getUart()
{
    unsigned char ch = 0;

    if (uart_isReadByteReady()) ch = uart_readByte();
    return ch;
}

void drawBoat(struct GameField field, int backgroundColor, int boatColor) {
    // draw blue background
    drawRect(field.x + 1, field.y + 1, field.x + FIELD_SIZE - 1, field.y + FIELD_SIZE -1, backgroundColor, 1);

    // draw bottom part of the boat (0x66 == ORANGE)
    for (int i = 0; i < 2; i++) {
        drawCircle(field.x + 20 + i * 40, field.y + 60, 10, boatColor, 1);
        drawRect(field.x + 10 + i * 40, field.y + 50, field.x + 30 + i * 40, field.y + 60, backgroundColor, 1);
    }
    drawRect(field.x + 20, field.y + 60, field.x + 60, field.y + 70, boatColor, 1);
    
    // draw sail and mast twice
    for (int i = 0; i < 2; i++) {
        // draw a circle and get rid of one half
        drawCircle(field.x + 50 - i * 20, field.y + 30, 20, WHITE, 1);
        drawRect(field.x + 30 - i * 20, field.y + 10, field.x + 50 - i * 20, field.y + 50, backgroundColor, 1);
        // add line for mast
        drawRect(field.x + 51 - i * 20, field.y + 50, field.x + 52 - i * 20, field.y + 60, WHITE, 1);
    }
}

void drawWaves(struct GameField field, int offsetX, int offsetY, int amountOfWaves, int darkerColor, int brighterColor) {
    //draws the wave pattern
    for (int i = 0; i < amountOfWaves; i++) {
        drawCircle(field.x + i * WAVE_LENGTH + offsetX + WAVE_RADIUS, field.y + offsetY + WAVE_RADIUS + 1, WAVE_RADIUS, brighterColor, 1);
    }
    for (int i = 0; i < amountOfWaves; i++) {
        drawCircle(field.x + i * WAVE_LENGTH + offsetX + WAVE_RADIUS, field.y + offsetY + WAVE_RADIUS + 5, WAVE_RADIUS, darkerColor, 1);
    }
}

void drawAttackSymbol(struct GameField field) {
    // draw red diagnoal (three times so that it looks thicker)
    drawLine(field.x + 1, field.y + 1, field.x + FIELD_SIZE, field.y + FIELD_SIZE, RED);
    drawLine(field.x + 1, field.y + 2, field.x + FIELD_SIZE - 1, field.y + FIELD_SIZE, RED);
    drawLine(field.x + 2, field.y + 1, field.x + FIELD_SIZE, field.y + FIELD_SIZE - 1, RED);
}

void drawFieldColors(struct GameField field, int isOpponent) {

    // if field was not found yet than there are some waves 

    if (field.wasFound == 0) {
    
        if (isOpponent && field.hasBoat) {
            drawBoat(field, BLUE, ORANGE);
        } else {
            // drawing blue background
            drawRect(field.x + 1, field.y + 1, field.x + FIELD_SIZE - 1, field.y + FIELD_SIZE - 1, BLUE, 1);
            // if it is our field than no boats will be shown
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 5; j++) {
                    drawWaves(field, 0, j * (WAVE_LENGTH - 6), 4, BLUE, LIGHT_BLUE_STRONG);
                }
            }
        }

    } else {
        // field.wasFound == 1 means that the field was chosen. If there was a boat, it will be shown.
        if (field.hasBoat) {
            drawBoat(field, GREY, LIGHT_GREY);
            // TO DO: draw attack symbol
        } else {
            // drawing background
            drawRect(field.x + 1, field.y + 1, field.x + FIELD_SIZE - 1, field.y + FIELD_SIZE - 1, GREY, 1);
            // drawing waves
            for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 5; j++) {
                    drawWaves(field, 0, j * (WAVE_LENGTH - 6), 4, GREY, LIGHT_GREY);
                }
            }
            
        }
        drawAttackSymbol(field);
    }
    // drawing white base line
    drawRect(field.x, field.y, field.x + FIELD_SIZE, field.y + FIELD_SIZE, WHITE, 0);
}

void drawDesign2 (struct GameField field) {
    //draws an alternitive wave pattern
    for (int i = 0; i < 2; i++) {
        drawRect(field.x, field.y + i * 2 * WAVE_LENGTH, field.x + FIELD_SIZE, field.y + WAVE_RADIUS + i * 2 * WAVE_LENGTH, BLUE, 1);
        drawRect(field.x, field.y + 1 * WAVE_RADIUS + i * 2 * WAVE_LENGTH, field.x + FIELD_SIZE, field.y + 3 * WAVE_RADIUS + i * 2 * WAVE_LENGTH, LIGHT_BLUE, 1);
        drawRect(field.x, field.y + 3 * WAVE_RADIUS + i * 2 * WAVE_LENGTH, field.x + FIELD_SIZE, field.y + 4 * WAVE_RADIUS + i * 2 * WAVE_LENGTH, BLUE, 1);
    }

    // drawing waves
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i % 2 == 0) {
                if (j % 2 == 0) {
                    drawCircle(field.x + i * WAVE_LENGTH + WAVE_RADIUS, field.y + j * WAVE_LENGTH + WAVE_RADIUS, WAVE_RADIUS, LIGHT_BLUE, 1);
                } else {
                    drawCircle(field.x + i * WAVE_LENGTH + WAVE_RADIUS, field.y + j * WAVE_LENGTH + WAVE_RADIUS, WAVE_RADIUS, BLUE, 1);
                }
            } else {
                if (j % 2 == 0) {
                    drawCircle(field.x + i * WAVE_LENGTH + WAVE_RADIUS, field.y + j * WAVE_LENGTH + WAVE_RADIUS, WAVE_RADIUS, BLUE, 1);
                } else {
                    drawCircle(field.x + i * WAVE_LENGTH + WAVE_RADIUS, field.y + j * WAVE_LENGTH + WAVE_RADIUS, WAVE_RADIUS, LIGHT_BLUE, 1);
                }                
            }
        }
    }
}

void drawDesign2Margin () {
    //draws with the other design a line of fields
    struct GameField field;
    field.x = 0;
    field.y = 1000;
    for (int i = 0; i < 24; i++) {
       drawDesign2(field);
       field.x = field.x + FIELD_SIZE;
    }
}

void drawBoardGame (struct GameField board[10][10], unsigned int offset, unsigned int offsetY, int isOpponent) {
    // Draws a playingboard
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int xValue = i * FIELD_SIZE + offset;
            int yValue = j * FIELD_SIZE + offsetY;

            // put values in array for the boardgame
            board[i][j].x = xValue; 
            board[i][j].y = yValue;
            drawFieldColors(board[i][j], isOpponent);
        }
    }
}

void drawMarginAroundField(struct GameField field, int thicknessOfMargin) {
    //Draws a margin around the field to indicate the selection box
    int xValue = field.x;
    int yValue = field.y;
    // right rectangle of the margin
    drawRect(xValue, yValue, xValue + thicknessOfMargin, yValue + FIELD_SIZE, WHITE, 1);
    // left rectangle of the margin
    drawRect(xValue + FIELD_SIZE - thicknessOfMargin, yValue, xValue + FIELD_SIZE, yValue + FIELD_SIZE, WHITE, 1);
    // upper rectangle of the margin
    drawRect(xValue, yValue, xValue + FIELD_SIZE, yValue + thicknessOfMargin, WHITE, 1);
    // lower rectangle of the margin
    drawRect(xValue, yValue + FIELD_SIZE - thicknessOfMargin, xValue + FIELD_SIZE, yValue + FIELD_SIZE, WHITE, 1);

}

//============GAMELOGIK=====================================================================================

void pointer(struct GameField field, int a){
	//Highlights the curentle selectet tile.
	//checks if there is already a Highliter via an invisible tile.
	if(fieldd->x != 0){
		//draws the ourField design to remove the last highliter
		drawFieldColors(*fieldd, a);
	}
	//draws the new Highliter
	drawMarginAroundField(field, 10);
	fieldd = &field;
}

int isHit(struct GameField field){
	//checks if there is a Boat
	return field.hasBoat;
}

void placeBoat(struct GameField field[10][10]){
	//Places a boat
    if(field[xPos][yPos].hasBoat == 0){
        field[xPos][yPos].hasBoat = 1;
        drawString((WIDTH/2)-252, (MARGIN+10), "you Placed a Boad", 0x0f, 5);
    }
}

void shoot(struct GameField field[10][10], int a){
	//Sets a tile to the used ones
           
            if(a == 0){
                if(field[xPos][yPos].wasFound == 0){
                    field[xPos][yPos].wasFound = 1;
                    if(isHit(field[xPos][yPos]) == 1){
                        boats--;
                    }else{
                        drawString((WIDTH/2)-252, (MARGIN+10), "you Hit this place already", 0x0f, 5);
                    }
                }
            }else{
                if(field[xEne][yEne].wasFound == 0){
                    field[xEne][yEne].wasFound = 1;
                    if(isHit(field[xEne][yEne]) == 1){
                        eBoats--;
                    }
                }else{
                    drawString((WIDTH/2)-252, (MARGIN+10), "you Hit this place already", 0x0f, 5);
                }
            }
}

int loser(struct GameField field[10][10]){
    //determentas the loser
	int sum = 0;
	for(int i = 0; i<10; i++){
		for(int j = 0; j < 10; j++){
			if(field[i][j].wasFound == 1 ){
				if(field[i][j].hasBoat == 1){
                    sum++;
                }
			}
		}
	}
	if(sum == 9){
		return 1;
	}else{
		return 0;
	}
}

void movePointer(struct GameField field[10][10], char richtung, int a){
    //moves the pointer throu user input
    switch(richtung){ 
        case '3':  if(xPos != 9){
                        pointer(field[xPos+1][yPos],a);
                        xPos++;
                    }else{
                        pointer(field[0][yPos],a);
                        xPos = 0;
                    } 
                   break;
        case '2':   if(xPos != 0){
                        pointer(field[xPos-1][yPos],a);
                        xPos--;
                    }else{
                        pointer(field[9][yPos],a);
                        xPos = 9;
                    }
                    break; 
        case 'Q':   if(yPos != 9){
                        pointer(field[xPos][yPos+1],a);
                        yPos++;
                    }else{
                        pointer(field[xPos][0],a);
                        yPos = 0;
                    }
                    break;
        case 's':   if(yPos != 0){
                        pointer(field[xPos][yPos-1],a);
                        yPos--;
                    }else{
                        pointer(field[xPos][9],a);
                        yPos = 9;
                    }
                    break;
    }
    fieldd = &field[xPos][yPos];
}

void enemyPlacement(struct GameField field[10][10]){
    //puts Boats on the enemy field but can be used to play them on any field
	for(int b = 0; b<10; b++){
        int rn = random[b];
        field[rn/10][rn % 10].hasBoat = 1;

    /*
    // manual placement
    field[0][0].hasBoat = 1;
    field[8][1].hasBoat = 1;
    field[4][4].hasBoat = 1;
    field[6][4].hasBoat = 1;
    field[9][9].hasBoat = 1;
    field[7][2].hasBoat = 1;
    field[3][7].hasBoat = 1;
    field[4][2].hasBoat = 1;
    field[1][7].hasBoat = 1;
    field[6][6].hasBoat = 1;
    */

    }
}

void enemyTurn(struct GameField field[10][10]){
    //the Brain of our enemy and his devies plans
    //he just checks one field at the time
    //TO DO: make him smart
    shoot(field, 1);
    drawFieldColors(field[xEne][yEne],1);
    if(xEne==9){
        xEne = 0;
        yEne++;
    }else{
        xEne++;
    }
    if(yEne==9){
        yEne = 0;
    }
}

int isInRandom(int a){
    //checks if nummer is in random already
    int isit = 0;
    for(int v = 0; v<10; v++){
        if(random[v]==a){
            isit = 1;
        }
    }
    return isit;
}

int punshNumbers(int a){
    //generats a new number
    int r = 0;
    for(int v = 0; v < a; v++){
        r = r + 13;
        if(r > 100){
            r = r % 100;
        }
    }
    return r;
}

void playerPlacment(struct GameField field[10][10]){
    //the Player should be able to place his ships in this little routien
    //The shoot button will be the way to place the ships (may need adjustments)
    int shipsToPlace = 9;
    char ch = 'a';
    int rand = 0;
    int randfill = 0;
    while(shipsToPlace > 0){
        // rand will be used to generate kinda random numbers
        if(rand == 100){
            rand = 0;
        }

        //the routien that checks for input
        if ( ( ch = getUart() ) ) {
            movePointer(field, ch,1);
            drawChar(ch, 50, 50, 0x0f, 3);
            // additional screen output for clarity and debugging purposes
            // keyboard: c, d, e -> up
            if (ch == 's') {
                drawRect((WIDTH/2)-252, (MARGIN + 10), WIDTH, (MARGIN + 10) , BLACK, 1);
                drawString((WIDTH/2)-252, (MARGIN + 10), "you went up", 0x0f, 5);
            } else if (ch == '3') {
                // keyboard: s, d, f -> right
                drawRect((WIDTH/2)-252, (MARGIN + 10), WIDTH, (MARGIN + 10) , BLACK, 1);
                drawString((WIDTH/2)-252, (MARGIN+10), "you went to the right", 0x0f, 5);
            } else if (ch == 'Q') {
                // keyboard: e, d, c -> down
                drawRect((WIDTH/2)-252, (MARGIN + 10), WIDTH, (MARGIN + 10) , BLACK, 1);
                drawString((WIDTH/2)-252, (MARGIN+10), "you went down", 0x0f, 5);
            } else if (ch == ';') {
                // keyboard: t, g, b -> choose
                //main differenc hier
                if(fieldd->hasBoat == 0){
                    placeBoat(field);
                    shipsToPlace--;
                    //filling random but not with the same numbers
                    if(randfill < 10){
                        if(isInRandom(rand) == 0){
                            random[randfill] = rand;
                            randfill++;
                        }else{
                            int r = rand;
                            while(isInRandom(r)!=1){
                                r = punshNumbers(r);
                            }
                            random[randfill] = r;
                        }
                    }
                }else{
                  drawRect((WIDTH/2)-252, (MARGIN + 10), WIDTH, (MARGIN + 10) , BLACK, 1);
                  drawString((WIDTH/2)-252, (MARGIN+10), "ther is already a Boad", 0x0f, 5);  
                }
                drawFieldColors(field[xPos][yPos],1);
                drawMarginAroundField(field[xPos][yPos],10);
            } else {
                //im leving this emty for now (Adrian)                
            }
        }
        
        rand++;

        uart_loadOutputFifo();
    }
}



//=============================================================================================================

void main() {
    oldPoint.hasBoat = 0;
    oldPoint.wasFound = 0;
    oldPoint.x=0; 
    oldPoint.y=0;
    
    // initializing the players field
    struct GameField ourField[10][10];
    initializeGameField(ourField);

    // initializing the field of the opponent
    struct GameField fieldOfOpponent[10][10];
    initializeGameField(fieldOfOpponent);

    // initialisation for the connection with I/O
    uart_init();
   
    fb_init();  

    // Displaying the Logo:

    drawChar('N', 700, 700, 0x0f, 10);  
    drawChar('A', 700, 750, 0x0f, 10);  
    drawString(720, 725, "OS", 0x0f, 3);
 
    drawString((WIDTH/2)-252, MARGIN-25, "Battleship >:D", 0x0f, 3);
 
    // calculating the offset by the half of the screen (540 or 960) minus half of the boardgame (400)
    int offsetY = (HEIGHT/2) - (FIELD_SIZE * 10 / 2);
    int offsetX = ((WIDTH/2) - (FIELD_SIZE * 10)) /2;
    
    // board of player is drawn here:
    drawBoardGame(ourField, offsetX, offsetY, 0);

    // board of opponent is drawn here:
    drawBoardGame(fieldOfOpponent, offsetX + WIDTH/2, offsetY, 1);

   // setting the selection box to its beginning point
    xPos = 0;
    yPos = 0;

    //Player can place ships
    playerPlacment(fieldOfOpponent);
    drawBoardGame(fieldOfOpponent, offsetX + WIDTH/2, offsetY, 1);
    //The Enemy places ships
    enemyPlacement(ourField);

    unsigned char ch = 0;

    int i = 0;
    
    int gameStillOn = 1;

    //set the first position
    
    pointer(ourField[0][0],0);
    drawFieldColors(ourField[0][0],0);

    //OS loop
    while (gameStillOn) {
        //showing how many ships there are
    	drawString((offsetX+(2*FIELD_SIZE)), (offsetY-30), "Ships left: ", 0x0f, 3);
    	drawChar(boats + 0x30, (offsetX+(6*FIELD_SIZE)), (offsetY-30), 0x0f, 3); 

        drawString((offsetX+ WIDTH/2+(2*FIELD_SIZE)), (offsetY-30), "Your Ships left: ", 0x0f, 3);
    	drawChar(eBoats + 0x30, (offsetX + WIDTH/2+(7*FIELD_SIZE)), (offsetY-30), 0x0f, 3); 

        if (i == 16) {i=0;}
        wait_msec(4000); 
        uart_writeText("1");
        //check for input
        if ( ( ch = getUart() ) ) {
            movePointer(ourField, ch,0);
            drawChar(ch, 50, 50, 0x0f, 3);
            // additional screen output for clarity and debugging purposes
            // keyboard: c, d, e -> up
            if (ch == 's') {
            drawString((WIDTH/2)-252, (MARGIN + 10), "you went up", 0x0f, 5);
            } else if (ch == '3') {
                // keyboard: s, d, f -> right
                drawString((WIDTH/2)-252, (MARGIN + 10), "you went to the right", 0x0f, 5);
            } else if (ch == 'Q') {
                // keyboard: e, d, c -> down
                drawString((WIDTH/2)-252, (MARGIN + 10), "you went down", 0x0f, 5);
            } else if (ch == ';') {
                // keyboard: t, g, b -> choose
                drawString((WIDTH/2)-252, (MARGIN + 10), "you choose a field", 0x0f, 5);
                shoot(ourField,0);
                drawFieldColors(ourField[xPos][yPos],1);
                drawMarginAroundField(ourField[xPos][yPos],10);
                enemyTurn(fieldOfOpponent);
                drawBoardGame(fieldOfOpponent, offsetX + WIDTH/2, offsetY, 1);
            } else if(ch == '2') {
                // keyboard: z, t, r -> left
                drawString((WIDTH/2)-252, (MARGIN + 10), "you went to the left", 0x0f, 5);
                
            }else{
                //drawString((WIDTH/2)-252, (MARGIN+10), "input plz", 0x0f, 5);
            }
        }  
        uart_loadOutputFifo();
        
        i++;
        //check for loser
        if(loser(ourField) == 1){
            drawString((50), (HEIGHT/2), "GAME OVER", 0x0f, 20);
        }
        //uart_update();
    } 
    //Stops the system from crashing after finishing main method   
    while (1);
}