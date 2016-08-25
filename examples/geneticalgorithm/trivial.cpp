#include <geneticalgorithm.hpp>
#include <iostream>
#include <string>
#include <random>
#include <tuple>

#define POOL_SIZE 256
#define MAX_GENERATION 1000000
#define MIN_ERROR 0.08

#define DATA_LENGTH 32

std::random_device *s_rd;
std::mt19937 *s_mt;
std::uniform_int_distribution<char> s_upperDist('A', 'Z');
std::uniform_int_distribution<char> s_lowerDist('a', 'z');
std::uniform_int_distribution<int> s_cass(0, 1);
std::uniform_real_distribution<double> s_range(0, 1);

// Candidate -----------------------------------------------------------------
class Candidate {
    public:
        Candidate();
        Candidate(const Candidate &);
        char *data() { return _data; }
        double distance(const Candidate &);
        const Candidate &operator=(const Candidate &);
    private:
        char _data[DATA_LENGTH];
};

Candidate::Candidate() {
    for(unsigned int i = 0; i < DATA_LENGTH; ++i) {
        _data[i] = '\0';
    }
}

Candidate::Candidate(const Candidate &orig) {
    for(unsigned int i = 0; i < DATA_LENGTH; ++i) {
        _data[i] = orig._data[i];
    }
}

double Candidate::distance(const Candidate &other) {
    double dist = 0;
    // Trivial distance computation.
    for(unsigned int i = 0; i < DATA_LENGTH; ++i) {
        int current = _data[i] - other._data[i];
        if(current != 0) {
            dist += current<0?-current:current;
        }
    }
    return dist/7.0;
}

const Candidate& Candidate::operator=(const Candidate &other) {
    for(unsigned int i = 0; i < DATA_LENGTH; ++i) {
        _data[i] = other._data[i];
    }
    return *this;
}

// Environment ---------------------------------------------------------------
class Environment {
    public:
        void set(Candidate &goal);
        void reserve(Candidate**&, unsigned int);
        void release(Candidate**, unsigned int);
        double evaluate(const Candidate *);
        Candidate *clone(const Candidate *);
    private:
        Candidate _goal;
};

void Environment::set(Candidate &goal) {
    _goal = goal;
}

void Environment::reserve(Candidate**& buffer, unsigned int size) {
    // perform an uniform distribution.
    for(unsigned int i = 0; i < size; ++i) {
        Candidate *candidate = new Candidate();
        char *data = candidate->data();
        for(unsigned int j = 0; j < (DATA_LENGTH - 1); ++j) {
            if(s_cass(*s_mt) == 1) { // upper case
                data[j] = s_upperDist(*s_mt);
            } else {
                data[j] = s_lowerDist(*s_mt);
            }
        }
        data[DATA_LENGTH - 1] = '\0';
        buffer[i] = candidate;
    }
}

void Environment::release(Candidate** buffer, unsigned int size) {
    for(unsigned int i = 0; i < size; ++i) {
        delete buffer[i];
    }
}

double Environment::evaluate(const Candidate *candidate) {
    return _goal.distance(*candidate);
}

Candidate *Environment::clone(const Candidate *candidate) {
    return new Candidate(*candidate);
}

// Mate Mutator --------------------------------------------------------------
class MateMutator {
    public:
        double threshold();
        void mutate(Candidate**, double*, double, unsigned int, Candidate*);
};

double MateMutator::threshold() { return 0.8; }

void MateMutator::mutate(Candidate** parents, double* score, double total, unsigned int size, Candidate* offspring) {
    // Choose two parents.
    double position = s_range(*s_mt) * total;
    unsigned int index;
    double cumulator = 0;
    for(index = 0; cumulator < position; cumulator += score[index++]);
    unsigned int mate;
    position = s_range(*s_mt) * total;
    cumulator = 0;
    for(mate = 0; cumulator < position; cumulator += score[mate++]);
    if(index == mate) { // Ugly, ugly, ugly ...
        if(mate == 0) {
            mate = 1;
        } else if(mate == size - 1) {
            mate = size - 2;
        } else {
            mate = index + 1;
        }
    }
    char *father = parents[index]->data();
    char *mother = parents[mate]->data();
    char *junior = offspring->data();

    for(unsigned int i = 0; i < DATA_LENGTH - 1; ++i) {
        junior[i] = (s_cass(*s_mt) == 0)?father[i]:mother[i];
    }
}

// Classic Mutator -----------------------------------------------------------
class ClassicMutator {
    public:
        double threshold();
        void mutate(Candidate**, double*, double, unsigned int, Candidate*);
};


double ClassicMutator::threshold() { return 0.3; }

void ClassicMutator::mutate(Candidate** parents, double* score, double total, unsigned int size, Candidate* offspring) {
    // Let's take one of the offspring and mutate its genes !
    double position = s_range(*s_mt) * total;
    unsigned int index;
    double cumulator = 0;
    for(index = 0; cumulator < position; cumulator += score[index++]);
    Candidate *parent = parents[index];
    *offspring = *parent; // Copy ...
    // ... and mutate one of the character.
    index = static_cast<unsigned int>(s_range(*s_mt) * (DATA_LENGTH - 1));
    char *data = offspring->data();
    if(s_cass(*s_mt) == 1) { // upper case
        data[index] = s_upperDist(*s_mt);
    } else {
        data[index] = s_lowerDist(*s_mt);
    }
}

// Visitor -------------------------------------------------------------------
class Visitor {
    public:
        void visit(Candidate **pool, unsigned int size) {
            std::cout << "----------" << std::endl;
            for(unsigned int i = 0; i < size; ++i) {
                std::cout << "#" << i << " "
                    << pool[i]->data() << std::endl;
            }
        }
};

// Example Entry Point -------------------------------------------------------
int main(void) {
    s_rd = new std::random_device();
    s_mt = new std::mt19937((*s_rd)());

    Headless::Logic::GA::Trivial<Candidate> engine(POOL_SIZE);

    Environment env;
    MateMutator mate;
    ClassicMutator mutate;
    Visitor visitor;

    Candidate goal;
    char *data = goal.data();
    data[0] = 'T';
    data[1] = 'e';
    data[2] = 's';
    data[3] = 't';
    data[4] = 'i';
    data[5] = 'n';
    data[6] = 'g';
    data[7] = 'A';
    data[8] = 'B';
    data[9] = 'u';
    data[10] = 'n';
    data[11] = 'c';
    data[12] = 'h';
    data[13] = 'O';
    data[14] = 'f';
    data[15] = 'S';
    data[16] = 't';
    data[17] = 'u';
    data[18] = 'f';
    data[19] = 'f';
    data[20] = 'W';
    data[21] = 'i';
    data[22] = 't';
    data[23] = 'h';
    data[24] = 'L';
    data[25] = 'o';
    data[26] = 'n';
    data[27] = 'g';
    data[28] = 'S';
    data[29] = 't';
    data[30] = 'r';
    data[31] = '\0';
    

    env.set(goal);

    Candidate **store = new Candidate*[POOL_SIZE];

    int result;
    double minimum;
    int number;
    std::tie(number, minimum, result) = engine.train(&env, &visitor,
            MAX_GENERATION, MIN_ERROR, 0.1,
            store, POOL_SIZE,
            &mutate, &mate);

    std::cout << "Number of generations : " << number << std::endl;
    std::cout << "Minimal score : " << minimum << std::endl;
    std::cout << "Results " << std::endl;

    for(unsigned int i = 0; i < result && store[i]->data()[0] != '\0'; ++i) {
        std::cout << "#" << i << " : " << store[i]->data() << std::endl;
    }

    for(unsigned int i = 0; i < POOL_SIZE; ++i) {
        delete store[i];
    }
    delete[] store;

    delete s_mt;
    delete s_rd;

    return 0;
}
