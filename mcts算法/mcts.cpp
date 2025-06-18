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
int dx[4] = { -1, 0, 1, 0 }; //��λ��
int dy[4] = { 0, -1, 0, 1 }; //��λ��

bool visited_by_air_judge[9][9] = { false }; //��air_judge�����ж�ĳһ��������ʱ����ǣ���ֹ�ظ�����ѭ��

//�ж��Ƿ���������
bool inBoard_judge(int x, int y) { return 0 <= x && x < 9 && 0 <= y && y < 9; }

//�ж��Ƿ�����
bool air_judge(int board[9][9], int x, int y)
{
    visited_by_air_judge[x][y] = true; //��ǣ���ʾ���λ���Ѿ��ѹ���������
    bool flag = false;
    for (int dir = 0; dir < 4; ++dir)
    {
        int x_dx = x + dx[dir], y_dy = y + dy[dir];
        if (inBoard_judge(x_dx, y_dy)) //����
        {
            if (board[x_dx][y_dy] == 0) //�Ա����λ��û������
                flag = true;
            if (board[x_dx][y_dy] == board[x][y] && !visited_by_air_judge[x_dx][y_dy]) //�Ա����λ����û����������ͬɫ��
                if (air_judge(board, x_dx, y_dy))
                    flag = true;
        }
    }
    return flag;
}

//�ж��ܷ�����ɫΪcolor����
bool put_available(int board[9][9], int x, int y, int color)
{
    if (!inBoard_judge(x, y))
        return false;
    if (board[x][y]) //�������㱾����������
        return false;

    board[x][y] = color;
    memset(visited_by_air_judge, 0, sizeof(visited_by_air_judge)); //����

    if (!air_judge(board, x, y)) //��������ⲽ�����û����,˵������ɱ����������
    {
        board[x][y] = 0;
        return false;
    }

    for (int i = 0; i < 4; ++i) //�ж������ⲽ��Χλ�õ������Ƿ�����
    {
        int x_dx = x + dx[i], y_dy = y + dy[i];
        if (inBoard_judge(x_dx, y_dy)) //��������
        {
            if (board[x_dx][y_dy] && !visited_by_air_judge[x_dx][y_dy]) //���������ӵ�λ�ã���Ƿ��ʹ�������ѭ����
                if (!air_judge(board, x_dx, y_dy))                      //�������(x_dx,y_dy)û���ˣ�������
                {
                    board[x][y] = 0; //����
                    return false;
                }
        }
    }
    board[x][y] = 0; //����
    return true;
}

//�ҵ����µ�λ��,result[9][9]��ʾ����λ�õ������0�����£�1�����£��ú�������ֵ�ǿ��µ�λ������Ҳ��result==1�ĵ���
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

//�ඨ�����ڵ�
class treeNode
{
public:
    treeNode* parent;                 //���ڵ�
    treeNode* children[MAXBranchNum]; //�ӽڵ�
    int board[9][9];
    int childrenAction[MAXBranchNum][2];
    int childrenCount;
    int childrenCountMax;
    double value;      //�ýڵ����value
    int n;             //��ǰ�ڵ�̽��������UCB�е�ni
    double UCB;        //��ǰ�ڵ��UCBֵ
    int* countPointer; //�ܽڵ�����ָ��
    //���캯��
    treeNode(int parentBoard[9][9], int opp_action[2], treeNode* parentPointer, int* countp) //���캯�� treeNode *p�Ǹ���ָ��, int *countpӦ������̽��������ָ��
    {
        for (int i = 0; i < 9; ++i) //�����̷�������Ҫ���ӷ���1 ��������-1
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
        childrenCount = 0;     //�Ѿ���չ���ӽڵ���
        countPointer = countp; //count��ָ��
        evaluate();            //�������µ�λ��,�޸���childrenCountMax��childrenAction
    }
    treeNode* treeRules() //��������
    {
        //���û��λ�����ˣ��վ֣�
        if (childrenCountMax == 0)
        {
            return this; //�����վֵ�ǰҶ�ڵ�
        }

        //�����Ҷ�ڵ㣬Node Expansion����չ��һ��ڵ�
        if (childrenCountMax > childrenCount)
        {
            treeNode* newNode = new treeNode(board, childrenAction[childrenCount], this, countPointer); //��չһ���ӽڵ�
            children[childrenCount] = newNode;
            childrenCount++; //����չ���ӽڵ���++
            return newNode;
        }

        //���㵱ǰ�ڵ��ÿ���ӽڵ��UCBֵ������ĳ���ڵ㣩
        for (int i = 0; i < childrenCount; ++i)
        {
            children[i]->UCB = children[i]->value / double(children[i]->n) + 0.2 * sqrt(log(double(*countPointer)) / double(children[i]->n)); //UCB��ʽ
        }
        int bestChild = 0;
        double maxUCB = 0;

        //�ҳ������ӽڵ���UCBֵ�����ӽڵ�
        for (int i = 0; i < childrenCount; ++i)
        {
            if (maxUCB < children[i]->UCB)
            {
                bestChild = i;
                maxUCB = children[i]->UCB;
            }
        }
        return children[bestChild]->treeRules(); //��UCB�����ӽڵ������һ������
    }
    //ģ��
    double simulation()
    {
        int board_opp[9][9]; //��������
        int res[9][9];
        for (int i = 0; i < 9; ++i)
        {
            for (int j = 0; j < 9; ++j)
            {
                board_opp[i][j] = -board[i][j];
            }
        }
        int x = getValidPositions(board, res);     //���ӷ�����λ����
        int y = getValidPositions(board_opp, res); //�����ӷ�����λ����
        return x - y;
    }
    void backup(double deltaValue) //�ش���ֵ,�ӵ�ǰҶ�ڵ��Լ����ϵ�ÿһ�����ڵ㶼���Ϲ�ֵ
    {
        treeNode* node = this;
        int side = 0;
        while (node != nullptr) //��node���Ǹ��ڵ�ĸ��ڵ�ʱ
        {
            if (side == 1) //���ӷ�
            {
                node->value += deltaValue;
                side--;
            }
            else //�����ӷ�
            {
                node->value -= deltaValue;
                side++;
            }
            node->n++; //��ǰ�ڵ㱻̽������++
            node = node->parent;
        }
    }

private:
    void evaluate() //�������µ�λ��,�޸���childrenCountMax��childrenAction
    {
        int result[9][9];
        int validPositionCount = getValidPositions(board, result); //���µ�λ����
        int validPositions[MAXBranchNum];                          //���µ�λ������
        int availableNum = 0;
        for (int i = 0; i < 9; ++i)
        {
            for (int j = 0; j < 9; ++j)
            {
                if (result[i][j])
                {
                    validPositions[availableNum] = i * 9 + j; //���µ�λ��
                    availableNum++;                           //���µ�λ����
                }
            }
        }
        childrenCountMax = validPositionCount; //�ܹ����µ�λ����
        for (int i = 0; i < validPositionCount; ++i)
        {
            childrenAction[i][0] = validPositions[i] / 9;
            childrenAction[i][1] = validPositions[i] % 9;
        }
    }
};
//�ඨ����� end of class definition

int main()
{
    int count = 0; //�ܼ���Ľڵ�������̽��������UCB�е�N��
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
    int opp_action[2] = { x, y }; //������һ����������

    treeNode rootNode(board, opp_action, nullptr, &count); //�������ڵ㣬���ڵ�ĸ��ڵ�Ϊ��

    while (clock() - start < timeout)
    {
        count++;                                //����Ľڵ���++
        treeNode* node = rootNode.treeRules(); //��չһ�Σ�nodeָ�����һ����չ��Ҷ�ڵ�
        double result = node->simulation();     //�����ֵ
        node->backup(result);
    }

    int bestChildren[MAXBranchNum] = { 0 }; //���������ӽڵ�����
    int bestChildrenNum = 0;              //�����ӽڵ����
    int maxValue = INT_MIN;               //��ǰ�����ӽڵ����
    for (int i = 0; i < rootNode.childrenCount; ++i)
    {
        if (maxValue < rootNode.children[i]->value)
        {
            //����
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

    int random = rand() % bestChildrenNum;                           //��������������ѡһ��
    int* bestAction = rootNode.childrenAction[bestChildren[random]]; //�����ӽڵ��Ӧ�߷�

    Json::Value ret;
    Json::Value action;
    action["x"] = bestAction[1];
    action["y"] = bestAction[0];
    ret["response"] = action;
    char buffer[4096];

    sprintf(buffer, "�����ڵ���:%d,ƽ��value:%.5f,��ʱ��%.3f", count, (((double)(rootNode.children[bestChildren[random]]->value)) / ((double)rootNode.children[bestChildren[random]]->n) + 1.0) * 0.5, (double)(clock() - start) / 1000);

    ret["debug"] = buffer;
    Json::FastWriter writer;
    cout << writer.write(ret) << endl;
}







