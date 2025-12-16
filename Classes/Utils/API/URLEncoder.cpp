#include "Utils/API/URLEncoder.h"

#include <iomanip>
#include <sstream>
std::string urlEncode(const std::string& str) {
  std::ostringstream encoded;
  encoded.fill('0');
  encoded << std::hex;

  for (char c : str) {
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded << c;
    } else {
      encoded << '%' << std::setw(2) << int((unsigned char)c);
    }
  }

  return encoded.str();
}