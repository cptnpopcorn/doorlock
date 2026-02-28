#ifndef B1D83281_5CF6_403F_8ED5_AE2D05720D99
#define B1D83281_5CF6_403F_8ED5_AE2D05720D99

#include <esp_err.h>

#include <string>

void check(esp_err_t err);
void check(esp_err_t err, std::string what);
void error(std::string what);

#endif /* B1D83281_5CF6_403F_8ED5_AE2D05720D99 */
