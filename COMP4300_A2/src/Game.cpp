#include "Game.h"

Game::Game(const std::string & config)
{ 
    init(config);
}

void Game::init(const std::string & path)
{
    // TODO
    // read in config file here

    // set up default window parameters
    m_window.create(sf::VideoMode(1280, 720), "Assignment 2");
    m_window.setFramerateLimit(60);

    spawnPlayer();
}

void Game::run()
{
    // TODO: add pause functionality in here
    while (m_running)
    {
        m_entities.update();
        sEnemySpawner();
        sMovement();
        sCollision();
        sUserInput();
        sRender();
    }
}

void Game::setPaused(bool paused)
{
    // TODO
}

// respawn the player in the middle of the screen
void Game::spawnPlayer()
{
    // TODO: Finish adding all properties of the player with the correct values from the config
    
    // We create every entity by calling EntityManager.addEntity(tag)
    // This returns a std::shared_ptr<Entity>, so we use 'auto' to save typing
    auto entity         = m_entities.addEntity("player");
    // Give this entity a Transform so it spawns at (200,200) with speed (1,1) and angle 0
    entity->cTransform  = new CTransform(Vec2(200, 200), Vec2(1, 1), 0);
    // The entity's shape will have radius 32, 8 sides, dark grey fill, and red outline of thickness 4
    entity->cShape      = new CShape(32, 8, sf::Color(10, 10, 10), sf::Color(255, 0, 0), 4);
	// Give the player input controls
	entity->cInput		= new CInput();
	// Give the player a collison 
	entity->cCollision	= new CCollision(32);
    // Since we want this Entity to be our player, set our Game's player variable to be this Entity
    m_player = entity;
}

// spawn an enemy at a random position
void Game::spawnEnemy()
{
	auto enemy = m_entities.addEntity("enemy");
	enemy->cTransform = new CTransform(Vec2(200, 200), Vec2(1, 1), 0);
	enemy->cShape = new CShape(32, 5, sf::Color(0, 100, 0), sf::Color(255, 255, 255), 4);
	enemy->cCollision	= new CCollision(32);
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
    // TODO
}

void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2 & target)
{
	float angle = atan2f((target - entity->cTransform->pos).x, (target - entity->cTransform->pos).y);
	auto bullet = m_entities.addEntity("bullet");
	bullet->cTransform = new CTransform(entity->cTransform->pos, Vec2(sin(angle), cos(angle))*5, 0);
	bullet->cShape = new CShape(5, 16, sf::Color(255, 255, 255), sf::Color(255, 255, 255), 1);
	bullet->cCollision	= new CCollision(5);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
    // TODO
}

void Game::sMovement()
{
	for (auto p : m_entities.getEntities("player")) 
	{
		if (!p->cInput) 
		{
			continue;
		}
		Vec2 speed(0,0);
		if (p->cInput->up) 
		{
			speed.y -= 5;
		}
		if (p->cInput->left) 
		{
			speed.x -= 5;
		}
		if (p->cInput->down) 
		{
			speed.y += 5;
		}
		if (p->cInput->right) 
		{
			speed.x += 5;
		}
		p->cTransform->speed = speed;
		
	}

	for (auto e : m_entities.getEntities())
	{
		e->cTransform->pos += e->cTransform->speed;
	}
}

void Game::sCollision()
{
    for (auto e1 : m_entities.getEntities())
	{
		if (!e1->cCollision)
		{
			continue;
		}
		for (auto e2 : m_entities.getEntities())
		{
			if (!e2->cCollision || e1->tag() == e2->tag())
			{
				continue;
			}	
			float dx = e1->cTransform->pos.x - e2->cTransform->pos.x;
			float dy = e1->cTransform->pos.y - e2->cTransform->pos.y;
			float distSq = dx*dx + dy*dy;
			float radiusSum = e1->cCollision->radius + e2->cCollision->radius;
			if (distSq < radiusSum*radiusSum)
			{
				if (e1->tag() == "enemy" || e2->tag() == "enemy"
					|| e1->tag() == "smallEnemy" || e2->tag() == "smallEnemy") 
				{
					if ((e1->tag() == "player" || e2->tag() == "player") 
						|| (e1->tag() == "bullet" || e2->tag() == "bullet"))
					{
						e1->destroy();
						e2->destroy();
					}	
				}
			}	
		}
	}
}

void Game::sEnemySpawner()
{
	if (m_clock.getElapsedTime().asMilliseconds() >= 1000)
	{
		spawnEnemy();
		m_clock.restart();
	}
}

void Game::sRender()
{
    m_window.clear();

	for (auto e : m_entities.getEntities())
	{
		e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);
		e->cTransform->angle += 1.0f;
		e->cShape->circle.setRotation(e->cTransform->angle);
		m_window.draw(e->cShape->circle);
	}

    m_window.display();
}

void Game::sUserInput()
{
    // TODO
    sf::Event event;
    while (m_window.pollEvent(event))
    {
		// this event triggers when the window is closed
        if (event.type == sf::Event::Closed)
        {
            m_running = false;
        }


        // this event is triggered when a key is pressed
        if (event.type == sf::Event::KeyPressed)
        {
            switch (event.key.code)
            {
                case sf::Keyboard::W:
					m_player->cInput->up = true;
                    break;
				case sf::Keyboard::A:
					m_player->cInput->left = true;
                    break;
				case sf::Keyboard::S:
					m_player->cInput->down = true;
                    break;
				case sf::Keyboard::D:
					m_player->cInput->right = true;
                    break;
            }
        }

        if (event.type == sf::Event::KeyReleased)
        {
            switch (event.key.code)
            {
                case sf::Keyboard::W:
					m_player->cInput->up = false;
                    break;
				case sf::Keyboard::A:
					m_player->cInput->left = false;
                    break;
				case sf::Keyboard::S:
					m_player->cInput->down = false;
                    break;
				case sf::Keyboard::D:
					m_player->cInput->right = false;
                    break;
            }
        }

        if (event.type == sf::Event::MouseButtonPressed)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                spawnBullet(m_player,Vec2(event.mouseButton.x, event.mouseButton.y));
            }

            if (event.mouseButton.button == sf::Mouse::Right)
            {
                std::cout << "Right Mouse Button Clicked at (" << event.mouseButton.x << "," << event.mouseButton.y << ")\n";
            }
        }
    }
}
