-------------------------------------------------------------------------------
COMP 4300 - Assignment 3
-------------------------------------------------------------------------------
Assignment Notes:
-------------------------------------------------------------------------------

Submission:

For this assignment you will zip and submit the entire 'src' folder of the
assignment. You are *NOT* allowed to add any files in addition to the ones
already in the src folder from the assignment zip file. This means that you
will not be submitting ANY changes you have made to the visual studio project,
if you used it for development.

YOU MUST INCLUDE THE FOLLOWING INFO IN A COMMENT AT THE TOP OF main.cpp:
- ALL students' full names, cs usernames, and student numbers
- If you did not get a specific feature to work, please explain what you tried 
  to do to get it to work, and partial marks may be given for effort.
 

-------------------------------------------------------------------------------
Program Specification
-------------------------------------------------------------------------------

IMPORTANT NOTE: In this assignment, treat (0,0) as the bottom-left corner of
    the display. Unlike previous assignments where (0,0) was the top-left.

In this assignment you will be writing the game that was presented in class.
This game must have the following features:

Assets:
- Entities in the game will be rendered using various Textures and Animations
  which we will be calling Assets (along with Fonts)
- Assets are loaded once at the beginning of the program and stored in the
  Assets class, which is stored by the GameEngine class
- All Assets are defined in assets.txt, with the syntax defined below

Player:
- The player Entity in the game is represented by Megaman, which has several
  different Animations: Stand, Run, and Air. You must determine which state
  the player is currently in and assign the correct Animation.
- The player moves with the following controls:
  Left: A key, Right: D key, Jump: W Key, Shoot: Space Key
- The player can move left, move right, or shoot at any time during the game
- The player can only jump if it is currently standing on a tile  
- Bullets shot by the player travel in the direction the player is facing
- The player collides with 'Tile' entities in the level (see level syntax) and
  cannot move through them. The player land on a Tile entity and stand in place
  if it falls on it from above.
- The player does not collide with 'Dec' (decoration) entities in the level
- If the player falls below a y position of 0, they respawn at the start
- If the player touches a tile with the Pole animation, respawn at start 
- The player should have a Gravity component which constantly accelerates it
  downward on the screen until it collides with a tile
- The player has a maximum speed specified in the Level file (see below) which
  it should not exceed in either x or y direction
- The player will be given a CBoundingBox of a size specified in the level file

Decoration Entities:
- Decoration entities ('Dec' in a level file) are simply drawn to the screen,
  and do not interact with any other entities in the game in any way
- Decorations can be given any Animation in the game, but intuitively they
  should be reserved for things like clouds, bushes, etc

Tiles:
- Tiles are Entities that define the level geometry and interact with players
- Tiles can be given any Animation that is defined in the Assets file
- Tiles will be given a CBoundingBox equal to the size of the animation
  tile->getComponent<CAnimation>()->animation.getSize()
- The current animation displayed for a tile can be retreieved with:
  tile->getComponent<CAnimation>()->animation.getName()
- Tiles have different behavior depending on which Animation they are given

  Brick Tiles:
  - Brick tiles are given the 'Brick' Animation
  - When a brick tile collides with a bullet, or is hit by a player from below:
    - Its animation should change to 'Explosion' (non-repeat)
    - Its CBoundingBox component should be removed

  Question Tiles:
  - Question tiles are given the 'Question' Animation when created
  - When a Question tile is hit by a player from below, 2 things happen:
    - Its Animation changes to the darker 'Question2' animation
    - A temporary lifespan entity with the 'Coin' animation should appear
      for 500ms 64 pixels above the location of the Question entity

Drawing:
- Entity rendering has been implemented for you, no need to change that system
  
Misc:
- The 'P' key should pause the game
- Pressing the 'R' key toggles drawing textures
- Pressing the 'F' key toggles drawing bounding boxes of entities
  This should be very helpful for debugging
- The 'ESC' key should go 'back' to the Main Menu, or quit if on the Main Menu

-------------------------------------------------------------------------------
Config Files
-------------------------------------------------------------------------------

There will be two configuration files in this assignment. The Assets config
file, and the Level configuration file.

-------------------------------------------------------------------------------
Assets File Specification
-------------------------------------------------------------------------------

There will be three different line types in the Assets file, each of which
correspond to a different type of Asset. They are as follows:

Texture Asset Specification:
Texture N P
  Texture Name     N        std::string (it will have no spaces)
  Texture FilePath P        std::string (it will have no spaces)

Animation Asset Specification:
Animation N T F S
  Animation Name   N        std::string (it will have no spaces)
  Texture Name     T        std::string (refers to an existing texture)
  Frame Count      F        int (number of frames in the Animation)
  Anim Speed       S        int (number of game frames between anim frames)
  
Font Asset Specification:
Font N P
  Font Name        N        std::string (it will have no spaces)
  Font File Path   P        std::string (it will have no spaces)
  
-------------------------------------------------------------------------------
Level Specification File
-------------------------------------------------------------------------------
  
Game levels will be specified by a Level file, which will contain a list of
entity specifications, one per line. It will also contain a single line which
specifies properties of the player in that level. In this way, you can define
an entire level in the data file, rather than in programming code. The syntax
of the lines of the Level file are as follows:
  
Tile Entity Specification:
Tile N X Y
  Animation Name   N        std::string (Animation asset name for this tile)
  X Position       X        float
  Y Position       Y        float
  
Decoration Entity Specification:
Dec N X Y
  Animation Name   N        std::string (Animation asset name for this tile)
  X Position       X        float
  Y Position       Y        float
  
Player Specification
Player X Y CW CH SX SY SM GY B
  X, Y Position    X, Y     float, float
  BoundingBox W/H  CW, CH   float, float
  Left/Right Speed SX       float
  Jump Speed       SY       float
  Max Speed        SM       float --Bullet speed 3x
  Gravity          GY       float
  Bullet Animation B        std::string (Animation asset to use for bullets)
  
-------------------------------------------------------------------------------
Assignment Hints
-------------------------------------------------------------------------------

I recommend approaching this assignment in the following order:

Note: Implementing sAnimation() can be done at any time, since it does not
      interfere in any way with the rest of the game working properly

1. Implement WASD movement for the player such that it moves u/l/d/r when
   those keys are pressed. This will allow you to test rectangular collisions
   as soon as possible without worrying about implementing gravity just yet
2. Implement GameState_Play::loadLevel(), which will allow you to load the
   assets for the level given to you to test the rest of the assignment
3. Implement spawnPlayer() so that it correctly uses the configuration
   specified by the level file so you can start testing collisions
4. Implement spawnBullet() so that it shoots when Space is pressed
5. Implement Physics::GetOverlap() which will allow you to start doing
   collision checking of rectangular bounding boxes
6. Implement collision checking with bullets / brick tiles such that the brick
   is destroyed when a bullet collides with it. Remember, a collision occurs
   when the overlap is non-zero in both the X and Y component. Bullets should
   always be destroyed when they collide with any non-decorative tile
6. Implement collision resolution such that when the player collides with a
   non-decorative tile, the player cannot enter it over overlap it
7. Implement a way of detecting which side the player collided with the tile
8. Change the controls such that they are the proper left/right/jump style
9. Implement gravity such that the player falls toward the bottom of the screen
   and lands on tiles when it collides with a tile from above

At this stage, all remaining functionality can be done in any order
