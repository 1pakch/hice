#pragma once

namespace hc {

template<class Value, class Tag>
struct Tagged {
    Value value;
    Tag tag;
};

template<class Value, class Tag>
auto tagged(Value v, Tag t) {
    return Tagged<Value, Tag>{v, t};
}

}
