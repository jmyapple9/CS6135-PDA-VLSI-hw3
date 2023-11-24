#include "classes.hpp"
#include "utility.hpp"

ifstream in_file;

unordered_map<string, int> moduleNameToId;
Die die;
int nSoftMod; // number of soft modules
int nFixedMod;
long long bestWL = INT_MAX, perturbWL, originWL, baselineWL;
int revert_x, revert_y, revert_w, revert_h, revert_m1, revert_m2, actDir;
// long long wirelength;
vector<Module> modules;
vector<Module> bestModules;
vector<Net> nets;
chrono::system_clock::time_point start_time;
chrono::system_clock::time_point end_time;

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

bool init_floorplan()
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
            if (chrono::system_clock::now() >= end_time)
                return true;
            // perturb modules in fix outline
            perturbModulesInBox(modules, die, nSoftMod, i);
            // try put softMod on
            TryJumpIntoBox(softMod, modules, die, nSoftMod, i);
        }
        cout << "Put module " << i << endl;
    }

    originWL = calWirelength(nets, modules);

    return false;
}

bool init_floorplan2()
{
    InitializeOutOfBound(modules, nSoftMod, die);
    SortModulesByArea(modules, moduleNameToId, nets, nSoftMod);
    double step = 0.1;
    for (int softIdx = 0; softIdx < nSoftMod; ++softIdx)
    {
        bool placed = false;
        Module &softMod = modules[softIdx];
        // for (double r = 0.5 ; r  <= 2; r += step) // start with flat shape, gradually becomes thin
        for (double r = 2; r >= 0.499; r -= step) // start with flat shape, gradually becomes thin
        {
            // softMod.r -= step;
            softMod.h = sqrt(softMod.area / r) + 1;
            softMod.w = (softMod.h * r);
            if (softMod.w * softMod.h < softMod.area)
                softMod.w++;
            // if(softIdx==0)
                cout << "putting mod " << softIdx << " with shape r = " << r << endl;
            for (int y = 0; y < die.H - softMod.h; ++y) // from left to right, from bottom to top
            {
                // if(r==0.6 && softIdx==15)
                //     cout << "y = " << y << ", ";
                softMod.y = y;
                for (int x = 0; x < die.W - softMod.w; ++x)
                {
                    // if(r==0.6 && softIdx==15)
                    //     cout << "x = " << x << ", ";
                    softMod.x = x;
                    int onFixed = overlapWithFixed(softMod, modules, nSoftMod);
                    if (onFixed != -1)
                    {
                        // cout << "Module " << softIdx << " is overlap with fix mod at " << x << ", " << y << endl;
                        Module &overlap = modules[onFixed];
                        x = overlap.x + overlap.w - 1;
                        if (x >= die.W - softMod.w)
                        {
                            break;
                        }
                        continue;
                    }

                    int onSoft = overlapWithSoft(softMod, modules, nSoftMod);
                    if (onSoft != -1)
                    {
                        Module &overlap = modules[onSoft];
                        x = overlap.x + overlap.w - 1;
                        if (x >= die.W - softMod.w)
                        {
                            break;
                        }
                        continue;
                    }
                    placed = true;
                    // cout << "Put module " << softIdx << endl;
                    break;
                }
                if (placed)
                    break;
            }
            if (placed)
                break;
        }
        if (!placed)
        {
            cout << "fail to put " << softIdx << " with all shapes, InitializeOutOfBound()" << endl;
            InitializeOutOfBound(modules, nSoftMod, die);
            softIdx = -1;
        }
    }

    for (int softIdx = 0; softIdx < nSoftMod; ++softIdx)
    {
        Module &softMod = modules[softIdx];
        if (!legal(softMod, modules, nSoftMod, nSoftMod, die))
        {
            // cout << "Module " << softIdx << " is invalid !" << endl;
            cout << "Module " << softIdx << ": " << softMod.x << ", " << softMod.y << endl;
        }
    }
    originWL = calWirelength(nets, modules);
    // don't need to check each modules respectively
    // just simply check if the smallest module is in bound or not.
    return !outOfBound(modules[nSoftMod - 1], die);
}

bool perturb(int act)
{
    // cout << "perturbing..." << endl;
    bool takeAction = false;
    while (!takeAction)
    {
        if (std::chrono::system_clock::now() >= end_time)
            return true;
        int cnt = 0;

        if (act == 0 or cnt++ == 10000) // reshape
        {
            revert_m1 = rand() % nSoftMod;
            Module &softMod = modules[revert_m1];
            revert_w = softMod.w, revert_h = softMod.h;
            mt19937 gen(1);
            uniform_real_distribution<double> distribution(0.5, 2.0);
            double step = 0.1;
            double randomRatio = floor(distribution(gen) / step) * step;

            softMod.h = sqrt(softMod.area / randomRatio) + 1;
            softMod.w = (softMod.h * randomRatio);
            if (softMod.w * softMod.h < softMod.area)
                softMod.w++;

            if (legal(softMod, modules, nSoftMod, nSoftMod, die))
            {
                takeAction = true;
            }
            else
            { // revert since answer illegal
                softMod.w = revert_w;
                softMod.h = revert_h;
            }
        }
        else if (act == 1) // swap
        {
            // if (beforeIdx == 1) // only one module in box, can't find two to swap
            //     continue;
            revert_m1 = rand() % nSoftMod;
            revert_m2 = rand() % nSoftMod;
            while (revert_m2 == revert_m1)
            {
                revert_m2 = rand() % nSoftMod;
            }
            Module &softMod1 = modules[revert_m1];
            Module &softMod2 = modules[revert_m2];
            softMod1.swapCoordinate(softMod2);
            if (legal(softMod1, modules, nSoftMod, nSoftMod, die) and legal(softMod2, modules, nSoftMod, nSoftMod, die))
            {
                takeAction = true;
            }
            else
            { // revert since answer illegal
                softMod1.swapCoordinate(softMod2);
            }
        }
        else if (act == 2) // jump
        {
            revert_m1 = rand() % nSoftMod;
            Module &softMod = modules[revert_m1];
            revert_x = softMod.x, revert_y = softMod.y;
            softMod.x = rand() % (die.W - softMod.w);
            softMod.y = rand() % (die.H - softMod.h);
            if (legal(softMod, modules, nSoftMod, nSoftMod, die))
            {
                takeAction = true;
            }
            else
            { // revert since answer illegal
                softMod.x = revert_x;
                softMod.y = revert_y;
            }
        }
        else if (act == 3)
        {
            actDir = rand() % 4;
            revert_m1 = rand() % nSoftMod;
            Module &softMod = modules[revert_m1];
            revert_x = softMod.x, revert_y = softMod.y;
            switch (actDir)
            {
            case 0: // move up
                softMod.y += rand() % 10 + 1;
                if (legal(softMod, modules, nSoftMod, nSoftMod, die))
                    takeAction = true;
                else
                    softMod.y = revert_y;
                break;
            case 1: // move down
                softMod.y -= rand() % 10 + 1;
                if (legal(softMod, modules, nSoftMod, nSoftMod, die))
                    takeAction = true;
                else
                    softMod.y = revert_y;
                break;
            case 2: // move right
                softMod.x += rand() % 10 + 1;
                if (legal(softMod, modules, nSoftMod, nSoftMod, die))
                    takeAction = true;
                else
                    softMod.x = revert_x;
                break;
            case 3: // move left
                softMod.x -= rand() % 10 + 1;
                if (legal(softMod, modules, nSoftMod, nSoftMod, die))
                    takeAction = true;
                else
                    softMod.x = revert_x;
                break;

            default:
                break;
            }
        }
    }
    return false;
}

bool accept(int cost, int T)
{
    double random = static_cast<double>(rand()) / RAND_MAX;
    bool ac = exp(-static_cast<double>(cost) / baselineWL / T) > random;
    // cout << (ac ? "accept" : "notAccept") << endl;

    return ac;
}

void revert(int act)
{
    Module &m1 = modules[revert_m1];
    Module &m2 = modules[revert_m2];
    switch (act)
    {
    case 0: // revert reshape
        m1.w = revert_w;
        m1.h = revert_h;
        break;

    case 1: // revert swap
        // Module &m1 = modules[revert_m1];
        m1.swapCoordinate(m2);
        break;

    case 2: // revert jump
        // Module &m1 = modules[revert_m1];
        m1.x = revert_x;
        m1.y = revert_y;
        break;
    case 3:
        if (actDir == 0 or actDir == 1) // move up or down
            m1.y = revert_y;
        else
        { // move left or right
            m1.x = revert_x;
        }
        break;
    default:
        cout << "[error] act value incorrect: " << act << endl;
        break;
    }
}

bool SA()
{
    baselineWL = originWL / 600000;
    double reject, reduceRatio = 0.9999;
    int nAns, uphill, T = 1000000, N = 70; // N: number of answer in T
    if (bestWL > originWL)
    {
        bestModules = modules;
        bestWL = originWL;
    }

    do
    {
        reject = nAns = uphill = 0.;

        while (uphill < N and nAns < 2 * N)
        {
            int act = rand() % 4;
            bool timeout = perturb(act);
            if (timeout)
                return true;
            ++nAns;
            perturbWL = calWirelength(nets, modules);

            int deltaCost = perturbWL - originWL;
            if (deltaCost <= 0 or accept(deltaCost, T))
            {
                if (deltaCost > 0)
                    ++uphill;

                originWL = perturbWL;
                if (originWL < bestWL)
                {
                    bestModules = modules;
                    bestWL = originWL;
                }
            }
            else
            {
                ++reject;
                revert(act);
            }
        }

        T *= reduceRatio;
        if (T % 10000 == 0)
            cout << "T: " << T << ", bestWL: " << bestWL << endl;
    } while (reject / nAns <= 0.95 and T > 10);
    // cout << "Done SA" << endl;
    return false;
}

void output(string outputPath)
{
    cout << "-------- bestWL:" << bestWL << "  --------";
    ofstream outputfile;
    outputfile.open(outputPath);
    outputfile << "Wirelength " << bestWL << '\n';
    outputfile << "NumSoftModules " << nSoftMod << '\n';
    for (int i = 0; i < nSoftMod; i++)
    {
        Module softMod = bestModules[i];
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
    // cout << "main!" << endl;
    chrono::duration<int, std::ratio<60>> minutes(9);
    chrono::duration<int> seconds(55);
    start_time = chrono::system_clock::now();
    end_time = start_time + minutes + seconds;

    // clock_t start = clock();
    auto start = chrono::system_clock::now();
    srand(1);
    // srand(time(0));
    auto [testcasePath, outputPath] = eatArg(argc, argv);
    parser(testcasePath);
    // check(nFixedMod, nSoftMod, modules, nets);
    /* while (chrono::system_clock::now() < end_time)
    {
        if (init_floorplan() or SA())
            break;
        cout << "-----------done one iteration----------" << endl;
        auto realDuration = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now() - start_time);
        cout << "iteration duration: " << realDuration.count() << " seconds." << endl << endl;
    } */
    // init_floorplan();
    init_floorplan2();
    SA();
    output(outputPath);

    cout << "-------------------------------" << endl;
    auto realDuration = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now() - start_time);
    cout << "Real duration: " << realDuration.count() << " seconds." << endl;
    // printf("Total exe Time = %f\n", ((double)(clock() - start)) / CLOCKS_PER_SEC);
    return 0;
}
// tar -zcvf CS6135_HW3_112065504.tar.gz HW3/