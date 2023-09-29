#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <math.h>
#include <time.h>

#include "../headers/structs.h"
#include "../headers/functions.h"
#include "../headers/globals.h"

int main(void){
    InitWindow(screenWidth,screenHeight,"Rocketman Adventures");
    
    u_int8_t display=GetCurrentMonitor();
    SetWindowSize(GetMonitorWidth(display),GetMonitorHeight(display));
    ToggleFullscreen();
    
    SetTargetFPS(60);
    float dt=1.f;

    u_int8_t gameState=0; //0 - not started, 1 - in progress, 2 - game over

    //load and resize images
    Images.redSoldier=LoadImage(pathToFile("red_soldier.png"));
    ImageResizeNN(&Images.redSoldier,12*5,20*5);

    Images.rocket=LoadImage(pathToFile("rocket.png"));
    ImageResizeNN(&Images.rocket,30*3,8*3);

    Images.launcher=LoadImage(pathToFile("launcher.png"));
    ImageResizeNN(&Images.launcher,20*5,8*5);

    Images.particleSmoke=LoadImage(pathToFile("particle_smoke.png"));
    ImageResizeNN(&Images.particleSmoke,12*10,12*10);

    Images.backgrounds[0]=LoadImage(pathToFile("background0.png"));
    ImageResizeNN(&Images.backgrounds[0],screenWidth,screenHeight);

    Images.backgrounds[1]=LoadImage(pathToFile("background1.png"));
    ImageResizeNN(&Images.backgrounds[1],screenWidth,screenHeight);

    Images.platform=LoadImage(pathToFile("platform.png"));
    ImageResizeNN(&Images.platform,30*5,2*5);

    //player
    Soldier redSoldier={
        .tx=LoadTextureFromImage(Images.redSoldier),
        .speedX=0,
        .speedY=0,
        .cooldown=0,
        .falling=0
    };
    redSoldier.x=(int)(screenWidth/2)-redSoldier.tx.width;
    redSoldier.y=screenHeight-redSoldier.tx.height;
    UnloadImage(Images.redSoldier);

    //rocket launcher
    Launcher rl={
        .tx=LoadTextureFromImage(Images.launcher),
        .x=0,
        .y=0,
        .rotation=0
    };
    UnloadImage(Images.launcher);

    //background
    Texture backgrounds[2];
    short bgY[2]; 
    for(u_int8_t i=0; i<2; i++){
        backgrounds[i]=LoadTextureFromImage(Images.backgrounds[i]);
        UnloadImage(Images.backgrounds[i]);
        bgY[i]=-i*screenHeight;
    }

    srand(time(NULL));

    u_int8_t numRockets=0;
    Rocket* rockets=malloc(numRockets*sizeof(Rocket));

    u_int8_t numParticles=0;
    Particle* particles=malloc(numParticles*sizeof(Particle));

    u_int8_t numPlatforms=5;
    Platform platforms[numPlatforms];
    for(u_int8_t i=0; i<numPlatforms; i++){
        Platform newPlatform={
            .tx=LoadTextureFromImage(Images.platform),
            .x=rand()%((1280-150-300)-300+1)+300,
            .y=screenHeight-(i+1)*200
        };

        platforms[i]=newPlatform;
    }
    UnloadImage(Images.platform);
    short *bgShift=NULL;

    //game loop
    while(!WindowShouldClose()){
        dt=GetFrameTime();

        for(u_int8_t i=0; i<numRockets; i++){
            rocketBorderCheck(&rockets[i]);

            if(rockets[i].collided){
                UnloadTexture(rockets[i].tx);

                //smoke particles
                if(rockets[i].y+rockets[i].tx.height>=screenHeight){
                    for(u_int8_t j=0; j<3; j++){
                        numParticles++;
                        Particle *buffer=malloc(sizeof(Particle)*numParticles);
                    
                        Particle newParticle={
                            .tx=LoadTextureFromImage(Images.particleSmoke),
                            .x=rockets[i].x+(int)(rockets[i].tx.width/2)+rand()%(50-(-50)+1)-50,
                            .y=rockets[i].y+(int)(rockets[i].tx.width/2)+rand()%(50-(-50)+1)-50,
                            .rotation=rand()%361,
                            .alpha=255,
                            .cooldownAlpha=0
                        };

                        for(u_int8_t i=0; i<numParticles-1; i++){
                            buffer[i]=particles[i];
                        }

                        buffer[numParticles-1]=newParticle;
                        particles=buffer;
                    }

                    //rocket jump
                    if(abs((int)(redSoldier.x+(int)(redSoldier.tx.width/2)-rockets[i].x-(int)(rockets[i].tx.width/2)))<100 
                    && abs((int)(redSoldier.y+(int)(redSoldier.tx.height/2)-rockets[i].y-(int)(rockets[i].tx.height/2)))<100){
                        redSoldier.speedX=-rockets[i].speedX;
                        redSoldier.speedY=-rockets[i].speedY; 
                    } 
                }
                

                //delete rockets
                numRockets--; 

                //shift elements in array
                for(u_int8_t j=i; j<numRockets; j++){
                    rockets[j]=rockets[j+1];
                }
               
                //TEMPORARY SOLUTION 
                Rocket* buffer=malloc(sizeof(Rocket)*numRockets);
                for(u_int8_t j=0; j<numRockets; j++){
                    buffer[j]=rockets[j];
                }
                rockets=buffer;

                break;
            }
        }

        //delete particles
        for(u_int8_t i=0; i<numParticles; i++){
            if(particles[i].alpha<5){
                numParticles--;

                //shift elements in array
                for(u_int8_t j=i; j<numParticles; j++){
                    particles[j]=particles[j+1];
                }

                Particle* buffer=malloc(sizeof(Particle)*numParticles);
                for(u_int8_t j=0; j<numParticles; j++){
                    buffer[j]=particles[j];
                }
                particles=buffer;

                break;
            }
        }

        //movement
        if(IsKeyDown(KEY_D)){
            redSoldier.x+=150*dt;
        }
        if(IsKeyDown(KEY_A)){
            redSoldier.x-=150*dt;
        } 
        if(IsKeyDown(KEY_SPACE) && !redSoldier.falling){
            redSoldier.falling=0;
            redSoldier.speedY=-300;
        } 

        //update player position
        redSoldier.x+=redSoldier.speedX*dt;
        if(redSoldier.speedY>0){
            if(redSoldier.falling){
                redSoldier.y+=redSoldier.speedY*dt; 
            }
        }
        else{
            redSoldier.y+=redSoldier.speedY*dt; 
        } 

        if(redSoldier.y<(int)(screenHeight/2)-(int)(redSoldier.tx.height/2)){
            redSoldier.y=(int)(screenHeight/2)-(int)(redSoldier.tx.height/2); 
            
            if(gameState==0){
                gameState=1;
            }
        }

        //gravity
        if(redSoldier.y+redSoldier.tx.height>=screenHeight){
            if(gameState!=1){
                redSoldier.y=screenHeight-redSoldier.tx.height;
                redSoldier.speedY=0;
                redSoldier.falling=0;
            }

            if(gameState==1){
                gameState=2;
            }
        }
        else{
            redSoldier.falling=1;
            redSoldier.speedY+=15;
        }

        //input
        if((IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_R)) && redSoldier.cooldown<0){
            redSoldier.cooldown=120;
            numRockets++;

            Rocket *buffer=malloc(sizeof(Rocket)*numRockets);

            Rocket newRocket={
                .tx=LoadTextureFromImage(Images.rocket),
                .x=redSoldier.x+(int)(redSoldier.tx.width/2),
                .y=redSoldier.y+(int)(redSoldier.tx.height/2),
                .rotation=90-atan2((redSoldier.x+(int)(redSoldier.tx.width/2)-GetMouseX()),(redSoldier.y+(int)(redSoldier.tx.height/2)-GetMouseY()))*180/PI,
                .collided=0
            };
            
            newRocket.speedX=-cos(newRocket.rotation*PI/180)*800;
            newRocket.speedY=-sin(newRocket.rotation*PI/180)*800;
            
            for(u_int8_t i=0; i<numRockets-1; i++){
                buffer[i]=rockets[i];
            }

            buffer[numRockets-1]=newRocket;
            rockets=buffer;
        }
       
        //horizontal friction
        if(redSoldier.speedX>0){
            if(redSoldier.speedX<5){
                redSoldier.speedX=0;
            }
            else{
                redSoldier.speedX-=8;
            }
        }
        else if(redSoldier.speedX<0){
            if(redSoldier.speedX>-5){
                redSoldier.speedX=0;
            }
            else{
                redSoldier.speedX+=8;
            }
        }

        soldierBorderCheck(&redSoldier);

        //update rocket launcher
        rl.x=redSoldier.x+(int)(redSoldier.tx.width/2);
        rl.y=redSoldier.y+(int)(redSoldier.tx.height/2); 
        rl.rotation=270-atan2((redSoldier.x+(int)(redSoldier.tx.width/2)-GetMouseX()),(redSoldier.y+(int)(redSoldier.tx.height/2)-GetMouseY()))*180/PI; 

        //update cooldowns
        redSoldier.cooldown-=150*GetFrameTime();

        //update rockets
        for(u_int8_t i=0; i<numRockets; i++){
            //position
            rockets[i].x+=rockets[i].speedX*dt;
            rockets[i].y+=rockets[i].speedY*dt;
        }  

        ClearBackground(BLACK); 
        BeginDrawing();

        //update background
        bgShift = malloc(sizeof(short)); //i have absolutely no idea why this works, but it does and therefore should not be touched
        *bgShift=redSoldier.speedY*dt/2;
        for(u_int8_t i=0; i<2; i++){
            //parallax scrolling
            if(bgY[i]>screenHeight){
                bgY[i]=-screenHeight;
                bgY[1-i]=0;
            } 
            if(redSoldier.y==(int)(screenHeight/2)-(int)(redSoldier.tx.height/2)){
                bgY[i]-=*bgShift;
            }

            //draw background
            DrawTexture(backgrounds[i],0,bgY[i],WHITE);
        }
        free(bgShift);

        //update platforms
        for(u_int8_t i=0; i<numPlatforms; i++){
            //soldier collisions
            if(redSoldier.speedY>0){
                platformCollisionCheckS(&platforms[i],&redSoldier);
            }

            if(redSoldier.y==(int)(screenHeight/2)-(int)(redSoldier.tx.height/2)){
                platforms[i].y-=redSoldier.speedY*dt;
            }

            //rocket collisions
            for(u_int8_t j=0; j<numRockets; j++){
                platformCollisionCheckR(&platforms[i],&rockets[j]);
            }

            if(platforms[i].y>screenHeight){
                platforms[i].x=rand()%((1280-150-300)-300+1)+300;
                platforms[i].y=-platforms[i].tx.height;
            }

            //draw platforms
            DrawTexture(platforms[i].tx,platforms[i].x,platforms[i].y,WHITE);
        }

        //draw player
        DrawTexture(redSoldier.tx,redSoldier.x,redSoldier.y,WHITE);

        //draw rockets
        for(u_int8_t i=0; i<numRockets; i++){
            DrawTexturePro(
                rockets[i].tx,
                (Rectangle){ //src
                    .x=0,
                    .y=0,
                    .width=rockets[i].tx.width,
                    .height=rockets[i].tx.height
                },
                (Rectangle){ //dest
                    .x=rockets[i].x,
                    .y=rockets[i].y,
                    .width=rockets[i].tx.width,
                    .height=rockets[i].tx.height
                },
                (Vector2){ //origin
                    .x=(int)(rockets[i].tx.width/2),
                    .y=(int)(rockets[i].tx.height/2)
                },
                rockets[i].rotation,
                WHITE
            );
        }
    
        if(GetMouseX()<redSoldier.x+(int)(redSoldier.tx.width/2)){
            rl.flip=-1;
        }
        else{
            rl.flip=1;
        }

        //draw rocket launcher
        DrawTexturePro(
            rl.tx,
            (Rectangle){ //src
                .x=0,
                .y=0,
                .width=rl.tx.width,
                .height=rl.flip*rl.tx.height
            },
            (Rectangle){ //dest
                .x=rl.x,
                .y=rl.y,
                .width=rl.tx.width,
                .height=rl.tx.height
            },
            (Vector2){ //origin
                .x=(int)(rl.tx.width/2),
                .y=(int)(rl.tx.height/2)
            },
            rl.rotation,
            WHITE
        ); 

        //update particles
        for(u_int8_t i=0; i<numParticles; i++){
            if(redSoldier.y==(int)(screenHeight/2)-(int)(redSoldier.tx.height/2)){
                particles[i].y-=redSoldier.speedY*dt;
            }  

            //cooldown
            particles[i].cooldownAlpha-=1000*GetFrameTime();
            if(particles[i].cooldownAlpha<0){
                particles[i].cooldownAlpha=20;
                particles[i].alpha-=2;
            }
            
            //draw
            DrawTexturePro(
                particles[i].tx,
                (Rectangle){ //src
                    .x=0,
                    .y=0,
                    .width=particles[i].tx.width,
                    .height=particles[i].tx.height
                },
                (Rectangle){ //dest
                    .x=particles[i].x,
                    .y=particles[i].y,
                    .width=particles[i].tx.width,
                    .height=particles[i].tx.height
                },
                (Vector2){ //origin
                    .x=(int)(particles[i].tx.width/2),
                    .y=(int)(particles[i].tx.height/2)
                },
                particles[i].rotation, //rotataion
                (Color){
                    255,
                    255,
                    255,
                    particles[i].alpha
                }
            );
        } 
    
        //text
        switch(gameState){
            case 0: //game not started
                DrawText( //TODO: TURN THIS INTO A FUNCTION DRAWTEXTCENTER
                    "ROCKETMAN ADVENTURES", 
                    (int)(screenWidth/2)-(int)(MeasureTextEx(GetFontDefault(), "ROCKETMAN ADVENTURES", 100, 10).x/2),
                    100,
                    100,
                    BLACK
                );

                DrawText(
                    "START JUMPING TO BEGIN", 
                    (int)(screenWidth/2)-(int)(MeasureTextEx(GetFontDefault(), "START JUMPING TO BEGIN", 64, 10).x/2),
                    200,
                    64,
                    BLACK
                ); 
                break;
            case 1: //game in progress
                DrawText("SCORE: 0", 10, 10, 64, BLACK);
                break;
            case 2: //game over
                DrawText(
                    "GAME OVER", 
                    (int)(screenWidth/2)-(int)(MeasureTextEx(GetFontDefault(), "GAME OVER", 100, 10).x/2),
                    (int)(screenHeight/2)-(int)(MeasureTextEx(GetFontDefault(), "GAME OVER", 100, 10).y/2),
                    100,
                    BLACK
                );  
                break;
            default:
                DrawText("ERROR", 100, 100, 120, BLACK);
        } 

        EndDrawing();
    }
   
    //unload images
    UnloadImage(Images.rocket);
    UnloadImage(Images.particleSmoke);

    //unload textures
    UnloadTexture(redSoldier.tx); 
    for(u_int8_t i=0; i<numRockets; i++){
        UnloadTexture(rockets[i].tx); 
    } 
    for(u_int8_t i=0; i<numParticles; i++){
        UnloadTexture(particles[i].tx);
    }
    for(u_int8_t i=0; i<numPlatforms; i++){
        UnloadTexture(platforms[i].tx);
    }

    CloseWindow();

    return 0;
}
