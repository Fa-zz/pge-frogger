#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include "olcPixelGameEngine.h"

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

class OneLoneCoder_Frogger : public olc::PixelGameEngine {

public:
    OneLoneCoder_Frogger() {
        sAppName = "Frogger";
    }

    int nScreenWidth = 128;
    int nScreenHeight = 80;
    int getScreenWidth() { return nScreenWidth; }
    int getScreenHeight() { return nScreenHeight; }

private:

    // vecLanes: Represents the "lanes" incl. safe and unsafe zones, and each corresponding velocity
	std::vector<std::pair<float, std::string>> vecLanes = {
        // 64 elements per lane
		{  0.0f,  "wwwhhwwwhhwwwhhwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww"  },  // escape
		{ -3.0f, ",,,jllk,,jllllk,,,,,,,jllk,,,,,jk,,,jlllk,,,,jllllk,,,,jlllk,,,,"  },  // logs
		{  3.0f,  ",,,,jllk,,,,,jllk,,,,jllk,,,,,,,,,jllk,,,,,jk,,,,,,jllllk,,,,,,,"  },  // logs
		{  2.0f,  ",,jlk,,,,,jlk,,,,,jk,,,,,jlk,,,jlk,,,,jk,,,,jllk,,,,jk,,,,,,jk,,"  },  // logs 
		{  0.0f,  "pppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppp"  },  // safe zone
		{ -3.0f, "....asdf.......asdf....asdf..........asdf........asdf....asdf..."  },  // buses
		{  3.0f, ".....ty..ty....ty....ty.....ty........ty..ty.ty......ty.......ty"  },  // cars
		{ -4.0f, "..zx.....zx.........zx..zx........zx...zx...zx....zx...zx...zx.."  },  // cars
		{  2.0f,  "..ty.....ty.......ty.....ty......ty..ty.ty.......ty....ty......."  },  // cars
		{  0.0f,  "pppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppp"  }   // safe zone
        };

    // Other private member vars go here
    float fTimeSinceStart = 0.0f;   // fTimeSinceStart: Used to draw lane info to screen.
    int nCellSize = 8;              // nCellSize: Size of each game "cell" (each elem in vecLanes constitutes a cell). Chose 8 b/c window is in multiples of 8
    int nLaneWidth = 18;            // nLaneWidth: The amount of the lane we want the screen to see at one time

    float fFrogX = 8.0f;
    float fFrogY = 9.0f;

    bool *bufDanger = nullptr;

    // Sprites
    olc::Sprite *spriteBus = nullptr;
	olc::Sprite *spriteLog = nullptr;
	olc::Sprite *spriteWater = nullptr;
	olc::Sprite *spriteFrog = nullptr;
	olc::Sprite *spritePavement = nullptr;
	olc::Sprite *spriteCar1 = nullptr;
	olc::Sprite *spriteCar2 = nullptr;
	olc::Sprite *spriteWall = nullptr;
	olc::Sprite *spriteHome = nullptr;

protected:
    // ON USER CREATE
    virtual bool OnUserCreate() {

        bufDanger = new bool[ScreenWidth() * ScreenHeight()];  //bufDanger: Array of bools, same dimensions as screen
        memset(bufDanger, 0, ScreenWidth() * ScreenHeight() * sizeof(bool));

        // Load sprites
        spriteBus = new olc::Sprite("FroggerSprites/bus.png");
		spriteLog = new olc::Sprite("FroggerSprites/log.png");
		spriteWater = new olc::Sprite("FroggerSprites/water.png");
		spriteFrog = new olc::Sprite("FroggerSprites/frog.png");
		spritePavement = new olc::Sprite("FroggerSprites/pavement.png");
		spriteWall = new olc::Sprite("FroggerSprites/wall.png");
		spriteHome = new olc::Sprite("FroggerSprites/home.png");
		spriteCar1 = new olc::Sprite("FroggerSprites/car1.png");
		spriteCar2 = new olc::Sprite("FroggerSprites/car2.png");

        return true;
    }

    // ON USER UPDATE
    virtual bool OnUserUpdate(float fElapsedTime) {
        fTimeSinceStart += fElapsedTime;

        // Handle input
        if (GetKey(olc::Key::UP).bReleased)   fFrogY -= 1.0f;
        if (GetKey(olc::Key::DOWN).bReleased)   fFrogY += 1.0f;
        if (GetKey(olc::Key::LEFT).bReleased)   fFrogX -= 1.0f;
        if (GetKey(olc::Key::RIGHT).bReleased)  fFrogX += 1.0f;

        // Frog is moved by platforms; velocity of lane that frog is on times elapsed time
        if (fFrogY <= 3)    fFrogX -= fElapsedTime * vecLanes[(int)fFrogY].first;

        // Collision detection
        bool tl = bufDanger[(int)(fFrogY*nCellSize + 1) * ScreenWidth() + (int)(fFrogX*nCellSize + 1)];
		bool tr = bufDanger[(int)(fFrogY*nCellSize + 1) * ScreenWidth() + (int)((fFrogX+1)*nCellSize - 1)];
		bool bl = bufDanger[(int)((fFrogY+1)*nCellSize - 1) * ScreenWidth() + (int)(fFrogX*nCellSize + 1)];
		bool br = bufDanger[(int)((fFrogY+1)*nCellSize - 1) * ScreenWidth() + (int)((fFrogX+1)*nCellSize - 1)];

		if (tl || tr || bl || br) {
			// Frogs been hit :-(
			fFrogX = 8.0f;
			fFrogY = 9.0f;
		}


        // Draw lanes
        //Clear(olc::BLACK);

        int x = -1, y = 0;  // X is offset by -1 for more smoother animation into screen 
        for (auto lane: vecLanes) { // Iterates through each lane

            // Find lane offset start
            // nStartPos: Used in determining which graphic to use. Time since start * lane velocity = graphic to be displayed at that time
            // nCellOffset: Scales and modulates the nStartPos equation so that the animation is less "blocky". When drawing graphics, offset is only applied in x direction since nothing is moving in y
            int nStartPos = (int)(fTimeSinceStart * lane.first) % 64;
            int nCellOffset = (int)((float)nCellSize * fTimeSinceStart * lane.first) % nCellSize;
            if (nStartPos < 0) nStartPos = 64 - (abs(nStartPos) % 64);  // This is used to prevent out of index error since nStartPos could be negative.

            for (int i = 0; i < nLaneWidth; i++) {
                
                // Draw graphics
                char graphic = lane.second[(nStartPos + i) % 64];

                //Fill((x + i) * nCellSize - nCellOffset, y * nCellSize, (x + i + 1)*nCellSize - nCellOffset, (y + 1) * nCellSize, graphic);
                
                switch (graphic) { // Graphics always make code verbose and less clear

                // DrawPartialSprite only draws a partial amount of the sprite. Hence, long sprites require multiple DrawPartialSprite calls -- e.g., bus takes up 24 cell units (each unit is 8 size)
                    case 'a':	DrawPartialSprite((x + i)*nCellSize - nCellOffset, y*nCellSize, spriteBus, 0, 0, 8, 8);		break; // Bus 1
                    case 's':	DrawPartialSprite((x + i)*nCellSize - nCellOffset, y*nCellSize, spriteBus, 8, 0, 8, 8);		break; // Bus 2
                    case 'd':	DrawPartialSprite((x + i)*nCellSize - nCellOffset, y*nCellSize, spriteBus, 16, 0, 8, 8);	break; // Bus 3
                    case 'f':	DrawPartialSprite((x + i)*nCellSize - nCellOffset, y*nCellSize, spriteBus, 24, 0, 8, 8);	break; // Bus 4

                    case 'j':	DrawPartialSprite((x + i)*nCellSize - nCellOffset, y*nCellSize, spriteLog, 0, 0, 8, 8);		break; // Log Start
                    case 'l':	DrawPartialSprite((x + i)*nCellSize - nCellOffset, y*nCellSize, spriteLog, 8, 0, 8, 8);		break; // Log Middle
                    case 'k':	DrawPartialSprite((x + i)*nCellSize - nCellOffset, y*nCellSize, spriteLog, 16, 0, 8, 8);	break; // Log End

                    case 'z': 	DrawPartialSprite((x + i)*nCellSize - nCellOffset, y*nCellSize, spriteCar1, 0, 0, 8, 8);	break; // Car1 Back
                    case 'x':	DrawPartialSprite((x + i)*nCellSize - nCellOffset, y*nCellSize, spriteCar1, 8, 0, 8, 8);	break; // Car1 Front

                    case 't': 	DrawPartialSprite((x + i)*nCellSize - nCellOffset, y*nCellSize, spriteCar2, 0, 0, 8, 8);	break; // Car2 Back
                    case 'y': 	DrawPartialSprite((x + i)*nCellSize - nCellOffset, y*nCellSize, spriteCar2, 8, 0, 8, 8);	break; // Car2 Front

                    case 'w': 	DrawPartialSprite((x + i)*nCellSize - nCellOffset, y*nCellSize, spriteWall, 0, 0, 8, 8);	break; // Wall
                    case 'h':	DrawPartialSprite((x + i)*nCellSize - nCellOffset, y*nCellSize, spriteHome, 0, 0, 8, 8);	break; // Home
                    case ',': 	DrawPartialSprite((x + i)*nCellSize - nCellOffset, y*nCellSize, spriteWater, 0, 0, 8, 8);	break; // Water
                    case 'p': 	DrawPartialSprite((x + i)*nCellSize - nCellOffset, y*nCellSize, spritePavement, 0, 0, 8, 8); break; // Pavement
                    case '.': 	FillRect((x + i)*nCellSize - nCellOffset, y*nCellSize, nCellSize, nCellSize, olc::BLACK);	break; // Road
                }

				//Fill Danger buffer
				for (int j = (x + i)*nCellSize - nCellOffset; j < (x + i + 1)*nCellSize - nCellOffset; j++) // Iterates through each cell in the screen
					for (int k = y*nCellSize; k < (y + 1)*nCellSize; k++)
						if (j >= 0 && j < ScreenWidth() && k >= 0 && k < ScreenHeight())
                            // If j and k are within the bounds of the screen, we need to determine if the cell is dangerous or not
							bufDanger[k*ScreenWidth() + j] =
								!(graphic == '.' ||                                     // Road
								graphic == 'j' || graphic == 'k' || graphic == 'l' ||   // Log
								graphic == 'p' ||                                       // Pavement
								graphic == 'h');                                        // Spawn point
            }
            y++;
       
        }

        // Draw Frog -- in mask mode
        SetPixelMode (olc::Pixel::MASK);
        DrawSprite(fFrogX*nCellSize, fFrogY*nCellSize, spriteFrog);
        SetPixelMode(olc::Pixel::NORMAL);

        return true;

    }
};

int main() {
    // Use olcConsoleGameEngine derived app
    OneLoneCoder_Frogger game;
    if (game.Construct(game.getScreenWidth(), game.getScreenHeight(), 12, 12))
    game.Start();
    return 0;
}