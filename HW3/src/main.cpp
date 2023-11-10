#include "classes.hpp"
#include "utility.hpp"

ifstream in_file;

unordered_map<string, int> moduleNameToId;
Die die;
int nSoftMod; // number of soft modules
int nFixedMod;
long long bestWL, perturbWL, originWL, baselineWL;
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

void init_floorplan()
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

    originWL = bestWL = calWirelength(nets, modules);
}

void perturb(int act)
{
    // cout << "perturbing..." << endl;
    bool takeAction = false;
    while (!takeAction)
    {
    if(std::chrono::system_clock::now() >= end_time) return;
        if (act == 0) // change shape
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

            // bool betterResult = abs((double)die.H / (double)die.W - revert_ratio) >= abs((double)die.H / (double)die.W - ratio());
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
            // bool betterResult = /* rand()%10==0 or */ abs((double)die.H / (double)die.W - prev_ratio) >= abs((double)die.H / (double)die.W - ratio());
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
            // bool betterResult = /* rand()%10==0 or */ abs((double)die.H / (double)die.W - prev_ratio) >= abs((double)die.H / (double)die.W - ratio());
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

void SA()
{
    baselineWL = originWL / 600000;
    double reject, reduceRatio = 0.9999;
    int nAns, uphill, T = 1000000000, N = 70; // N: number of answer in T
    bestModules = modules;

    do
    {
        reject = nAns = uphill = 0.;

        while (uphill < N and nAns < 2 * N)
        {
            int act = rand() % 4;
            perturb(act);
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
        if(T%10000==0) cout << "T: " << T << ", bestWL: " << bestWL << endl;
        // if(T==15381) break;
    } while (reject / nAns <= 0.95 and T > 10);
}

void output(string outputPath)
{
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
    std::chrono::duration<int, std::ratio<60>> minutes(9);
    std::chrono::duration<int> seconds(40);
    start_time = chrono::system_clock::now();
    end_time = start_time + minutes + seconds;

    // clock_t start = clock();
    auto start = chrono::system_clock::now();
    srand(1);
    // srand(time(0));
    auto [testcasePath, outputPath] = eatArg(argc, argv);
    parser(testcasePath);
    // check(nFixedMod nSoftMod, modules, nets);
    init_floorplan();
    // auto floorplanTime = chrono::system_clock::now();
    SA();
    output(outputPath);

    cout << "-------------------------------" << endl;
    auto realDuration = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now() - start_time);
    cout << "Real duration: " << realDuration.count() << " seconds." << endl;
    // printf("Total exe Time = %f\n", ((double)(clock() - start)) / CLOCKS_PER_SEC);
    return 0;
}