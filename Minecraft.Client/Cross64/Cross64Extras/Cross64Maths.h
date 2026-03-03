#pragma once

#include <glm/glm.hpp>

typedef glm::vec4 XMVECTOR;
typedef glm::mat4 XMMATRIX;
typedef glm::vec4 XMFLOAT4;

inline XMMATRIX XMMatrixMultiply(XMMATRIX a, XMMATRIX b) {
    return a * b;
}

inline XMVECTOR XMMatrixDeterminant(XMMATRIX a) {
    return XMVECTOR(glm::determinant(a));
}

inline XMMATRIX XMMatrixInverse(XMVECTOR* a, XMMATRIX b) {
    if (a) *a = XMMatrixDeterminant(b);
    return glm::inverse(b);
}