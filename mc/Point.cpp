#include "Point.h"

Point::Point() {}

Point::Point(int segment, int remainder) 
  : m_segment(segment),
    m_remainder(remainder) {
  
}

int Point::getSegment() const {
  return this->m_segment;
}

int Point::getRemainder() const {
  return this->m_remainder;
}


void Point::setSegment(int segment) {
  this->m_segment = segment;
}

void Point::setRemainder(int remainder) {
  this->m_remainder = remainder;
}
