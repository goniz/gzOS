#include "TcpState.h"

TcpState::TcpState(TcpStateEnum stateEnum, TcpSession& session)
    : _stateEnum(stateEnum),
      _session(session)
{

}

TcpStateEnum TcpState::state_enum(void) const {
    return _stateEnum;
}

bool TcpState::handle_output_trigger(void) {
    return false;
}

bool TcpState::has_second_stage(void) const {
    return false;
}

void TcpState::handle_second_stage(void) {

}
