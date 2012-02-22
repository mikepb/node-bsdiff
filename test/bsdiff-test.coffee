assert = require 'assert'
crypto = require 'crypto'

bsdiff = require '../lib'

describe 'bsdiff', ->
  size = 16384
  block = 8
  cur = null
  ref = null

  ctrl = null
  diff = null
  xtra = null

  before ->
    ref = crypto.randomBytes size
    cur = crypto.randomBytes size

    for i in [0..size / block - 1]
      pos = i * block
      if Math.random() > 0.85
        ref.copy cur, pos, pos, pos + block

  it 'should diff', (done) ->
    bsdiff.diff cur, ref, (err, c, d, x) ->
      ctrl = c
      diff = d
      xtra = x
      done err

  it 'should patch', (done) ->
    bsdiff.patch cur.length, ref, ctrl, diff, xtra, (err, c) ->
      assert.ifError err
      assert.equal cur.length, c.length
      assert.equal cur[i], c[i] for i in [0, size - 1]
      done()
