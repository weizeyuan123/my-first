
#include "jsoncpp/json.h"
#include <climits>
#include <cstring>
#include <ctime>
#include <iostream>
using namespace std;

int board[9][9] = {0};                     //棋盘，黑子1，白子-1，没有棋子是0
bool visited_by_air_judge[9][9] = {false}; //在air_judge函数判断某一点有无气时作标记，防止重复而死循环
int value[9][9] = {0};                     //储存每个位置的“权利值

int dx[4] = {-1, 0, 1, 0}; //行位移
int dy[4] = {0, -1, 0, 1}; //列位移

//对手的color
int opponent_color(int color)
{
    if (color == 1)
        return -1;
    else
        return 1;
}

//判断点(x,y)是否在棋盘内
bool inBoard_judge(int x, int y) { return 0 <= x && x < 9 && 0 <= y && y < 9; }

//判断是否有气
bool air_judge(int x, int y)
{
    visited_by_air_judge[x][y] = true; //标记，表示这个位置已经搜过有无气了
    bool flag = false;
    for (int dir = 0; dir < 4; dir++)
    {
        int x_dx = x + dx[dir], y_dy = y + dy[dir];
        if (inBoard_judge(x_dx, y_dy)) //界内
        {
            if (board[x_dx][y_dy] == 0) //旁边这个位置没有棋子
                flag = true;
            if (board[x_dx][y_dy] == board[x][y] && !visited_by_air_judge[x_dx][y_dy]) //旁边这个位置是没被搜索过的同色棋
                if (air_judge(x_dx, y_dy))
                    flag = true;
        }
    }
    return flag;
}

//判断能否下颜色为color的棋
bool put_available(int x, int y, int color) //no problem
{
    if (board[x][y]) //如果这个点本来就有棋子
        return false;
    board[x][y] = color;
    memset(visited_by_air_judge, 0, sizeof(visited_by_air_judge)); //重置

    if (!air_judge(x, y)) //如果下完这步这个点没气了,说明是自杀步，不能下
    {
        board[x][y] = 0;
        return false;
    }

    for (int i = 0; i < 4; i++) //判断下完这步周围位置的棋子是否有气
    {
        int x_dx = x + dx[i], y_dy = y + dy[i];
        if (inBoard_judge(x_dx, y_dy)) //在棋盘内
        {
            if (board[x_dx][y_dy] && !visited_by_air_judge[x_dx][y_dy]) //对于有棋子的位置（标记访问过避免死循环）
                if (!air_judge(x_dx, y_dy))                             //如果导致(x_dx,y_dy)没气了，则不能下
                {
                    board[x][y] = 0; //回溯
                    return false;
                }
        }
    }
    board[x][y] = 0; //回溯
    return true;
}

//估值函数，对当前局面进行评估，计算颜色为color的一方比另一方可落子的位置数目多多少（权利值比较）
int evaluate(int color)
{
    int right = 0;
    int op_color = opponent_color(color);
    for (int x = 0; x < 9; x++)
    {
        for (int y = 0; y < 9; y++)
        {
            if (put_available(x, y, color))
                right++;
            if (put_available(x, y, op_color))
                right--;
        }
    }
    return right;
}

int main()
{
    srand((unsigned)time(0));
    string str;
    int x, y;
    // 读入JSON
    getline(cin, str);

    int start = clock(); //时间
    int timeout = (int)(0.9 * (double)CLOCKS_PER_SEC);

    //getline(cin, str);
    Json::Reader reader;
    Json::Value input;
    reader.parse(str, input);
    // 分析自己收到的输入和自己过往的输出，并恢复状态
    int turnID = input["responses"].size();
    //复原棋盘
    for (int i = 0; i < turnID; i++) //下一回合，复原上一回合的棋局
    {
        x = input["requests"][i]["x"].asInt(), y = input["requests"][i]["y"].asInt();
        if (x != -1)
            board[x][y] = 1;
        x = input["responses"][i]["x"].asInt(), y = input["responses"][i]["y"].asInt();
        if (x != -1)
            board[x][y] = -1;
    }
    x = input["requests"][turnID]["x"].asInt(), y = input["requests"][turnID]["y"].asInt();
    if (x != -1)
        board[x][y] = 1;
    // 输出决策JSON
    Json::Value ret;
    Json::Value action;

    //以下是搜索策略:贪心算法
    int color = -1;
    int max_value = INT_MIN;
    int best_i[81] = {0}, best_j[81] = {0}, best_num = 0;
    memset(value, 0, sizeof(value));
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            if (put_available(i, j, color))
            {
                board[i][j] = color;
                value[i][j] = evaluate(color);
                if (value[i][j] > max_value)
                    max_value = value[i][j];
                board[i][j] = 0;
            }
            else
                value[i][j] = INT_MIN;
            if (clock() - start > timeout)
            break;
        }
        //if (clock() - start > timeout)
        //break;
        //cout << clock() - start << endl;
    }
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            if (value[i][j] == max_value)
            {
                best_i[best_num] = i;
                best_j[best_num] = j;
                best_num++;
            }

    int random = rand() % best_num; //在所有最大value里面随机选
    int decision_x = best_i[random];
    int decision_y = best_j[random];

    action["x"] = decision_x;
    action["y"] = decision_y;
    ret["response"] = action;
    Json::FastWriter writer;

    cout << writer.write(ret) << endl;
    //cout << clock() - start << endl;
    //cout<<max_value << endl;
    return 0;
}






































