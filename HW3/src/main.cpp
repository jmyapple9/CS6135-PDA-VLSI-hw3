#include "main.hpp"

ifstream in_file;

unordered_map<string, int> moduleNameToId;
Die die;
int nSoftMod; // number of soft modules
int nFixedMod;
long long wirelength;
vector<Module> modules;
vector<Net> nets;

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
        cout << "net: " << modules[net.m1].name
             << " to " << modules[net.m2].name
             << " has weight " << net.weight << endl;
    }
}

/*
Return fix module id that overlapping with
 */
int overlapWithFixed(Module &softMod)
{
    int size = modules.size();
    for (int i = nSoftMod; i < size; i++)
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
            // cout << softMod.name << " overlap with " << modules[i].name << endl;
            return i;
        }
    }
    return 0;
}

void placeAdjacently(int idx){
    int fixModID;
    while (1)
    {
        if (fixModID = overlapWithFixed(modules[idx]))
            modules[idx].x = modules[fixModID].x + modules[fixModID].w;
        else
            break;
    }
}

void calWirelength()
{
    wirelength = 0;
    for (auto net : nets)
    {
        Module &M1 = modules[net.m1];
        Module &M2 = modules[net.m2];
        wirelength += abs((M1.x + M1.w / 2) - (M2.x + M2.w / 2)) +
                      abs((M1.y + M1.h / 2) - (M2.y + M2.h / 2));
    }
}

void init_floorplan()
{
    cout << "init_floorplan" << endl;

    modules[0].x = modules[0].y = 0;
    placeAdjacently(0);

    for (int i = 1; i < nSoftMod; ++i)
    {
        modules[i].x = modules[i - 1].x + modules[i - 1].w;
        placeAdjacently(i);
    }
    calWirelength();
}

void SA()
{
}

void output(string outputPath)
{
    ofstream outputfile;
    outputfile.open(outputPath);
    outputfile << "Wirelength " << wirelength << '\n';
    outputfile << "NumSoftModules " << nSoftMod << '\n';
    for (int i = 0; i < nSoftMod; i++)
    {
        Module softMod = modules[i];
        outputfile << softMod.name << " "
                   << softMod.x << " "
                   << softMod.y << " "
                   << softMod.w << " "
                   << softMod.h << '\n';
    }

    outputfile.flush();
    outputfile.close();
}



int main(int argc, char *argv[])
{
    auto [testcasePath, outputPath] = eatArg(argc, argv);
    parser(testcasePath);    
    // check();
    init_floorplan();
    SA();
    output(outputPath);
    return 0;
}