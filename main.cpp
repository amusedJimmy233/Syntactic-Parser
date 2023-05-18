#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <stack>
#include <set>

#define ERR -1
#define RESERVED 1
#define SEPARATOR 2
#define OPERATOR 3
#define CONSTANT 4
#define VARIABLE 5

using namespace std;

map<string, string> reserved_word = {{"if",   "1"},
                                     {"then", "2"},
                                     {"else", "3"},
                                     {"int",  "4"},
                                     {"char", "5"},
                                     {"for",  "6"},
                                     {"float","7"}};
map<string, string> operator_word = {{"=",  "1"},
                                     {">=", "2"},
                                     {"==", "3"},
                                     {"+",  "4"},
                                     {"/",  "5"},
                                     {"%",  "6"},
                                     {"++", "7"}};
map<string, string> separator_word = {{"\"", "1"},
                                      {";",  "2"}};
map<string, string> state;
vector<string> constant_word;
vector<string> variable_word;
int constant_num = 1;
int variable_num = 1;

//从文件读入到string里
string read_file_to_string(const char *filename) {
    ifstream ifile(filename);
    //将文件读入到ostringstream对象buf中
    ostringstream buf;
    char ch;
    while (buf && ifile.get(ch)) {
        buf.put(ch);
    }

    //返回与流对象buf关联的字符串
    return buf.str();
}

// 删除注释
string delete_comments(string target) {
    //单行注释
    size_t start = target.find("//");
    while (start != std::string::npos) {
        int cnt = 0;
        size_t i = start;
        while (target[i] != '\n') {
            i++;
            cnt++;
        }
        target.erase(start, cnt);
        start = target.find("//");
    }

    //多行注释
    start = target.find("/*");
    while (start != std::string::npos) {
        size_t end = target.find("*/");
        target.erase(start, end - start + 2);
        start = target.find("/*");
    }

    return target;
}

string file_delete_elements(string file1) {
    //删除注释
    file1 = delete_comments(file1);

    return file1;
}

string modify_file(string original_file) {
    //去除注释
    string file1 = file_delete_elements(std::move(original_file));

    istringstream iss(file1);
    string modified_file;
    string word;
    while (iss >> word) {
        modified_file += word;
        modified_file += " ";
    }

    return modified_file;
}

bool isalpha(char word) {
    if ((word >= 'a' && word <= 'z') || (word >= 'A' && word <= 'Z'))
        return true;
    return false;
}

bool isdigit(char word) {
    if (word >= '0' && word <= '9')
        return true;
    return false;
}

void add_state(const string &character, int state_num) {
    if (state_num == ERR) {
        state[character] = "err";
        return;
    }
    if (character == "over") return;

    string category = to_string(state_num);
    if (state.count(character) == 0) {
        switch (state_num) {
            case RESERVED:
                category += reserved_word[character];
                break;
            case SEPARATOR:
                category += separator_word[character];
                break;
            case OPERATOR:
                category += operator_word[character];
                break;
            case CONSTANT:
                constant_word.push_back(character);
                category += to_string(constant_num);
                constant_num++;
                break;
            case VARIABLE:
                variable_word.push_back(character);
                category += to_string(variable_num);
                variable_num++;
                break;
            default:
                break;
        }
        state[character] = category;
    }
}

void print_state(const string &character) {
    if (character == "over") cout << "over" << endl;
    else {
        cout << character << "\t\t(" << character << "," << state[character] << ")" << endl;
    }
}

void print_information() {
    cout << "constant: ";
    for (int i = 0; i < constant_word.size(); i++) {
        if (i) cout << " ";
        cout << constant_word[i];
    }
    cout << endl << "variable: ";
    for (int i = 0; i < variable_word.size(); i++) {
        if (i) cout << " ";
        cout << variable_word[i];
    }
}

//判别词法种类
void state_transition(string modified_file) {
    for (int i = 0; i < modified_file.size(); i++) {
        string character;
        //提取数字
        if (isdigit(modified_file[i])) {
            //数字第一位
            character.push_back(modified_file[i]);
            for (int j = i + 1;; j++) {
                if (isdigit(modified_file[j])) {
                    character.push_back(modified_file[j]);
                } else {
                    i = j - 1;
                    break;
                }
            }
            add_state(character, CONSTANT); //常量
        }
            // 提取变量/保留字
        else if (isalpha(modified_file[i])) {
            character.push_back(modified_file[i]);
            int cnt = 0;
            int flag = 0;
            for (int j = i + 1; cnt < 10; j++, cnt++) {
                //不是字母或数字，即符号
                if (isalpha(modified_file[j]) || isdigit(modified_file[j])) {
                    if (isdigit(modified_file[j])) {
                        flag = 1; //标记变量中有数字
                    }
                    character.push_back(modified_file[j]);
                } else {
                    i = j - 1;
                    break;
                }
            }
//            //变量中有数字，非法
//            if (flag) {
//                add_state(character, ERR);
//            }
            string re;
            for (auto i: character) {
                re.push_back(tolower(i));//i = tolower(i);
            }
            if (reserved_word.count(re)) {
                character = re;
                add_state(character, RESERVED);     //保留字
            } else {
                add_state(character, VARIABLE);     //变量
            }
        } else if (modified_file[i] == ' ') continue;
            //符号
        else {
            character.push_back(modified_file[i]);
            if (modified_file[i] == '>' || modified_file[i] == '=' || modified_file[i] == '<') {
                if (modified_file[i + 1] == '=') {
                    character.push_back(modified_file[i + 1]);
                    i++;
                }
            } else if (modified_file[i] == '+' && modified_file[i + 1] == '+') {
                character.push_back(modified_file[i + 1]);
                i++;
            } else if (modified_file[i] == '\\' && modified_file[i + 1] == 'n') {
                character.push_back(modified_file[i + 1]);
                i++;
            }
            if (operator_word.count(character)) {
                add_state(character, OPERATOR);     //运算符
            } else if (separator_word.count(character)) {
                add_state(character, SEPARATOR);    //分隔符
            } else {
                add_state(character, ERR);          //err符号
            }
        }
        //print_state(character);
    }
}

bool isBoundaryChar(char c) {
    return !isalnum(c) && c != '_';
}

void replace_variables(string& buf) {
    for (const auto& var : variable_word) {
        size_t pos = 0;
        while ((pos = buf.find(var, pos)) != string::npos) {
            if ((pos == 0 || isBoundaryChar(buf[pos - 1])) &&
                (pos + var.length() == buf.length() || isBoundaryChar(buf[pos + var.length()]))) {
                buf.replace(pos, var.length(), "id");
                pos += 2; // 从替换后的位置开始继续查找
            } else {
                pos += var.length(); // 避免匹配到部分变量名的情况
            }
        }
    }
}

void replace_digits(string& str) {
    int flag=1;
    while (flag)
    {
        for (int i=0;i<str.length();i++)
        {
            // 如果有数字，则拆开再拼接，将数字替换成id
            if (isdigit(str[i])){
                int end=i;
                while(true){
                    if (isdigit(str[end])){
                        end++;
                    }
                    else{
                        break;
                    }
                }
                string s1=str.substr(0,i);
                s1+="id";
                string s2=str.substr(end+1);
                s1+=s2;
                str=s1;
                flag=1;
                break;
            }
            flag=0;
        }
    }

}

void remove_operator(std::string& str,string target) {
    size_t pos = 0;
    while ((pos = str.find(target, pos)) != std::string::npos) {
        str.erase(pos, 1);
    }
}

void predict(string input,const string& first,const string& name,map<pair<string,string>,string > G,const set<string>& terminal,const set<string>& nonterminal)
{
    stack<string> st;
    st.push("$");
    st.push(first);
    int ip=0;
    while (true)
    {
        string top=st.top();
        if (st.top() == "$" && input[ip] == '$') {
            cout<<name<<"匹配成功!"<<endl;
            break;
        }
        st.pop();
        string cur_terminal;
        for (int i=1;i<input.size();i++)
        {
            //遍历截取字符串，判断终结符
            string s=input.substr(ip,i);
            if (terminal.count(s)){
                //第一个终结符
                cur_terminal=s;
                break;
            }
        }
        //匹配
        if (top==cur_terminal){
            //指针移动
            ip+=(int)cur_terminal.length();
            continue;
        }
        // 不匹配
        else{
            string target=G[{top,cur_terminal}];
            // 表格为空
            if (target.empty()){
                cout<<name<<"不匹配"<<endl;
                break;
            }
            //如果是ε，且输入的字符串已经到最后了，则不用移动ip指针
            if (target=="\u03B5") {
                continue;
            }
            reverse(target.begin(),target.end());
            int target_ip=0;
            for (int i=1;i<=target.length();i++)
            {
                string s = target.substr(target_ip,i);
                // 遍历截取长度字符串，并且倒置判断是否是集合中的符号
                reverse(s.begin(),s.end());
                if (terminal.count(s)!=0||nonterminal.count(s)!=0){
                    // 非终结符/终结符 入栈，指向下一个，将长度清零
                    st.push(s);
                    target_ip+=i;
                    i=0;
                }
            }
        }
    }
}

void predict_G1(string input)
{
    map<pair<string,string>,string > G1={
            {{"L","id"}, "id=E"},
            {{"E","id"}, "FE'"},
            {{"E","("}, "FE'"},
            {{"E'","+"}, "+FE'"},
            {{"E'","-"}, "-FE'"},
            {{"E'",")"}, "\u03B5"},
            {{"E'","$"}, "\u03B5"},
            {{"F","id"}, "id"},
            {{"F","("}, "(E)"}
    };
    set<string> terminal={"id","+","-","(",")","=","$"};
    set<string> nonterminal={"L","E","E'","F"};
    predict(input,"L","G1",G1,terminal,nonterminal);
}

void predict_G2(string input)
{
    map<pair<string,string>,string > G2={
            {{"L","id"}, "id=E"},
            {{"E","id"}, "TE'"},
            {{"E","("}, "TE'"},
            {{"E'","*"}, "*TE'"},
            {{"E'",")"}, "\u03B5"},
            {{"E'","$"}, "\u03B5"},
            {{"T","id"}, "FT'"},
            {{"T","("}, "FT'"},
            {{"T'","*"}, "\u03B5"},
            {{"T'","/"}, "/FT'"},
            {{"T'","%"}, "%FT'"},
            {{"T'",")"}, "\u03B5"},
            {{"T'","$"}, "\u03B5"},
            {{"F","id"}, "id"},
            {{"F","("}, "(E)"}
    };
    set<string> terminal={"id","*","/","%","(",")","=","$"};
    set<string> nonterminal={"L","E","E'","T","T'","F"};
    predict(input,"L","G2",G2,terminal,nonterminal);
}

void predict_G3(string input)
{
    map<pair<string,string>,string > G3={
            {{"D","int"}, "TL"},
            {{"D","float"}, "TL"},
            {{"T","int"}, "int"},
            {{"T","float"}, "float"},
            {{"L","id"}, "idR"},
            {{"R",","}, ",idR"},
            {{"R","$"}, "\u03B5"}
    };
    set<string> terminal={"id",",","int","float","$"};
    set<string> nonterminal={"D","T","L","R"};
    predict(input,"D","G3",G3,terminal,nonterminal);
}

void analyze(const string& input)
{
    cout<<"替换后："<< input << endl;
    predict_G1(input);
    predict_G2(input);
    predict_G3(input);
}

//预处理
void predict()
{
    fstream fin("../input.txt"); //打开文件

    string original_file;
    while (getline(fin, original_file)) //逐行读取，直到结束
    {
        cout<<endl<<"文法："<<original_file<<endl;
        string modified_file = modify_file(original_file);
        state_transition(modified_file);

        // 将变量换成id
        replace_variables(modified_file);
        //replace_digits(modified_file);
        remove_operator(modified_file," ");
        remove_operator(modified_file,";");
        modified_file+="$";
        analyze(modified_file);
    }
}



int main()
{
    predict();

    return 0;
}