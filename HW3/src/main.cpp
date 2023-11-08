#include "main.hpp"

ifstream in_file;

unordered_map<string, int> moduleNameToId;
Die die;
// vector<SoftModule> softModules;
// vector<FixedModule> fixedModules;
int nSoftMod; // number of soft modules
int nFixedMod;
long long wirelength;
vector<Module> modules;
vector<Net> nets;
list<Segment> contour;

pair<string, string> eatArg(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "Input format should be the following." << endl;
        cout << "eg.  ./hw3 ../testcase/public1.txt ../output/public1.floorplan" << endl;
        cout << "exiting..." << endl;
        exit(1);
    }
    return {argv[1], argv[2]};
}

void getIss(istringstream &iss, string line)
{
    iss.clear();
    iss.str("");
    getline(in_file, line);
    iss.str(line);
}

void softModuleParser()
{
    istringstream iss;
    string line, s1, name;
    long long area;
    // eat newline
    getline(in_file, line);

    getIss(iss, line);
    iss >> s1 >> nSoftMod;
    // modules.reserve(nSoftMod);
    for (int i = 0; i < nSoftMod; i++)
    {
        getIss(iss, line);
        iss >> s1 >> name >> area;
        modules.emplace_back(Module(name, area));
        moduleNameToId[name] = i;
        // cout << "softmodule: " << softModules[i].name << " has area " << softModules[i].area << endl;
    }
}

void fixedModuleParser()
{
    istringstream iss;
    string line, s1, name;
    int x, y, w, h;

    // eat newline
    getline(in_file, line);

    getIss(iss, line);
    iss >> s1 >> nFixedMod;
    modules.reserve(nFixedMod + nSoftMod);
    // int FixedModBeginIdx = softModules.size();
    // int FixedModEndIdx = FixedModBeginIdx + nFixedMod;
    for (int i = nSoftMod; i < nFixedMod + nSoftMod; i++)
    {
        getIss(iss, line);
        iss >> s1 >> name >> x >> y >> w >> h;
        modules.emplace_back(Module(name, x, y, w, h));
        moduleNameToId[name] = i;
        // cout << "FixedModule: " << fixedModeules[i].name << " has x, y, w, h = " << fixedModeules[i].x << ", "<< fixedModeules[i].y << ", "<< fixedModeules[i].w << ", "<< fixedModeules[i].h << endl;
    }
}

void netParser()
{
    istringstream iss;
    string line, s1, m1_name, m2_name;
    int nNets, weight, m1, m2;
    // eat newline
    getline(in_file, line);

    getIss(iss, line);
    iss >> s1 >> nNets;
    nets.reserve(nNets);
    for (int i = 0; i < nNets; i++)
    {
        getIss(iss, line);
        iss >> s1 >> m1_name >> m2_name >> weight;

        int m1 = moduleNameToId[m1_name];
        int m2 = moduleNameToId[m2_name];

        nets.emplace_back(Net(m1, m2, weight));
        // cout << "FixedModule: " << fixedModeules[i].name << " has x, y, w, h = " << fixedModeules[i].x << ", "<< fixedModeules[i].y << ", "<< fixedModeules[i].w << ", "<< fixedModeules[i].h << endl;
    }
}

void parser(string testcasePath)
{
    in_file.open(testcasePath);
    if (in_file.fail())
    {
        cout << "Fail opening file: " << testcasePath << endl;
        exit(1);
    }

    istringstream iss;
    string line, s1;

    getIss(iss, line);
    iss >> s1 >> die.W >> die.H;

    softModuleParser();
    fixedModuleParser();
    netParser();

    in_file.close();
}

void check()
{
    cout << "Checking softmodules: " << endl;
    for (int i = 0; i < nSoftMod; i++)
    {
        cout << "softmodule: " << modules[i].name << " has area " << modules[i].area << endl;
    }

    cout << "\nChecking fixedModeules: " << endl;
    for (int i = nSoftMod; i < nSoftMod + nFixedMod; i++)
    {
        cout << "FixedModule: " << modules[i].name << " has x, y, w, h = " << modules[i].x << ", " << modules[i].y << ", " << modules[i].w << ", " << modules[i].h << endl;
    }
    cout << "\nChecking nets: " << endl;
    for (auto &net : nets)
    {
        // cout << "net: " << (net.m1<softModules.size()?moduleNameToId[softModules[net.m1].name]:moduleNameToId[fixedModeules[net.m1-softModules.size()].name]) \
        //     << " to " << (net.m2<softModules.size()?moduleNameToId[softModules[net.m2].name]:moduleNameToId[fixedModeules[net.m2-softModules.size()].name]) \
        //     << " has weight " << net.weight << endl;
        cout << "net: " << modules[net.m1].name
             << " to " << modules[net.m2].name
             << " has weight " << net.weight << endl;
    }
}

// pair<int, int> onContour(Module& softMod){
//     pair<int, int> ret = make_pair(contour.front().x, contour.front().y);

//     return ret;
// }

/*
    softMod's coordinate (x, y) need to be initialized before calling this function
    return: the right up corner coordinate of the overlapping fix module
 */
/*
pair<int, int> overlapWithFixed(Module &softMod)
{
    int size = modules.size();
    for (int i = nFixedMod; i < size; i++)
    {
        Module &fixMod = modules[i];
        int xi = softMod.x;
        int yi = softMod.y;
        int wi = softMod.w;
        int hi = softMod.h;

        int xj = fixMod.x;
        int yj = fixMod.y;
        int wj = fixMod.w;
        int hj = fixMod.h;

        if (!(xi + wi <= xj or yi + hi <= yj or xi - wj >= xj or yi - hj >= yj))
        {
            return {xj + wj, yj + hj};
        }
    }
    return {-1, -1};
}
 */

/* void init_floorplan()
{
    // contour.emplace_back(Segment(0, 0, die.W));

    int x_pos, y_pos, ceiling, baseline;
    x_pos = y_pos = baseline = 0;
    pair<int, int> ret;
    modules[0].x = modules[0].y = 0;
    ceiling = modules[0].h;

    while (1)
    {
        ret = overlapWithFixed(modules[0]);
        if (ret.first == -1 && ret.second == -1)
        {
            break;
        }
        modules[0].x = ret.first;
        ceiling = max(ret.second, ceiling);
    }

    int cnt = 0;
    for (int i = 1; i < nSoftMod;)
    {
        Module &softMod = modules[i];
        if (cnt == 4)
        {
            cnt = 0;
            baseline = ceiling;
            softMod.x = 0, softMod.y = baseline;
            while (1)
            {
                ret = overlapWithFixed(softMod);
                if (ret.first == -1 && ret.second == -1)
                {
                    break;
                }
                softMod.x = ret.first;
                ceiling = max(ret.second, ceiling);
            }
        }
        else
        {
            modules[i].x = modules[i - 1].x + modules[i - 1].w;
            modules[i].y = modules[i - 1].y;
        }

        // do
        // {
        //     ret = overlapWithFixed(modules[i]);
        // } while (ret.first != -1 && ret.second != -1);
        // auto [pos_x, pos_y] = onContour(softMod);

        ++i;
    }
}
 */

/*
Return fix module id that overlapping with
 */
int overlapWithFixed(Module &softMod)
{
    int size = modules.size();
    for (int i = nFixedMod; i < size; i++)
    {
        Module &fixMod = modules[i];
        int xi = softMod.x;
        int yi = softMod.y;
        int wi = softMod.w;
        int hi = softMod.h;

        int xj = fixMod.x;
        int yj = fixMod.y;
        int wj = fixMod.w;
        int hj = fixMod.h;

        if (!(xi + wi <= xj or yi + hi <= yj or xi - wj >= xj or yi - hj >= yj))
        {
            return i;
        }
    }
    return 0;
}

void init_floorplan()
{
    int fixModID;

    while (1)
    {
        modules[0].x = modules[0].y = 0;
        if (fixModID = overlapWithFixed(modules[0]))
            modules[0].x = modules[fixModID].x + modules[fixModID].w;
        else
            break;
    }

    for (int i = 1; i < nSoftMod; ++i)
    {
        while (1)
        {
            modules[i].x = modules[i - 1].x + modules[i - 1].w;
            if (fixModID = overlapWithFixed(modules[i]))
                modules[i].x = modules[fixModID].x + modules[fixModID].w;
            else
                break;
        }
    }

}
void SA()
{
}

void output(string outputPath)
{
    ofstream outputfile;
    outputfile.open(outputPath);

    
    outputfile.flush();
    outputfile.close();
}

long long getWirelength(){
    
}

int main(int argc, char *argv[])
{
    auto [testcasePath, outputPath] = eatArg(argc, argv);
    parser(testcasePath);
    // check();
    // init_floorplan();
    SA();
    output(outputPath);
    return 0;
}