enum State { STATE_OK, STATE_WORKING, STATE_DONE };
enum RetStatus { RET_OK, RET_FAIL };

int isOk(enum State state) {
    return state == RET_OK;
}
