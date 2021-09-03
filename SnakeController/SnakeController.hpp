#pragma once

#include <list>
#include <memory>
#include <functional>
#include <stdexcept>

#include "IEventHandler.hpp"
#include "SnakeInterface.hpp"

class Event;
class IPort;

namespace Snake
{
struct ConfigurationError : std::logic_error
{
    ConfigurationError();
};

struct UnexpectedEventException : std::runtime_error
{
    UnexpectedEventException();
};

//class World;
//class SnakeSegment;

class Controller : public IEventHandler
{
public:
    Controller(IPort& p_displayPort, IPort& p_foodPort, IPort& p_scorePort, std::string const& p_config);

    Controller(Controller const& p_rhs) = delete;
    Controller& operator=(Controller const& p_rhs) = delete;

    void receive(std::unique_ptr<Event> e) override;

private:
    // Controller
    IPort& m_displayPort;
    IPort& m_foodPort;
    IPort& m_scorePort;
    bool m_paused;

    // world
    //std::pair<int, int> m_mapDimension;
    //std::pair<int, int> m_foodPosition;

    // Smake
    /*
    struct Segment
    {
        int x;
        int y;
    };
    */

    // std::list<Segment> m_segments;
    // Direction m_currentDirection;

    // World
    /*
    void handleTimeoutInd();
    void handleDirectionInd(std::unique_ptr<Event>);
    void handleFoodInd(std::unique_ptr<Event>);
    void handleFoodResp(std::unique_ptr<Event>);
    void handlePauseInd(std::unique_ptr<Event>);
    */

    // Snake
    /*
    bool isSegmentAtPosition(int x, int y) const;
    Segment calculateNewHead() const;
    void updateSegmentsIfSuccessfullMove(Segment const& newHead);
    void addHeadSegment(Segment const& newHead);
    void removeTailSegmentIfNotScored(Segment const& newHead);
    void removeTailSegment();
    */

    // World
    /*
    bool isPositionOutsideMap(int x, int y) const;
    void updateFoodPosition(int x, int y, std::function<void()> clearPolicy);
    void sendClearOldFood();
    void sendPlaceNewFood(int x, int y);
    */

    // Controller
};

class World
{
private:
    std::pair<int, int> m_mapDimension_;
    std::pair<int, int> m_foodPosition_;

    bool isPositionOutsideMap(int x, int y) const;
    void updateFoodPosition(int x, int y, std::function<void()> clearPolicy);
    void sendClearOldFood();
    void sendPlaceNewFood(int x, int y);
public:
    //World() = default;
    void setmapDimension(std::pair<int, int> md ) { m_mapDimension_ = md; }
    void setfoodPosition(std::pair<int, int> fp ) { m_foodPosition_ = fp; }

    std::pair<int, int> getmapDimension() const { return m_mapDimension_; }
    std::pair<int, int> getfoodPosition() const { return m_foodPosition_; }

    void handleTimeoutInd();
    void handleDirectionInd(std::unique_ptr<Event>);
    void handleFoodInd(std::unique_ptr<Event>);
    void handleFoodResp(std::unique_ptr<Event>);
    void handlePauseInd(std::unique_ptr<Event>);
};

class SnakeSegment
{
private:
    struct Segment
    {
        int x;
        int y;
    };

    std::list<Segment> m_segments;
    Direction m_currentDirection;

    bool isSegmentAtPosition(int x, int y) const;
    Segment calculateNewHead() const;
    void updateSegmentsIfSuccessfullMove(Segment const& newHead);
    void addHeadSegment(Segment const& newHead);
    void removeTailSegmentIfNotScored(Segment const& newHead);
    void removeTailSegment();
}
public:
    void setSegments( std::list<Segment> sg ) { m_segments = sg; }
    void setCurrentDirection( Direction cd ) { m_currentDirection = cd; }

    std::list<Segment> getSegments() const { return m_segments; }
    Direction getCurrentDirection() const { return m_currentDirection; }

} // namespace Snake
