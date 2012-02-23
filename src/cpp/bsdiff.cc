/*-
 * Copyright 2003-2005 Colin Percival
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <string.h>

#include <algorithm>
#include <vector>

#include "endian.h"
#include "bsdiff.h"

namespace node_bsdiff {

static void split(int *I, int *V, int start, int len, int h) {
  int i, j, k, x, tmp, jj, kk;

  if (len < 16) {
    for (k = start; k < start + len; k += j) {
      x = V[I[k] + h];
      j = 1;

      for (i = 1; k + i < start + len; ++i) {
        if (V[I[k + i] + h] < x) {
          x = V[I[k + i] + h];
          j = 0;
        }
        if (V[I[k + i] + h] == x) {
          tmp = I[k + j];
          I[k + j] = I[k + i];
          I[k + i] = tmp;
          ++j;
        }
      }

      for (i = 0; i < j; ++i) V[I[k + i]] = k + j - 1;
      if (j == 1) I[k] = -1;
    }
    return;
  }

  x = V[I[start + len / 2] + h];
  jj = 0;
  kk = 0;
  for (i = start; i < start + len; ++i) {
    if (V[I[i] + h] < x) ++jj;
    if (V[I[i] + h] == x) ++kk;
  }
  jj += start;
  kk += jj;

  i = start;
  j = 0;
  k = 0;
  while (i < jj) {
    if (V[I[i] + h] < x) {
      ++i;
    } else if (V[I[i] + h] == x) {
      tmp = I[i];
      I[i] = I[jj + j];
      I[jj + j] = tmp;
      ++j;
    } else {
      tmp = I[i];
      I[i] = I[kk + k];
      I[kk+k] = tmp;
      ++k;
    }
  }

  while (jj + j < kk) {
    if(V[I[jj + j] + h] == x) {
      ++j;
    } else {
      tmp = I[jj + j];
      I[jj + j] = I[kk + k];
      I[kk + k] = tmp;
      ++k;
    }
  }

  if (jj > start) split(I, V, start, jj - start, h);

  for (i = 0; i < kk - jj; ++i) V[I[jj + i]] = kk - 1;
  if (jj == kk - 1) I[jj] = -1;

  if (start + len > kk) split(I, V, kk, start + len - kk, h);
}

static void qsufsort(
  int *I, int *V,
  const unsigned char *refdat, const size_t reflen)
{
  size_t buckets[256];
  size_t i, h, len;

  memset(buckets, 0, sizeof(buckets));
  for (i = 0; i < reflen; ++i) ++buckets[refdat[i]];
  for (i = 1; i < 256; ++i) buckets[i] += buckets[i - 1];
  for (i = 255; i > 0; --i) buckets[i] = buckets[i - 1];
  buckets[0] = 0;

  for (i = 0; i < reflen; ++i) I[++buckets[refdat[i]]] = i;
  I[0] = reflen;
  for (i = 0; i < reflen; ++i) V[i] = buckets[refdat[i]];
  V[reflen] = 0;
  for (i = 1; i < 256; ++i)
    if (buckets[i] == buckets[i - 1] + 1) I[buckets[i]] = -1;
  I[0] = -1;

  for (h = 1; I[0] != -(int)(reflen + 1); h *= 2) {
    len = 0;
    for (i = 0; i < reflen + 1;) {
      if (I[i] < 0) {
        len -= I[i];
        i -= I[i];
      } else {
        if (len) I[i - len] = -len;
        len = V[I[i]] + 1 - i;
        split(I, V, i, len, h);
        i += len;
        len = 0;
      }
    }
    if (len) I[i - len] = -len;
  }

  for (i = 0; i < reflen + 1; ++i) I[V[i]] = i;
}

static size_t matchlen(
  const unsigned char *curdat, const size_t curlen,
  const unsigned char *refdat, const size_t reflen)
{
  size_t end = std::min(reflen, curlen);
  size_t idx = 0;
  while (idx < end && refdat[idx] == curdat[idx]) ++idx;
  return idx;
}

static size_t search(
  int *pos, const int *I,
  const unsigned char *refdat, const size_t reflen,
  const unsigned char *curdat, const size_t curlen)
{
  size_t st = 0, en = reflen;
  size_t x, y;
  while (1) {

    if (en - st < 2) {
      x = matchlen(curdat, curlen, refdat + I[st], reflen - I[st]);
      y = matchlen(curdat, curlen, refdat + I[en], reflen - I[en]);

      if (x > y) {
        *pos = I[st];
        return x;
      } else {
        *pos = I[en];
        return y;
      }
    }

    x = st + (en - st) / 2;
    if (memcmp(refdat + I[x], curdat, std::min(reflen - I[x], curlen)) < 0) {
      st = x;
    } else {
      en = x;
    }
  }
}

int bsdiff(bsdiff_dat *args)
{
  const unsigned char *curdat = (unsigned char *)args->curdat;
  const unsigned char *refdat = (unsigned char *)args->refdat;
  const size_t curlen = args->curlen;
  const size_t reflen = args->reflen;

  /* Output */
  unsigned char *diff;
  unsigned char *xtra;

  std::vector<int>& ctrl = args->ctrl;

  size_t difflen = 0;
  size_t xtralen = 0;

  /* Compute */
  int scan = 0;
  int pos = 0;
  int len = 0;

  int lastscan = 0;
  int lastpos = 0;
  int lastoffset = 0;

  int oldscore;
  int scsc;

  int s;
  int Sf;
  int lenf;
  int Sb;
  int lenb;

  int overlap;
  int Ss;
  int lens;

  int i;

  int *I;
  int *V;

  I = new (std::nothrow) int[reflen + 1];
  if (I == NULL) goto e_malloc_I;

  V = new (std::nothrow) int[reflen + 1];
  if (V == NULL) goto e_malloc_V;

  qsufsort(I, V, refdat, reflen);

  delete V;

  diff = new (std::nothrow) unsigned char[curlen + 1];
  if (diff == NULL) goto e_malloc_diff;

  xtra = new (std::nothrow) unsigned char[curlen + 1];
  if (xtra == NULL) goto e_malloc_xtra;

  /* Compute the differences */
  while (scan < static_cast<const int>(curlen)) {
    oldscore = 0;
    for (scsc = scan += len; scan < static_cast<const int>(curlen); scan++) {
      len = search(&pos, I, refdat, reflen, curdat + scan, curlen - scan);

      for (; scsc < scan + len; scsc++)
        if (scsc + lastoffset < static_cast<const int>(reflen) &&
            refdat[scsc + lastoffset] == curdat[scsc]) oldscore++;

      if ((len != 0 && len == oldscore) || len > oldscore + 8) break;

      if (scan + lastoffset < static_cast<const int>(reflen) &&
          refdat[scan + lastoffset] == curdat[scan]) oldscore--;

    }

    if (len != oldscore || scan == static_cast<const int>(curlen)) {
      s = 0;
      Sf = 0;
      lenf = 0;
      for (i = 0; lastscan + i < scan && lastpos + i < static_cast<const int>(reflen);) {
        if (refdat[lastpos + i] == curdat[lastscan + i]) ++s;
        ++i;
        if (s * 2 - i > Sf * 2 - lenf) {
          Sf = s;
          lenf = i;
        }
      }

      lenb = 0;
      if (scan < static_cast<const int>(curlen)) {
        s = 0;
        Sb = 0;
        for (i = 1; scan >= lastscan + i && pos >= i; ++i) {
          if (refdat[pos - i] == curdat[scan - i]) s++;
          if (s * 2 - i > Sb * 2 - lenb) {
            Sb = s;
            lenb = i;
          }
        }
      }

      if (lastscan + lenf > scan - lenb) {
        overlap = (lastscan + lenf) - (scan - lenb);
        s = 0;
        Ss = 0;
        lens = 0;
        for (i = 0; i < overlap; ++i) {
          if (curdat[lastscan + lenf - overlap + i] ==
              refdat[lastpos + lenf - overlap + i]) ++s;
          if (curdat[scan - lenb + i] == refdat[pos - lenb + i]) --s;
          if (s > Ss) {
            Ss = s;
            lens = i + 1;
          }
        }

        lenf += lens - overlap;
        lenb -= lens;
      }

      for (i = 0; i < lenf; ++i)
        diff[difflen + i] = curdat[lastscan + i] - refdat[lastpos + i];
      for (i = 0; i < (scan - lenb) - (lastscan + lenf); ++i)
        xtra[xtralen + i] = curdat[lastscan + lenf + i];

      difflen += lenf;
      xtralen += (scan - lenb) - (lastscan + lenf);

      /* add x bytes from oldfile to x bytes from the diff block */
      ctrl.push_back(b32le(lenf));

      /* copy y bytes from the xtra block */
      ctrl.push_back(b32le((scan - lenb) - (lastscan + lenf)));

      /* seek forwards in oldfile by z bytes */
      ctrl.push_back(b32le((pos - lenb) - (lastpos + lenf)));

      lastscan = scan - lenb;
      lastpos = pos - lenb;
      lastoffset = pos - scan;
    }
  }

  /* Free the memory we used */
  delete I;

  /* Write diff data */
  args->diff = reinterpret_cast<char *>(diff);
  args->difflen = difflen;

  /* Write xtra data */
  args->xtra = reinterpret_cast<char *>(xtra);
  args->xtralen = xtralen;

  return 0;

  /* Error handling */

e_malloc_xtra:  delete diff;
e_malloc_diff:  ;
e_malloc_V:     delete I;
e_malloc_I:     ;
  return -2;
}

int bspatch(bsdiff_dat *args)
{
  if (args->curdat == NULL) {
    args->curdat = new (std::nothrow) char[args->curlen + 1];
    if (args->curdat == NULL) return -2;
  }

  unsigned char *curdat = reinterpret_cast<unsigned char *>(args->curdat);
  const unsigned char *refdat = reinterpret_cast<unsigned char *>(args->refdat);
  const unsigned char *diff = reinterpret_cast<unsigned char *>(args->diff);
  const unsigned char *xtra = reinterpret_cast<unsigned char *>(args->xtra);
  const std::vector<int>& ctrl = args->ctrl;

  const size_t curlen = args->curlen;
  const size_t reflen = args->reflen;
  const size_t difflen = args->difflen;
  const size_t xtralen = args->xtralen;

  size_t destIdx = 0, srcIdx = 0;
  size_t ctrlpos = 0, diffIdx = 0, xtraIdx = 0;
  size_t addN, copyN, seekN;
  size_t i;

  while (destIdx < curlen) {

    /* Unpack control data */
    addN = b32le(ctrl[ctrlpos++]);
    copyN = b32le(ctrl[ctrlpos++]);
    seekN = b32le(ctrl[ctrlpos++]);

    /* Sanity-check */
    if (destIdx + addN > curlen || srcIdx + addN > reflen ||
        diffIdx + addN > difflen) return -1;

    /* Read diff string */
    memcpy(curdat + destIdx, diff + diffIdx, addN);

    /* Add refdat data to diff string */
    for (i = 0; i < addN; ++i)
      if (srcIdx + i >= 0 && srcIdx + i < reflen)
        curdat[destIdx + i] += refdat[srcIdx + i];

    /* Adjust pointers */
    destIdx += addN;
    srcIdx += addN;

    /* Sanity-check */
    if (destIdx + copyN > curlen || xtraIdx + copyN > xtralen) return -1;

    /* Read xtra string */
    memcpy(curdat + destIdx, xtra + xtraIdx, copyN);

    /* Adjust pointers */
    destIdx += copyN;
    srcIdx += seekN;
  };

  return 0;
}

} // node_bsdiff
