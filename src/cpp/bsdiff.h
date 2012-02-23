/*

    Copyright 2012 Michael Phan-Ba

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

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
