#include "fsm.hpp"

#include "../hlt/log.hpp"

FSM_TRANSITION::FSM_TRANSITION()
{
    OutputSate = nullptr;
    TransitionCbk = nullptr;
}

FSM_TRANSITION::FSM_TRANSITION(TRANSITION_CBK cbk, FSM_STATE* outputState)
{
    TransitionCbk = cbk;
    OutputSate = outputState;
}

FSM_TRANSITION::~FSM_TRANSITION() 
{

}

float FSM_TRANSITION::Evaluate(void* data)
{
    //hlt::log::log("FSM_TRANSITION::Evaluate");

    //if(TransitionCbk)
        //hlt::log::log("FSM_TRANSITION : OUI");

    if (TransitionCbk)
        return TransitionCbk(data);
    return 0.f;
}

FSM_STATE* FSM_TRANSITION::GetOutputState()
{
    return OutputSate;
}




FSM_STATE::FSM_STATE()
{
    BehaviorCbk = nullptr;
    TransitionsCount = 0;
    SubFSM = 0;

    Transitions = nullptr;
}

FSM_STATE::~FSM_STATE()
{
    if (Transitions)
        delete[] Transitions;
}

FSM_STATE::FSM_STATE(BEHAVIOR_CBK cbk)
{
    BehaviorCbk = cbk;
    TransitionsCount = 0;
    SubFSM = 0;
    Transitions = nullptr;
}

FSM_STATE::FSM_STATE(FSM* subFSM)
{
    BehaviorCbk = 0;
    TransitionsCount = 0;
    SubFSM = subFSM;
    Transitions = nullptr;
}

void FSM_STATE::InitTransitions(size_t count, ...) {
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

FSM_STATE* FSM_STATE::Evaluate(void* data) {
    //hlt::log::log("FSM_STATE::Evaluate");
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

FSM_STATE* FSM_STATE::Behave(void* data) {
    if (BehaviorCbk)
        BehaviorCbk(data);
    else if (SubFSM)
        return SubFSM->Behave(data);
    return this;
}

void FSM_STATE::Reset() {
    if (SubFSM) SubFSM->Reset();
}




FSM::FSM()
{
    CurrentState = nullptr;
    States = nullptr;
    StatesCount = 0;
};

FSM::~FSM() {
    if (States)
        delete[] States;
};

FSM::FSM(size_t count, ...) {
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

FSM_STATE* FSM::Evaluate(void* data) {
    //hlt::log::log("FSM::Evaluate");
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

void FSM::Reset()
{
    if (CurrentState)
        CurrentState->Reset(); 
    CurrentState = 0;
}

FSM_STATE* FSM::Behave(void* data)
{ 
    if (CurrentState) 
        return CurrentState->Behave(data); 
    return CurrentState;
}