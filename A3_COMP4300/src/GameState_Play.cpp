#include "GameState_Play.h"
#include "Common.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include <fstream>

GameState_Play::GameState_Play(GameEngine & game, const std::string & levelPath)
    : GameState(game)
    , m_levelPath(levelPath)
{
    init(m_levelPath);
}

void GameState_Play::init(const std::string & levelPath)
{
    loadLevel(levelPath);

    spawnPlayer();

    // some sample entities
    //auto brick = m_entityManager.addEntity("tile");
    //brick->addComponent<CTransform>(Vec2(100, 400));
    //brick->addComponent<CAnimation>(m_game.getAssets().getAnimation("Brick"), true);

    //if (brick->getComponent<CAnimation>()->animation.getName() == "Brick")
    //{
    //    std::cout << "This could be a good way of identifying if a tile is a brick!\n";
    //}

    //auto block = m_entityManager.addEntity("tile");
    //block->addComponent<CTransform>(Vec2(200, 400));
    //block->addComponent<CAnimation>(m_game.getAssets().getAnimation("Block"), true);
    // add a bounding box, this will now show up if we press the 'F' key
    //block->addComponent<CBoundingBox>(m_game.getAssets().getAnimation("Block").getSize());

    //auto question = m_entityManager.addEntity("tile");
    //question->addComponent<CTransform>(Vec2(300, 400));
    //question->addComponent<CAnimation>(m_game.getAssets().getAnimation("Question"), true);
}

void GameState_Play::loadLevel(const std::string & filename)
{
    // reset the entity manager every time we load a level
    m_entityManager = EntityManager();
    
	std::ifstream level;
	std::string input, anim;
	int x,y;
	
	level.open(filename, std::ifstream::in);
	while(!level.eof())
	{
		level >> input;
		if (input == "Tile" || input == "Dec")
		{
			level >> anim >> x >> y;
			auto block = m_entityManager.addEntity("tile");
			block->addComponent<CTransform>(Vec2(x, y));
			block->addComponent<CAnimation>(m_game.getAssets().getAnimation(anim), true);
			if (input == "Tile") 
			{
    			block->addComponent<CBoundingBox>(m_game.getAssets().getAnimation(anim).getSize());
			}
		}
		if (input == "Player")
		{
			level >> m_playerConfig.X >> m_playerConfig.Y >> m_playerConfig.CX >> m_playerConfig.CY >> 
				m_playerConfig.SPEED >> m_playerConfig.JUMP >> m_playerConfig.MAXSPEED >> m_playerConfig.GRAVITY >> m_playerConfig.WEAPON;
		}
	}
}

void GameState_Play::spawnPlayer()
{
    // here is a sample player entity which you can use to construct other entities
    m_player = m_entityManager.addEntity("player");
    m_player->addComponent<CTransform>(Vec2(m_playerConfig.X, m_playerConfig.Y));
    m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CX, m_playerConfig.CY));
    m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("Stand"), true);
	m_player->addComponent<CInput>();
        
    // TODO: be sure to add the remaining components to the player
}

void GameState_Play::spawnBullet(std::shared_ptr<Entity> entity)
{
    // TODO: this should spawn a bullet at the given entity, going in the direction the entity is facing
	auto bullet = m_entityManager.addEntity("bullet");
	auto entityTrans = entity->getComponent<CTransform>();
    bullet->addComponent<CTransform>(entityTrans->pos);
	auto bulletTrans = bullet->getComponent<CTransform>();
	bulletTrans->speed.x = m_playerConfig.SPEED * 3 * entityTrans->scale.x;
	bulletTrans->scale.x = entityTrans->scale.x;
    bullet->addComponent<CAnimation>(m_game.getAssets().getAnimation(m_playerConfig.WEAPON), true);
    bullet->addComponent<CBoundingBox>(m_game.getAssets().getAnimation(m_playerConfig.WEAPON).getSize());
}

void GameState_Play::update()
{
    m_entityManager.update();

    // TODO: implement pause functionality

    sMovement();
    sLifespan();
    sCollision();
    sAnimation();
    sUserInput();
    sRender();
}

void GameState_Play::sMovement()
{
    // TODO: Implement player movement / jumping based on its CInput component
    // TODO: Implement gravity's effect on the player
    // TODO: Implement the maxmimum player speed in both X and Y directions
	if (m_player->hasComponent<CInput>() && m_player->hasComponent<CTransform>())
	{
		auto input = m_player->getComponent<CInput>();
		auto transform = m_player->getComponent<CTransform>();

		m_player->getComponent<CTransform>()->speed = Vec2(0,0);

		if (input->up) {
			transform->speed.y += m_playerConfig.SPEED;
		}
		if (input->down) {
			transform->speed.y -= m_playerConfig.SPEED;
		}
		if (input->left) {
			transform->speed.x -= m_playerConfig.SPEED;
			transform->scale.x = -1;
		}
		if (input->right) {
			transform->speed.x += m_playerConfig.SPEED;
			transform->scale.x = 1;
		}
		if (input->shoot && input->canShoot) {
			input->canShoot = false;
			spawnBullet(m_player);
		}

		for (auto b : m_entityManager.getEntities("bullet"))
		{
			if (b->hasComponent<CTransform>())
			{
				auto t = b->getComponent<CTransform>();
				t->pos += t->speed;
			}
		}

		transform->prevPos = transform->pos;
		transform->pos += transform->speed;
	}
}

void GameState_Play::sLifespan()
{
    // TODO: Check lifespawn of entities that have them, and destroy them if they go over
}

void GameState_Play::sCollision()
{
    // TODO: Implement Physics::GetOverlap() function, use it inside this function

    // TODO: Implement bullet / tile collisions
    //       Destroy the tile if it has a Brick animation
    // TODO: Implement player / tile collisions and resolutions
    //       Update the CState component of the player to store whether
    //       it is currently on the ground or in the air. This will be
    //       used by the Animation system
    // TODO: Check to see if the player has fallen down a hole (y < 0)
    // TODO: Don't let the player walk off the left side of the map
}

void GameState_Play::sUserInput()
{
    // TODO: implement the rest of the player input

    sf::Event event;
    while (m_game.window().pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            m_game.quit();
        }

        if (event.type == sf::Event::KeyPressed)
        {
            switch (event.key.code)
            {
                case sf::Keyboard::Escape:  { m_game.popState(); break; }
                case sf::Keyboard::Z:       { init(m_levelPath); break; }
                case sf::Keyboard::R:       { m_drawTextures = !m_drawTextures; break; }
                case sf::Keyboard::F:       { m_drawCollision = !m_drawCollision; break; }
                case sf::Keyboard::P:       { setPaused(!m_paused); break; }
				case sf::Keyboard::W:		
				{ 
					if (m_player->hasComponent<CInput>()) 
					{ 
						m_player->getComponent<CInput>()->up = true; 
					} 
					break;
				}
				case sf::Keyboard::A:		
				{ 
					if (m_player->hasComponent<CInput>()) 
					{ 
						m_player->getComponent<CInput>()->left = true; 
					} 
					break;
				}
				case sf::Keyboard::S:		
				{ 
					if (m_player->hasComponent<CInput>()) 
					{ 
						m_player->getComponent<CInput>()->down = true; 
					} 
					break;
				}
				case sf::Keyboard::D:		
				{ 
					if (m_player->hasComponent<CInput>()) 
					{ 
						m_player->getComponent<CInput>()->right = true; 
					} 
					break;
				}
				case sf::Keyboard::Space:		
				{ 
					if (m_player->hasComponent<CInput>()) 
					{ 
						auto playerInput = m_player->getComponent<CInput>();
						if (playerInput->canShoot)
						{
							playerInput->shoot = true; 
						}
					} 
					break;
				}
            }
        }

        if (event.type == sf::Event::KeyReleased)
        {
            switch (event.key.code)
            {
                case sf::Keyboard::Escape: 	{ break; }
				case sf::Keyboard::W:		
				{ 
					if (m_player->hasComponent<CInput>()) 
					{ 
						m_player->getComponent<CInput>()->up = false; 
					}
					break;
				}
				case sf::Keyboard::A:		
				{ 
					if (m_player->hasComponent<CInput>()) 
					{ 
						m_player->getComponent<CInput>()->left = false; 
					} 
					break;
				}
				case sf::Keyboard::S:		
				{ 
					if (m_player->hasComponent<CInput>()) 
					{ 
						m_player->getComponent<CInput>()->down = false; 
					} 
					break;
				}
				case sf::Keyboard::D:		
				{ 
					if (m_player->hasComponent<CInput>()) 
					{ 
						m_player->getComponent<CInput>()->right = false; 
					} 
					break;
				}
				case sf::Keyboard::Space:		
				{ 
					if (m_player->hasComponent<CInput>()) 
					{ 
						auto playerInput = m_player->getComponent<CInput>();
						playerInput->shoot = false; 
						playerInput->canShoot = true;
					} 
					break;
				}
            }
        }
    }
}

void GameState_Play::sAnimation()
{
    // TODO: set the animation of the player based on its CState component
    // TODO: for each entity with an animation, call entity->getComponent<CAnimation>()->animation.update()
    //       if the animation is not repeated, and it has ended, destroy the entity
}

// this function has been completed for you
void GameState_Play::sRender()
{
    // color the background darker so you know that the game is paused
    if (!m_paused)  { m_game.window().clear(sf::Color(100, 100, 255)); }
    else            { m_game.window().clear(sf::Color(50, 50, 150)); }

    // set the viewport of the window to be centered on the player if it's far enough right
    auto pPos = m_player->getComponent<CTransform>()->pos;
    float windowCenterX = fmax(m_game.window().getSize().x / 2.0f, pPos.x);
    sf::View view = m_game.window().getView();
    view.setCenter(windowCenterX, m_game.window().getSize().y - view.getCenter().y);
    m_game.window().setView(view);
        
    // draw all Entity textures / animations
    if (m_drawTextures)
    {
        for (auto e : m_entityManager.getEntities())
        {
            auto transform = e->getComponent<CTransform>();

            if (e->hasComponent<CAnimation>())
            {
                auto animation = e->getComponent<CAnimation>()->animation;
                animation.getSprite().setRotation(transform->angle);
                animation.getSprite().setPosition(transform->pos.x, m_game.window().getSize().y - transform->pos.y);
                animation.getSprite().setScale(transform->scale.x, transform->scale.y);
                m_game.window().draw(animation.getSprite());
            }
        }
    }

    // draw all Entity collision bounding boxes with a rectangleshape
    if (m_drawCollision)
    {
        for (auto e : m_entityManager.getEntities())
        {
            if (e->hasComponent<CBoundingBox>())
            {
                auto box = e->getComponent<CBoundingBox>();
                auto transform = e->getComponent<CTransform>();
                sf::RectangleShape rect;
                rect.setSize(sf::Vector2f(box->size.x-1, box->size.y-1));
                rect.setOrigin(sf::Vector2f(box->halfSize.x, box->halfSize.y));
                rect.setPosition(transform->pos.x, m_game.window().getSize().y - transform->pos.y);
                rect.setFillColor(sf::Color(0, 0, 0, 0));
                rect.setOutlineColor(sf::Color(255, 255, 255, 255));
                rect.setOutlineThickness(1);
                m_game.window().draw(rect);
            }
        }
    }

    m_game.window().display();
}
