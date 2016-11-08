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
#ifndef HEADLESS_LOGIC_FLOCKING
#define HEADLESS_LOGIC_FLOCKING

// Search Tree are needed to maintain a swarm.
#include "searchtree.hpp"


namespace Headless {
    namespace Logic {
        namespace Flock {

            /**
             * Swarm. A set of agent interacting together in order to simulate a crowd.
             * @param <K> Agent Key concept. See Search Tree Node.
             * @param <R> Region concept. Manage subdivision and key testing. See Search Tree Node.
             * @param <E> Agent concept. An agent is defined by its key, its goal and must
             *              define the following set of methods:
             *                  Get the key:
             *                      const K& key() const;
             *                  Set the key:
             *                      void key(const K&);
             *                  Get the goal:
             *                      const G& goal() const;
             *                  Get the perception tool:
             *                      const P& detector() const;
             * @param <G> Goal concept.
             * @param <P> Perception tool concept. This concept must implement the following methods:
             *              int contains(const R&); <- Partially or fully contains a region.
             *              bool contains(const K&); <- Contains a key.
             */
            template <typename K, class R, class E, class G, class P> class Swarm {
                private:
                    /**
                     * Search tree.
                     */
                    SearchTree::Node<K, R, E> *_tree;
                    /**
                     * Swarm Pool.
                     */
                    E** _swarm;
                    /**
                     * Swarm Cardinality.
                     */
                    unsigned int _cardinality;
                    /**
                     * Maximum number of agent.
                     */
                    unsigned int _capacity;
                    /**
                     * Buffer for new key iteration.
                     */
                    K* _keys;
                    /**
                     * Buffer for perception tool.
                     */
                    E** _perceived;
                public:
                    /**
                     * Constructor.
                     * @param region Managed region.
                     * @param capacity Maximum number of agent.
                     */
                    Swarm(R region, unsigned int capacity) : _cardinality(0), _capacity(capacity) {
                        _tree = new SearchTree::Node<K, R, E>(region);
                        _perceived = new E*[capacity];
                        _swarm = new E*[capacity];
                        _keys = new K[capacity];
                    }

                    /**
                     * Destructor.
                     */
                    ~Swarm() {
                        delete _tree;
                        delete []_swarm;
                        delete []_perceived;
                        delete []_keys;
                    }

                    /**
                     * Add an agent.
                     * @param element Agent to add.
                     * @return true if succeeded in adding the agent.
                     */
                    bool add(E *element);

                    /**
                     * Remove an agent.
                     * @param element Agent to remove.
                     */
                    void remove(E *element);

                    /**
                     * Move an agent.
                     * @param element Agent to move.
                     * @param target New agent key.
                     */
                    void move(E *element, K& target);

                    // TODO
                    template<typename V, class A, class ... F>
                        void update(float elapsed, A& adaptor, F& ... forces) {
                            // Compute forces.
                            for(unsigned int i = 0; i < _cardinality; ++i) {
                                const P& tool = _swarm[i]->detector();
                                const G& goal = _swarm[i]->goal();
                                unsigned int count = _tree->retrieve(tool, _perceived, _capacity);
                                V velocity = apply(elapsed, _swarm[i], _perceived, count, forces...);
                                _keys[i] = adaptor.compute(velocity, _swarm[i], elapsed, count);
                            }
                            // Move agents
                            for(unsigned int i = 0; i < _cardinality; ++i) {
                                _tree->move(_swarm[i], _keys[i]);
                            }
                        }

                   private:

                    // TODO
                    template<typename V, class F, class ... O>
                        V& apply(double elapsed, E* subject, E** perceived, unsigned int count, F& force, O& ... others) {
                             V result = force.compute(elapsed, subject, perceived, count);
                             result += apply(elapsed, subject, perceived, count, others...);
                             return result;
                        }

                    // TODO
                    template<typename V, class F>
                        V& apply(double elapsed, E* subject, E** perceived, unsigned int count, F& force) {
                            return force.compute(elapsed, subject, perceived, count);
                        }

            };

        } // Namespace 'Flock'
    } // Namespace 'Logic'
} // Namespace 'Headless'

#endif
