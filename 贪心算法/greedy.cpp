
#include "jsoncpp/json.h"
#include <climits>
#include <cstring>
#include <ctime>
#include <iostream>
using namespace std;

int board[9][9] = {0};                     //���̣�����1������-1��û��������0
bool visited_by_air_judge[9][9] = {false}; //��air_judge�����ж�ĳһ��������ʱ����ǣ���ֹ�ظ�����ѭ��
int value[9][9] = {0};                     //����ÿ��λ�õġ�Ȩ��ֵ

int dx[4] = {-1, 0, 1, 0}; //��λ��
int dy[4] = {0, -1, 0, 1}; //��λ��

//���ֵ�color
int opponent_color(int color)
{
    if (color == 1)
        return -1;
    else
        return 1;
}

//�жϵ�(x,y)�Ƿ���������
bool inBoard_judge(int x, int y) { return 0 <= x && x < 9 && 0 <= y && y < 9; }

//�ж��Ƿ�����
bool air_judge(int x, int y)
{
    visited_by_air_judge[x][y] = true; //��ǣ���ʾ���λ���Ѿ��ѹ���������
    bool flag = false;
    for (int dir = 0; dir < 4; dir++)
    {
        int x_dx = x + dx[dir], y_dy = y + dy[dir];
        if (inBoard_judge(x_dx, y_dy)) //����
        {
            if (board[x_dx][y_dy] == 0) //�Ա����λ��û������
                flag = true;
            if (board[x_dx][y_dy] == board[x][y] && !visited_by_air_judge[x_dx][y_dy]) //�Ա����λ����û����������ͬɫ��
                if (air_judge(x_dx, y_dy))
                    flag = true;
        }
    }
    return flag;
}

//�ж��ܷ�����ɫΪcolor����
bool put_available(int x, int y, int color) //no problem
{
    if (board[x][y]) //�������㱾����������
        return false;
    board[x][y] = color;
    memset(visited_by_air_judge, 0, sizeof(visited_by_air_judge)); //����

    if (!air_judge(x, y)) //��������ⲽ�����û����,˵������ɱ����������
    {
        board[x][y] = 0;
        return false;
    }

    for (int i = 0; i < 4; i++) //�ж������ⲽ��Χλ�õ������Ƿ�����
    {
        int x_dx = x + dx[i], y_dy = y + dy[i];
        if (inBoard_judge(x_dx, y_dy)) //��������
        {
            if (board[x_dx][y_dy] && !visited_by_air_judge[x_dx][y_dy]) //���������ӵ�λ�ã���Ƿ��ʹ�������ѭ����
                if (!air_judge(x_dx, y_dy))                             //�������(x_dx,y_dy)û���ˣ�������
                {
                    board[x][y] = 0; //����
                    return false;
                }
        }
    }
    board[x][y] = 0; //����
    return true;
}

//��ֵ�������Ե�ǰ�������������������ɫΪcolor��һ������һ�������ӵ�λ����Ŀ����٣�Ȩ��ֵ�Ƚϣ�
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
    // ����JSON
    getline(cin, str);

    int start = clock(); //ʱ��
    int timeout = (int)(0.9 * (double)CLOCKS_PER_SEC);

    //getline(cin, str);
    Json::Reader reader;
    Json::Value input;
    reader.parse(str, input);
    // �����Լ��յ���������Լ���������������ָ�״̬
    int turnID = input["responses"].size();
    //��ԭ����
    for (int i = 0; i < turnID; i++) //��һ�غϣ���ԭ��һ�غϵ����
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
    // �������JSON
    Json::Value ret;
    Json::Value action;

    //��������������:̰���㷨
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

    int random = rand() % best_num; //���������value�������ѡ
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






































