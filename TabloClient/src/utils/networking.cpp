#include "networking.h"

#include <iostream>

Networking::Networking() {}

void Networking::networkingCycle(std::string tabloMaster) {
}

bool Networking::hasSolution() {
  std::lock_guard<std::mutex> lock(mtx);
  return !solutionCollection.empty();
}

std::string Networking::popSolution() {
  std::lock_guard<std::mutex> lock(mtx);
  std::string solution = solutionCollection[0];
  solutionCollection.erase(solutionCollection.begin());
  return solution;
}

void Networking::pushOrder(int method, std::string content) {
  std::lock_guard<std::mutex> lock(mtx);
  std::map<int, std::string> order;
  order[method] = content;
  orderCollection.push_back(order);
}

