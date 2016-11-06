#ifndef HEADLESS_LOGIC_COMMON_TEST
#define HEADLESS_LOGIC_COMMON_TEST

#include <glm/glm.hpp>
#include <iostream>
#include <set>
#include <headless-logic/searchtree.hpp>

class Region {
    public:
        Region() : _boundary(0.0, 0.0, 0.0, 0.0) {}
        Region(const glm::vec4 boundary) : _boundary(boundary) {}
        Region(const Region &r) { _boundary = r._boundary; }

        inline Region &operator=(glm::vec4 bound) {
            _boundary = bound; return *this; }

        inline unsigned int dimension() const { return 4; }
        const Region *divide() const;
        inline glm::vec4 boundary() const { return _boundary; }
        bool contains(const glm::vec2 &) const;
        int contains(const Region &) const;
    private:
        glm::vec4 _boundary;
};

class Disc {
    public:
        Disc() {}
        void set(glm::vec2 center, double radius) {
            _center = center;
            _radius = radius;
            _sqradius = radius * radius;
        }
        bool contains(const glm::vec2 &) const;
        int contains(const Region &) const;
    private:
        glm::vec2 _center;
        double _radius;
        double _sqradius;
};



class Element {
    public:
        Element(glm::vec2 key, std::string name) : _key(key), _velocity(0, 0), _name(name) {}
        const glm::vec2 &key() const { return _key; }
        void key(glm::vec2 &key) { _key = key; }
        const std::string &name() const { return _name; }
        inline void set(const glm::vec2 pos) { _key = pos; }
        void velocity(const glm::vec2 vel) { _velocity = vel; }
        const glm::vec2 &velocity() const { return _velocity; }
        //const glm::vec2 &move(double delta) { _key += _velocity * delta; return _key; }
    private:
        glm::vec2 _key;
        glm::vec2 _velocity;
        std::string _name;
};

class DepthVisitor {
    private:
        unsigned int _depth;
        unsigned int _maxDepth;
    public:
        DepthVisitor() : _depth(0), _maxDepth(0) {}
        void init() { _depth = 0; _maxDepth = 0; }
        unsigned int depth() { return _maxDepth; }
        void enter(const Region &) {
            ++_depth;
            if(_depth > _maxDepth) {
                _maxDepth = _depth;
            }
        }
        void exit(const Region &) {
            --_depth;
        }
        void inspect(Element **, unsigned int) {}
};

class Visitor {
    public:
        Visitor() : _depth(0) {}
        void enter(const Region &region) {
            indent();
            std::cout << "< [";
            glm::vec4 bound = region.boundary();
            std::cout << bound.x << "x" << bound.y << " - ";
            std::cout << bound.p << "x" << bound.q << "] ";
            if(_depth > 0) {
                if (currentRegion.contains(region) >= 0) {
                    std::cout << "OK";
                } else {
                    std::cout << "NOK";
                }
            }
            std::cout << std::endl;
            ++_depth;
            currentRegion = region;
        }

        void inspect(Element **elements, unsigned int count) {
            for(unsigned int i = 0; i < count; ++i) {
                inspect(elements[i]);
            }
        }

        void inspect(Element *element) {
            indent();
            std::cout << element->name();
            const glm::vec2& key = element->key();
            std::cout << " (" << key.x << ", " << key.y << ") ";

            if(currentRegion.contains(key)) {
                std::cout << "OK";
            } else {
                std::cout << "NOK !!";
            }
            std::cout << std::endl;
        }

        void exit(const Region &region) {
            currentRegion = region;
            --_depth;
            indent();
            std::cout << ">" << std::endl;
        }

        void indent() {
            for(int i = 0; i < _depth; ++i) {
                std::cout << " ";
            }
        }

    private:
        int _depth;
        Region currentRegion;
};

class ConsistencyVisitor {
    public:
        ConsistencyVisitor(unsigned int card) : _cardinality(card), _depth(0) {
            for(unsigned int i = 0; i < 128; ++i) {
                _count[i] = 0;
            }
        }

        void inspect(Element **, unsigned int count) {
            _count[_depth] = count;
            _leaf = true;
        }

        void enter(const Region &) {
            ++_depth;
            _count[_depth] = 0;
        }

        void exit(const Region &) {
            --_depth;
            _count[_depth] += _count[_depth + 1];
            std::cout << _depth << " : " << _count[_depth + 1];
            if(!_leaf) {
                if(_count[_depth+1] < _cardinality) {
                    std::cout << " }}";
                }
            } else {
                std::cout << " >";
            }
            std::cout << std::endl;
            _leaf = false;
        }
    private:
        bool _leaf;
        unsigned int _cardinality;
        unsigned int _count[128];
        unsigned int _depth;
};

class MemoryInspector {
    public:
        void init() {
            std::cout << "digraph G {" << std::endl;
            std::cout << "node [width=0.3 height=0.3 label=\"\"]"
                << std::endl;
        }

        void visit(Headless::Logic::SearchTree::Node<glm::vec2, Region, Element>* target,
                const Region* region, Element** elements,
                Headless::Logic::SearchTree::Node<glm::vec2, Region, Element>** nodes,
                Headless::Logic::SearchTree::Node<glm::vec2, Region, Element>* parent,
                bool leaf,
                unsigned int count,
                unsigned int cardinality) {
            // Tree topology.
            if(parent != nullptr) {
                std::cout << "    \"" << parent << "\" -> \"";
                std::cout << target << "\";" << std::endl;
            }
        }

        void close() {
            std::cout << "}" << std::endl;
        }
};


#endif
