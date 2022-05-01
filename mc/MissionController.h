#ifndef MISSION_CONTROLLER_H
#define MISSION_CONTROLLER_H

class MissionController {
  public:
    MissionController();
    void runLoop();
  private:
    enum class States {
      MANUAL,
      PROGRAMMING,
      ACTION,
      ERROR
    };
    
    States currentState = States::MANUAL;

    void setStartProgramming();
    void setFinishProgramming();
    void setA();
    void setB();
};

#endif
