#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

#include <iostream>
#include <vector>
#include <windows.h>
#include <chrono> 
#include <ctime> 

using namespace std;
using namespace cv;

vector<vector<bool>> getArr(Mat img) {
    vector<vector<bool>> out(img.rows, vector<bool>(img.cols, 0));

    for (int i = 0; i < img.rows; ++i) {
        for (int j = 0; j < img.cols; ++j) {
            Vec3b intensity = img.at<Vec3b>(i, j);
            if ((intensity[0] + intensity[1] + intensity[2]) / 3 > 127) {
                out[i][j] = false;
            } else {
                out[i][j] = true;
            }
        }
    }

    return out;
}

struct node {
    int type;
    int exists = 0;

    bool operator ==(const node& rhs) const 
    {
        return this->type == rhs.type && this->exists == rhs.exists; 
    }
};

node genNode (int ty) {
    node tNode;
    tNode.type = ty;
    tNode.exists = 1;
    return tNode;
}

map<pair<int, int>, node> getNodes(vector<vector<bool>> bArr) {
    map<pair<int, int>, node> nodeMap;

    for (int i = 1; i < bArr.size() - 1; ++i) {
        for (int j = 1; j < bArr[i].size() - 1; ++j) {
            if ([&]() -> bool {
                    if ((bArr[i - 1.0][j] || bArr[i + 1.0][j]) && (bArr[i][j + 1.0] || bArr[i][j - 1.0]) && bArr[i][j]) return true;
                    
                    int tester = 0;
                    if (bArr[i - 1.0][j]) tester++;
                    if (bArr[i + 1.0][j]) tester++;
                    if (bArr[i][j + 1.0]) tester++;
                    if (bArr[i][j - 1.0]) tester++;

                    if (tester == 1 && bArr[i][j]) return true;

                    return false;
                }()) {
                nodeMap[make_pair(i, j)] = genNode(0);
            }
        }
    }

    return nodeMap;
}

map<pair<int, int>, node> getDeadEnds(map<pair<int, int>, node> nodeMap, vector<vector<bool>> bArr) {
    for (auto& [k, v] : nodeMap) {
        auto& key = k;
        auto& value = v;
        if ([&]() -> bool {
                int tester = 0;
                if (bArr[key.first - 1.0][key.second]) tester++;
                if (bArr[key.first + 1.0][key.second]) tester++;
                if (bArr[key.first][key.second + 1.0]) tester++;
                if (bArr[key.first][key.second - 1.0]) tester++;

                if (tester == 1 && bArr[key.first][key.second]) return true;

                return false;
            }()) {
            value.type = 1;
        }
    }
    return nodeMap;
}

map<pair<int, int>, node> degrade(map<pair<int, int>, node> nodeMap, vector<vector<bool>>& bArr) {
    vector<pair<int, int>> delQ;
    vector<pair<int, int>> modQ;

    auto evalDead  = [&](pair<int,int>key, int what) {
        int tester = 0;
        if (bArr[key.first - 1.0][key.second] && what!=0) tester++;
        if (bArr[key.first + 1.0][key.second] && what!=1) tester++;
        if (bArr[key.first][key.second + 1.0] && what!=2) tester++;
        if (bArr[key.first][key.second - 1.0] && what!=3) tester++;

        if (tester == 1 && bArr[key.first][key.second]) {
            modQ.push_back(key);
        }
    };

    //Eliminate dead ends
    for (auto& [k, v] : nodeMap) {
        if (v.type == 1) {
            bArr[k.first][k.second] = false;
            if (bArr[k.first - 1.0][k.second]) {
                int i = k.first - 1;
                for (; nodeMap.count(make_pair(i, k.second)) == 0 && i > 0; --i) { bArr[i][k.second] = false; }
                evalDead(make_pair(i, k.second), 1);
            } else if (bArr[k.first + 1.0][k.second]) {
                int i = k.first + 1;
                for (; nodeMap.count(make_pair(i, k.second)) == 0 && i < bArr.size()-1; ++i) { bArr[i][k.second] = false; }
                evalDead(make_pair(i, k.second), 0);
            } else if (bArr[k.first][k.second + 1.0]) {
                int i = k.second + 1;
                for (; nodeMap.count(make_pair(k.first, i)) == 0 && i < bArr.size() - 1; ++i) { bArr[k.first][i] = false; }
                evalDead(make_pair(k.first, i), 3);
            } else if (bArr[k.first][k.second - 1.0]) {
                int i = k.second - 1;
                for (; nodeMap.count(make_pair(k.first, i)) == 0 && i > 0; --i) { bArr[k.first][i] = false; }
                evalDead(make_pair(k.first, i), 2);
            }

            //Submit node for deletion
            delQ.push_back(k);
        }
    }

    for (pair<int, int> p : delQ)
        nodeMap.erase(p);
    for (pair<int, int> p : modQ)
        nodeMap[p].type = 1;

    return nodeMap;
}

Mat nodeImg(map<pair<int, int>, node> nodeMap, vector<vector<bool>> bArr, Mat img) {
    for (int i = 0; i < bArr.size(); ++i) {
        for (int j = 0; j < bArr[0].size(); ++j) {
            Vec3b& color = img.at<Vec3b>(i, j);
            if (!bArr[i][j]) {
                color[0] = 255; color[1] = 255; color[2] = 255;
            } else {
                color[0] = 0; color[1] = 0; color[2] = 0;
            }
        }
    }

    for (auto& [k, v] : nodeMap) {
        Vec3b& color = img.at<Vec3b>(k.first, k.second);
        if (v.type == 0) {
            color[0] = 255; color[1] = 0; color[2] = 255;
        } else if (v.type == 1) {
            color[0] = 255; color[1] = 0; color[2] = 0;
        }
    }

    return img;
}

int main()
{
    SetConsoleCP(437);
    SetConsoleOutputCP(437);

    cout << "Maze File: ";
    string fname;
    cin >> fname;

    Mat img = imread(fname);

    vector<vector<bool>> bVec = getArr(img);
    map<pair<int,int>,node> nodeMap = getNodes(bVec);
    nodeMap = getDeadEnds(nodeMap, bVec);

    //img = nodeImg(nodeMap, img);
    //imwrite(("outTemp/out" + to_string(0) + ".bmp"), img);

    img = nodeImg(nodeMap, bVec, img);
    imwrite(("outTemp/outfirst.bmp"), img);
    
    map<pair<int, int>, node> tester = {};

    clock_t timeStart = clock();

    int pic = 0;
    while (nodeMap != tester) {
        tester = nodeMap;
        nodeMap = degrade(nodeMap, bVec);

        //img = nodeImg(nodeMap, bVec, img);
        //imwrite(("outTemp/out" + to_string(++pic) + ".bmp"), img);
    }

    cout << to_string(clock() - timeStart) + " milliseconds to complete maze.\n\n";

    img = nodeImg(nodeMap, bVec, img);
    imwrite(("outTemp/out" + to_string(0) + ".bmp"), img);

    //system(("C:/Users/abc/Desktop/ffmpeg/bin/ffmpeg -start_number 0 -i C:/Users/abc/source/repos/MazeOptimal/MazeOptimal/outTemp/out%d.bmp -vf scale=" + to_string(1000) + ':' + to_string(1000) + " C:/Users/abc/source/repos/MazeOptimal/MazeOptimal/out.wmv").c_str());

    return 0;
}
