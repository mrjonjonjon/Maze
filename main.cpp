#include <SFML/Graphics.hpp>

const int WIDTH = 800;
const int HEIGHT = 600;
const int CELL_SIZE = 10;
const int LINE_THICKNESS = 1;

class Cell
{
public:
    Cell(int x, int y) : x(x), y(y), top(true), right(true), bottom(true), left(true) {}

    void draw(sf::RenderWindow &window)
    {
        sf::RectangleShape line(sf::Vector2f(CELL_SIZE, LINE_THICKNESS)); // Horizontal line

        line.setFillColor(sf::Color::Yellow);
        if (top)
        {
            line.setPosition(x, y);
            window.draw(line);
        }
        if (bottom)
        {
            line.setPosition(x, y + CELL_SIZE);
            window.draw(line);
        }

        line.setSize(sf::Vector2f(LINE_THICKNESS, CELL_SIZE)); // Vertical line
        if (left)
        {
            line.setPosition(x, y);
            window.draw(line);
        }
        if (right)
        {
            line.setPosition(x + CELL_SIZE, y);
            window.draw(line);
        }
    }

    bool top, right, bottom, left;
    bool visited = false;

    int x, y;
};

class Player
{
public:
    Player(float x, float y) : position(x, y)
    {
        shape.setSize(sf::Vector2f((float)CELL_SIZE / 1.5, (float)CELL_SIZE / 1.5)); // Slightly smaller than the cell
        shape.setFillColor(sf::Color::Green);
        shape.setPosition(x + shape.getSize().x / 2, y + shape.getSize().y / 2); // Small offset to center the player in the cell
    }

    void move(float dx, float dy)
    {
        position.x += dx;
        position.y += dy;
        shape.setPosition(position.x - (shape.getSize().x - (float)CELL_SIZE) / 2, position.y - (shape.getSize().y - (float)CELL_SIZE) / 2);
    }

    void draw(sf::RenderWindow &window)
    {
        window.draw(shape);
    }

    sf::Vector2f position;
    sf::RectangleShape shape;
};

#include <SFML/Graphics.hpp>
#include <cstdlib> // For rand()

void generateMaze(Cell *cells[][WIDTH / CELL_SIZE], int startX, int startY)
{
    std::vector<Cell *> stack;
    Cell *current = cells[startY][startX];
    current->visited = true;
    stack.push_back(current);

    // Lambda function to get unvisited neighbors
    auto getUnvisitedNeighbors = [&](Cell *cell)
    {
        std::vector<std::pair<Cell *, std::string>> neighbors;

        int x = cell->x / CELL_SIZE;
        int y = cell->y / CELL_SIZE;

        if (y > 0 && !cells[y - 1][x]->visited)
            neighbors.push_back({cells[y - 1][x], "top"});
        if (y < HEIGHT / CELL_SIZE - 1 && !cells[y + 1][x]->visited)
            neighbors.push_back({cells[y + 1][x], "bottom"});
        if (x > 0 && !cells[y][x - 1]->visited)
            neighbors.push_back({cells[y][x - 1], "left"});
        if (x < WIDTH / CELL_SIZE - 1 && !cells[y][x + 1]->visited)
            neighbors.push_back({cells[y][x + 1], "right"});

        return neighbors;
    };

    while (!stack.empty())
    {
        current = stack.back();
        auto neighbors = getUnvisitedNeighbors(current);

        if (!neighbors.empty())
        {
            int randIndex = rand() % neighbors.size();
            auto [next, direction] = neighbors[randIndex];

            if (direction == "top")
            {
                current->top = false;
                next->bottom = false;
            }
            else if (direction == "bottom")
            {
                current->bottom = false;
                next->top = false;
            }
            else if (direction == "left")
            {
                current->left = false;
                next->right = false;
            }
            else if (direction == "right")
            {
                current->right = false;
                next->left = false;
            }

            next->visited = true;
            stack.push_back(next);
        }
        else
        {
            stack.pop_back();
        }
    }
}

int main()
{

    const sf::Time MOVE_COOLDOWN = sf::milliseconds(100); // Adjust as needed
    sf::Clock moveClock;
    // Get the screen resolution
    sf::VideoMode screenResolution = sf::VideoMode::getDesktopMode();

    // Create a fullscreen window using the screen resolution
    // sf::RenderWindow window(screenResolution, "Fullscreen Window", sf::Style::Fullscreen);
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "SFML Maze");

    // Create a 2D array of cells
    Cell *cells[HEIGHT / CELL_SIZE][WIDTH / CELL_SIZE];
    for (int i = 0; i < HEIGHT / CELL_SIZE; ++i)
    {
        for (int j = 0; j < WIDTH / CELL_SIZE; ++j)
        {
            cells[i][j] = new Cell(j * CELL_SIZE, i * CELL_SIZE);
        }
    }

    generateMaze(cells, 0, 0);

    Player player(0, 0); // Starting at top-left with a small offset

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        float dx = 0, dy = 0;
        sf::Time timeSinceLastMove = moveClock.getElapsedTime();

        if (timeSinceLastMove >= MOVE_COOLDOWN)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            {
                dy = -CELL_SIZE;
                moveClock.restart();
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            {
                dx = -CELL_SIZE;
                moveClock.restart();
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            {
                dy = CELL_SIZE;
                moveClock.restart();
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            {
                dx = CELL_SIZE;
                moveClock.restart();
            }
        }

        // The rest of your movement and boundary checking logic...

        int cellY = (player.position.y + dy) / CELL_SIZE;
        int cellX = (player.position.x + dx) / CELL_SIZE;
        if (cellY < 0 || cellY >= HEIGHT / CELL_SIZE)
            continue;
        if (cellX < 0 || cellX >= WIDTH / CELL_SIZE)
            continue;

        if (dy == -CELL_SIZE && cells[cellY][cellX]->bottom)
            continue;
        if (dy == CELL_SIZE && cells[cellY][cellX]->top)
            continue;
        if (dx == -CELL_SIZE && cells[cellY][cellX]->right)
            continue;
        if (dx == CELL_SIZE && cells[cellY][cellX]->left)
            continue;

        // Assuming you're moving horizontally

        player.move(dx, dy);

        window.clear(sf::Color::Black); // Sets background color to black

        // Draw the maze
        for (int i = 0; i < HEIGHT / CELL_SIZE; ++i)
        {
            for (int j = 0; j < WIDTH / CELL_SIZE; ++j)
            {
                cells[i][j]->draw(window);
            }
        }
        player.draw(window);

        window.display();
    }

    // Cleanup
    for (int i = 0; i < HEIGHT / CELL_SIZE; ++i)
    {
        for (int j = 0; j < WIDTH / CELL_SIZE; ++j)
        {
            delete cells[i][j];
        }
    }

    return 0;
}
