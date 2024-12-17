#include "hfsm.hpp"

#include "../hlt/log.hpp"


HFSM::HFSM()
{
	hlt::log::log("BONDOUR HFSM");
};

HFSM::~HFSM()
{
	hlt::log::log("AUREVOIE HFSM");
};

#include <cstdarg>  // used for variables args in FSM_STATE constructor

class FSM_STATE;  // pre declaration to use in FSM_TRANSITION

//--------------------
class FSM_TRANSITION {
    //---
public:
    typedef float (*TRANSITION_CBK)(void* data);
    //---
private:
    TRANSITION_CBK  TransitionCbk;
    FSM_STATE* OutputSate;
    FSM_TRANSITION() {}
    //---
public:
    FSM_TRANSITION(TRANSITION_CBK cbk, FSM_STATE* outputState) {
        TransitionCbk = cbk;
        OutputSate = outputState;
    }
    //---
    ~FSM_TRANSITION() {}
    //---
    float Evaluate(void* data) {
        if (TransitionCbk)
            return TransitionCbk(data);
        return 0.f;
    }
    //---
    FSM_STATE* GetOutputState() { return OutputSate; }
};
//--------------------
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
    FSM_STATE() { SubFSM = 0; }
    //---
public:
    ~FSM_STATE() {
        if (Transitions)
            delete[] Transitions;
    }
    //---
    FSM_STATE(BEHAVIOR_CBK cbk) { BehaviorCbk = cbk; TransitionsCount = 0; SubFSM = 0; }
    FSM_STATE(FSM* subFSM) { BehaviorCbk = 0; TransitionsCount = 0; SubFSM = subFSM; }
    //---
    void InitTransitions(size_t count, ...) {
        if (TransitionsCount) // already initialized
            return;
        TransitionsCount = count;
        va_list transitionsList;          // create the variable arguments list
        va_start(transitionsList, count); // init the variable arguments list
        Transitions = new FSM_TRANSITION * [TransitionsCount];  // initialize the array of transitions
        for (size_t iTransition = 0; iTransition < TransitionsCount; ++iTransition) {
            Transitions[iTransition] = va_arg(transitionsList, FSM_TRANSITION*);  // set each transition
        }
        va_end(transitionsList);  // close the variable arguments list
    }
    //---
    FSM_STATE* Evaluate(void* data) {
        float           bestTransitionScore = 0.f;
        FSM_TRANSITION* bestTransition = 0;
        for (size_t iTransition = 0; iTransition < TransitionsCount; ++iTransition) {
            FSM_TRANSITION* transition = Transitions[iTransition];
            float score = transition->Evaluate(data);
            if (score > bestTransitionScore) {
                bestTransitionScore = score;
                bestTransition = transition;
            }
        }
        if (bestTransition)
            return bestTransition->GetOutputState();
        return this;  // Default state is self
    }
    //---
    FSM_STATE* Behave(void* data) {  // Using FSM, so this code should be in a cpp file
        if (BehaviorCbk)
            BehaviorCbk(data);
        else if (SubFSM)
            return SubFSM->Behave(data);
        return this;
    }
    //---
    void  Reset() {  // Using FSM, so this code should be in a cpp file
        if (SubFSM) SubFSM->Reset();
    }
};
//--------------------
class FSM {
    //---
    FSM_STATE** States;
    FSM_STATE* CurrentState;
    size_t      StatesCount;
    //---
    FSM() {};
    //---
public:
    ~FSM() {
        if (States)
            delete[] States;
    };
    //---
    FSM(size_t count, ...) {
        StatesCount = count;
        CurrentState = 0;
        va_list statesList;          // create the variable arguments list
        va_start(statesList, count); // init the variable arguments list
        States = new FSM_STATE * [StatesCount];  // initialize the array of transitions
        for (size_t iState = 0; iState < StatesCount; ++iState) {
            States[iState] = va_arg(statesList, FSM_STATE*);  // set each transition
        }
        va_end(statesList);  // close the variable arguments list
    };
    //---
    FSM_STATE* Evaluate(void* data) {
        if (!CurrentState) {
            if (!StatesCount)
                return 0;
            CurrentState = States[0];
        }
        FSM_STATE* newCurrentState = CurrentState->Evaluate(data);
        // If new state is different, reset the previous active state (important if it includes a sub FSM)
        if (CurrentState != newCurrentState)
            CurrentState->Reset();
        CurrentState = newCurrentState;
        // It is possible to call Behave here directly
        return CurrentState;
    }
    //---
    void  Reset() { if (CurrentState) CurrentState->Reset(); CurrentState = 0; }
    //---
    FSM_STATE* Behave(void* data) { if (CurrentState) return CurrentState->Behave(data); return CurrentState; }
};