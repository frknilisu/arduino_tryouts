#ifndef POINT_H
#define POINT_H

#include <Arduino.h>

class Point {
  public:
    Point();
    Point(int segment, int remainder);
    int getSegment() const;
    int getRemainder() const;
    void setSegment(int segment);
    void setRemainder(int remainder);
    
  private:
    int m_segment;      // counter
    int m_remainder;    // raw encoder
};

#endif
