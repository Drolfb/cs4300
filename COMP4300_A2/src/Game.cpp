#include "Game.h"
#include <iostream>
#include <fstream>

Game::Game(const std::string & config)
{ 
    init(config);
}

void Game::init(const std::string & path)
{
	srand(time(NULL));
	std::string input, fontFile;
	int wWidth, wHeight, frameLimit, fullScreen, fontSize, fR, fG, fB;
    std::ifstream config;

	config.open(path, std::ifstream::in);
	while (!config.eof())
	{
		config >> input;
		if (input == "Window")
		{
			config >> wWidth >> wHeight >> frameLimit >> fullScreen;
		}
		else if (input == "Font")
		{
			config >> fontFile >> fontSize >> fR >> fG >> fB;
		}
		else if (input == "Player")
		{
			config >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V;
		}
		else if (input == "Enemy")
		{
			config >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
		}
		else if (input == "Bullet")
		{
			config >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
		}
		else 
		{
			if (input != "") 
			{
				std::cerr << "Invalid Input: " << input << "\n";
			}
		}
	}

	if (!m_font.loadFromFile(fontFile))
	{
		std::cerr << "Issue loading Font File\n";
	}
	m_text.setFont(m_font);
	m_text.setCharacterSize(fontSize);
	m_text.setFillColor(sf::Color(fR,fG,fB));

    // set up default window parameters
    m_window.create(sf::VideoMode(wWidth, wHeight), "Assignment 2", fullScreen ? sf::Style::Fullscreen : sf::Style::Default);
    m_window.setFramerateLimit(frameLimit);

    spawnPlayer();
}

void Game::run()
{
    while (m_running)
    {
		if (!m_paused)
		{
			m_entities.update();
			sEnemySpawner();
			sMovement();
			sCollision();
			sRender();
		}
		sUserInput();
    }
}

void Game::setPaused(bool paused)
{
	m_paused = paused;
}

// respawn the player in the middle of the screen
void Game::spawnPlayer()
{    // We create every entity by calling EntityManager.addEntity(tag)
    // This returns a std::shared_ptr<Entity>, so we use 'auto' to save typing
    auto entity         = m_entities.addEntity("player");
    // Give this entity a Transform so it spawns at (200,200) with speed (1,1) and angle 0
    entity->cTransform  = new CTransform(Vec2(m_window.getSize().x/2, m_window.getSize().y/2), Vec2(0, 0), 0);
    // The entity's shape will have radius 32, 8 sides, dark grey fill, and red outline of thickness 4
    entity->cShape      = new CShape(m_playerConfig.SR, m_playerConfig.V, sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB), sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);
	// Give the player input controls
	entity->cInput		= new CInput();
	// Give the player a collison 
	entity->cCollision	= new CCollision(m_playerConfig.CR);
    // Since we want this Entity to be our player, set our Game's player variable to be this Entity
    m_player = entity;
}

// spawn an enemy at a random position
void Game::spawnEnemy()
{
	int eX = (rand() % (m_window.getSize().x - m_enemyConfig.SR*2)) + m_enemyConfig.SR;
	int eY = (rand() % (m_window.getSize().y - m_enemyConfig.SR*2)) + m_enemyConfig.SR;
	float eSpdX = m_enemyConfig.SMIN + static_cast<float>(rand() / (static_cast<float>(RAND_MAX / (m_enemyConfig.SMAX - m_enemyConfig.SMIN))));
	float eSpdY = m_enemyConfig.SMIN + static_cast<float>(rand() / (static_cast<float>(RAND_MAX / (m_enemyConfig.SMAX - m_enemyConfig.SMIN))));
	int vertices = m_enemyConfig.VMIN + rand() % (m_enemyConfig.VMAX+1 - m_enemyConfig.VMIN);
	eSpdX = (rand() % 2) ? -eSpdX : eSpdX;
	eSpdY = (rand() % 2) ? -eSpdY : eSpdY;

	auto enemy = m_entities.addEntity("enemy");
	enemy->cTransform = new CTransform(Vec2(eX, eY), Vec2(eSpdX, eSpdY), 0);
	enemy->cShape = new CShape(m_enemyConfig.SR, vertices, sf::Color(rand() % 255, rand() % 255, rand() % 255), sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), m_enemyConfig.OT);
	enemy->cCollision = new CCollision(m_enemyConfig.CR);
	enemy->cScore = new CScore(vertices*100);
}

//	When an enemy is destroyed it calls this method which spawns smaller versions of the same shape around it.
void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
	sf::CircleShape circle = e->cShape->circle;
	float vInterval = 360 / circle.getPointCount();
	float size = 0.25f;
	for (size_t i=1; i <= circle.getPointCount(); i++)
	{
		auto sEnemy = m_entities.addEntity("smallEnemy");
		float rads = 6.28*((i*vInterval) / 360);
		sEnemy->cTransform = new CTransform(e->cTransform->pos, Vec2(cos(rads), sin(rads)), 0);
		sEnemy->cShape = new CShape(m_enemyConfig.SR*size, circle.getPointCount(), circle.getFillColor(), circle.getOutlineColor(), circle.getOutlineThickness());
		sEnemy->cCollision = new CCollision(m_enemyConfig.CR*size);
		sEnemy->cLifespan = new CLifespan(m_enemyConfig.L);
		sEnemy->cScore = new CScore(e->cScore->score*2);
	}
}

/
void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2 & target)
{
	float angle = atan2f((target - entity->cTransform->pos).x, (target - entity->cTransform->pos).y);
	auto bullet = m_entities.addEntity("bullet");
	bullet->cTransform = new CTransform(entity->cTransform->pos, Vec2(sin(angle), cos(angle))*m_bulletConfig.S, 0);
	bullet->cShape = new CShape(5, 16, sf::Color(255, 255, 255), sf::Color(255, 255, 255), 1);
	bullet->cCollision	= new CCollision(5);
	bullet->cLifespan = new CLifespan(m_bulletConfig.L);
}

//	Method for creating the special weapon around the player.

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
	sf::CircleShape circle = entity->cShape->circle;
	float vInterval = 360 / circle.getPointCount();
	float size = 0.25f;
	for (size_t i = 1; i <= circle.getPointCount(); i++)
	{
		auto sBullet = m_entities.addEntity("specialBullet");
		float rads = 6.28*((i*vInterval) / 360);
		sBullet->cTransform = new CTransform(entity->cTransform->pos, Vec2(cos(rads), sin(rads))*m_bulletConfig.S, 0);
		sBullet->cShape = new CShape(m_playerConfig.SR*size, circle.getPointCount(), circle.getFillColor(), circle.getOutlineColor(), circle.getOutlineThickness());
		sBullet->cCollision = new CCollision(m_playerConfig.CR*size);
		sBullet->cLifespan = new CLifespan(m_bulletConfig.L);
	}
}

//	Player movement based on input given.
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
			speed.y -= m_playerConfig.S;
		}
		if (p->cInput->left) 
		{
			speed.x -= m_playerConfig.S;
		}
		if (p->cInput->down) 
		{
			speed.y += m_playerConfig.S;
		}
		if (p->cInput->right) 
		{
			speed.x += m_playerConfig.S;
		}
		p->cTransform->speed = speed;
		
	}

	for (auto e : m_entities.getEntities())
	{
		if (e->tag() == "bullet" || e->tag() == "smallEnemy" || e->tag() == "specialBullet")
		{
			if (e->tag() == "smallEnemy" || e->tag() == "specialBullet")
			{
				sf::Color fColor = e->cShape->circle.getFillColor();
				sf::Color oColor = e->cShape->circle.getOutlineColor();
				float percentLeft = 1 - static_cast<float>(e->cLifespan->clock.getElapsedTime().asMilliseconds()) / static_cast<float>(e->cLifespan->lifespan);
				e->cShape->circle.setFillColor(sf::Color(fColor.r, fColor.g, fColor.b, (255 * percentLeft) > 0? 255 * percentLeft : 0));
				e->cShape->circle.setOutlineColor(sf::Color(oColor.r, oColor.g, oColor.b, (255 * percentLeft) > 0 ? 255 * percentLeft : 0));
			}
			if (e->cLifespan->clock.getElapsedTime().asMilliseconds() >= e->cLifespan->lifespan) 
			{
				e->destroy();
			}
		}

		e->cTransform->pos += e->cTransform->speed;

		if (e->tag() == "player" || e->tag() == "bullet" || e->tag() == "smallEnemy" || e->tag() == "specialBullet")
		{
			if (e->cTransform->pos.x < 0)
			{
				e->cTransform->pos.x = m_window.getSize().x - 1;
			}
			else if (e->cTransform->pos.x > m_window.getSize().x)
			{
				e->cTransform->pos.x = 1;
			}
			else if (e->cTransform->pos.y < 0)
			{
				e->cTransform->pos.y = m_window.getSize().y - 1;
			}
			else if (e->cTransform->pos.y > m_window.getSize().y)
			{
				e->cTransform->pos.y = 1;
			}
		}
	}
}

void Game::sCollision()
{
    for (auto e1 : m_entities.getEntities())
	{
		if (!e1->cCollision || !e1->isActive())
		{
			continue;
		}

		if (e1->tag() != "bullet" && e1->tag() != "player" && e1->tag() != "smallEnemy" && e1->tag() != "specialBullet")
		{
			float eX = e1->cTransform->pos.x;
			float eY = e1->cTransform->pos.y;
			float eR = e1->cCollision->radius;
			if (eX-eR <= 0 || eX+eR >= m_window.getSize().x)
			{
				e1->cTransform->speed.x *= -1;
			}
			if (eY - eR <= 0 || eY + eR >= m_window.getSize().y)
			{
				e1->cTransform->speed.y *= -1;
			}
		}

		for (auto e2 : m_entities.getEntities())
		{
			if (!e2->cCollision || e1->tag() == e2->tag() || !e2->isActive())
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
						|| (e1->tag() == "bullet" || e2->tag() == "bullet")
						|| (e1->tag() == "specialBullet") || e2->tag() == "specialBullet")
					{
						e1->destroy();
						e2->destroy();
						if (!m_player->isActive())
						{
							spawnPlayer();
							m_score = 0;
						}
						if (e1->tag() == "enemy")
						{
							spawnSmallEnemies(e1);
							if (e2->tag() == "bullet" || e2->tag() == "specialBullet")
							{
								m_score += e1->cScore->score;
							}
						}
						else if (e2->tag() == "enemy")
						{
							spawnSmallEnemies(e2);
							if (e1->tag() == "bullet" || e1->tag() == "specialBullet")
							{
								m_score += e2->cScore->score;
							}
						}
						else if (e1->tag() == "smallEnemy" && (e2->tag() == "bullet" || e2->tag() == "specialBullet"))
						{
							m_score += e1->cScore->score;
						}
						else if (e2->tag() == "smallEnemy" && (e1->tag() == "bullet" || e1->tag() == "specialBullet"))
						{
							m_score += e2->cScore->score;
						}
					}	
				}
			}	
		}
	}
}

void Game::sEnemySpawner()
{
	if (m_clock.getElapsedTime().asMilliseconds() >= m_enemyConfig.SI)
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

	m_text.setString("Score: " + std::to_string(m_score));
	m_window.draw(m_text);

    m_window.display();
}

void Game::sUserInput()
{
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

	// this event is triggered when a key is released. P will pause the game and ESC will exit it.
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
				case sf::Keyboard::P:
					m_paused = !m_paused;
					break;
				case sf::Keyboard::Escape:
					m_running = false;
					break;
            }
        }

	// Calls the bullet spawn method on left click.
        if (event.type == sf::Event::MouseButtonPressed)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
				if (!m_paused)
				{
					spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
				}
            }

	// Calls the special weapon attack on right click.	
            if (event.mouseButton.button == sf::Mouse::Right)
            {
				if (!m_paused)
				{
					spawnSpecialWeapon(m_player);
				}
			}
        }
    }
}
