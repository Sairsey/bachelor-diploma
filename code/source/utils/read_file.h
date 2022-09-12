#pragma once

/* Project namespace */
namespace gdr
{
  // Read everything from file into vector
  bool ReadFileContent(LPCTSTR filename, std::vector<char>& data);
}