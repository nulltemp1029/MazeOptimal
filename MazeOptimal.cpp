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
    node* adjacents;
    int type;
    vector<int> pos;
};

node genNode (node* adj, int ty, int x, int y) {
    node tNode;
    tNode.adjacents = adj;
    tNode.type = ty;
    vector<int> tPos = {x, y};
    tNode.pos = tPos;
    return tNode;
}

vector<node> getNodes(vector<vector<bool>> bArr) {
    vector<node> nodeMap;

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
                nodeMap.push_back(genNode(NULL, 0, i, j));
            }
        }
    }

    return nodeMap;
}

vector<node> getDeadEnds(vector<node> nodeMap, vector<vector<bool>> bArr) {
    for (node& n : nodeMap) {
        if ([&]() -> bool {
                int tester = 0;
                if (bArr[n.pos[0] - 1.0][n.pos[1]]) tester++;
                if (bArr[n.pos[0] + 1.0][n.pos[1]]) tester++;
                if (bArr[n.pos[0]][n.pos[1] + 1.0]) tester++;
                if (bArr[n.pos[0]][n.pos[1] - 1.0]) tester++;

                if (tester == 1 && bArr[n.pos[0]][n.pos[1]]) return true;

                return false;
            }()) {
            cout << "z";
            n.type = 1;
        }
    }
    return nodeMap;
}

Mat nodeImg(vector<node> nodeMap, Mat img) {
    for (node& n : nodeMap) {
        Vec3b& color = img.at<Vec3b>(n.pos[0], n.pos[1]);
        if (n.type == 0) {
            cout << "a";
            color[0] = 255; color[1] = 0; color[2] = 255;
        } else if (n.type == 1) {
            cout << "b";
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

    clock_t timeStart = clock();

    vector<vector<bool>> bVec = getArr(img);
    vector<node> nodeMap = getNodes(bVec);
    nodeMap = getDeadEnds(nodeMap, bVec);

    img = nodeImg(nodeMap, img);
    imwrite(("outTemp/out" + to_string(1) + ".bmp"), img);

    cout << to_string(clock() - timeStart) + " milliseconds to complete maze.\n\n";

    system(("C:/Users/abc/Desktop/ffmpeg/bin/ffmpeg -start_number 0 -i C:/Users/abc/source/repos/MazeOptimal/MazeOptimal/outTemp/out%d.bmp -vf scale=" + to_string(1000) + ':' + to_string(1000) + " C:/Users/abc/source/repos/MazeOptimal/MazeOptimal/out.wmv").c_str());

    return 0;
}
