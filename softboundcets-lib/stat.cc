#include <map>
#include <iostream>

std::map<std::pair<void *, unsigned long>, unsigned long> AccessCount;
std::map<void *, unsigned long> MallocCount;
std::map<std::pair<void *, unsigned long>, unsigned long> ColoringAccessCount;

extern "C" {
void incAccessCount(void * base) {
  AccessCount[std::make_pair(base,MallocCount[base])]++;
}
void incMallocCount(void * base) {
  MallocCount[base]++;
}
void incColoringAccessCount(void * base) {
  ColoringAccessCount[std::make_pair(base,MallocCount[base])]++;
}

void printAccessCount() {
  std::multimap<unsigned long,std::pair<void *,unsigned long>, std::greater<unsigned long> > AccessCountSorted;
  for (auto I : AccessCount)
    AccessCountSorted.insert(std::pair<unsigned long, std::pair<void*,unsigned long> >(I.second, I.first));

  int i = 0;
  for (auto I : AccessCountSorted) {
    std::cout << I.first << " : " << I.second.first << " / " << I.second.second << '\n';
    if (i++ > 15)
      break;
  }
}
void printColoringAccessCount() {
  std::multimap<unsigned long, std::pair<void *,unsigned long>, std::greater<unsigned long> > CAccessCountSorted;
  for (auto I : ColoringAccessCount)
    CAccessCountSorted.insert(std::pair<unsigned long, std::pair<void *,unsigned long> >(I.second, I.first));

  int i = 0;
  for (auto I : CAccessCountSorted) {
    std::cout << I.first << " : " << I.second.first << "/" << I.second.second << '\n';
    if (i++ > 15)
      break;
  }
}
}
