/**
 * CS-130   Fall 2013
 * Software Development Project
     *** Spaceship Game ***

 * Group Members:
 *  Daniel Hakimian
 *  Taylor Martin
 *  Mason Neal
 *  Rob Schneider
 */

#ifdef __APPLE__
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#else
#include <SDL.h>
#include <SDL_image.h>
#endif
#include <iostream>
#include <string>
#include <vector>

#include "Constants.h"
#include "Util.h"
#include "LTexture.h"
//#include "LTimer.h"
#include "Player.h"

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//The texture that everything adjacent will be rendered to before being rotated relative to the ship
LTexture gTargetTexture;

static const std::string imgarr[] = {
    "media/bg_image.gif",
    //"media/solid_bg.png",
    "media/player/SF_Ship/ship_body.png",
    "media/player/SF_Ship/jet_forward.png",
    "media/player/SF_Ship/jet_reverse.png",
    "media/player/SF_Ship/jet_leftTurn.png",
    "media/player/SF_Ship/jet_rightTurn.png",
    "media/player/SF_Ship/wings_reverse.png",
    "media/player/SF_Ship/wings_normal.png",
    "media/player/SF_Ship/wings_forward.png",
    "media/player/SF_Ship/wings_leftTurn.png",
    "media/player/SF_Ship/wings_rightTurn.png",
    "media/player/SF_Ship/ship_body_Rtilt.png",
    "media/player/SF_Ship/ship_body_Ltilt.png",
    "media/player/Health/health15.png",
    "media/player/Health/health14.png",
    "media/player/Health/health13.png",
    "media/player/Health/health12.png",
    "media/player/Health/health11.png",
    "media/player/Health/health10.png",
    "media/player/Health/health9.png",
    "media/player/Health/health8.png",
    "media/player/Health/health7.png",
    "media/player/Health/health6.png",
    "media/player/Health/health5.png",
    "media/player/Health/health4.png",
    "media/player/Health/health3.png",
    "media/player/Health/health2.png",
    "media/player/Health/health1.png",
    "media/player/Health/health0.png"
};
std::vector<std::string> images (imgarr, imgarr + sizeof(imgarr) / sizeof(imgarr[0]) );

std::vector<LTexture> textures (images.size());

std::vector<Object*> objects; // list of all the objects currently in the level
Player player;

//std::vector<ImgInstance> cur_images; // the images to be rendered this frame, with their coords and angle

bool init()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        //Enable VSync
        if( !SDL_SetHint( SDL_HINT_RENDER_VSYNC, "1" ) )
        {
            printf( "Warning: VSync not enabled!" );
        }

        //Set texture filtering to linear
        if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
        {
            printf( "Warning: Linear texture filtering not enabled!" );
        }

        //Create window
        gWindow = SDL_CreateWindow( "Spaceship Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( gWindow == NULL )
        {
            printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            //Create renderer for window
            gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
            if( gRenderer == NULL )
            {
                printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
                success = false;
            }
            else
            {
                //Initialize renderer color
                SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

                //Initialize PNG loading
                int imgFlags = IMG_INIT_PNG;
                if( !( IMG_Init( imgFlags ) & imgFlags ) )
                {
                    printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                    success = false;
                }
            }
        }
    }

    return success;
}

bool loadMedia()
{
    //Loading success flag
    bool success = true;

    for( unsigned int i=0; i<images.size(); i++ )
    {
        textures[i].setRenderer(gRenderer);
        if( !textures[i].loadFromFile( images[i] ) )
        {
            std::cout << "Failed to load '" << images[i] << "'!" << std::endl;
            success = false;
        }
    }

    //Load texture target
    gTargetTexture.setRenderer(gRenderer);
    if( !gTargetTexture.createBlank( SCREEN_WIDTH + Render_Radius, SCREEN_HEIGHT + Render_Radius, SDL_TEXTUREACCESS_TARGET ) )
    {
        printf( "Failed to create target texture!\n" );
        success = false;
    }

    objects.push_back( &player );
    return success;
}

//bool loadObjects()
//{
//}

void render()
{
    //set the image as the render target
    gTargetTexture.setAsRenderTarget();
    SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0x00 );
    SDL_RenderClear( gRenderer );

    int xScreenPos = (SCREEN_WIDTH+Render_Radius)/2 ;    //The position on the screen where the ship resides..
    int yScreenPos = (SCREEN_HEIGHT+Render_Radius)/2;  //..as the world moves relative to it
    yScreenPos += SCREEN_HEIGHT/4;

    float xPos_pl, yPos_pl, Angle_pl; // pl -> player
    player.get_values(&xPos_pl, &yPos_pl, &Angle_pl);
    
    //render the background tiles that are within a certain radius of the player, and also...
    //...render tiles on the other side of a wrap to replace the void
    int tile_w = textures[BACKGROUND].getWidth();
    int tile_h = textures[BACKGROUND].getHeight();
    for( int i=0; i<LEVEL_WIDTH; i+= tile_w )
    {
        for( int j=0; j<LEVEL_HEIGHT; j+=tile_h )
        {
            bool render = false;
            int xrc = i+xScreenPos-xPos_pl; // X render coordinate
            int yrc = j+yScreenPos-yPos_pl; // Y render coordinate

            int close_enough_x = tile_w;  // how close a tile must be to be rendered
            int close_enough_y = tile_h; // subject to change

            if( abs( i + tile_w/2 - xPos_pl ) < close_enough_x &&
                abs( j + tile_h/2 - yPos_pl ) < close_enough_y )
            {
                textures[BACKGROUND].render( i+xScreenPos-xPos_pl, j+yScreenPos-yPos_pl);
            }
            if( abs( i + LEVEL_WIDTH - xPos_pl ) < close_enough_x ) //r
                xrc += LEVEL_WIDTH; render = true;
            if( abs( j + LEVEL_HEIGHT - yPos_pl ) < close_enough_y ) //b
                yrc += LEVEL_HEIGHT; render = true;
            if( abs( i + tile_w - LEVEL_WIDTH - xPos_pl ) < close_enough_x ) //l
                xrc -= LEVEL_WIDTH; render = true;
            if( abs( j + tile_h - LEVEL_HEIGHT - yPos_pl ) < close_enough_y ) //t
                yrc -= LEVEL_HEIGHT; render = true;

            if( render )
                textures[BACKGROUND].render( xrc, yrc );
        }
    }

    SDL_Point center;
    center.x = xScreenPos;
    center.y = yScreenPos;
/*
    for( unsigned int i=0; i<objects.size(); i++ )
    {
        float xPos, yPos, Angle; 
        objects[i]->get_values(&xPos, &yPos, &Angle);

        //render if within a certain radius of the player
        if( abs(xPos - xPos_pl) < Render_Radius && abs(yPos - yPos_pl) < Render_Radius )
        {
        //render it
        }else{
        //do nothing
        }
*/
        xScreenPos -= textures[PLAYER].getWidth()/2;  //shift the coords so that the center of the image..
        yScreenPos -= textures[PLAYER].getHeight()/2; //..(not the top left corner of the image) will be...
        //                                            //..on the point of the ship's position

        textures[PLAYER].render( xScreenPos, yScreenPos, NULL, Angle_pl );

        const Uint8* currentKeyStates = SDL_GetKeyboardState( NULL );

    // here is my(robs) added code that implements a cool ship that has some moving parts.

    //local variables resembling if movement keys are pushed. Includes both wasd and updownleftright.
    //makes less to type in each if statement
    bool upKey = currentKeyStates[SDL_SCANCODE_UP] || currentKeyStates[SDL_SCANCODE_W];
    bool downKey = currentKeyStates[SDL_SCANCODE_DOWN] || currentKeyStates[SDL_SCANCODE_S];
    bool leftKey = currentKeyStates[SDL_SCANCODE_LEFT] || currentKeyStates[SDL_SCANCODE_A];
    bool rightKey = currentKeyStates[SDL_SCANCODE_RIGHT] || currentKeyStates[SDL_SCANCODE_D];
    bool strafeLeft = currentKeyStates[SDL_SCANCODE_Q];
    bool strafeRight = currentKeyStates[SDL_SCANCODE_E];

    //renders the thruster images according to which button you pushed. works for both wasd and up/down/left/rt keys 
    //
    if(upKey)
        textures[PLAYER_THR_B].render( xScreenPos, yScreenPos, NULL, Angle_pl );
    if(leftKey && !downKey)
        textures[PLAYER_THR_L].render( xScreenPos, yScreenPos, NULL, Angle_pl );
    if(rightKey && !downKey)
        textures[PLAYER_THR_R].render( xScreenPos, yScreenPos, NULL, Angle_pl );
    if(downKey)
        textures[PLAYER_THR_F].render( xScreenPos, yScreenPos, NULL, Angle_pl );
    if(rightKey && downKey)
        textures[PLAYER_THR_R].render( xScreenPos, yScreenPos, NULL, Angle_pl );
    if(leftKey && downKey)
        textures[PLAYER_THR_L].render( xScreenPos, yScreenPos, NULL, Angle_pl );

    //these conditionals draw different wing orentations depending on which direction the ship is turning.
    if(downKey && !upKey) {
        textures[PLAYER].render( xScreenPos, yScreenPos, NULL, Angle_pl);
        textures[PLAYER_WNG_B].render(xScreenPos, yScreenPos, NULL, Angle_pl);
    } else if(leftKey && !rightKey && !downKey) {
        textures[PLAYER_Tlt_L].render( xScreenPos, yScreenPos, NULL, Angle_pl);
        textures[PLAYER_WNG_L].render( xScreenPos, yScreenPos, NULL, Angle_pl);
    } else if(rightKey && !leftKey && !downKey) {
        textures[PLAYER_Tlt_R].render( xScreenPos, yScreenPos, NULL, Angle_pl);
        textures[PLAYER_WNG_R].render( xScreenPos, yScreenPos, NULL, Angle_pl);
    } else if(upKey && !downKey && !leftKey && !rightKey && !strafeLeft && !strafeRight) {
        textures[PLAYER].render( xScreenPos, yScreenPos, NULL, Angle_pl);
        textures[PLAYER_WNG_NORM].render( xScreenPos, yScreenPos, NULL, Angle_pl);
    } else if(downKey && leftKey && rightKey) {
        textures[PLAYER].render( xScreenPos, yScreenPos, NULL, Angle_pl);
        textures[PLAYER_WNG_B].render( xScreenPos, yScreenPos, NULL, Angle_pl);
    } else if(leftKey && rightKey && downKey) {
        textures[PLAYER].render( xScreenPos, yScreenPos, NULL, Angle_pl);
        textures[PLAYER_WNG_B].render( xScreenPos, yScreenPos, NULL, Angle_pl);
    } else if((downKey && upKey) || (leftKey && rightKey)) {
        textures[PLAYER].render( xScreenPos, yScreenPos, NULL, Angle_pl);
        textures[PLAYER_WNG_NORM].render( xScreenPos, yScreenPos, NULL, Angle_pl);
    } else if(strafeRight && !strafeLeft) {
        textures[PLAYER_Tlt_R].render( xScreenPos, yScreenPos, NULL, Angle_pl);
        textures[PLAYER_WNG_NORM].render( xScreenPos, yScreenPos, NULL, Angle_pl);
    } else if(strafeLeft && !strafeRight) {
        textures[PLAYER_Tlt_L].render( xScreenPos, yScreenPos, NULL, Angle_pl);
        textures[PLAYER_WNG_NORM].render( xScreenPos, yScreenPos, NULL, Angle_pl);
    } else if(upKey && !downKey && !leftKey && !rightKey && strafeLeft && strafeRight) {
        textures[PLAYER].render( xScreenPos, yScreenPos, NULL, Angle_pl);
        textures[PLAYER_WNG_NORM].render( xScreenPos, yScreenPos, NULL, Angle_pl);
    } else if(!downKey && !upKey && !leftKey && !rightKey) {
        textures[PLAYER].render( xScreenPos, yScreenPos, NULL, Angle_pl);
        textures[PLAYER_WNG_NORM].render( xScreenPos, yScreenPos, NULL, Angle_pl);
    }

    //Reset render target to the window
    SDL_SetRenderTarget( gRenderer, NULL );

    gTargetTexture.render( -(Render_Radius/2), -(Render_Radius/2), NULL, -Angle_pl, &center );

    //anything rendered past here will be overlayed on top of the screen
    //everything before gets rotated with the ship

    //this code is the health bar
    float xp, yp;
    xp = (SCREEN_WIDTH / 2) - 151;
    yp = 1;
    //code for drawing the right ammount of health increments depending on palyers health
    int player_health = player.hitpoints;
    if(player_health > 93.5)
        textures[HEALTH_15].render(xp, yp);
    else if(player_health > 87)
        textures[HEALTH_14].render(xp, yp);
    else if(player_health > 80.5)
        textures[HEALTH_13].render(xp, yp);
    else if(player_health > 74)
        textures[HEALTH_12].render(xp, yp);
    else if(player_health > 67.5)
        textures[HEALTH_11].render(xp, yp);
    else if(player_health > 61)
        textures[HEALTH_10].render(xp, yp);
    else if(player_health > 54.5)
        textures[HEALTH_9].render(xp, yp);
    else if(player_health > 48)
        textures[HEALTH_8].render(xp, yp);
    else if(player_health > 41.5)
        textures[HEALTH_7].render(xp, yp);
    else if(player_health > 35)
        textures[HEALTH_6].render(xp, yp);
    else if(player_health > 28.5)
        textures[HEALTH_5].render(xp, yp);
    else if(player_health > 22)
        textures[HEALTH_4].render(xp, yp);
    else if(player_health > 15.5)
        textures[HEALTH_3].render(xp, yp);
    else if(player_health > 9)
        textures[HEALTH_2].render(xp, yp);
    else if(player_health > 2.5)
        textures[HEALTH_1].render(xp, yp);
    else
        textures[HEALTH_0].render(xp, yp);

    //test code to make sure hit fuction in the ship class is working. it lowers the players health if you press the K key     
    if(currentKeyStates[SDL_SCANCODE_K])
    {
        player.hitpoints -= 1;
    }

}

//handle actions based on current key state
void handle_keystate()
{
    const Uint8* currentKeyStates = SDL_GetKeyboardState( NULL );

    bool upKey = currentKeyStates[SDL_SCANCODE_UP] || currentKeyStates[SDL_SCANCODE_W];
    bool downKey = currentKeyStates[SDL_SCANCODE_DOWN] || currentKeyStates[SDL_SCANCODE_S];
    bool leftKey = currentKeyStates[SDL_SCANCODE_LEFT] || currentKeyStates[SDL_SCANCODE_A];
    bool rightKey = currentKeyStates[SDL_SCANCODE_RIGHT] || currentKeyStates[SDL_SCANCODE_D];
    bool strafeLeft = currentKeyStates[SDL_SCANCODE_Q];
    bool strafeRight = currentKeyStates[SDL_SCANCODE_E];

    if(upKey)
        player.thrust_b();
    if(downKey)
        player.thrust_f();
    if(leftKey)
        player.rot_l();
    if(rightKey)
        player.rot_r();
    if(strafeLeft)
        player.thrust_l();
    if(strafeRight)
        player.thrust_r();

}


void close()
{
    //Free loaded images
    for( unsigned int i=0; i<textures.size(); i++ )
    {
        textures[i].free();
    }

    //Destroy window    
    SDL_DestroyRenderer( gRenderer );
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;
    gRenderer = NULL;

    //Quit SDL subsystems
    IMG_Quit();
    SDL_Quit();
}


int main( int argc, char* args[] )
{
    //Start up SDL and create window
    if( !init() )
    {
        printf( "Failed to initialize!\n" );
    }
    else
    {
        //Load media
        if( !loadMedia() )
        {
            printf( "Failed to load media!\n" );
        }
        else
        {
            //Main loop flag
            bool quit = false;

            //Event handler
            SDL_Event e;


            //While application is running
            while( !quit )
            {
                //Handle events on queue
                while( SDL_PollEvent( &e ) != 0 )
                {
                    //User requests quit
                    if( e.type == SDL_QUIT )
                    {
                        quit = true;
                    }
                    if( e.type == SDL_KEYDOWN )
                    {
                        switch( e.key.keysym.sym ) {
                            case SDLK_f:
                                toggle_fullscreen(gWindow);
                                break;
                            case SDLK_ESCAPE:
                                quit = true;
                                break;
                            case SDLK_c:
                                float xPos, yPos, Angle; 
                                player.get_values(&xPos, &yPos, &Angle);
                                std::cout << "x: " << xPos << " | y: " << yPos << std::endl;
                                break;
                        }
                    }
                }

                //handle actions based on current key state
                handle_keystate();

                //Move the ship
                player.update();

                //Clear screen
                //SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
                SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0x00 );
                SDL_RenderClear( gRenderer );

                //Render objects
                render();
                //render( objects );

                //Update screen
                SDL_RenderPresent( gRenderer );
            }
        }
    }

    //Free resources and close SDL
    close();

    return 0;
}
