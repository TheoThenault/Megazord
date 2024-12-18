#pragma once

/*
	Class générique pour les FSM
*/

/*

    This code has been borrowed from https://www.spirops.com/courses/DAIVG/maef.html

*/

#include <cstdarg>  // used for variables args in FSM_STATE constructor

class FSM_STATE;  // pre declaration to use in FSM_TRANSITION


class FSM_TRANSITION {
    //---
public:
    typedef float (*TRANSITION_CBK)(void* data);
    //---
private:
    TRANSITION_CBK  TransitionCbk;
    FSM_STATE* OutputSate;
    FSM_TRANSITION();
    //---
public:
    FSM_TRANSITION(TRANSITION_CBK cbk, FSM_STATE* outputState);
    //---
    ~FSM_TRANSITION();
    //---
    float Evaluate(void* data);
    //---
    FSM_STATE* GetOutputState();
};


class FSM;  // pre declaration to use in FSM_STATE


class FSM_STATE {
    //---
public:
    typedef void (*BEHAVIOR_CBK)(void* data);
    //---
private:
    BEHAVIOR_CBK    BehaviorCbk;
    FSM* SubFSM;
    FSM_TRANSITION** Transitions;
    size_t          TransitionsCount;
    //---
    FSM_STATE();
    //---
public:
    ~FSM_STATE();
    //---
    FSM_STATE(BEHAVIOR_CBK cbk);
    FSM_STATE(FSM* subFSM);
    //---
    void InitTransitions(size_t count, ...);
    //---
    FSM_STATE* Evaluate(void* data);
    //---
    FSM_STATE* Behave(void* data);
    //---
    void  Reset();
};



class FSM {
    //---
    FSM_STATE** States;
    FSM_STATE* CurrentState;
    size_t      StatesCount;
    //---
    FSM();
    //---
public:
    ~FSM();
    //---
    FSM(size_t count, ...);
    //---
    FSM_STATE* Evaluate(void* data);
    //---
    void  Reset();
    //---
    FSM_STATE* Behave(void* data);
};

