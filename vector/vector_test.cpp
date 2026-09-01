#include "vector.hpp"
#include <string>
int main() {
  my_std::vector<int> v;
  v.push_back(1);
  v.push_back(1);
  v.push_back(3);
  v.push_back(4);
  for (int i = 0; i < v.size(); ++i)
    std::cout << v[i];
  std::cout << std::endl;
  v.pop_back();
  for (int i = 0; i < v.size(); ++i)
    std::cout << v[i];
  std::cout << std::endl;
  my_std::vector<std::string> vecS;
  std::string s1 = "aaa";
  std::string s2 = "bbb";
  std::string s3 = "ccc";
  vecS.push_back(s1);
  vecS.push_back(s2);
  vecS.push_back(s3);
  for (int i = 0; i < vecS.size(); ++i)
    std::cout << vecS[i];
  vecS.pop_back();
  for (int i = 0; i < vecS.size(); ++i)
    std::cout << vecS[i];
  return 0;
}
