#include "MTriangle.h"

#include "MathFunctions.h"

MTriangle::MTriangle(double eqArea) {

    setEqArea(eqArea);

}

void MTriangle::calcArea() {
    /*
        This calculation depends on the result of the length calculation of edges.
    */
    
    auto& v0 = _pTriangle->getVertices()[0]->getBead()->coordinate;
    auto& v1 = _pTriangle->getVertices()[1]->getBead()->coordinate;
    auto& v2 = _pTriangle->getVertices()[2]->getBead()->coordinate;

    double l0 = _pTriangle->getEdges()[0]->getMEdge()->getLength(); // length v0 - v1
    double l1 = _pTriangle->getEdges()[1]->getMEdge()->getLength(); // length v1 - v2
    double l2 = _pTriangle->getEdges()[2]->getMEdge()->getLength(); // length v2 - v0

    double dot12 = scalarProduct(v0, v1, v0, v2);

    _currentArea = 0.5 * magnitude(vectorProduct(v0, v1, v0, v2));

    // Calculate gradients
    for(int coordIdx = 0; coordIdx < 3; ++coordIdx) {
        double tmpR01 = v1[coordIdx] - v0[coordIdx];
        double tmpR02 = v2[coordIdx] - v0[coordIdx];

        _dCurrentArea[0][coordIdx] = (-l0*l0*tmpR02 - l2*l2*tmpR01 + dot12*(tmpR01 + tmpR02)) / _currentArea / 4;
        _dCurrentArea[1][coordIdx] = (l2*l2*tmpR01 - dot12*tmpR02) / _currentArea / 4;
        _dCurrentArea[2][coordIdx] = (l0*l0*tmpR02 - dot12*tmpR01) / _currentArea / 4;
    }

}

void MTriangle::calcTheta() {
    /*
        This calculation depends on the result of the length calculation of edges.
    */

    // calculate thetas
    for(int angleIdx = 0; angleIdx < 3; ++angleIdx) {

        // Start from this vertex notated temporarily as v0, and go counter-clockwise
        // All the temporary variables are indexed RELATIVELY, i.e. starting from 0, NOT from angleIdx.
        auto& v0 = _pTriangle->getVertices()[angleIdx]->getBead()->coordinate;
        auto& v1 = _pTriangle->getVertices()[(angleIdx+1) % 3]->getBead()->coordinate;
        auto& v2 = _pTriangle->getVertices()[(angleIdx+2) % 3]->getBead()->coordinate;

        auto& m0 = _pTriangle->getEdges()[angleIdx]->getMEdge();
        auto& m1 = _pTriangle->getEdges()[(angleIdx+1) % 3]->getMEdge();
        auto& m2 = _pTriangle->getEdges()[(angleIdx+2) % 3]->getMEdge();

        double l0 = m0->getLength(); // length v0 - v1
        double l1 = m1->getLength(); // length v1 - v2
        double l2 = m2->getLength(); // length v2 - v0

        // first derivative index which should be used in getDLength
        // i.e. index to be used for d(l0) / d(v0), d(l1) / d(v1), d(l2) / d(v2)
        // value is either 0 or 1.
        size_t i0 = _pTriangle->getEdgeHead()[angleIdx];
        size_t i1 = _pTriangle->getEdgeHead()[(angleIdx+1) % 3];
        size_t i2 = _pTriangle->getEdgeHead()[(angleIdx+2) % 3];

        std::array<std::array<double, 3>, 3> dDot12, dCosTheta;

        double dot12 = scalarProduct(v0, v1, v0, v2);
        for(int coordIdx = 0; coordIdx < 3; ++coordIdx) {
            dDot12[0][coordIdx] = 2*v0[coordIdx] - v1[coordIdx] - v2[coordIdx];
            dDot12[1][coordIdx] = v2[coordIdx] - v0[coordIdx];
            dDot12[2][coordIdx] = v1[coordIdx] - v0[coordIdx];
        }

        double cosTheta = dot12 / (l0 * l2);
        for(int coordIdx = 0; coordIdx < 3; ++coordIdx) {
            dCosTheta[0][coordIdx] = (l0*l2*dDot12[0][coordIdx] - dot12*(l0*m2->getDLength()[(i2+1) % 2][coordIdx] + m0->getDLength()[i0][coordIdx]*l2)) / (l0*l0 * l2*l2);
            dCosTheta[1][coordIdx] = (l0*dDot12[1][coordIdx] - dot12*m0->getDLength()[(i0+1) % 2][coordIdx]) / (l0*l0 * l2);
            dCosTheta[2][coordIdx] = (l2*dDot12[2][coordIdx] - dot12*m2->getDLength()[i2][coordIdx]) / (l0 * l2*l2);
        }

        // Calculate sinTheta, theta and cotTheta
        _sinTheta[angleIdx] = sqrt(1 - cosTheta*cosTheta);
        _theta[angleIdx] = acos(cosTheta);
        _cotTheta[angleIdx] = cosTheta / _sinTheta[angleIdx];
        for(int coordIdx = 0; coordIdx < 3; ++coordIdx) {
            for(int relVtxIdx = 0; relVtxIdx < 3; ++relVtxIdx) {
                size_t vtxIdx = (angleIdx + relVtxIdx) % 3;
                _dTheta[angleIdx][vtxIdx][coordIdx] = -dCosTheta[relVtxIdx][coordIdx] / _sinTheta[angleIdx];
                _dSinTheta[angleIdx][vtxIdx][coordIdx] = cosTheta * _dTheta[angleIdx][vtxIdx][coordIdx];
                _dCotTheta[angleIdx][vtxIdx][coordIdx] = -_dTheta[angleIdx][vtxIdx][coordIdx] / (_sinTheta[angleIdx] * _sinTheta[angleIdx]);
            }
        }

    }
}