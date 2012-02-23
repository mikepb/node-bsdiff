###

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

###

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
