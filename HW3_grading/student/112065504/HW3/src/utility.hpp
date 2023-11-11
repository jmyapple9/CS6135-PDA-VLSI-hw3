
int overlapWithSoft(Module &softMod_j, vector<Module> &modules, int nSoftMod)
{
    for (int i = 0; i < nSoftMod; i++)
    {
        if (softMod_j.id == i)
            continue;
        Module &softMod_i = modules[i];
        int xi = softMod_i.x;
        int yi = softMod_i.y;
        int wi = softMod_i.w;
        int hi = softMod_i.h;

        int xj = softMod_j.x;
        int yj = softMod_j.y;
        int wj = softMod_j.w;
        int hj = softMod_j.h;

        if (!(xi + wi <= xj or yi + hi <= yj or xi - wj >= xj or yi - hj >= yj))
        {
            // cout << softMod.name << " overlap with " << modules[i].name << endl;
            return i;
        }
    }
    return -1;
}

/*
Return fix module id that overlapping with
 */
int overlapWithFixed(Module &softMod, vector<Module> &modules, int nSoftMod)
{
    int size = modules.size();
    for (int i = nSoftMod; i < size; i++) // May have bug: nSoftMod could be beforeIdx from perturbModulesInBox
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
    return -1;
}

bool outOfBound(Module &softMod, Die &die)
{
    return (softMod.x + softMod.w > die.W or softMod.y + softMod.h > die.H or softMod.x < 0 or softMod.y < 0);
}

bool legal(Module &softMod, vector<Module> &modules, int beforeIdx, int nSoftMod, Die die)
{
    return overlapWithSoft(softMod, modules, beforeIdx) == -1 and overlapWithFixed(softMod, modules, nSoftMod) == -1 and !outOfBound(softMod, die);
}

void InitializeOutOfBound(vector<Module> &modules, int nSoftMod, Die &die)
{
    modules[0].x = die.W;
    modules[0].y = 0;
    for (int i = 1; i < nSoftMod; ++i)
    {
        modules[i].x = modules[i - 1].x + modules[i - 1].w;
    }
}


bool compareByArea(const Module &a, const Module &b)
{
    return a.area > b.area;
}

/* 
Sort module by area in decreasing order
 */
void SortModulesByArea(vector<Module> &modules, unordered_map<string, int> &moduleNameToId, vector<Net> &nets, int nSoftMod)
{
    int size = modules.size();
    sort(modules.begin(), modules.begin() + nSoftMod, compareByArea);
    for (int i = 0; i < size; i++)
    {
        Module &m = modules[i];
        m.id = moduleNameToId[m.name] = i;
    }
    for (auto &n : nets)
    {
        n.update(moduleNameToId[n.m1_name], moduleNameToId[n.m2_name]);
    }
}

void TryJumpIntoBox(Module &softMod, vector<Module> &modules, Die &die, int nSoftMod, int beforeIdx)
{
    for (int time = 0; time < 30; ++time)
    {
        int prev_x = softMod.x, prev_y = softMod.y;
        softMod.x = rand() % (die.W - softMod.w);
        softMod.y = rand() % (die.H - softMod.h);

        // bool betterResult = /* rand()%10==0 or */ abs((double)die.H / (double)die.W - prev_ratio) >= abs((double)die.H / (double)die.W - ratio());
        if (legal(softMod, modules, beforeIdx, nSoftMod, die))
        {
            return;
        }
        else
        { // revert
            softMod.x = prev_x;
            softMod.y = prev_y;
        }
    }
}
void check(int nFixedMod, int nSoftMod, vector<Module> &modules, vector<Net> &nets)
{
    cout << "-------- check ---------" << endl;
    cout << "Checking softmodules: " << endl;
    for (int i = 0; i < nSoftMod; i++)
    {
        cout << "softmodule[" << modules[i].id << "]: " << modules[i].name << " has area " << modules[i].area << endl;
    }

    cout << "\nChecking fixedModeules: " << endl;
    for (int i = nSoftMod; i < nSoftMod + nFixedMod; i++)
    {
        cout << "FixedModule[" << modules[i].id << "]: " << modules[i].name << " has x, y, w, h = " << modules[i].x << ", " << modules[i].y << ", " << modules[i].w << ", " << modules[i].h << endl;
    }
    cout << "\nChecking nets: " << endl;
    for (auto &net : nets)
    {
        cout << "Net " << modules[net.m1].name
             << " " << modules[net.m2].name
             << " " << net.weight << endl;
    }
    cout << "-------- Done check ---------" <<endl;
}

void placeAdjacently(int idx, vector<Module> &modules, int nSoftMod)
{
    int fixModID;
    while (1)
    {
        if ((fixModID = overlapWithFixed(modules[idx], modules, nSoftMod)) != -1)
            modules[idx].x = modules[fixModID].x + modules[fixModID].w;
        else
            break;
    }
}

long long calWirelength(vector<Net> &nets, vector<Module> &modules)
{
    long long wl = 0;
    for (auto net : nets)
    {
        Module &M1 = modules[net.m1];
        Module &M2 = modules[net.m2];
        wl += (abs((M1.x + M1.w / 2) - (M2.x + M2.w / 2)) +
              abs((M1.y + M1.h / 2) - (M2.y + M2.h / 2))) * net.weight;
    }
    return wl;
}

bool allLegal(Die &die, vector<Module> &modules, int nSoftMod)
{
    int idx = 0;
    for (auto &softMod : modules)
    {
        if (!legal(softMod, modules, nSoftMod, nSoftMod, die))
            return false;
        if (++idx == nSoftMod)
            break;
    }
    return true;
}

double ratio(vector<Module> &modules)
{
    double w = -1, h = -1;
    for (auto &softMod : modules)
    {
        w = max(w, (double)(softMod.w + softMod.x));
        h = max(h, (double)(softMod.h + softMod.y));
    }
    return h / w;
}

/*
Ensure at least one perturb action was executed
 */
void perturbModulesInBox(vector<Module> &modules, Die &die, int nSoftMod, int beforeIdx)
{
    bool takeAction = false;
    int cnt = 0;
    while (!takeAction and cnt++ < 10000)
    {
        int act = rand() % 3;
        if (act == 0) // change shape
        {
            Module &softMod = modules[rand() % beforeIdx];
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
            if (legal(softMod, modules, beforeIdx, nSoftMod, die))
            {
                takeAction = true;
            }
            else
            { // revert
                softMod.w = prev_w;
                softMod.h = prev_h;
            }
        } 
        else if (act == 1) // swap
        {
            if (beforeIdx == 1) // only one module in box, can't find two to swap
                continue; 
            int m1 = rand() % beforeIdx;
            int m2 = rand() % beforeIdx;
            while (m2 == m1)
            {
                m2 = rand() % beforeIdx;
            }
            Module &softMod1 = modules[m1];
            Module &softMod2 = modules[m2];
            softMod1.swapCoordinate(softMod2);
            // bool betterResult = /* rand()%10==0 or */ abs((double)die.H / (double)die.W - prev_ratio) >= abs((double)die.H / (double)die.W - ratio());
            if (legal(softMod1, modules, beforeIdx, nSoftMod, die) and legal(softMod2, modules, beforeIdx, nSoftMod, die))
            {
                takeAction = true;
            }
            else
            { // revert
                softMod1.swapCoordinate(softMod2);
            }
        }
        else if (act == 2) // jump
        {
            int idx = rand() % beforeIdx;
            Module &softMod = modules[idx];
            int prev_x = softMod.x, prev_y = softMod.y;
            softMod.x = rand() % (die.W - softMod.w);
            softMod.y = rand() % (die.H - softMod.h);
            // bool betterResult = /* rand()%10==0 or */ abs((double)die.H / (double)die.W - prev_ratio) >= abs((double)die.H / (double)die.W - ratio());
            if (legal(softMod, modules, beforeIdx, nSoftMod, die))
            {
                takeAction = true;
            }
            else
            { // revert
                softMod.x = prev_x;
                softMod.y = prev_y;
            }
        }
    }
}