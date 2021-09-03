#include "SnakeController.hpp"

#include <algorithm>
#include <sstream>

#include "EventT.hpp"
#include "IPort.hpp"

namespace Snake
{
class World world;
class SnakeSegment snake;

ConfigurationError::ConfigurationError()
    : std::logic_error("Bad configuration of Snake::Controller.")
{}

UnexpectedEventException::UnexpectedEventException()
    : std::runtime_error("Unexpected event received!")
{}

Controller::Controller(IPort& p_displayPort, IPort& p_foodPort, IPort& p_scorePort, std::string const& p_config)
    : m_displayPort(p_displayPort),
      m_foodPort(p_foodPort),
      m_scorePort(p_scorePort),
      m_paused(false)
{
    std::istringstream istr(p_config);
    char w, f, s, d;

    int width, height, length;
    int foodX, foodY;
    istr >> w >> width >> height >> f >> foodX >> foodY >> s;

    if (w == 'W' and f == 'F' and s == 'S') {
        //m_mapDimension = std::make_pair(width, height);
        //m_foodPosition = std::make_pair(foodX, foodY);
        
        world.setmapDimension(std::make_pair(width, height));
        world.setfoodPosition(std::make_pair(foodX, foodY));

        istr >> d;
        switch (d) {
            case 'U':
                m_currentDirection = Direction_UP;
                break;
            case 'D':
                m_currentDirection = Direction_DOWN;
                break;
            case 'L':
                m_currentDirection = Direction_LEFT;
                break;
            case 'R':
                m_currentDirection = Direction_RIGHT;
                break;
            default:
                throw ConfigurationError();
        }
        istr >> length;

        while (length--) {
            Segment seg;
            istr >> seg.x >> seg.y;
            m_segments.push_back(seg);
        }
    } else {
        throw ConfigurationError();
    }
}

bool SnakeSegment::isSegmentAtPosition(int x, int y) const
{
    return m_segments.end() !=  std::find_if(m_segments.cbegin(), m_segments.cend(),
        [x, y](auto const& segment){ return segment.x == x and segment.y == y; });
}

bool World::isPositionOutsideMap(int x, int y) const
{
    // return x < 0 or y < 0 or x >= m_mapDimension.first or y >= m_mapDimension.second;
    return x < 0 or y < 0 or x >= world.getfoodPosition().first or y >= world.getfoodPosition().second;
}

void World::sendPlaceNewFood(int x, int y)
{
    //m_foodPosition = std::make_pair(x, y);
    world.setfoodPosition(std::make_pair(x, y));

    DisplayInd placeNewFood;
    placeNewFood.x = x;
    placeNewFood.y = y;
    placeNewFood.value = Cell_FOOD;

    m_displayPort.send(std::make_unique<EventT<DisplayInd>>(placeNewFood));
}

void World::sendClearOldFood()
{
    DisplayInd clearOldFood;
    //clearOldFood.x = m_foodPosition.first;
    //clearOldFood.y = m_foodPosition.second;

    clearOldFood.x = world.getfoodPosition().first;
    clearOldFood.y = world.getfoodPosition().second;

    clearOldFood.value = Cell_FREE;

    m_displayPort.send(std::make_unique<EventT<DisplayInd>>(clearOldFood));
}

namespace
{
bool isHorizontal(Direction direction)
{
    return Direction_LEFT == direction or Direction_RIGHT == direction;
}

bool isVertical(Direction direction)
{
    return Direction_UP == direction or Direction_DOWN == direction;
}

bool isPositive(Direction direction)
{
    return (isVertical(direction) and Direction_DOWN == direction)
        or (isHorizontal(direction) and Direction_RIGHT == direction);
}

bool perpendicular(Direction dir1, Direction dir2)
{
    return isHorizontal(dir1) == isVertical(dir2);
}
} // namespace

SnakeSegment::Segment SnakeSegment::calculateNewHead() const
{
    Segment const& currentHead = m_segments.front();

    Segment newHead;
    newHead.x = currentHead.x + (isHorizontal(m_currentDirection) ? isPositive(m_currentDirection) ? 1 : -1 : 0);
    newHead.y = currentHead.y + (isVertical(m_currentDirection) ? isPositive(m_currentDirection) ? 1 : -1 : 0);

    return newHead;
}

void SnakeSegment::removeTailSegment()
{
    auto tail = m_segments.back();

    DisplayInd l_evt;
    l_evt.x = tail.x;
    l_evt.y = tail.y;
    l_evt.value = Cell_FREE;
    m_displayPort.send(std::make_unique<EventT<DisplayInd>>(l_evt));

    m_segments.pop_back();
}

void SnakeSegment::addHeadSegment(SnakeSegment::Segment const& newHead)
{
    m_segments.push_front(newHead);

    DisplayInd placeNewHead;
    placeNewHead.x = newHead.x;
    placeNewHead.y = newHead.y;
    placeNewHead.value = Cell_SNAKE;

    m_displayPort.send(std::make_unique<EventT<DisplayInd>>(placeNewHead));
}

void SnakeSegment::removeTailSegmentIfNotScored(SnakeSegment::Segment const& newHead)
{
    //if (std::make_pair(newHead.x, newHead.y) == m_foodPosition)
    if (std::make_pair(newHead.x, newHead.y) == world.getfoodPosition()) {
        m_scorePort.send(std::make_unique<EventT<ScoreInd>>());
        m_foodPort.send(std::make_unique<EventT<FoodReq>>());
    } else {
        snake.removeTailSegment();
    }
}

void SnakeSegment::updateSegmentsIfSuccessfullMove(SnakeSegment::Segment const& newHead)
{
    if (snake.isSegmentAtPosition(newHead.x, newHead.y) or world.isPositionOutsideMap(newHead.x, newHead.y)) {
        m_scorePort.send(std::make_unique<EventT<LooseInd>>());
    } else {
        snake.addHeadSegment(newHead);
        snake.removeTailSegmentIfNotScored(newHead);
    }
}

void World::handleTimeoutInd()
{
    snake.updateSegmentsIfSuccessfullMove(snake.calculateNewHead());
}

void World::handleDirectionInd(std::unique_ptr<Event> e)
{
    auto direction = payload<DirectionInd>(*e).direction;

    if (perpendicular(m_currentDirection, direction)) {
        m_currentDirection = direction;
    }
}

void World::updateFoodPosition(int x, int y, std::function<void()> clearPolicy)
{
    if (snake.isSegmentAtPosition(x, y) || world.isPositionOutsideMap(x,y)) {
        m_foodPort.send(std::make_unique<EventT<FoodReq>>());
        return;
    }

    clearPolicy();
    world.sendPlaceNewFood(x, y);
}

void World::handleFoodInd(std::unique_ptr<Event> e)
{
    auto receivedFood = payload<FoodInd>(*e);

    world.updateFoodPosition(receivedFood.x, receivedFood.y, std::bind(&World::sendClearOldFood, this));
}

void World::handleFoodResp(std::unique_ptr<Event> e)
{
    auto requestedFood = payload<FoodResp>(*e);

    world.updateFoodPosition(requestedFood.x, requestedFood.y, []{});
}

void World::handlePauseInd(std::unique_ptr<Event> e)
{
    m_paused = not m_paused;
}

void Controller::receive(std::unique_ptr<Event> e)
{
    switch (e->getMessageId()) {
        case TimeoutInd::MESSAGE_ID:
            if (!m_paused) {
                return world.handleTimeoutInd();
            }
            return;
        case DirectionInd::MESSAGE_ID:
            if (!m_paused) {
                return world.handleDirectionInd(std::move(e));
            }
            return;
        case FoodInd::MESSAGE_ID:
            return world.handleFoodInd(std::move(e));
        case FoodResp::MESSAGE_ID:
            return world.handleFoodResp(std::move(e));
        case PauseInd::MESSAGE_ID:
            return world.handlePauseInd(std::move(e));
        default:
            throw UnexpectedEventException();
    }
}

} // namespace Snake
