#include "string_utils.h"

vector<string> split(const string& s, const string& delimiter)
{
    vector<string> tokens;
    string copy_of_s =string(s);
    int pos=0;
    while ( (pos = copy_of_s.find_first_of(delimiter)) != string::npos )  {
        tokens.push_back(copy_of_s.substr(0, pos));
        copy_of_s = copy_of_s.substr(pos + delimiter.length()) ; 
    }
    if ( copy_of_s.length() != 0 && copy_of_s != delimiter) {
        tokens.push_back(copy_of_s);
    }
    return tokens;
}

/*
string ltrim(const string& s)
{
    int i=0;
    for (int i=0; i<s.length(); i++) {
        if (!(isspace(s[i]))) {
                break;
        }
    }
    cout << i<< endl;
    return s.substr(i);
}

string rtrim(const string& s)
{
    int i=s.length()-1;
    for (; i>=0; i--) {
        if (!(isspace(s[i]))) {
                break;
        }
    }
    return s.substr(0,s.length()-i);
}

string trim(const string& s)
{
    return(rtrim(ltrim(s) ));
}
*/

#ifdef __TEST_STRING_UTILS__
int main(int argc, char *argv[])
{
    string s(argv[1]);
    string delim(argv[2]);

    vector<string> tokens = split(s, delim);

    for (vector<string>::iterator it=tokens.begin(); it!=tokens.end(); it++) {
        cout << *it << endl;
    }

}
#endif
