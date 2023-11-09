#include "classes.hpp"
#include "utility.hpp"

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
        modules.emplace_back(Module(name, area, i));
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
    for (int i = nSoftMod; i < nFixedMod + nSoftMod; i++)
    {
        getIss(iss, line);
        iss >> s1 >> name >> x >> y >> w >> h;
        modules.emplace_back(Module(name, x, y, w, h, i));
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

        // int m1 = moduleNameToId[m1_name];
        // int m2 = moduleNameToId[m2_name];

        nets.emplace_back(Net(weight, m1_name, m2_name));
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

void init_floorplan()
{
    cout << "init_floorplan" << endl;

    modules[0].x = modules[0].y = 0;
    placeAdjacently(0, modules, nSoftMod);

    for (int i = 1; i < nSoftMod; ++i)
    {
        modules[i].x = modules[i - 1].x + modules[i - 1].w;
        placeAdjacently(i, modules, nSoftMod);
    }

    // sort module area in decreasing order
    sort(modules.begin(), modules.begin() + nSoftMod, compareByArea);
    for (int i = 0; i < nSoftMod + nFixedMod; i++)
    {
        Module &m = modules[i];
        m.id = moduleNameToId[m.name] = i;
    }
    for (auto &n : nets)
    {
        n.update(moduleNameToId[n.m1_name], moduleNameToId[n.m2_name]);
    }

    wirelength = calWirelength(nets, modules);

    while (!allLegal(die, modules, nSoftMod))
    {
        double prev_ratio = ratio(modules);
        int act = rand() % 4;
        // int act = 3;
        if (act == 0) // change shape
        {
            Module &softMod = modules[rand() % nSoftMod];
            int prev_w = softMod.w, prev_h = softMod.h;
            mt19937 gen(1);

            uniform_real_distribution<double> distribution(0.5, 2.0);
            double step = 0.1;
            double randomRatio = floor(distribution(gen) / step) * step;

            softMod.h = sqrt(softMod.area / randomRatio) + 1;
            softMod.w = (softMod.h * randomRatio);
            if (softMod.w * softMod.h < softMod.area)
                softMod.w++;

            // bool betterResult = abs((double)die.H / (double)die.W - prev_ratio) >= abs((double)die.H / (double)die.W - ratio());
            if (legal(softMod, modules, nSoftMod, nSoftMod, die))
            {
            }
            else
            { // revert
                softMod.w = prev_w;
                softMod.h = prev_h;
            }
        }
        else if (act == 1) // swap
        {
            int m1 = rand() % nSoftMod;
            int m2 = rand() % nSoftMod;
            while (m2 == m1)
                m2 = rand() % nSoftMod;
            Module &softMod1 = modules[m1];
            Module &softMod2 = modules[m2];
            softMod1.swapCoordinate(softMod2);
            // bool betterResult = /* rand()%10==0 or */ abs((double)die.H / (double)die.W - prev_ratio) >= abs((double)die.H / (double)die.W - ratio());
            if (legal(softMod1, modules, nSoftMod, nSoftMod, die) and legal(softMod2, modules, nSoftMod, nSoftMod, die))
            {
            }
            else
            { // revert
                softMod1.swapCoordinate(softMod2);
            }
        }
        else if (act == 2) // move
        {
        }
        else // jump
        {
            Module &softMod = modules[rand() % nSoftMod];
            int prev_x = softMod.x, prev_y = softMod.y;
            softMod.x = rand() % (die.W - softMod.w);
            softMod.y = rand() % (die.H - softMod.h);

            // bool betterResult = /* rand()%10==0 or */ abs((double)die.H / (double)die.W - prev_ratio) >= abs((double)die.H / (double)die.W - ratio());
            if (legal(softMod, modules, nSoftMod, nSoftMod, die))
            {
            }
            else
            { // revert
                softMod.x = prev_x;
                softMod.y = prev_y;
            }
        }
    }
}

void init_floorplan2()
{
    // initialize modules position to the outside of die
    InitializeOutOfBound(modules, nSoftMod, die);

    // sort module area in decreasing order
    SortModulesByArea(modules, moduleNameToId, nets, nSoftMod);
    // check(nFixedMod, nSoftMod, modules, nets);
    TryJumpIntoBox(modules[0], modules, die, nSoftMod, 0);
    for (int i = 1; i < nSoftMod; i++)
    {
        Module &softMod = modules[i];
        
        while (!legal(softMod, modules, i, nSoftMod, die))
        {
            // perturb modules in fix outline
            perturbModulesInBox(modules, die, nSoftMod, i); 
            // try put softMod on
            TryJumpIntoBox(softMod, modules, die, nSoftMod, i);
        }
        cout << "Put module " << i << endl;
    }

    wirelength = calWirelength(nets, modules);
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
    clock_t start = clock();

    srand(1);
    // srand(time(0));
    auto [testcasePath, outputPath] = eatArg(argc, argv);
    parser(testcasePath);
    // check(nFixedMod nSoftMod, modules, nets);
    // init_floorplan();
    init_floorplan2();
    SA();
    output(outputPath);

    cout << "-------------------------------" << endl;
    printf("Total exe Time = %f\n", ((double)(clock() - start)) / CLOCKS_PER_SEC);
    return 0;
}