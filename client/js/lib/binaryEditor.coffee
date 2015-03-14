define (require, exports, module) ->
  $ = require 'jquery'
  util = require 'util'
  #EE is EventEmitter
  EE = (require 'events').EventEmitter
  Buffer = (require 'buffer').Buffer
  
  module.exports = BinaryEditor = (options = {})->
    if not this instanceof BinaryEditor
      return new BinaryEditor()
    EE.apply this 
    @_table = undefined
    @_oriOptions = options
    @_bytes = 0;
    this
  util.inherits BinaryEditor, EE
  
  asHex = (i)->
    h = i.toString(16).toUpperCase()
    if h.length % 2 != 0
     '0' + h 
    else
      h
  
  asAscii = (i)->
    if(i < 0x7f && i >= 0x20)
      String.fromCharCode(i)
    else
      '.'
  
  BinaryEditor::commit = ($to)->
    @_table = $('<table />').append $('<tr><td></td><td><pre>00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F</pre></td><td></td></tr>')
    @_offset = $('<pre />').text('0\n')
    @_hex = $('<pre />')
    @_ascii = $('<pre />')
    [@_offset, @_hex, @_ascii].map(($now)->
      $('<td />').append($now)
    ).reduce(($a, $b)->
      $a.add($b)
    ,$()).appendTo($('<tr />').appendTo @_table)
    @_table.appendTo $to
  
  BinaryEditor::addData = (buf)->
    for data in buf
      @_hex.text(@_hex.text() + asHex(data) + ' ')
      @_ascii.text(@_ascii.text() + asAscii(data))
      @_bytes = (@_bytes + 1) % 16
      if @_bytes is 0
        @_hex.text(@_hex.text() + '\n')
        @_ascii.text(@_ascii.text() + '\n')
  
  BinaryEditor