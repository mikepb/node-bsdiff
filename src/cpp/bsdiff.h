#ifndef NODE_BSDIFF_H_
#define NODE_BSDIFF_H_

#include <vector>

namespace node_bsdiff {

class bsdiff_dat {
 public:
  char *curdat;
  char *refdat;
  char *diff;
  char *xtra;

  size_t curlen;
  size_t reflen;
  size_t difflen;
  size_t xtralen;

  std::vector<int> ctrl;

  bsdiff_dat() {
    memset(this, 0, sizeof(bsdiff_dat));
  }
};

int bsdiff(bsdiff_dat *args);
int bspatch(bsdiff_dat *args);

} // node_bsdiff

#endif // NODE_BSDIFF_H_
