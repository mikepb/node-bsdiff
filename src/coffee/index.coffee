binding = require './bsdiff.node'
module.exports = exports = binding

###

    bsdiff.diff

    @param {Buffer} cur Current (new) data.
    @param {Buffer} ref Reference (old) data.
    @param {Function} callback
      @param {Error} err If successful `null`, otherwise an `Error` object.
      @param {Buffer} ctrl Control data.
      @param {Buffer} diff Diff data.
      @param {Buffer} xtra Extra data.

###


###

    bsdiff.patch

    @param {Integer} curlen Size of current (new) data.
    @param {Buffer} ref Reference (old) data.
    @param {Buffer} ctrl Control data.
    @param {Buffer} diff Diff data.
    @param {Buffer} xtra Extra data.
    @param {Function} callback
      @param {Error} err If successful `null`, otherwise an `Error` object.
      @param {Buffer} cur Current (new) data.

###
