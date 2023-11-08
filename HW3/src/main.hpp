#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <vector>
using namespace std;

class Die
{
public:
    int W, H;
    Die() {}
};

class SoftModule
{
public:
    string name;
    long long area;
    SoftModule(string _name, long long _area)
    {
        name = _name;
        area = _area;
    }
};

class FixedModule
{
public:
    string name;
    int x, y, w, h;
    FixedModule(string _name, int _x, int _y, int _w, int _h)
    {
        name = _name;
        x = _x;
        y = _y;
        w = _w;
        h = _h;
    }
};