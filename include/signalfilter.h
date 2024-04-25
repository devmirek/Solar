#pragma once

template <typename T>
class Median3Filter  {
public:
  Median3Filter() {
    _samples[0]=0;
    _samples[1]=0;
    _samples[2]=0;
  }

  T medianFilter(T sample) {
    _samples[0] = _samples[1];
    _samples[1] = _samples[2];
    _samples[2] = sample;
    T median;
    if (_samples[2] < _samples[1]) {
      if (_samples[2] < _samples[0]) {
        if (_samples[1] < _samples[0])
          median = _samples[1];
        else
          median = _samples[0];
      }
      else
        median = _samples[2];
    } else {
      if (_samples[2] < _samples[0])
        median = _samples[2];
      else {
        if (_samples[1] < _samples[0])
          median = _samples[0];
        else
          median = _samples[1];
      }
    }
    return median;
  }
private:
  T _samples[3];
};


template <typename T, typename Total, uint16_t N>
class MovingAverage {
public:
  MovingAverage() : _total(0){
    _buffPos=0;
    _buffSize=0;
  }
  T movingAverageFilter(T sample) {
    _total += sample;
    //Serial.println( "ma[" + String(_buffPos) + "] total=" + String( _total) + " size=" + String(_buffSize));
    if (_buffSize < N)
      _samples[_buffPos] = sample;
    else {  //full buffer - subtract the latest value
      T oldest = _samples[_buffPos];
      _total -= oldest;
      _samples[_buffPos] = sample;  //overwrite the oldest value
    }
    if ( _buffSize < N)
      _buffSize++;
    _buffPos = (_buffPos+1) % N;
    /*for (int i = 0; i < N; i++)
      Serial.print( String(_samples[i]) + ",");
    Serial.println();*/
    return _total / _buffSize;
  }
private:
  T _samples[N];
  uint16_t _buffPos;
  uint16_t _buffSize;
  Total _total;
};
