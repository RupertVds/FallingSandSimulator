#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <glm/glm.hpp>
#include <functional>

// Invoke a function at every cell position along the line
template <typename Func>
void BresenhamLine(const glm::ivec2& start, const glm::ivec2& end, Func&& func)
{
    int x0 = start.x;
    int y0 = start.y;
    int x1 = end.x;
    int y1 = end.y;

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1; // Step direction for x
    int sy = (y0 < y1) ? 1 : -1; // Step direction for y
    int err = dx - dy;

    // Calculate the number of steps (equal to the length of the longer side)
    int steps = std::max(dx, dy) + 1;

    for (int i = 0; i < steps; ++i)
    {
        func(x0, y0); // Invoke the function at the current position

        int e2 = 2 * err;
        if (e2 > -dy) { // Step in x direction
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) { // Step in y direction
            err += dx;
            y0 += sy;
        }
    }
}



#endif // UTILS_H
