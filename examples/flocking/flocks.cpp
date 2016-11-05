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
 * This example uses SFML. Compile using the following:
 * g++ flocks.cpp -o flocks -lsfml-graphics -lsfml-window -lsfml-system
 */

#include <SFML/Graphics.hpp>
#include <iostream>

/**
 * Flock Agent.
 */

template <typename O, typename I, typename K> class Agent {
    public:

};


int main() {

    sf::Texture texture;
    if(!texture.loadFromFile("agent.png")) {
        std::cerr << "Can't load agent texture !" << std::endl;
        return -1;
    }
    texture.setSmooth(true);

    sf::Sprite sprite;
    sprite.setTexture(texture);
    sprite.setOrigin(sf::Vector2f(32, 32));

    sf::RenderWindow window(sf::VideoMode(1024, 768), "Flocking",
            sf::Style::Titlebar | sf::Style::Close);

    window.setFramerateLimit(60);
    while(window.isOpen()) {
        sf::Event event;
        while(window.pollEvent(event)) {
            if(event.type == sf::Event::Closed) {
                window.close();
            }
        }
        window.clear(sf::Color::Black);

        sf::Vector2i localPosition = sf::Mouse::getPosition(window);

        sprite.setColor(sf::Color(0, 255, 0));
        sprite.setPosition(localPosition.x, localPosition.y);
        sprite.setScale(sf::Vector2f(.5f, .5f));
        window.draw(sprite);

        window.display();
    }


    return 0;
}
