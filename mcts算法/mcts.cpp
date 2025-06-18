#pragma GCC optimize(3)
#include "./jsoncpp/json.h"
#include <climits>
#include <cstring>
#include <ctime>
#include <iostream>
#include <math.h>
#include <random>
#include <string>
#define MAXBranchNum 81
using namespace std;
int dx[4] = { -1, 0, 1, 0 }; //行位移
int dy[4] = { 0, -1, 0, 1 }; //列位移

bool visited_by_air_judge[9][9] = { false }; //在air_judge函数判断某一点有无气时作标记，防止重复而死循环

//判断是否在棋盘内
bool inBoard_judge(int x, int y) { return 0 <= x && x < 9 && 0 <= y && y < 9; }

//判断是否有气
bool air_judge(int board[9][9], int x, int y)
{
    visited_by_air_judge[x][y] = true; //标记，表示这个位置已经搜过有无气了
    bool flag = false;
    for (int dir = 0; dir < 4; ++dir)
    {
        int x_dx = x + dx[dir], y_dy = y + dy[dir];
        if (inBoard_judge(x_dx, y_dy)) //界内
        {
            if (board[x_dx][y_dy] == 0) //旁边这个位置没有棋子
                flag = true;
            if (board[x_dx][y_dy] == board[x][y] && !visited_by_air_judge[x_dx][y_dy]) //旁边这个位置是没被搜索过的同色棋
                if (air_judge(board, x_dx, y_dy))
                    flag = true;
        }
    }
    return flag;
}

//判断能否下颜色为color的棋
bool put_available(int board[9][9], int x, int y, int color)
{
    if (!inBoard_judge(x, y))
        return false;
    if (board[x][y]) //如果这个点本来就有棋子
        return false;

    board[x][y] = color;
    memset(visited_by_air_judge, 0, sizeof(visited_by_air_judge)); //重置

    if (!air_judge(board, x, y)) //如果下完这步这个点没气了,说明是自杀步，不能下
    {
        board[x][y] = 0;
        return false;
    }

    for (int i = 0; i < 4; ++i) //判断下完这步周围位置的棋子是否有气
    {
        int x_dx = x + dx[i], y_dy = y + dy[i];
        if (inBoard_judge(x_dx, y_dy)) //在棋盘内
        {
            if (board[x_dx][y_dy] && !visited_by_air_judge[x_dx][y_dy]) //对于有棋子的位置（标记访问过避免死循环）
                if (!air_judge(board, x_dx, y_dy))                      //如果导致(x_dx,y_dy)没气了，则不能下
                {
                    board[x][y] = 0; //回溯
                    return false;
                }
        }
    }
    board[x][y] = 0; //回溯
    return true;
}

//找到能下的位置,result[9][9]表示各个位置的情况，0不能下，1可以下；该函数返回值是可下的位置数，也即result==1的点数
int getValidPositions(int board[9][9], int result[9][9])
{
    memset(result, 0, MAXBranchNum * 4);
    int right = 0;
    for (int x = 0; x < 9; ++x)
    {
        for (int y = 0; y < 9; ++y)
        {
            if (put_available(board, x, y, 1))
            {
                right++;
                result[x][y] = 1;
            }
        }
    }
    return right;
}

//类定义树节点
class treeNode
{
public:
    treeNode* parent;                 //父节点
    treeNode* children[MAXBranchNum]; //子节点
    int board[9][9];
    int childrenAction[MAXBranchNum][2];
    int childrenCount;
    int childrenCountMax;
    double value;      //该节点的总value
    int n;             //当前节点探索次数，UCB中的ni
    double UCB;        //当前节点的UCB值
    int* countPointer; //总节点数的指针
    //构造函数
    treeNode(int parentBoard[9][9], int opp_action[2], treeNode* parentPointer, int* countp) //构造函数 treeNode *p是父类指针, int *countp应该是总探索次数的指针
    {
        for (int i = 0; i < 9; ++i) //把棋盘反过来，要落子方是1 ，对手是-1
        {
            for (int j = 0; j < 9; ++j)
            {
                board[i][j] = -parentBoard[i][j];
            }
        }
        if (opp_action[0] >= 0 && opp_action[0] < 9 && opp_action[1] >= 0 && opp_action[1] < 9)
            board[opp_action[0]][opp_action[1]] = -1;
        parent = parentPointer;
        value = 0;
        n = 0;
        childrenCount = 0;     //已经拓展的子节点数
        countPointer = countp; //count的指针
        evaluate();            //计算能下的位置,修改了childrenCountMax、childrenAction
    }
    treeNode* treeRules() //搜索法则
    {
        //如果没有位置下了（终局）
        if (childrenCountMax == 0)
        {
            return this; //到达终局当前叶节点
        }

        //如果是叶节点，Node Expansion，拓展下一层节点
        if (childrenCountMax > childrenCount)
        {
            treeNode* newNode = new treeNode(board, childrenAction[childrenCount], this, countPointer); //拓展一个子节点
            children[childrenCount] = newNode;
            childrenCount++; //已拓展的子节点数++
            return newNode;
        }

        //计算当前节点的每个子节点的UCB值（点亮某个节点）
        for (int i = 0; i < childrenCount; ++i)
        {
            children[i]->UCB = children[i]->value / double(children[i]->n) + 0.2 * sqrt(log(double(*countPointer)) / double(children[i]->n)); //UCB公式
        }
        int bestChild = 0;
        double maxUCB = 0;

        //找出所有子节点中UCB值最大的子节点
        for (int i = 0; i < childrenCount; ++i)
        {
            if (maxUCB < children[i]->UCB)
            {
                bestChild = i;
                maxUCB = children[i]->UCB;
            }
        }
        return children[bestChild]->treeRules(); //对UCB最大的子节点进行下一层搜索
    }
    //模拟
    double simulation()
    {
        int board_opp[9][9]; //对手棋盘
        int res[9][9];
        for (int i = 0; i < 9; ++i)
        {
            for (int j = 0; j < 9; ++j)
            {
                board_opp[i][j] = -board[i][j];
            }
        }
        int x = getValidPositions(board, res);     //落子方可下位置数
        int y = getValidPositions(board_opp, res); //非落子方可下位置数
        return x - y;
    }
    void backup(double deltaValue) //回传估值,从当前叶节点以及往上的每一个父节点都加上估值
    {
        treeNode* node = this;
        int side = 0;
        while (node != nullptr) //当node不是根节点的父节点时
        {
            if (side == 1) //落子方
            {
                node->value += deltaValue;
                side--;
            }
            else //非落子方
            {
                node->value -= deltaValue;
                side++;
            }
            node->n++; //当前节点被探索次数++
            node = node->parent;
        }
    }

private:
    void evaluate() //计算能下的位置,修改了childrenCountMax、childrenAction
    {
        int result[9][9];
        int validPositionCount = getValidPositions(board, result); //能下的位置数
        int validPositions[MAXBranchNum];                          //能下的位置坐标
        int availableNum = 0;
        for (int i = 0; i < 9; ++i)
        {
            for (int j = 0; j < 9; ++j)
            {
                if (result[i][j])
                {
                    validPositions[availableNum] = i * 9 + j; //可下的位置
                    availableNum++;                           //可下的位置数
                }
            }
        }
        childrenCountMax = validPositionCount; //总共能下的位置数
        for (int i = 0; i < validPositionCount; ++i)
        {
            childrenAction[i][0] = validPositions[i] / 9;
            childrenAction[i][1] = validPositions[i] % 9;
        }
    }
};
//类定义结束 end of class definition

int main()
{
    int count = 0; //总计算的节点数（总探索次数，UCB中的N）
    int board[9][9] = { 0 };
    srand(clock());
    string str;
    getline(cin, str);
    int start = clock();
    int timeout = (int)(0.98 * (double)CLOCKS_PER_SEC);
    Json::Reader reader;
    Json::Value input;
    reader.parse(str, input);
    int turnID = input["responses"].size();
    int x, y;
    for (int i = 0; i < turnID; ++i)
    {
        x = input["requests"][i]["y"].asInt(), y = input["requests"][i]["x"].asInt();
        if (x != -1)
            board[x][y] = 1;
        x = input["responses"][i]["y"].asInt(), y = input["responses"][i]["x"].asInt();
        if (x != -1)
            board[x][y] = -1;
    }
    x = input["requests"][turnID]["y"].asInt(), y = input["requests"][turnID]["x"].asInt();
    int opp_action[2] = { x, y }; //对面上一步走了哪里

    treeNode rootNode(board, opp_action, nullptr, &count); //创建根节点，根节点的父节点为空

    while (clock() - start < timeout)
    {
        count++;                                //计算的节点数++
        treeNode* node = rootNode.treeRules(); //拓展一次，node指向的是一次拓展的叶节点
        double result = node->simulation();     //结果估值
        node->backup(result);
    }

    int bestChildren[MAXBranchNum] = { 0 }; //所有最优子节点的序号
    int bestChildrenNum = 0;              //最优子节点个数
    int maxValue = INT_MIN;               //当前最优子节点分数
    for (int i = 0; i < rootNode.childrenCount; ++i)
    {
        if (maxValue < rootNode.children[i]->value)
        {
            //重置
            memset(bestChildren, 0, sizeof(bestChildren));
            bestChildrenNum = 0;

            bestChildren[bestChildrenNum++] = i;
            maxValue = rootNode.children[i]->value;
        }
        else if (maxValue == rootNode.children[i]->value)
        {
            bestChildren[bestChildrenNum++] = i;
        }
    }

    int random = rand() % bestChildrenNum;                           //在所有最优中任选一个
    int* bestAction = rootNode.childrenAction[bestChildren[random]]; //最优子节点对应走法

    Json::Value ret;
    Json::Value action;
    action["x"] = bestAction[1];
    action["y"] = bestAction[0];
    ret["response"] = action;
    char buffer[4096];

    sprintf(buffer, "搜索节点数:%d,平均value:%.5f,用时：%.3f", count, (((double)(rootNode.children[bestChildren[random]]->value)) / ((double)rootNode.children[bestChildren[random]]->n) + 1.0) * 0.5, (double)(clock() - start) / 1000);

    ret["debug"] = buffer;
    Json::FastWriter writer;
    cout << writer.write(ret) << endl;
}







