#include <map>
#include <iostream>

std::map<void *, unsigned long> AccessCount;

extern "C" {
void incAccessCount(void * base) {
  AccessCount[base]++;
}

void printAccessCount() {
  std::multimap<unsigned long, void *, std::greater<unsigned long> > AccessCountSorted;
  for (auto I : AccessCount)
    AccessCountSorted.insert(std::pair<unsigned long, void*>(I.second, I.first));

  int i = 0;
  for (auto I : AccessCountSorted) {
    std::cout << I.first << " : " << I.second << '\n';
    if (i++ > 15)
      break;
  }
}
}
