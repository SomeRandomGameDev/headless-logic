/*
 * Copyright 2016 Stoned Xander
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * This example is not meant to simulate flocking, but only "almost"
 * straight line going agents and how it affects the search tree.
 */
#include <iostream>
#include <random>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <headless-logic/searchtree.hpp>
#include "../common.hpp"

int loadTexture(sf::Texture &texture, std::string path) {
    if(!texture.loadFromFile(path)) {
        std::cerr << "Can't load texture '"
            << path << "' !" << std::endl;
        return 0;
    }
    texture.setSmooth(true);
    return 1;
}

class DisplayerVisitor {
    private:
        sf::Sprite *_sprite;
        sf::RenderWindow *_window;
    public:
        DisplayerVisitor() : _sprite(nullptr), _window(nullptr) {}
        void set(sf::Sprite *sprite, sf::RenderWindow *window) {
            _sprite = sprite;
            _window = window;
        }
        void init() {}
        void enter(const Region& region) {
            glm::vec4 boundary = region.boundary();
#ifdef DISPLAY_CENTER
            _sprite->setPosition(boundary.x + (boundary.p / 2.0),
                    boundary.y + (boundary.q / 2.0));
            _window->draw(*_sprite);
#else
            _sprite->setPosition(boundary.x, boundary.y);
            _window->draw(*_sprite);
            _sprite->setPosition(boundary.x + boundary.p, boundary.y);
            _window->draw(*_sprite);
            _sprite->setPosition(boundary.x + boundary.p, boundary.y + boundary.q);
            _window->draw(*_sprite);
            _sprite->setPosition(boundary.x, boundary.y + boundary.q);
            _window->draw(*_sprite);
#endif
        }
        void exit(const Region&) {}
        void inspect(Element **, unsigned int count) {
            if(count == 0) {
               // std::cout << "Empty leaf !" << std::endl;
            }
            if(count > 3) {
                std::cout << "Node Overflow !" << std::endl;
            }
        }
};

#define AGENT_COUNT 256
#define AREA_SIZE 800

/**
 * Main procedure.
 */
int main(void) {
    sf::Vector2f textureOffset(32, 32);

    sf::Texture agentTexture;
    if(!loadTexture(agentTexture, "resources/agent.png")) { return -1; }

    sf::Texture boundaryTexture;
    if(!loadTexture(boundaryTexture, "resources/cross.png")) { return -1; }

    sf::Sprite sprite;

    sf::RenderWindow window(sf::VideoMode(AREA_SIZE, AREA_SIZE), "Crowd Control",
            sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    DisplayerVisitor visitor;
    visitor.set(&sprite, &window);

    Region region(glm::vec4(0.0, 0.0, AREA_SIZE, AREA_SIZE));
    Headless::Logic::SearchTree::Node<glm::vec2, Region, Element> tree(&region, 3);

    Element **pool = new Element*[AGENT_COUNT];
    Element **searchResult = new Element*[AGENT_COUNT];
    std::random_device randomDevice;
    std::mt19937 mt(randomDevice());
    std::uniform_real_distribution<double> posDist(0.0, AREA_SIZE);
    std::uniform_real_distribution<double> velDist(-128.0, 128.0);

    for(unsigned int i = 0; i < AGENT_COUNT; ++i) {
        pool[i] = new Element(glm::vec2(posDist(mt), posDist(mt)),
                std::string("Agent#").append(std::to_string(i)));
        pool[i]->velocity(glm::vec2(velDist(mt), velDist(mt)));
        tree.add(pool[i]);
    }

    sf::Clock clock;
    sf::Time elapsed;
    glm::vec2 target;
    glm::vec2 velocity;

    Disc searchDisc;

    while (window.isOpen()) {
        // Logic update
        elapsed = clock.restart();
        float sec = elapsed.asSeconds();

        for(unsigned int i = 0; i < AGENT_COUNT; ++i) {
            target = pool[i]->key();
            velocity = pool[i]->velocity();
            target.x += velocity.x * sec;
            target.y += velocity.y * sec;
            if(target.x < 0 || target.y < 0 || target.x > AREA_SIZE || target.y > AREA_SIZE) {
                target.x = posDist(mt);
                target.y = posDist(mt);
                velocity.x = velDist(mt);
                velocity.y = velDist(mt);
                pool[i]->velocity(velocity);
            }
            tree.move(pool[i], target);

            // Search neighbor and take mean velocity.
            searchDisc.set(target, 32.0);
            unsigned int count = tree.retrieve(searchDisc, searchResult, AGENT_COUNT);
            glm::vec2 meanVelocity(0.0, 0.0);
            for(unsigned int j = 0; j < count; ++j) {
                meanVelocity += searchResult[j]->velocity();
            }
            if(count > 0) {
                meanVelocity.x /= (double) count;
                meanVelocity.y /= (double) count;
                pool[i]->velocity(meanVelocity);
            }
        }

        // Event handling.
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // Draw
        window.clear(sf::Color::Black);

        sf::Vector2i localPosition = sf::Mouse::getPosition(window);

        sprite.setTexture(agentTexture);
        sprite.setOrigin(textureOffset);
        sprite.setColor(sf::Color(0, 255, 0));
        sprite.setPosition(localPosition.x, localPosition.y);
        sprite.setScale(sf::Vector2f(.5f, .5f));
        window.draw(sprite);

        sprite.setColor(sf::Color(0, 128, 255));
        for(unsigned int i = 0; i < AGENT_COUNT; ++i) {
            glm::vec2 pos = pool[i]->key();
            sprite.setPosition(pos.x, pos.y);
            window.draw(sprite);
        }

        sprite.setColor(sf::Color(255, 255, 255, 32));
        sprite.setTexture(boundaryTexture);
        tree.visit(visitor);

        window.display();

    }

    // Clean Exit.
    for(unsigned int i = 0; i < AGENT_COUNT; ++i) {
        delete pool[i];
    }
    delete []pool;
    delete []searchResult;
    return 0;
}
