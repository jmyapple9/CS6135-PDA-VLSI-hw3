#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <chrono>
#include <climits>
#include <random>
#include <vector>
#include <ctime>
#include <cmath>
#include <list>
#include <algorithm>
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
    int id;
    long long area;
    int x, y, h, w;
    /* Soft module constructor */
    Module(string _name, long long _area, int _id)
        : name(_name), area(_area), id(_id), fix(false), x(0), y(0)
    {
        h = w = sqrt(area) + 1;
    }
    /* Fixed module constructor */
    Module(string _name, int _x, int _y, int _w, int _h, int _id)
        : name(_name), x(_x), y(_y), w(_w), h(_h), id(_id), fix(true)
    {
    }
    void show()
    {
        cout << "module " << name << " info: ";
        cout << "x: " << x << ", ";
        cout << "y: " << y << ", ";
        cout << "w: " << w << ", ";
        cout << "h: " << h << endl;
    }
    void swapCoordinate(Module &softMod)
    {
        swap(x, softMod.x);
        swap(y, softMod.y);
    }
};

class Net
{
public:
    int m1, m2; // 2 module id of this net
    int weight;
    string m1_name, m2_name;
    Net(int _weight, string _m1_name, string _m2_name)
        : weight(_weight), m1_name(_m1_name), m2_name(_m2_name)
    {
    }
    void update(int _m1, int _m2)
    {
        m1 = _m1;
        m2 = _m2;
    }
};
