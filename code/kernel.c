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

struct GameField {
    int hasBoat; //1 for yes, 0 for no
    int wasFound; //1 for yes, 0 for no
    unsigned int x; // x value on the screen
    unsigned int y; // y value on the screen
};

struct GameField oldPoint;
int xPos;
int yPos;

int boats = 9;

void initializeGameField(struct GameField field[10][10]) {
    for (int i = 0; i < 10; i++) {
        for (int j= 0; j < 10; j++) {
            field[i][j].hasBoat = 0;
            field[i][j].wasFound = 0;
            field[i][j].x = 0; // has to be changed to the x value of the field
            field[i][j].y = 0; // has to be changed to the y value of the field
        }
    }
}


enum {
    BLUE   = 0x11,
    GREEN = 0x22,
    LIGHT_BLUE_STRONG  = 0x33,
    LIGHT_GREEN = 0xAA,
    LIGHT_BLUE = 0x99,
    LIGHT_BLUE_COLD = 0xBB,
    ORANGE = 0x66,
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

void drawBoat(struct GameField field) {
    // draw blue background
    drawRect(field.x + 1, field.y + 1, field.x + FIELD_SIZE - 1, field.y + FIELD_SIZE -1, BLUE, 1);

    // draw bottom part of the boat (0x66 == ORANGE)
    for (int i = 0; i < 2; i++) {
        drawCircle(field.x + 20 + i * 40, field.y + 60, 10, ORANGE, 1);
        drawRect(field.x + 10 + i * 40, field.y + 50, field.x + 30 + i * 40, field.y + 60, BLUE, 1);
    }
    drawRect(field.x + 20, field.y + 60, field.x + 60, field.y + 70, ORANGE, 1);
    

    // draw sail and mast twice
    for (int i = 0; i < 2; i++) {
        // draw a circle and get rid of one half
        drawCircle(field.x + 50 - i * 20, field.y + 30, 20, WHITE, 1);
        drawRect(field.x + 30 - i * 20, field.y + 10, field.x + 50 - i * 20, field.y + 50, BLUE, 1);
        // add line for mast
        drawRect(field.x + 50 - i * 20, field.y + 50, field.x + 52 - i * 20, field.y + 60, WHITE, 1);
    }
}

void drawWaves(struct GameField field, int offsetX, int offsetY, int amountOfWaves, int darkerColor, int brighterColor) {
    for (int i = 0; i < amountOfWaves; i++) {
        drawCircle(field.x + i * WAVE_LENGTH + offsetX + WAVE_RADIUS, field.y + offsetY + WAVE_RADIUS + 1, WAVE_RADIUS, brighterColor, 1);
    }
    for (int i = 0; i < amountOfWaves; i++) {
        drawCircle(field.x + i * WAVE_LENGTH + offsetX + WAVE_RADIUS, field.y + offsetY + WAVE_RADIUS + 5, WAVE_RADIUS, darkerColor, 1);
    }
}


void drawFieldColors(struct GameField field) {
    // drawing blue background
    drawRect(field.x + 1, field.y + 1, field.x + FIELD_SIZE - 1, field.y + FIELD_SIZE - 1, BLUE, 1);

    // if field was not found yet than there are some waves 
    if (field.wasFound == 0) {
    

        for (int i = 0; i < 4; i++) {
           for (int j = 0; j < 5; j++) {
                drawWaves(field, 0, j * (WAVE_LENGTH - 6), 4, BLUE, LIGHT_BLUE_STRONG);
            }
        }

        // drawing white base line
        drawRect(field.x, field.y, field.x + FIELD_SIZE, field.y + FIELD_SIZE, WHITE, 0);
    } else {
        for (int i = 0; i < 4; i++) {
           for (int j = 0; j < 5; j++) {
                drawWaves(field, 0, j * (WAVE_LENGTH - 6), 4, GREY, LIGHT_GREY);
            }
        }
    }
}

void drawDesign2 (struct GameField field) {
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
    struct GameField field;
    field.x = 0;
    field.y = 1000;
    for (int i = 0; i < 24; i++) {
       drawDesign2(field);
       field.x = field.x + FIELD_SIZE;
    }
}

void drawBoardGame (struct GameField board[10][10], unsigned int offset, unsigned int offsetY) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int xValue = i * FIELD_SIZE + offset;
            int yValue = j * FIELD_SIZE + offsetY;

            // put values in array for the boardgame
            board[i][j].x = xValue; 
            board[i][j].y = yValue;
            drawFieldColors(board[i][j]);
        }
    }
}

void drawMarginAroundField(struct GameField field, int thicknessOfMargin) {
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

void pointer(struct GameField field){
	//Highlights the curentle selectet tile.
	//checks if there is already a Highliter via an invisible tile.
	if(oldPoint.x != 0){
		//draws the ourField design to remove the last highliter
		drawFieldColors(oldPoint);
			
	}
	//draws the new Highliter
	drawMarginAroundField(field, 10);
	oldPoint = field;
}

int isHit(struct GameField field){
	//checks if there is a Boat
	return field.hasBoat;
}

void placeBoat(struct GameField field){
	//Places a boat
	field.hasBoat = 1;
}

void shoot(struct GameField field){
	//Sets a tile to the used ones
	field.wasFound = 1;
    if(isHit(field) == 1){
        boats--;
    }
}

void enemyPlacement(struct GameField field[10][10]){
	for(int b = 0; b<10; b++){
		
    /*
	for(int b = 1; b<10; b++){
		if(b+1 < 10){
			field[b-1][b+1].hasBoat = 1;
			drawBoat(field[b-1][b+1]);
		}else{
			field[b-1][b].hasBoat = 1;
			drawBoat(field[b-1][b]);
		}
	}
    */
    field[0][0].hasBoat = 1;
    field[8][1].hasBoat = 1;
    field[4][4].hasBoat = 1;
    field[6][4].hasBoat = 1;
    field[9][9].hasBoat = 1;
    field[7][2].hasBoat = 1;
    field[3][7].hasBoat = 1;
    field[4][2].hasBoat = 1;
    field[4][7].hasBoat = 1;
    field[6][6].hasBoat = 1;
    }
}

int loser(struct GameField field[10][10]){
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
	if(sum >= 10){
		return 1;
	}else{
		return 0;
	}
}

void movePointer(struct GameField field[10][10], char richtung){
    switch(richtung) : 
        case 'd':  pointer(field[xPos+1][yPos]); break;
        case 'a':  pointer(field[xPos-1][yPos]); break;
        case 'w':  pointer(field[xPos][yPos+1]); break;
        case 's':  pointer(field[xPos][yPos-1]); break;
}



//=============================================================================================================

void main() {
    // creating an array with colors available
    static int colors[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

    // two arrays with random numbers since we cannot add user C libraries to kernels.
    int random1[] = {8,50,74,59,31,73,45,79,24,10,41,66,93,43,88,4,28,30,41,13,4,70,10,
    58,61,34,100,79,17,36,98,27,13,68,11,34,80,50,80,22,68,73,94,37,86,46,29,92,95,58,
    2,54,9,45,69,91,25,97,31,4,23,67,50,25,2,54,78,9,29,34,99,82,36,14,66,15,64,37,26,
    70,16,95,30,2,18,96,6,5,52,99,89,24,6,83,53,67,17,38,39,45};

    int random2[] = {3,1,1,2,2,0,0,2,3,1,0,1,0,2,3,3,3,1,0,0,3,1,1,1,0,1,3,2,0,3,1,2,0,
    3,2,1,3,1,3,1,3,0,1,0,1,1,0,3,2,1,1,1,0,0,0,2,0,0,2,3,2,2,1,0,1,1,1,0,0,1,2,1,3,1,
    1,2,3,0,1,1,3,2,1,1,1,3,1,0,3,2,0,3,1,2,0,2,0,1,2,0};
    
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
    uart_writeText("Hello world!\n");
    fb_init();


    // TO DO: necessary to give the player the opportunity to choose his/her fields here.
    // TO DO: add ships in the opponents fields.
    // TO DO: start game in loop

    // Test des outputs:

    drawPixel(50, 50, 0x0f);
    
    drawRect(300, 400, 500, 500, 0x11, 0);
    
     
    drawChar('A', 700, 700, 0x0f, 10);  
    drawChar('B', 700, 750, 0x0f, 10);  
    
    drawString((WIDTH/2)-252, MARGIN-25, "Battleship >:D", 0x0f, 3);
 




    // calculating the offset by the half of the screen (540 or 960) minus half of the boardgame (400)
    int offsetY = (HEIGHT/2) - (FIELD_SIZE * 10 / 2);
    int offsetX = ((WIDTH/2) - (FIELD_SIZE * 10)) /2;
    
    // board of player is drawn here:
    drawBoardGame(ourField, offsetX, offsetY);

    // board of opponent is drawn here:
    drawBoardGame(fieldOfOpponent, offsetX + WIDTH/2, offsetY);

    drawDesign2Margin();

    // margin tests
    //drawMarginAroundField(ourField[1][1], MARGIN_FIELD);
    
    enemyPlacement(fieldOfOpponent);
    enemyPlacement(ourField);
    drawBoat(ourField[9][0]);
    unsigned char ch = 0;

    int i = 0;
    int j = 0;
    int t = 0;
    int gameStillOn = 1;
    while (gameStillOn) {
    	drawString((WIDTH/2)-252, MARGIN-5, "Ships left: ", 0x0f, 3);
    	drawChar(boats + 0x30, (WIDTH/2), MARGIN-5, 0x0f, 3); 

        if (i == 16) {i=0;}
        wait_msec(480000); // Wait a little...
        //wait_msec(4000); // Wait a little...
        drawChar(i + 0x30, (WIDTH/2)-252 + (8*8*3), MARGIN-25, 0x0f, 3);

        drawRect(100, 600, 350, 700, colors[i], 1);	
	    pointer(ourField[j][t]);
        xPos = j;
        yPos = t;
        shoot(oldPoint);

        if ( ( ch = getUart() ) ) {
            movePointer(ourField, ch);
        if (ch == '9') {

        }
        if (ch == 'h') {

        }
        }
        uart_loadOutputFifo();
	
        i++;
        if(loser(ourField) == 1){
            drawString((WIDTH/2)-252, (HEIGHT/2), "GAME OVER", 0x0f, 20);
        }
    }   
    while (1);
}
