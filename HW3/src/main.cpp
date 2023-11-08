#include "main.hpp"

ifstream in_file;

Die die;
vector<SoftModule *> softModeules;
vector<FixedModule *> fixedModeules;

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
    int nSoftMod; // number of soft modules
    long long area;
    // eat newline
    getline(in_file, line);

    getIss(iss, line);
    iss >> s1 >> nSoftMod;
    for (int i = 0; i < nSoftMod; i++)
    {
        getIss(iss, line);
        iss >> s1 >> name >> area;
        softModeules.emplace_back(new SoftModule(name, area));
        // cout << "softmodule: " << softModeules[i]->name << " has area " << softModeules[i]->area << endl;
    }

}

void fixedModuleParser()
{
    istringstream iss;
    string line, s1, name;
    int nFixedMod, x, y, w, h;

    // eat newline
    getline(in_file, line);

    getIss(iss, line);
    iss >> s1 >> nFixedMod;
    for (int i = 0; i < nFixedMod; i++)
    {
        getIss(iss, line);
        iss >> s1 >> name >> x >> y >> w >> h;
        fixedModeules.emplace_back(new FixedModule(name, x, y, w, h));
        // cout << "FixedModule: " << name << " has x, y, w, h = " << fixedModeules[i]->x << ", "<< fixedModeules[i]->y << ", "<< fixedModeules[i]->w << ", "<< fixedModeules[i]->h << endl;
    }
}

void netParser()
{
    istringstream iss;
    string line, s1;
    int nNets;
    // eat newline
    getline(in_file, line);


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

void output(string outputPath)
{
}

int main(int argc, char *argv[])
{
    auto [testcasePath, outputPath] = eatArg(argc, argv);
    parser(testcasePath);

    output(outputPath);
    return 0;
}