#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <list>
#include <unordered_map>

using namespace std;

class Die
{
public:
    int W, H;
    Die() {}
};

class Module
{
public:
    string name;
    bool fix;
    long long area;
    int x, y, h, w;
    /* Soft module constructor */
    Module(string _name, long long _area)
        : name(_name), area(_area), fix(false), x(0), y(0)
    {
        h = w = sqrt(area) + 1;
    }
    /* Fixed module constructor */
    Module(string _name, int _x, int _y, int _w, int _h)
        : name(_name), x(_x), y(_y), w(_w), h(_h), fix(true)
    {
    }
};

class Net
{
public:
    int m1, m2; // 2 module id of this net
    int weight;

    Net(int _m1, int _m2, int _weight)
        : m1(_m1), m2(_m2), weight(_weight)
    {
    }
};
