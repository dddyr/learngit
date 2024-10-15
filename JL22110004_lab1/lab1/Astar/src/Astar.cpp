#include <vector>
#include <iostream>
#include <queue>
#include <map>
#include <fstream>
#include <sstream>
#include <string>
#include <stack>
#include <algorithm>

using namespace std;

// TODO: 定义地图信息
struct Map_Cell
{
    int type;            //记录每块方格的作用
    int current_length;  //起点到该点的当前路径长度；
    string path;         //记录当前路径
    int material;        //记录当前剩余物资
    char state;          // F：未进行处理(初始状态)   O：在open_list中  C：在close_list中或不可达 
};

// TODO: 定义搜索状态
struct Search_Cell
{
    int h; //节点n到目标节点的最小代价路径的估计值
    int g; //从开始节点到节点n的路径代价
    //定义节点的位置
    int x;
    int y;
};


// 自定义比较函数对象，按照 Search_Cell 结构体的 g + h 属性进行比较
struct CompareF 
{
    bool operator()(const Search_Cell *a, const Search_Cell *b) const 
    {
        return (a->g + a->h) > (b->g + b->h); // 较小的 g + h 值优先级更高
    }
};

// TODO: 定义启发式函数-------使用曼哈顿距离 |x1-x2|+|y1-y2|
// pair<int, int> end_point;   // 终点
int Heuristic_Funtion(Search_Cell *node, pair<int,int> end_point)
{
    int distance = abs(end_point.first - node->x) + abs(end_point.second - node->y);
    return distance;
}

void Astar_search(const string input_file, int &step_nums, string &way)
{
    ifstream file(input_file);//打开输入文件
    if (!file.is_open()) {
        cout << "Error opening file!" << endl;
        return;
    }
    // 读取第一行，这里使用一个字符串流stringstream来分割每个单词
    string line;
    getline(file, line); // 读取第一行
    stringstream ss(line);
    string word;
    vector<string> words;
    //将当前分割的单词存储到word中
    while (ss >> word) {
        words.push_back(word);
    }
    // 分析第一行数据，将其转换为整数类型：M, N, T
    int M = stoi(words[0]); // M：行
    int N = stoi(words[1]); // N：列
    int T = stoi(words[2]); // T：物资份额

    pair<int, int> start_point; // 起点
    pair<int, int> end_point;   // 终点
    Map_Cell **Map = new Map_Cell *[M]; // 创建二维数组Map，包含Map_Cell (M行)
    // 加载地图
    for(int i = 0; i < M; i++)
    {
        Map[i] = new Map_Cell[N]; // 为每一行分配空间
        // 读取当前行的数据
        getline(file, line);
        stringstream ss(line);
        string word;
        vector<string> words;
        while (ss >> word) {
            words.push_back(word);
        }
        // 将当前行的数字信息赋值给Map对应的位置，并记录起点和终点
        for(int j = 0; j < N; j++)
        {
            Map[i][j].type = stoi(words[j]);
            Map[i][j].current_length = 0;
            Map[i][j].path += "";
            Map[i][j].material = 0;
            Map[i][j].state = 'F';
            if(Map[i][j].type == 3)      //起点：3
            {
                start_point = {i, j};
                Map[i][j].material = T;
            }
            else if(Map[i][j].type == 4) //终点：4
            {
                end_point = {i, j};
            }
            else if(Map[i][j].type == 1) //不可通行块：1
            {
                Map[i][j].state = 'C';
            }
        }
    }
    // 以上为预处理部分
    // ------------------------------------------------------------------

    //创建一个名为search_cell的Search_Cell指针，并分配内存空间
    Search_Cell *search_cell = new Search_Cell;

    search_cell->x = start_point.first;
    search_cell->y = start_point.second;
    search_cell->h = Heuristic_Funtion(search_cell,end_point);
    search_cell->g = 0; 

    // 创建一个优先队列open_list，其中元素为Search_Cell指针，按照CompareF中定义的比较规则排序
    priority_queue<Search_Cell *, vector<Search_Cell *>, CompareF> open_list;
    vector<Search_Cell *> close_list;
    open_list.push(search_cell);

    int cur_material; // 记录当前所剩物资
    int cur_step; // 记录当前步数

    // TODO: A*搜索过程实现
    while(!open_list.empty())
    {
        Search_Cell* cur_node=open_list.top();
        // 如果只剩end_point在open_list中，说明该路径已经找到
        if(cur_node->x==end_point.first&&cur_node->y==end_point.second) 
        {
            break;
        }
        // 如果cur_node不是终点
        else
        {
            cur_material = Map[cur_node->x][cur_node->y].material;
            cur_step = Map[cur_node->x][cur_node->y].current_length;

            open_list.pop(); //delete n frim open_list
            close_list.push_back(cur_node); //add n to close_list
            Map[cur_node->x][cur_node->y].state='C';

            //如果当前剩余物资为0，且该节点不是补给站，无法继续前进，跳过该点
            if(cur_material==0 && Map[cur_node->x][cur_node->y].type!=2)
            {
                continue;
            }
            //当前方块是补给站，补满物资
            if(Map[cur_node->x][cur_node->y].type==2) 
            {
                cur_material = T;
                for(int i=0;i<M;i++)
                {
                    for(int j=0;j<N;j++)
                    {
                        if(Map[i][j].type==0 && Map[i][j].state=='C')// type---0：可通行块 state---C：不可达
                        {
                            Map[i][j].state='F';
                        }
                    }
                }
            }
            // 遍历cur_node的所有相邻方块
            for(int i=-1;i<=1;i++)
            {
                for(int j=-1;j<=1;j++)
                {
                    if(abs(i-j)==1 && cur_node->x+i>=0 && cur_node->x+i<=M-1 && cur_node->y+j>=0 && cur_node->y+j<=N-1)
                    {
                        if(Map[cur_node->x+i][cur_node->y+j].state == 'C') //若不可达或在close_list中，则忽略
                        {
                            continue;
                        }
                        else if(Map[cur_node->x+i][cur_node->y+j].state == 'F')//若不在open_list和close_list，则加入open_list
                        {
                            Search_Cell* new_code=new Search_Cell;
                            new_code->x=cur_node->x+i;
                            new_code->y=cur_node->y+j;
                            new_code->g=cur_node->g+1;
                            new_code->h=Heuristic_Funtion(new_code,end_point);
                            open_list.push(new_code);
                            Map[new_code->x][new_code->y].state='O'; //O：在open_list中
                            Map[new_code->x][new_code->y].current_length=cur_step+1;
                            Map[new_code->x][new_code->y].material=cur_material-1;
                            //判断父节点在当前节点的方位 
                            char judge_1;
                            if(i==-1 && j==0)      judge_1='u';
                            else if(i==1 && j==0)  judge_1='d';
                            else if(i==0 && j==-1) judge_1='l';
                            else if(i==0 && j==1)  judge_1='r';
                            switch (judge_1) // 记录路径
                            {
                                case 'd': //父节点在当前节点的上方
                                    Map[new_code->x][new_code->y].path=Map[cur_node->x][cur_node->y].path+'D';
                                    break;
                                case 'u': //父节点在当前节点的下方
                                    Map[new_code->x][new_code->y].path=Map[cur_node->x][cur_node->y].path+'U';
                                    break;
                                case 'r': //父节点在当前节点的左方
                                    Map[new_code->x][new_code->y].path=Map[cur_node->x][cur_node->y].path+'R';
                                    break;
                                case 'l': //父节点在当前节点的右方
                                    Map[new_code->x][new_code->y].path=Map[cur_node->x][cur_node->y].path+'L';
                                    break;
                            }
                        }
                        else if(Map[cur_node->x+i][cur_node->y+j].state == 'O') //如果在open_list中，则检查经过该节点的路径是不是更优
                        {
                            Search_Cell* cmp_node=new Search_Cell; //存储需要check的节点
                            cmp_node->x=cur_node->x+i;
                            cmp_node->y=cur_node->y+j;
                            cmp_node->h=Heuristic_Funtion(cmp_node,end_point);//节点cmp_node到目标节点的最小代价路径的估计值
                            cmp_node->g=cur_node->g+1;//从开始节点到节点cmp_node的路径代价
                            // 如果新路径更短，则更新当前路径
                            if(cmp_node->g < Map[cmp_node->x][cmp_node->y].current_length ||
                              (cmp_node->g == Map[cmp_node->x][cmp_node->y].current_length && cur_material-1 > Map[cmp_node->x][cmp_node->y].material))
                            {
                                    Map[cmp_node->x][cmp_node->y].current_length = cmp_node->g;
                                    Map[cmp_node->x][cmp_node->y].material=cur_material-1;
                                    //判断父节点在当前节点的方位
                                    char judge_2;
                                    if(i==-1 && j==0)      judge_2='u';
                                    else if(i==1 && j==0)  judge_2='d';
                                    else if(i==0 && j==-1) judge_2='l';
                                    else if(i==0 && j==1)  judge_2='r';
                                    switch (judge_2) // 记录路径
                                    {
                                        case 'd': //父节点在当前节点的上方
                                            Map[cmp_node->x][cmp_node->y].path=Map[cur_node->x][cur_node->y].path+'D';
                                            break;
                                        case 'u': //父节点在当前节点的下方
                                            Map[cmp_node->x][cmp_node->y].path=Map[cur_node->x][cur_node->y].path+'U';
                                            break;
                                        case 'r': //父节点在当前节点的左方
                                            Map[cmp_node->x][cmp_node->y].path=Map[cur_node->x][cur_node->y].path+'R';
                                            break;
                                        case 'l': //父节点在当前节点的右方
                                            Map[cmp_node->x][cmp_node->y].path=Map[cur_node->x][cur_node->y].path+'L';
                                            break;
                                    }
                                    open_list.push(cmp_node);
                            }
                            else
                            {
                                delete cmp_node;
                            }
                        }
                    }
                }
            }
        }
    }

    // ------------------------------------------------------------------
    // TODO: 填充step_nums与way
    step_nums = -1; //最短可行路径的长度
    way = "";       //相应路径的动作序列
    if(!open_list.empty())
    {
        step_nums=Map[end_point.first][end_point.second].current_length;
        way=Map[end_point.first][end_point.second].path;
    }
    
    // ------------------------------------------------------------------
    // 释放动态内存
    for(int i = 0; i < M; i++)
    {
        delete[] Map[i];
    }
    delete[] Map;
    while(!open_list.empty())
    {
        auto temp = open_list.top();
        delete[] temp;
        open_list.pop();
    }
    for(int i = 0; i < close_list.size(); i++)
    {
        delete[] close_list[i];
    }

    return;
}

void output(const string output_file, int &step_nums, string &way)
{
    ofstream file(output_file);
    if(file.is_open())
    {
        file << step_nums << endl;
        if(step_nums >= 0)
        {
            file << way << endl;
        }

        file.close();
    }
    else
    {
        cerr << "Can not open file: " << output_file << endl;
    }
    return;
}

int main(int argc, char *argv[])
{
    string input_base = "../input/input_";
    string output_base = "../output/output_";
    // input_0为讲义样例，此处不做测试
    for(int i = 0; i < 11; i++)
    {
        int step_nums = -1; //步数初始化为-1
        string way = "";
         // 调用 Astar_search 函数处理当前文件，并更新步数和路径信息
        Astar_search(input_base + to_string(i) + ".txt", step_nums, way);
        // 将结果输出到指定的输出文件中
        output(output_base + to_string(i) + ".txt", step_nums, way);
    }
    return 0;
}