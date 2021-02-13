#pragma once

class Action {

public:
    Action(int _a) : a(_a) {}

    int getA() const {
        return a;
    }
private:
    int a;
};



