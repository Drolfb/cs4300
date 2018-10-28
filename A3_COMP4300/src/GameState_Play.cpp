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
    m_player = m_entityManager.addEntity("player");
    m_player->addComponent<CTransform>(Vec2(m_playerConfig.X, m_playerConfig.Y));
    m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CX, m_playerConfig.CY));
    m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("Stand"), true);
	m_player->addComponent<CInput>();
	m_player->addComponent<CState>("jumping");
}

void GameState_Play::spawnBullet(std::shared_ptr<Entity> entity)
{
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

	if (!m_paused)
	{
		sMovement();
		sLifespan();
		sCollision();
		sAnimation();
	}
    sUserInput();
    sRender();
}

void GameState_Play::sMovement()
{
	if (m_player->hasComponent<CInput>() && m_player->hasComponent<CTransform>() && m_player->hasComponent<CState>())
	{
		auto input = m_player->getComponent<CInput>();
		auto transform = m_player->getComponent<CTransform>();
		auto state = m_player->getComponent<CState>();

		transform->speed.x = 0;

		if (input->up && state->state != "jumping") {
			transform->speed.y = m_playerConfig.JUMP;
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

		transform->speed.y += m_playerConfig.GRAVITY;

		if (abs(transform->speed.y) > m_playerConfig.MAXSPEED)
		{
			transform->speed.y = transform->speed.y > 0 ? m_playerConfig.MAXSPEED : -m_playerConfig.MAXSPEED;
		}
		if (abs(transform->speed.x) > m_playerConfig.MAXSPEED)
		{
			transform->speed.x = transform->speed.x > 0 ? m_playerConfig.MAXSPEED : -m_playerConfig.MAXSPEED;
		}
	}

	for (auto e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CTransform>())
		{
			auto t = e->getComponent<CTransform>();
			t->prevPos = t->pos;
			t->pos += t->speed;
		}
	}

}

void GameState_Play::sLifespan()
{
	for (auto e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CLifeSpan>())
		{
			auto l = e->getComponent<CLifeSpan>();
			if (l->clock.getElapsedTime().asMilliseconds() > l->lifespan)
			{
				e->destroy();
			}
		}
	}
}

void GameState_Play::sCollision()
{

	for (auto e1 : m_entityManager.getEntities())
	{
		if (e1->isActive() && e1->hasComponent<CBoundingBox>() && e1->hasComponent<CTransform>() && e1->hasComponent<CAnimation>())
		{
			if (!(e1->tag() == "player") && !(e1->tag() == "bullet")) continue;

			auto ea1 = e1->getComponent<CAnimation>();
			auto eb1 = e1->getComponent<CBoundingBox>();
			auto et1 = e1->getComponent<CTransform>();

			if (e1->tag() == "player")
			{
				if (m_player->hasComponent<CState>())
				{
					m_player->getComponent<CState>()->state = "jumping";
				}
				if (et1->pos.x - eb1->halfSize.x < 0) et1->pos.x = eb1->halfSize.x;
				if (et1->pos.y < 0) et1->pos = Vec2(m_playerConfig.X, m_playerConfig.Y);
			}


			for (auto e2 : m_entityManager.getEntities("tile"))
			{
				if (e1 == e2) { continue; }
				if (e2->isActive() && e2->hasComponent<CBoundingBox>() && e2->hasComponent<CTransform>() && e1->hasComponent<CAnimation>())
				{
					Vec2 overlap = Physics::GetOverlap(e1, e2);
					if (overlap.x <= 0 || overlap.y <= 0) continue;

					auto ea2 = e2->getComponent<CAnimation>();
					auto eb2 = e2->getComponent<CBoundingBox>();
					auto et2 = e2->getComponent<CTransform>();

					if (e1->tag() == "bullet")
					{
						e1->destroy();
						if (ea2->animation.getName() == "Brick") 
						{
							ea2->animation = m_game.getAssets().getAnimation("Explosion");
							ea2->repeat = false;
						}
					}

					if (e1->tag() == "player")
					{
						float prevOverlapX = eb1->halfSize.x + eb2->halfSize.x - abs(et1->prevPos.x - et2->prevPos.x);
						float prevOverlapY = eb1->halfSize.y + eb2->halfSize.y - abs(et1->prevPos.y - et2->prevPos.y);
						if (prevOverlapX <= 0 && prevOverlapY <= 0) // Special Collision
						{
							continue;
						}
						else if (et1->prevPos.y - eb1->halfSize.y >= et2->prevPos.y + eb2->halfSize.y) // Top Collision
						{
							if (m_player->hasComponent<CState>())
							{
								m_player->getComponent<CState>()->state = "ground";
							}
							et1->pos.y += overlap.y;
							et1->speed.y = 0;
						}
						else if (et1->prevPos.y + eb1->halfSize.y <= et2->prevPos.y - eb2->halfSize.y) // Bottom Collision
						{
							et1->pos.y -= overlap.y;
							et1->speed.y = 0;
							if (ea2->animation.getName() == "Brick")
							{
								e2->removeComponent<CBoundingBox>();
								ea2->animation = m_game.getAssets().getAnimation("Explosion");
								ea2->repeat = false;
							}
							else if (ea2->animation.getName() == "Question")
							{
								ea2->animation = m_game.getAssets().getAnimation("Question2");
								auto coin = m_entityManager.addEntity("coin");
								coin->addComponent<CAnimation>(m_game.getAssets().getAnimation("Coin"), false);
								auto ca = coin->getComponent<CAnimation>();
								coin->addComponent<CTransform>(et2->pos + Vec2(0, eb2->halfSize.y + ca->animation.getSize().y/2));
							}
						}
						else if (et1->prevPos.x - eb1->halfSize.x >= et2->prevPos.x + eb2->halfSize.x) // Right Collision
						{
							et1->pos.x += overlap.x;
						}
						else if (et1->prevPos.x + eb1->halfSize.x <= et2->prevPos.x - eb2->halfSize.x) // Left Collision
						{
							et1->pos.x -= overlap.x;
						}
					}
				}
			}
		}
	}
}

void GameState_Play::sUserInput()
{
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
				case sf::Keyboard::P: { setPaused(!m_paused); break; }
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
	for (auto e : m_entityManager.getEntities()) 
	{
		if (!e->hasComponent<CAnimation>()) continue;

		auto ea = e->getComponent<CAnimation>();

		if (e->tag() == "player" && e->hasComponent<CState>() && e->hasComponent<CTransform>())
		{
			auto es = e->getComponent<CState>();
			auto et = e->getComponent<CTransform>();

			if (es->state == "jumping") ea->animation = m_game.getAssets().getAnimation("Air");
			if (es->state == "ground") 
			{
				if (abs(et->speed.x) > 0 && ea->animation.getName() != "Run")
				{
					ea->animation = m_game.getAssets().getAnimation("Run");
				}	
				else if (et->speed.x == 0)
				{
					ea->animation = m_game.getAssets().getAnimation("Stand");
				}
			}
		}

		ea->animation.update();

		if (!ea->repeat && ea->animation.hasEnded()) e->destroy();
	}
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
