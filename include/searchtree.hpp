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
#ifndef HEADLESS_LOGIC_SEARCH_TREE
#define HEADLESS_LOGIC_SEARCH_TREE
#define DEFAULT_CARD 16
#define VISIT_BUFFER_SIZE 32
namespace Headless {
    namespace Logic {
        namespace SearchTree {

            /**
             * Search Tree Node.
             *
             * Given that an element can be associated to a (non-unique) key,
             * Given that distance between two keys can be computed and expressed as a scalar,
             * it handles the following operations:
             * - Add an element given a key.
             * - Remove an element (by instance or by key).
             * - Find elements within the distance from a key.
             * @param <K> Key concept. The distance computation is assumed by
             *     the provided Region instance.
             * @param <R> Region concept. The unfamous one. Must implement the following methods:
             *     Key containment.
             *         bool contains(const K&) const;
             *     Provide the cardinality of region subdivision.
             *         unsigned int dimension() const;
             *     Divide the region.
             *         R* divide() const;
             * @param <E> Element concept. Must expose the following methods :
             *     Get the key.
             *         const K& key() const;
             *     Set the key.
             *         void key(const K&);
             */
            template <typename K, typename R, typename E> class Node {
                public:
                    /**
                     * Default visitor.
                     */
                    class Visitor {
                        public:
                            void enter(const R&) {}
                            void exit(const R&) {}
                            void inspect(E**, unsigned int) {}
                            void inspect(E*) {}
                    };
                public:
                    /**
                     * Constructor.
                     * At creation, the node is a leaf and does not contains
                     * elements.
                     * @param region A node is defined for a particular region key.
                     * @param cardinality Maximum number of stored elements.
                     * @param parent optional parent. nullptr if root.
                     */
                    Node(const R* region,
                            unsigned int cardinality = DEFAULT_CARD, Node *parent = nullptr);
                    /**
                     * Destructor.
                     */
                    ~Node();
                    /**
                     * Add an element.
                     * @param element Pointer to the element to add.
                     */
                    void add(E* element);
                    /**
                     * Remove an element.
                     * @param element Pointer to the element instance to remove.
                     */
                    void remove(E* element);
                    /**
                     * Move an element within the tree.
                     * @param element Element to be moved.
                     * @param key Target key.
                     */
                    void move(E* element, K &key);
                    /**
                     * Retrieve elements with a certain distance from the
                     * specified key.
                     * @param func Search function.
                     * @param buffer Storage for eligible elements.
                     * @param size Size of the buffer.
                     * @param visitor Optional visitor.
                     * @param <S> Search function type. This concept must implement
                     * the following methods:
                     *   int contains(const R&); <- Partially or fully contains a region.
                     *   bool contains(const K&); <- Contains a key.
                     * @param <V> Visitor concept.
                     * @return Number of eligible elements. If there's more
                     * eligible elements than 'size', return the negative value.
                     */
                    template <typename S, typename V = Visitor> unsigned int retrieve(const S& func,
                            E** buffer, unsigned int size, V* visitor = nullptr) const;
                    /**
                     * Recursive visit of the tree.
                     * @param <V> Visitor concept.
                     *   Must implement 'void enter(const R &)', 'void exit()'
                     *   and 'void inspect(const E **, unsigned int count)'.
                     * @param visitor Visitor.
                     */
                    template <typename V> void visit(V& visitor);

#ifdef TREE_DEBUG
                    template <typename V> void deepVisit(V &visitor);
#endif

                private:
                    /**
                     * Fetch the entire content of the tree.
                     * @param buffer Array in which to fetch elements.
                     * @param size Size of this array.
                     * @param visitor Optional visitor.
                     * @param <V> Visitor concept.
                     * @return Number of retrieved elements.
                     */
                    template <typename V = Visitor> unsigned int fetch(E** buffer,
                            unsigned int size, V* visitor = nullptr) const;
                    /**
                     * Find the leaf that can possibly host the key.
                     * @param key Node key to locate.
                     * @return A leaf or nullptr if the key is outside the master region.
                     */
                    Node* find(const K& key);
                private:
                    /** Region of interest. */
                    const R*                 _region;
                    /** Stored elements. 'null' if not leaf. */
                    E**                      _elements;
                    /** Element or node count. */
                    unsigned int             _count;
                    /** Maximum number of elements. */
                    unsigned int             _cardinality;
                    /** Sub-node. 'null' if leaf. */
                    Node<K, R, E>**          _nodes;
                    /** Parent node. */
                    Node<K, R, E>*           _parent;
                    /** Leaf indicator. Indirect recycling info. */
                    bool                     _leaf;
            };

            template <typename K, typename R, typename E>
                Node<K, R, E>::Node(const R* region, unsigned int card, Node<K, R, E>* parent) :
                    _region(region), _elements(new E*[card]), _count(0),
                    _cardinality(card), _nodes(nullptr), _parent(parent), _leaf(true) {
                    }

            template <typename K, typename R, typename E>
                Node<K, R, E>::~Node() {
                    delete []_elements;
                    if(_nodes != nullptr) {
                        unsigned int dimension = _region->dimension();
                        const R* region = _nodes[0]->_region;
                        for(unsigned int i = 0; i < dimension; ++i) {
                            delete _nodes[i];
                        }
                        delete []region;
                        delete []_nodes;
                    }
                }

            template <typename K, typename R, typename E>
                Node<K, R, E>* Node<K, R, E>::find(const K& key) {
                    Node<K, R, E>* result;
                    if(_region->contains(key)) {
                        result = this;
                        Node<K, R, E>** nodes;
#ifdef TREE_DEBUG
                        bool loop;
#endif
                        while(!result->_leaf) {
                            nodes = result->_nodes;
#ifdef TREE_DEBUG
                            loop = true;
#endif
                            unsigned int count = result->_region->dimension();
                            for(unsigned int i = 0; i < count; ++i) {
                                if(nodes[i]->_region->contains(key)) {
                                    result = nodes[i];
#ifdef TREE_DEBUG
                                    loop = false;
#endif
                                    break;
                                }
                            }
#ifdef TREE_DEBUG
                            if(loop) {
                                result = nullptr;
                                break;
                            }
#endif
                        }
                    } else {
                        result = nullptr;
                    }
                    return result;
                }

            template <typename K, typename R, typename E>
                void Node<K, R, E>::add(E* element) {
                    const K& key = element->key();
                    Node<K, R, E>* node = find(key);
                    if(nullptr != node) {
                        while(_cardinality == node->_count) {
                            node->_leaf = false;
                            unsigned int dimension = node->_region->dimension();
                            if(nullptr == node->_nodes) {
                                node->_nodes = new Node<K, R, E>*[dimension];
                                const R* regions = node->_region->divide();
                                for(unsigned int i = 0; i < dimension; ++i) {
                                    node->_nodes[i] = new Node<K, R, E>(regions + i, _cardinality, node);
                                }
                            }
                            E** toShare = node->_elements;
                            unsigned int shareCount = node->_count;
                            Node<K, R, E>* target;
                            for(unsigned int i = 0; i < dimension; ++i) {
                                target = node->_nodes[i];
                                target->_count = 0;
                                for(unsigned int j = 0; j < shareCount;) {
                                    if(target->_region->contains(toShare[j]->key())) {
                                        target->_elements[target->_count] = toShare[j];
                                        ++target->_count;
                                        --shareCount;
                                        toShare[j] = toShare[shareCount];
                                    } else {
                                        ++j;
                                    }
                                }
                            }
                            node->_count = dimension;
                            for(unsigned int i = 0; i < dimension; ++i) {
                                target = node->_nodes[i];
                                if(target->_region->contains(key)) {
                                    node = target;
                                    break;
                                }
                            }
                        }
                        node->_elements[node->_count] = element;
                        ++node->_count;
                    }
                }

            template <typename K, typename R, typename E>
                void Node<K, R, E>::remove(E* element) {
                    const K& key = element->key();
                    Node<K, R, E>* node = find(key);
                    if(nullptr != node) {
                        unsigned int count = node->_count;
                        E** elements = node->_elements;
                        for(unsigned int i = 0; i < count; ++i, ++elements) {
                            if(element == (*elements)) {
                                --node->_count;
                                *elements = node->_elements[node->_count];
                                break;
                            }
                        }

                        while(nullptr != node->_parent) {
                            node = node->_parent;
                            unsigned int global = 0;
                            unsigned int count = node->_count;
                            for(unsigned int i = 0; i < count; ++i) {
                                if(node->_nodes[i]->_leaf) {
                                    global += node->_nodes[i]->_count;
                                } else {
                                    global += _cardinality + 1;
                                }
                            }
                            if(global <= _cardinality) {
                                node->_leaf = true;
                                node->_count = 0;
                                for(unsigned int i = 0; i < count; ++i) {
                                    Node<K, R, E>* target = node->_nodes[i];
                                    unsigned int toRetrieve = target->_count;
                                    for(unsigned int j = 0; j < toRetrieve; ++j) {
                                        node->_elements[node->_count] = target->_elements[j];
                                        ++node->_count;
                                    }
                                }
                            } else {
                                break;
                            }
                        }
                    }
                }

            template <typename K, typename R, typename E>
                void Node<K, R, E>::move(E* element, K& key) {
                    K elementKey = element->key();
                    Node<K, R, E>* sourceNode = find(elementKey);
                    Node<K, R, E>* destinationNode = find(key);
                    if(destinationNode != sourceNode) {
                        sourceNode->remove(element);
                        element->key(key);
                        destinationNode->add(element);
                    } else {
                        element->key(key);
                    }
                }

            template <typename K, typename R, typename E>
                template <typename S, typename V>
                unsigned int Node<K, R, E>::retrieve(const S& func, E** buffer, unsigned int size, V* visitor) const {
                    unsigned int result;
                    if(nullptr != visitor) {
                        visitor->enter(*_region);
                    }
                    // Here comes the fun.
                    if(_leaf) {
                        // We're in a leaf. There're two cases that leads here :
                        // 1. This leaf intersects with the search function.
                        // 2. This leaf is the root node and might not be relevant ...
                        // In all case, we must confront all the elements to 'func'.
                        unsigned int max = size < _count ? size : _count;
                        E** dest = buffer;
                        E** cur = _elements;
                        result = 0;
                        for(unsigned int i = 0; i < max; ++i, ++cur) {
                            if(func.contains((*cur)->key())) {
                                if(nullptr != visitor) {
                                    visitor->inspect(*cur);
                                }
                                *dest = *cur;
                                ++dest;
                                ++result;
                            }
                        }
                    } else {
                        // We're in a node.
                        // Let's test all the subs against the 'func'. In some cases,
                        // fetch the whole sub-tree, in other cases, just recurse the retrieval.
                        E** dest = buffer;
                        unsigned int remaining = size;
                        unsigned int retrieved;
                        int intersects;
                        Node<K, R, E>** nodes = _nodes;
                        for(unsigned int i = 0; i < _count; ++i, ++nodes) {
                            intersects = func.contains(*((*nodes)->_region));
                            if(intersects >= 0) {
                                if(intersects != 0) {
                                    retrieved = (*nodes)->fetch(dest, remaining, visitor);
                                } else {
                                    retrieved = (*nodes)->retrieve(func, dest, remaining, visitor);
                                }
                                remaining -= retrieved;
                                dest += retrieved;
                            }
                        }
                        result = size - remaining;
                    }
                    if(nullptr != visitor) {
                        visitor->exit(*_region);
                    }
                    return result;
                }

            template <typename K, typename R, typename E>
                template <typename V>
                unsigned int Node<K, R, E>::fetch(E** buffer, unsigned int size, V* visitor) const {
                    if(nullptr != visitor) {
                        visitor->enter(*_region);
                    }
                    unsigned int result;
                    if(_leaf) {
                        if(nullptr != visitor) {
                            visitor->inspect(_elements, _count);
                        }
                        // Get all the elements.
                        result = size < _count ? size : _count;
                        E** dest = buffer;
                        E** src = _elements;
                        for(unsigned int i = 0; i < result; ++i, ++dest, ++src) {
                            *dest = *src;
                        }
                    } else {
                        result = 0;
                        E** dest = buffer;
                        unsigned int remaining = size;
                        unsigned int retrieved;
                        for(unsigned int i = 0; i < _count; ++i) {
                            retrieved = _nodes[i]->fetch(dest, remaining);
                            remaining -= retrieved;
                            dest += retrieved;
                        }
                        result = size - remaining;
                    }
                    if(nullptr != visitor) {
                        visitor->exit(*_region);
                    }
                    return result;
                }

            template <typename K, typename R, typename E>
                template <typename V>
                void Node<K, R, E>::visit(V &visitor) {
                    visitor.enter(*_region);
                    if(_leaf) {
                        visitor.inspect(_elements, _count);
                    } else {
                        for(unsigned int i = 0; i < _count; ++i) {
                            _nodes[i]->visit(visitor);
                        }
                    }
                    visitor.exit(*_region);
                }

#ifdef TREE_DEBUG
            template <typename K, typename R, typename E>
                template <typename V>
                void Node<K, R, E>::deepVisit(V &visitor) {
                    visitor.visit(this, _region, _elements, _nodes, _parent, _leaf, _count, _cardinality);
                    if(_nodes) {
                        unsigned int dimension = _region->dimension();
                        for(unsigned int i = 0; i < dimension; ++i) {
                            _nodes[i]->deepVisit(visitor);
                        }
                    }
                }
#endif

        } // Namespace 'SearchTree'
    } // Namespace 'Logic'
} // Namespace 'Headless'

#endif
