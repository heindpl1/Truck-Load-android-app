#pragma once
#include <Arduino.h>
#include <math.h>
namespace truck {
enum class VehicleState:uint8_t { NO_FIX, AT_PLANT, DRIVING, STOPPED_AWAY };
inline const char* vehicleStateName(VehicleState s){
  switch(s){case VehicleState::AT_PLANT:return"AT_PLANT";case VehicleState::DRIVING:return"DRIVING";case VehicleState::STOPPED_AWAY:return"STOPPED_AWAY";default:return"NO_FIX";}
}
class VehicleStateMachine {
 public:
  VehicleState update(bool fix,float mph,double plantM,bool plantSet,float radiusM){
    if(!fix)return state_=VehicleState::NO_FIX;
    if(plantSet&&!isnan(plantM)&&plantM<=radiusM)return state_=VehicleState::AT_PLANT;
    if(mph>=3.0f)return state_=VehicleState::DRIVING;
    return state_=VehicleState::STOPPED_AWAY;
  }
  VehicleState state()const{return state_;}
 private: VehicleState state_=VehicleState::NO_FIX;
};
}
