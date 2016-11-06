#include "common.hpp"


const Region *Region::divide() const {
    Region *ary = new Region[4];
    double width = _boundary.p / 2.0;
    double height = _boundary.q / 2.0;
    double upX = _boundary.x;
    double upY = _boundary.y;
    ary[0]._boundary = glm::vec4(upX, upY, width, height);
    ary[1]._boundary = glm::vec4(upX + width, upY, width, height);
    ary[2]._boundary = glm::vec4(upX + width, upY + height, width, height);
    ary[3]._boundary = glm::vec4(upX, upY + height, width, height);
    return ary;
}

bool Region::contains(const glm::vec2 &key) const {
    bool result = key.x >= _boundary.x &&
        key.x <= _boundary.x + _boundary.p &&
        key.y >= _boundary.y &&
        key.y <= _boundary.y + _boundary.q;
    return result;
}

void Region::diagnostic(const glm::vec2 &key) const {
    std::cout << "Test against " << key.x << ", " << key.y << std::endl;
    std::cout << "Boundary is [" << _boundary.x << " - "
        << _boundary.x + _boundary.p
        << "] : [" << _boundary.y << " - "
        << _boundary.y + _boundary.q << "]" << std::endl;
    if(key.x >= _boundary.x && key.x <= _boundary.x + _boundary.p &&
            key.y >= _boundary.y &&
            key.y <= _boundary.y + _boundary.q) {
        std::cout << "Test Ok" << std::endl;
    } else {
        std::cout << "Not in region" << std::endl;
    }
}

int Region::contains(const Region &region) const {
    // Overlap test.
    // Convert region definition.
    glm::vec4 myRect = glm::vec4(_boundary.x, _boundary.y,
            _boundary.x + _boundary.p,
            _boundary.y + _boundary.q);
    glm::vec4 boundary = region.boundary();
    glm::vec4 testRect =  glm::vec4(boundary.x, boundary.y,
            boundary.x + boundary.p,
            boundary.y + boundary.q);
    bool overlap = (myRect.x <= testRect.p) && (myRect.p >= testRect.x) &&
        (myRect.y <= testRect.q) && (myRect.q >= testRect.y);
    if(overlap) {
        // Test complete containment.
        bool complete = (testRect.x >= myRect.x) && (testRect.y >= myRect.y) &&
            (testRect.p <= myRect.p) && (testRect.q <= myRect.q);
        return complete?1:0;
    } else {
        return -1; // Not contained, no overlap.
    }
}

bool Disc::contains(const glm::vec2& key) const {
    double dx = key.x - _center.x;
    double dy = key.y - _center.y;

    return ((dx * dx) + (dy * dy)) <= _sqradius;
}

int Disc::contains(const Region& region) const {
    // Simplified version.
    glm::vec4 boundary = region.boundary();
    double xMin = boundary.x - _radius;
    double xMax = boundary.x + boundary.p + _radius;
    double yMin = boundary.y - _radius;
    double yMax = boundary.y + boundary.q + _radius;
    return (xMin <= _center.x) && (xMax >= _center.x) &&
        (yMin <= _center.y) && (yMax >= _center.y) ? 0:-1;
    // No full containment test.
}

