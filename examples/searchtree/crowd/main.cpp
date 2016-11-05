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
        void inspect(Element **, unsigned int) {}
};

#define AGENT_COUNT 256

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

    sf::RenderWindow window(sf::VideoMode(1024, 1024), "Crowd Control",
            sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    DisplayerVisitor visitor;
    visitor.set(&sprite, &window);

    Region region(glm::vec4(0.0, 0.0, 1024.0, 1024.0));
    Headless::Logic::SearchTree::Node<glm::vec2, Region, Element> tree(&region, 3);

    Element **pool = new Element*[AGENT_COUNT];
    std::random_device randomDevice;
    std::mt19937 mt(randomDevice());
    std::uniform_real_distribution<double> distribution(0.0, 1024.0);

    for(unsigned int i = 0; i < AGENT_COUNT; ++i) {
        pool[i] = new Element(glm::vec2(distribution(mt), distribution(mt)),
                std::string("Agent#").append(std::to_string(i)));
        tree.add(pool[i]);
    }

    while (window.isOpen()) {
        // Event handling.
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

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

    // Clean exit.
    return 0;
}
