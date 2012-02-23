[![Build Status](https://secure.travis-ci.org/mikepb/node-bsdiff.png)](http://travis-ci.org/mikepb/node-bsdiff)


# node-bsdiff

`node-bsdiff` implements a port of the `bsdiff` binary diff tool. Internally,
the implementation uses 32-bit integers instead of 64-bit integers in the
original. As such, `node-bsdiff` is meant for use on smaller datasets, such
as reducing the size of maintaining document revision history.


## Usage

```js
var assert = require('assert')
  , bsdiff = require('bsdiff')
  , crypto = require('crypto');

var cur = crypto.randomBytes(1024)
  , ref = crypto.randomBytes(1024);

bsdiff.diff(cur, ref, function(err, ctrl, diff, xtra) {
  if (err) throw err;
  bsdiff.patch(cur.length, ref, ctrl, diff, xtra, function(err, out) {
    if (err) throw err;
    for (var i = 0; i < cur.length; i++) {
      if (cur[i] !== out[i]) throw 'Patch did not work';
    }
    console.log('Patch worked!');
  });
});
```


## Installing

To compile native code and CoffeeScript do:

    npm install


## Testing

To run tests do:

    npm test

You can also view the current build status on
[Travis-CI](http://travis-ci.org/mikepb/node-bsdiff).


## Licenses

### node-bsdiff

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


### bsdiff

    Copyright 2003-2005 Colin Percival
    All rights reserved

    Redistribution and use in source and binary forms, with or without
    modification, are permitted providing that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.


### Boost

    Copyright 2005 Caleb Epstein
    Copyright 2006 John Maddock
    Copyright 2010 Rene Rivera
    Distributed under the Boost Software License, Version 1.0. (See
    accompany-ing file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)

    Copyright (c) 1997
    Silicon Graphics Computer Systems, Inc.

    Permission to use, copy, modify, distribute and sell this software
    and its documentation for any purpose is hereby granted without fee,
    provided that the above copyright notice appear in all copies and
    that both that copyright notice and this permission notice appear
    in supporting documentation.  Silicon Graphics makes no
    representations about the suitability of this software for any
    purpose.  It is provided "as is" without express or implied warranty.

    Copyright notice reproduced from <boost/detail/limits.hpp>, from
    which this code was originally taken.

    Modified by Caleb Epstein to use <endian.h> with GNU libc and to
    defined the BOOST_ENDIAN macro.


### GNU C Library

    Copyright (C) 1997 Free Software Foundation, Inc.
    This file is part of the GNU C Library.

    The GNU C Library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    The GNU C Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with the GNU C Library; if not, see
    <http://www.gnu.org/licenses/>.
