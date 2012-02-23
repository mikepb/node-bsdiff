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
