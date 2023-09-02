#pragma once

struct State {
  const char *name;
  void (*action)(const State &current);

  void execute() const { action(*this); }
};
